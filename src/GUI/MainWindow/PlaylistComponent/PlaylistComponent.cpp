/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015, 2019 Igor Kushnir <igorkuo AT Google mail>

 Venturous is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Venturous is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 Venturous.  If not, see <http://www.gnu.org/licenses/>.
*/

# ifdef DEBUG_VENTUROUS_PLAYLIST_COMPONENT
# include <iostream>
# endif

# include "PlaylistComponent.hpp"

# include "CommonTypes.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <VenturousCore/ItemTree-inl.hpp>
# include <VenturousCore/AddingItems.hpp>

# include <QtWidgetsUtilities/InputController.hpp>
# include <QtWidgetsUtilities/HandleErrors.hpp>

# include <QtCoreUtilities/String.hpp>
# include <QtCoreUtilities/Miscellaneous.hpp>

# include <CommonUtilities/ExceptionsToStderr.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QFile>
# include <QFileInfo>
# include <QAction>
# include <QMessageBox>
# include <QFileDialog>
# include <QMainWindow>

# include <utility>
# include <algorithm>
# include <string>


namespace
{
inline QString ioError() { return QObject::tr("I/O error"); }
inline QString loadingFailed()
{
    return QObject::tr("Loading playlist failed");
}
inline QString loadingErrorFullMessage(QString errorMessage)
{
    return QObject::tr("Error: ") + std::move(errorMessage);
}
inline QString savingFailed() { return QObject::tr("Saving playlist failed."); }

inline const QString & editModeTitle()
{
    static const QString s = QObject::tr("Edit mode");
    return s;
}

}


PlaylistComponent::PlaylistComponent(
    QMainWindow & mainWindow, const Actions & actions,
    QtUtilities::Widgets::InputController & inputController,
    const Preferences & preferences,
    IsRecentHistoryEntry isRecentHistoryEntry,
    CommonTypes::PlayItems playItems,
    const std::string & preferencesDir, bool & cancelled)
    : actions_(actions), inputController_(inputController),
      addingPatterns_(preferences.addingPatterns),
      addingPolicy_(preferences.addingPolicy),
      skipRecentHistoryItemCount_(
          preferences.playback.skipRecentHistoryItemCount),
      isRecentHistoryEntry_(std::move(isRecentHistoryEntry)),
      playItems_(std::move(playItems)),
      itemsFilename_(preferencesDir + "items"),
      qItemsFilename_(QtUtilities::toQString(itemsFilename_)),
      qBackupItemsFilename_(qItemsFilename_ + ".backup"),
      treeAutoCleanup_(preferences.treeAutoCleanup),
      treeWidget_(itemTree_, temporaryTree_, preferences.customActions,
                  playItems_)
{
# ifdef DEBUG_VENTUROUS_PLAYLIST_COMPONENT
    std::cout << "itemsFilename_ = " << itemsFilename_ << std::endl;
# endif
    treeWidget_.setAutoUnfoldedLevels(preferences.treeAutoUnfoldedLevels);

    if (QFileInfo(qItemsFilename_).isFile()) {
        if (QtUtilities::Widgets::HandleErrors {
        [&] {
                const std::string errorMessage = itemTree_.load(itemsFilename_);
                return errorMessage.empty() ? QString() :
                loadingErrorFullMessage(QtUtilities::toQString(errorMessage));
            }
        } .blocking(inputController_, loadingFailed(), & cancelled)) {
            itemTree_.nodesChanged();
            treeWidget_.updateTree();
        }
        else if (cancelled)
            return;
        else {
            itemTree_.topLevelNodes().clear();
            itemTree_.nodesChanged();
        }
    }
    else
        cancelled = false;

    mainWindow.setCentralWidget(& treeWidget_);

    connect(actions.playback.nextRandom, SIGNAL(triggered(bool)),
            SLOT(playbackNextRandom()));
    connect(actions.playback.playAll, SIGNAL(triggered(bool)),
            SLOT(playbackPlayAll()));

    {
        const Actions::Playlist & p = actions.playlist;
        connect(p.editMode, SIGNAL(triggered(bool)),
                SLOT(onEditModeStateChanged()));
        connect(p.applyChanges, SIGNAL(triggered(bool)), SLOT(applyChanges()));
        connect(p.cancelChanges, SIGNAL(triggered(bool)),
                SLOT(cancelChanges()));

        connect(p.addFiles, SIGNAL(triggered(bool)), SLOT(onAddFiles()));
        connect(p.addDirectory, SIGNAL(triggered(bool)),
                SLOT(onAddDirectory()));
        connect(p.cleanUp, SIGNAL(triggered(bool)), SLOT(onCleanUp()));
        connect(p.clear, SIGNAL(triggered(bool)), SLOT(onClear()));
        connect(p.restorePrevious, SIGNAL(triggered(bool)),
                SLOT(onRestorePrevious()));

        connect(p.load, SIGNAL(triggered(bool)), SLOT(onLoad()));
        connect(p.saveAs, SIGNAL(triggered(bool)), SLOT(onSaveAs()));
    }

    updateActionsState();
}

