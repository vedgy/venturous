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

# include "MainWindow-inl.hpp"

# include "PreferencesWindow.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QSharedMemory>
# include <QCoreApplication>
# include <QAction>
# include <QMenu>
# include <QMessageBox>
# include <QCloseEvent>
# include <QKeyEvent>
# include <QLabel>
# include <QSystemTrayIcon>


namespace
{
QString getStatus(bool isPlayerRunning)
{
    return isPlayerRunning ? QObject::tr("playing") : QObject::tr("stopped");
}

QString getIconToolTip(bool isPlayerRunning)
{
    return APPLICATION_NAME " - " + getStatus(isPlayerRunning);
}

}


void MainWindow::showLoadingPlaylistErrorMessage(
    const std::string & errorMessage, const QString & suffix)
{
    QString message = tr("Error: ") + QtUtilities::toQString(errorMessage);
    if (! suffix.isEmpty())
        message += '\n' + suffix;
    QMessageBox::critical(this, tr("Loading playlist failed"), message);
}


void MainWindow::setWindowTitle()
{
    QMainWindow::setWindowTitle(
        APPLICATION_NAME + tr(" - %1 playable items - %2").
        arg(itemTree_.itemCount()).arg(getStatus(isPlayerRunning_)));
}

void MainWindow::showNotificationAreaIcon()
{
    if (notificationAreaIcon_ == nullptr) {
        QMenu * const iconMenu = new QMenu(APPLICATION_NAME, this);
        iconMenu->addActions( { actions_->playback.play,
                                actions_->playback.stop,
                                actions_->playback.next,
                                actions_->file.preferences, actions_->file.quit
                              });
        iconMenu->insertSeparator(actions_->file.preferences);
        notificationAreaIcon_ = new QSystemTrayIcon(windowIcon(), this);
        notificationAreaIcon_->setContextMenu(iconMenu);
        connect(notificationAreaIcon_,
                SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                SLOT(onNotificationAreaIconActivated(
                         QSystemTrayIcon::ActivationReason)));
        notificationAreaIcon_->setToolTip(getIconToolTip(isPlayerRunning_));
    }
    notificationAreaIcon_->show();
}

void MainWindow::hideNotificationAreaIcon()
{
    if (notificationAreaIcon_ != nullptr)
        notificationAreaIcon_->hide();
}

void MainWindow::showWindowProperly()
{
    const Qt::WindowStates state = windowState();
    if (state & Qt::WindowMinimized) {
        if (state & Qt::WindowMaximized)
            showMaximized();
        else
            showNormal();
    }
    else
        show();
    activateWindow();
}

void MainWindow::hideWindowProperly()
{
    if (preferences_.notificationAreaIcon)
        hide();
    else
        showMinimized();
}

void MainWindow::playerStateChanged(const bool isRunning)
{
    if (isPlayerRunning_ != isRunning) {
        isPlayerRunning_ = isRunning;
        setWindowTitle();
        if (notificationAreaIcon_ != nullptr)
            notificationAreaIcon_->setToolTip(getIconToolTip(isPlayerRunning_));
    }
}

void MainWindow::preferencesChanged()
{
    mediaPlayer_.setExternalPlayerTimeout(preferences_.externalPlayerTimeout);
    mediaPlayer_.setAutoSetOptions(preferences_.autoSetExternalPlayerOptions);

    treeWidget_.setAutoUnfoldedLevels(preferences_.treeAutoUnfoldedLevels);

    if (preferences_.notificationAreaIcon)
        showNotificationAreaIcon();
    else
        hideNotificationAreaIcon();

    if (int(preferences_.ventoolCheckInterval) != ventoolCheckInterval_) {
        ventoolCheckInterval_ = int(preferences_.ventoolCheckInterval);
        killTimer(timerIdentifier_);
        if (ventoolCheckInterval_ != 0)
            timerIdentifier_ = startTimer(ventoolCheckInterval_);
    }

    Actions::AddingPolicy & dest = actions_->addingPolicy;
    const AddingItems::Policy & source = preferences_.addingPolicy;
    dest.audioFile->setChecked(source.addFiles);
    dest.mediaDir->setChecked(source.addMediaDirs);
    dest.bothAudioFile->setChecked(source.ifBothAddFiles);
    dest.bothMediaDir->setChecked(source.ifBothAddMediaDirs);
    dest.primaryActionChanged();
}

