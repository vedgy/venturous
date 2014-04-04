/*
 This file is part of Venturous.
 Copyright (C) 2014 Igor Kushnir <igorkuo AT Google mail>

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
# include <QtCoreUtilities/String.hpp>
# include <iostream>
# endif

# include "PlaylistComponent.hpp"

# include "CommonTypes.hpp"
# include "InputController.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <VenturousCore/ItemTree-inl.hpp>
# include <VenturousCore/AddingItems.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QDir>
# include <QFile>
# include <QFileInfo>
# include <QAction>
# include <QMessageBox>
# include <QMainWindow>

# include <utility>
# include <algorithm>
# include <iostream>


namespace
{
QString ioError() { return QObject::tr("I/O error"); }
QString savingFailed() { return QObject::tr("Saving playlist failed."); }

QString editModeTitle() { return QObject::tr("Edit mode"); }
QString defaultPath() { return QDir::homePath(); }

}

PlaylistComponent::PlaylistComponent(
    QMainWindow & mainWindow, const Actions & actions,
    InputController & inputController, const Preferences & preferences,
    CommonTypes::PlayItems playItems, const std::string & preferencesDir)
    : actions_(actions.playlist), inputController_(inputController),
      addingPolicy_(preferences.addingPolicy), playItems_(std::move(playItems)),
      itemsFilename_(preferencesDir + "items"),
      qItemsFilename_(QtUtilities::toQString(itemsFilename_)),
      qBackupItemsFilename_(qItemsFilename_ + ".backup"),
      treeAutoCleanup_(preferences.treeAutoCleanup),
      treeWidget_(itemTree_, temporaryTree_)
{
# ifdef DEBUG_VENTUROUS_PLAYLIST_COMPONENT
    std::cout << "itemsFilename_ = " << itemsFilename_ << std::endl;
# endif
    treeWidget_.setAutoUnfoldedLevels(preferences.treeAutoUnfoldedLevels);

    const std::string errorMessage = itemTree_.load(itemsFilename_);
    if (errorMessage.empty()) {
        itemTree_.nodesChanged();
        treeWidget_.updateTree();
    }
    else {
        itemTree_.topLevelNodes().clear();
        itemTree_.nodesChanged();
        showLoadingPlaylistErrorMessage(
            errorMessage,
            tr("If you have not yet created (and saved) %1 playlist for "
               "current user, ignore this error.").arg(APPLICATION_NAME));
    }

    mainWindow.setCentralWidget(& treeWidget_);

    connect(actions.playback.playAll, SIGNAL(triggered(bool)),
            SLOT(onPlayAll()));

    connect(actions_.editMode, SIGNAL(triggered(bool)),
            SLOT(onEditModeStateChanged()));
    connect(actions_.applyChanges, SIGNAL(triggered(bool)),
            SLOT(applyChanges()));
    connect(actions_.cancelChanges, SIGNAL(triggered(bool)),
            SLOT(cancelChanges()));

    connect(actions_.addFiles, SIGNAL(triggered(bool)), SLOT(onAddFiles()));
    connect(actions_.addDirectory, SIGNAL(triggered(bool)),
            SLOT(onAddDirectory()));
    connect(actions_.cleanUp, SIGNAL(triggered(bool)), SLOT(onCleanUp()));
    connect(actions_.clear, SIGNAL(triggered(bool)), SLOT(onClear()));
    connect(actions_.restorePrevious, SIGNAL(triggered(bool)),
            SLOT(onRestorePrevious()));

    connect(actions_.load, SIGNAL(triggered(bool)), SLOT(onLoad()));
    connect(actions_.saveAs, SIGNAL(triggered(bool)), SLOT(onSaveAs()));

    connect(& treeWidget_, SIGNAL(itemActivated(QString)),
            SIGNAL(itemActivated(QString)));

    updateActionsState();
}

PlaylistComponent::~PlaylistComponent()
{
    if (! treeWidget_.editMode() || noChanges())
        return;
    const bool renamed = prepareForApplyingChanges();
    if (! saveTemporaryTree()) {
        std::cerr << ERROR_PREFIX <<
                  QtUtilities::qStringToString(savingFailed()) << std::endl;
        if (renamed)
            restoreBackupFile();
    }
}

void PlaylistComponent::setPreferences(const Preferences & preferences)
{
    treeWidget_.setAutoUnfoldedLevels(preferences.treeAutoUnfoldedLevels);
    treeAutoCleanup_ = preferences.treeAutoCleanup;
}

bool PlaylistComponent::playRandomItem()
{
    if (itemTree_.itemCount() > 0) {
        playItems_( { randomItemChooser_.randomPath(itemTree_) });
        return true;
    }
    return false;
}

bool PlaylistComponent::quit()
{
    return ensureAskOutOfEditMode();
}


void PlaylistComponent::showLoadingPlaylistErrorMessage(
    const std::string & errorMessage, const QString & suffix)
{
    QString message = tr("Error: ") + QtUtilities::toQString(errorMessage);
    if (! suffix.isEmpty())
        message += '\n' + suffix;
    inputController_.showMessage(tr("Loading playlist failed"), message);
}

void PlaylistComponent::updateActionsState()
{
    actions_.editMode->setChecked(treeWidget_.editMode());
    actions_.applyChanges->setEnabled(treeWidget_.editMode());
    actions_.cancelChanges->setEnabled(treeWidget_.editMode());
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
}

bool PlaylistComponent::prepareForApplyingChanges()
{
    treeWidget_.assertValidTemporaryTree();
    temporaryTree_->nodesChanged();
    if (treeAutoCleanup_)
        temporaryTree_->cleanUp();

    QFile::remove(qBackupItemsFilename_);
    return QFile::rename(qItemsFilename_, qBackupItemsFilename_);
}

void PlaylistComponent::restoreBackupFile()
{
    QFile::remove(qItemsFilename_);
    QFile::copy(qBackupItemsFilename_, qItemsFilename_);
}

bool PlaylistComponent::applyChangesChanged()
{
    const bool renamed = prepareForApplyingChanges();
    while (! saveTemporaryTree()) {
        if (renamed)
            restoreBackupFile();
        const auto selectedButton =
            inputController_.showMessage(
                ioError(), savingFailed(),
                QMessageBox::Retry | QMessageBox::Cancel | QMessageBox::Ignore,
                QMessageBox::Cancel);
        if (selectedButton == QMessageBox::Retry)
            continue;
        if (selectedButton == QMessageBox::Ignore)
            break;
        return false;
    }
    itemTree_ = std::move(* temporaryTree_);
    cancelChanges(false);

    emit treeChanged();
    return true;
}

void PlaylistComponent::cancelChanges(const bool noChanges)
{
    treeWidget_.setEditMode(false, ! noChanges);
    temporaryTree_.reset();
    updateActionsState();
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

bool PlaylistComponent::saveTemporaryTree() const
{
    {
        const QString absolutePath = QFileInfo(qItemsFilename_).absolutePath();
# ifdef DEBUG_VENTUROUS_PLAYLIST_COMPONENT
        std::cout << "Location of itemsFilename: " <<
                  QtUtilities::qStringToString(absolutePath) << std::endl;
# endif
        QDir dir;
        dir.mkpath(absolutePath);
    }
    return temporaryTree_->save(itemsFilename_);
}


void PlaylistComponent::onPlayAll()
{
    playItems_(itemTree_.getAllItems<CommonTypes::ItemCollection>());
}

void PlaylistComponent::onEditModeStateChanged()
{
    if (treeWidget_.editMode()) {
        if (noChanges())
            cancelChanges(true);
        else {
            if (! leaveAskChangedEditMode())
                actions_.editMode->setChecked(true);
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
        AddingItems::addDir(dir, addingPolicy_, * temporaryTree_);
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
        if (! itemTree_.save(QtUtilities::qStringToString(file)))
            inputController_.showMessage(ioError(), savingFailed());
    }
}