PlaylistComponent::~PlaylistComponent()
{
    CommonUtilities::exceptionsToStderr([this] {
        if (treeWidget_.editMode() && ! noChanges())
            saveTemporaryTree();
    }, VENTUROUS_ERROR_PREFIX "In ~PlaylistComponent(): ");
}

void PlaylistComponent::setPreferences(const Preferences & preferences)
{
    skipRecentHistoryItemCount_ =
            preferences.playback.skipRecentHistoryItemCount;
    treeWidget_.setAutoUnfoldedLevels(preferences.treeAutoUnfoldedLevels);
    treeAutoCleanup_ = preferences.treeAutoCleanup;
}

bool PlaylistComponent::playRandomItem()
{
    if (itemTree_.itemCount() > 0) {
        playItems_( { getNextRandomItem() });
        return true;
    }
    return false;
}

bool PlaylistComponent::quit()
{
    return ! treeWidget_.editMode() || noChanges() || leaveAskChangedEditMode();
}


void PlaylistComponent::updateActionsState()
{
    const Actions::Playlist & p = actions_.playlist;
    p.editMode->setChecked(treeWidget_.editMode());
    p.applyChanges->setEnabled(treeWidget_.editMode());
    p.cancelChanges->setEnabled(treeWidget_.editMode());
}

bool PlaylistComponent::noChanges() const
{
    treeWidget_.assertValidTemporaryTree();
    return * temporaryTree_ == itemTree_;
}

void PlaylistComponent::enterEditMode()
{
    temporaryTree_.reset(new ItemTree::Tree(itemTree_));
    treeWidget_.setEditMode(true, false);
    updateActionsState();
    emit editModeChanged();
}

bool PlaylistComponent::applyChangesChanged()
{
    bool cancelled;
    saveTemporaryTree(& cancelled);
    if (cancelled)
        return false;
    itemTree_ = std::move(* temporaryTree_);
    cancelChanges(false);
    return true;
}

void PlaylistComponent::cancelChanges(const bool noChanges)
{
    treeWidget_.setEditMode(false, ! noChanges);
    temporaryTree_.reset();
    updateActionsState();
    emit editModeChanged();
}

