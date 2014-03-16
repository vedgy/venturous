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

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW_PLAYLIST
# include <iostream>
# endif

# include "MainWindow-inl.hpp"

# include "Actions.hpp"

# include <VenturousCore/AddingItems.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QDir>
# include <QFile>
# include <QFileInfo>
# include <QMessageBox>
# include <QAction>
# include <QFileDialog>

# include <utility>
# include <algorithm>


namespace
{
const QString & qItemsFilename()
{
    static const QString s = QtUtilities::toQString(MainWindow::itemsFilename);
    return s;
}
const QString & qBackupItemsFilename()
{
    static const QString s = qItemsFilename() + ".backup";
    return s;
}

const QString ioError = QObject::tr("I/O error"),
              savingFailed = QObject::tr("Saving playlist failed.");

const QString editModeTitle = QObject::tr("Edit mode");
const QString defaultPath = QDir::homePath();

}

void MainWindow::updateActionsState()
{
    actions_->playlist.editMode->setChecked(treeWidget_.editMode());
    actions_->playlist.applyChanges->setEnabled(treeWidget_.editMode());
    actions_->playlist.cancelChanges->setEnabled(treeWidget_.editMode());
}

bool MainWindow::noChanges() const
{
    if (! temporaryTree_)
        throw Error("valid temporaryTree_ expected, nullptr found.");
    return * temporaryTree_ == itemTree_;
}

void MainWindow::enterEditMode()
{
    temporaryTree_.reset(new ItemTree::Tree(itemTree_));
    treeWidget_.setEditMode(true, false);
    updateActionsState();
}

bool MainWindow::applyChangesChanged()
{
    temporaryTree_->nodesChanged();
    if (preferences_.treeAutoCleanup)
        temporaryTree_->cleanUp();

    QFile::remove(qBackupItemsFilename());
    const bool renamed = QFile::rename(qItemsFilename(),
                                       qBackupItemsFilename());

    while (! saveTemporaryTree()) {
        if (renamed) {
            QFile::remove(qItemsFilename());
            QFile::copy(qBackupItemsFilename(), qItemsFilename());
        }
        const auto selectedButton =
            QMessageBox::critical(this, ioError, savingFailed,
                                  QMessageBox::Retry | QMessageBox::Cancel,
                                  QMessageBox::Cancel);
        if (selectedButton == QMessageBox::Cancel)
            return false;
    }
    itemTree_ = std::move(* temporaryTree_);
    cancelChanges(false);

    setWindowTitle();
    return true;
}

void MainWindow::cancelChanges(const bool noChanges)
{
    treeWidget_.setEditMode(false, ! noChanges);
    temporaryTree_.reset();
    updateActionsState();
}

bool MainWindow::enterAskEditMode()
{
    const auto selectedButton =
        QMessageBox::question(
            this, editModeTitle, tr("Enter playlist edit mode?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (selectedButton == QMessageBox::Yes) {
        enterEditMode();
        return true;
    }
    return false;
}

bool MainWindow::leaveAskEditMode()
{
    return noChanges() ? leaveAskUnchangedEditMode()
           : leaveAskChangedEditMode();
}

bool MainWindow::leaveAskChangedEditMode()
{
    const auto selectedButton =
        QMessageBox::question(
            this, editModeTitle, tr("Save changes to playlist?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Cancel);

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

bool MainWindow::leaveAskUnchangedEditMode()
{
    const auto selectedButton =
        QMessageBox::question(
            this, editModeTitle, tr("Cancel editing?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (selectedButton == QMessageBox::Yes) {
        cancelChanges(true);
        return true;
    }
    return false;
}

bool MainWindow::ensureAskInEditMode()
{
    return treeWidget_.editMode() ? true : enterAskEditMode();
}

bool MainWindow::ensureAskOutOfEditMode()
{
    return treeWidget_.editMode() ? leaveAskEditMode() : true;
}

bool MainWindow::saveTemporaryTree() const
{
    {
        const QString absolutePath = QFileInfo(qItemsFilename()).absolutePath();
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW_PLAYLIST
        std::cout << "Absolute path to itemsFilename: " <<
                  QtUtilities::qStringToString(absolutePath) << std::endl;
# endif
        QDir dir;
        dir.mkpath(absolutePath);
    }
    return temporaryTree_->save(itemsFilename);
}


void MainWindow::onEditModeStateChanged()
{
    if (treeWidget_.editMode()) {
        if (noChanges())
            cancelChanges(true);
        else {
            if (! leaveAskChangedEditMode())
                actions_->playlist.editMode->setChecked(true);
        }
    }
    else
        enterEditMode();
}

void MainWindow::applyChanges()
{
    if (noChanges())
        cancelChanges(true);
    else
        applyChangesChanged();
}

void MainWindow::cancelChanges()
{
    cancelChanges(noChanges());
}

void MainWindow::onAddFiles()
{
    if (! ensureAskInEditMode())
        return;
    const QStringList files =
        QFileDialog::getOpenFileNames(this, tr("Add files"), defaultPath,
                                      QString(), nullptr,
                                      QFileDialog::ReadOnly);
    if (! files.empty()) {
        std::for_each(files.begin(), files.end(),
        [this](const QString & filename) {
            temporaryTree_->insertItem(QtUtilities::qStringToString(filename));
        });
        treeWidget_.updateTree();
    }
}

void MainWindow::onAddDirectory()
{
    if (! ensureAskInEditMode())
        return;
    const QString dir =
        QFileDialog::getExistingDirectory(
            this, tr("Add directory"), defaultPath,
            QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if (! dir.isEmpty()) {
        AddingItems::addDir(dir, preferences_.addingPolicy, * temporaryTree_);
        treeWidget_.updateTree();
    }
}

void MainWindow::onCleanUp()
{
    if (treeWidget_.editMode())
        temporaryTree_->nodesChanged();
    else if (! enterAskEditMode())
        return;
    temporaryTree_->cleanUp();
    treeWidget_.updateTree();
}

void MainWindow::onClear()
{
    if (! ensureAskInEditMode())
        return;
    temporaryTree_->topLevelNodes().clear();
    treeWidget_.updateTree();
}

void MainWindow::onRestorePrevious()
{
    loadTemporaryPlaylist([] {
        return qBackupItemsFilename();
    });
}

void MainWindow::onLoad()
{
    loadTemporaryPlaylist([this] {
        return QFileDialog::getOpenFileName(this, tr("Load playlist"),
        defaultPath, QString(), nullptr, QFileDialog::ReadOnly);
    });
}

void MainWindow::onSaveAs()
{
    if (! ensureAskOutOfEditMode())
        return;
    const QString file = QFileDialog::getSaveFileName(
                             this, tr("Save playlist as"), defaultPath);
    if (! file.isEmpty()) {
        if (! itemTree_.save(QtUtilities::qStringToString(file)))
            QMessageBox::critical(this, ioError, savingFailed);
    }
}