void MainWindow::playedItemChanged(const int playedItem,
                                   const QString & itemAbsolutePath)
{
    QString text = itemAbsolutePath;
    if (! text.isEmpty())
        text += ' ';
    text += QString("<b>(%1)</b>").arg(
                Preferences::PlayedItem::toQString(playedItem));

    lastPlayedItemLabel->setText(tr("Last played item: ") + text);
    lastPlayedItemLabel->setToolTip(text);

    preferences_.lastPlayedItem = playedItem;
}

void MainWindow::copyWindowGeometryAndStateToPreferences()
{
    preferences_.windowGeometry = saveGeometry();
    preferences_.windowState = saveState();
}

void MainWindow::timerEvent(QTimerEvent *)
{
    sharedMemory_->lock();
    const char command = *(const char *)sharedMemory_->constData();
    if (command != 0)
        *(char *)sharedMemory_->data() = 0;
    sharedMemory_->unlock();

    if (command != 0) {
        switch (command) {
            case 'P':
                playbackPlay();
                break;
            case 'S':
                playbackStop();
                break;
            case 'N':
                playbackNext();
                break;
            case 'A':
                onPlayAll();
                break;
            case 'Q':
                onFileQuit();
                break;
            case 'W':
                showWindowProperly();
                break;
            case 'H':
                hideWindowProperly();
                break;
            default:
                QMessageBox::critical(this, TOOL_NAME + tr(" command"),
                                      tr("Unknown %1 command: '%2'.")
                                      .arg(TOOL_NAME).arg(command));
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent * const event)
{
    if (event->key() == Qt::Key_Escape)
        hideWindowProperly();
    else
        QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent * const event)
{
    if (preferences_.notificationAreaIcon &&
            preferences_.closeToNotificationArea && ! quitState_) {
        hide();
        event->ignore();
    }
    else {
        if (isPreferencesWindowOpen_)
            preferencesWindow_->close();
        if (! ensureAskOutOfEditMode()) {
            quitState_ = false;
            event->ignore();
            return;
        }
        copyWindowGeometryAndStateToPreferences();
        if (preferences_ != savedPreferences_) {
            handlePreferencesErrors([this] {
                preferences_.save(preferencesFilename);
            }, saveErrorPrefix);
        }
        quitState_ = true;
        QCoreApplication::quit();
    }
}

void MainWindow::onNotificationAreaIconActivated(
    const QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Context) {
        if (isVisible())
            hide();
        else
            showWindowProperly();
    }
}

void MainWindow::onAudioFileStateChanged()
{
    preferences_.addingPolicy.addFiles =
        actions_->addingPolicy.audioFile->isChecked();
    actions_->addingPolicy.primaryActionChanged();
}

void MainWindow::onMediaDirStateChanged()
{
    preferences_.addingPolicy.addMediaDirs =
        actions_->addingPolicy.mediaDir->isChecked();
    actions_->addingPolicy.primaryActionChanged();
}

void MainWindow::onBothAudioFileStateChanged()
{
    preferences_.addingPolicy.ifBothAddFiles =
        actions_->addingPolicy.bothAudioFile->isChecked();
}

void MainWindow::onBothMediaDirStateChanged()
{
    preferences_.addingPolicy.ifBothAddMediaDirs =
        actions_->addingPolicy.bothMediaDir->isChecked();
}

void MainWindow::onAboutToQuit()
{
    if (! quitState_) {
        if (temporaryTree_ != nullptr)
            applyChanges();
        onFileQuit();
    }
}