bool PlaylistComponent::enterAskEditMode()
{
    const auto selectedButton =
        inputController_.showMessage(
            editModeTitle(), tr("Enter playlist edit mode?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No,
            QMessageBox::Question);

    if (selectedButton == QMessageBox::Yes) {
        enterEditMode();
        return true;
    }
    return false;
}

bool PlaylistComponent::leaveAskEditMode()
{
    return noChanges() ? leaveAskUnchangedEditMode()
           : leaveAskChangedEditMode();
}

bool PlaylistComponent::leaveAskChangedEditMode()
{
    const auto selectedButton =
        inputController_.showMessage(
            editModeTitle(), tr("Save changes to playlist?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Cancel, QMessageBox::Question);

    switch (selectedButton) {
        case QMessageBox::Yes:
            return applyChangesChanged();
        case QMessageBox::No:
            cancelChanges(false);
            return true;
        default:
            return false;
    }
}

bool PlaylistComponent::leaveAskUnchangedEditMode()
{
    const auto selectedButton =
        inputController_.showMessage(
            editModeTitle(), tr("Cancel editing?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No,
            QMessageBox::Question);

    if (selectedButton == QMessageBox::Yes) {
        cancelChanges(true);
        return true;
    }
    return false;
}

bool PlaylistComponent::ensureAskInEditMode()
{
    return treeWidget_.editMode() ? true : enterAskEditMode();
}

bool PlaylistComponent::ensureAskOutOfEditMode()
{
    return treeWidget_.editMode() ? leaveAskEditMode() : true;
}

std::string PlaylistComponent::getNextRandomItem()
{
    // When recentEntryCount is close to the total tree item count, the
    // performance of this function can randomly degrade due to many successive
    // history searches. Let maxRecentEntryCount be substantially lower than
    // the total item count to limit the impact on performance.
    const auto maxRecentItemCount = 0.8 * itemTree_.itemCount();
    const auto recentEntryCount = std::min(
                    skipRecentHistoryItemCount_,
                    static_cast<unsigned>(maxRecentItemCount));
# ifdef DEBUG_VENTUROUS_PLAYLIST_COMPONENT
    std::cout << "recentEntryCount = " << recentEntryCount << std::endl;
# endif
    std::string item;
    do {
        item = randomItemChooser_.randomPath(itemTree_);
# ifdef DEBUG_VENTUROUS_PLAYLIST_COMPONENT
        std::cout << "Random filename: "
                  << item.substr(item.find_last_of('/') + 1) << std::endl;
# endif
    }
    while (isRecentHistoryEntry_(recentEntryCount, item));
    return item;
}

bool PlaylistComponent::makeBackup() const
{
    QFile::remove(qBackupItemsFilename_);
    return QFile::rename(qItemsFilename_, qBackupItemsFilename_);
}

void PlaylistComponent::restoreBackup() const
{
    QFile::remove(qItemsFilename_);
    QFile::copy(qBackupItemsFilename_, qItemsFilename_);
}

void PlaylistComponent::saveTemporaryTree(bool * const cancelled)
{
    treeWidget_.assertValidTemporaryTree();
    temporaryTree_->nodesChanged();
    if (treeAutoCleanup_)
        temporaryTree_->cleanUp();
    const bool backedUp = makeBackup();

    QtUtilities::Widgets::HandleErrors handleErrors {
        [&] {
            QtUtilities::makePathTo(qItemsFilename_);
            if (temporaryTree_->save(itemsFilename_))
                return QString();
            if (backedUp)
                restoreBackup();
            return savingFailed();
        }
    };
    if (cancelled == nullptr)
        handleErrors.nonBlocking();
    else
        handleErrors.blocking(inputController_, ioError(), cancelled);
}

void PlaylistComponent::loadTemporaryPlaylist(FilenameGetter filenameGetter)
{
    if (! ensureAskInEditMode())
        return;
    const QString file = filenameGetter();
    if (! file.isEmpty()) {
        if (QFileInfo(file).isFile()) {
            const std::string errorMessage =
                temporaryTree_->load(QtUtilities::qStringToString(file));
            if (! errorMessage.empty()) {
                temporaryTree_->topLevelNodes().clear();
                inputController_.showMessage(
                    loadingFailed(),
                    loadingErrorFullMessage(
                        QtUtilities::toQString(errorMessage)));
            }
            treeWidget_.updateTree();
        }
        else {
            inputController_.showMessage(
                loadingFailed(),
                loadingErrorFullMessage(
                    tr("\"%1\" - no such file.").arg(file)));
        }
    }
}


void PlaylistComponent::playbackNextRandom()
{
    if (! playRandomItem())
        actions_.playback.stop->trigger();
}

void PlaylistComponent::playbackPlayAll()
{
    auto all = itemTree_.getAllItems<CommonTypes::ItemCollection>();
    if (all.empty())
        actions_.playback.stop->trigger();
    else
        playItems_(std::move(all));
}

void PlaylistComponent::onEditModeStateChanged()
{
    if (treeWidget_.editMode()) {
        if (noChanges())
            cancelChanges(true);
        else {
            if (! leaveAskChangedEditMode())
                actions_.playlist.editMode->setChecked(true);
        }
    }
    else
        enterEditMode();
}

void PlaylistComponent::applyChanges()
{
    if (noChanges())
        cancelChanges(true);
    else
        applyChangesChanged();
}

void PlaylistComponent::cancelChanges()
{
    cancelChanges(noChanges());
}

void PlaylistComponent::onAddFiles()
{
    if (! ensureAskInEditMode())
        return;
    const QStringList files =
        inputController_.getOpenFileNames(tr("Add files"));
    if (! files.empty()) {
        std::for_each(files.begin(), files.end(),
        [this](const QString & filename) {
            temporaryTree_->insertItem(QtUtilities::qStringToString(filename));
        });
        treeWidget_.updateTree();
    }
}

void PlaylistComponent::onAddDirectory()
{
    if (! ensureAskInEditMode())
        return;
    const QString dir = inputController_.getFileOrDirName(
                            tr("Add directory"), QFileDialog::AcceptOpen,
                            QFileDialog::Directory);
    if (! dir.isEmpty()) {
        AddingItems::addDir(dir, addingPatterns_.enabledPatternLists(),
                            addingPolicy_, * temporaryTree_);
        treeWidget_.updateTree();
    }
}

void PlaylistComponent::onCleanUp()
{
    if (treeWidget_.editMode())
        temporaryTree_->nodesChanged();
    else if (! enterAskEditMode())
        return;
    temporaryTree_->cleanUp();
    treeWidget_.updateTree();
}

void PlaylistComponent::onClear()
{
    if (! ensureAskInEditMode())
        return;
    temporaryTree_->topLevelNodes().clear();
    treeWidget_.updateTree();
}

void PlaylistComponent::onRestorePrevious()
{
    loadTemporaryPlaylist([this] {
        return qBackupItemsFilename_;
    });
}

void PlaylistComponent::onLoad()
{
    loadTemporaryPlaylist([this] {
        return inputController_.getFileOrDirName(tr("Load playlist"),
        QFileDialog::AcceptOpen, QFileDialog::ExistingFile);
    });
}

void PlaylistComponent::onSaveAs()
{
    if (! ensureAskOutOfEditMode())
        return;
    const QString file = inputController_.getFileOrDirName(
                             tr("Save playlist as"), QFileDialog::AcceptSave,
                             QFileDialog::AnyFile);
    if (! file.isEmpty()) {
        QtUtilities::makePathTo(file);
        if (! itemTree_.save(QtUtilities::qStringToString(file)))
            inputController_.showMessage(ioError(), savingFailed());
    }
}
