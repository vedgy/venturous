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

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
# include <iostream>
# endif


# include "MainWindow.hpp"

# include "WindowUtilities.hpp"
# include "PlaybackComponent.hpp"
# include "PlaylistComponent.hpp"
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




void MainWindow::setWindowTitle()
{
    QMainWindow::setWindowTitle(
        APPLICATION_NAME + tr(" - %1 playable items - %2").
        arg(playlistComponent_->tree().itemCount()).
        arg(getStatus(playbackComponent_->isPlayerRunning())));
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
        notificationAreaIcon_->setToolTip(
            getIconToolTip(playbackComponent_->isPlayerRunning()));
    }
    notificationAreaIcon_->show();
}

void MainWindow::hideNotificationAreaIcon()
{
    if (notificationAreaIcon_ != nullptr) {
        if (notificationAreaIcon_->isVisible()) {
            notificationAreaIcon_->hide();
            show();
        }
    }
}

void MainWindow::showWindowProperly()
{
    WindowUtilities::showAndActivateWindow(* this);
}

void MainWindow::hideWindowProperly()
{
    if (preferences_.notificationAreaIcon)
        hide();
    else
        showMinimized();
}

void MainWindow::onPlayerStateChanged(const bool isPlayerRunning)
{
    setWindowTitle();
    if (notificationAreaIcon_ != nullptr)
        notificationAreaIcon_->setToolTip(getIconToolTip(isPlayerRunning));
}

void MainWindow::preferencesChanged()
{
    playlistComponent_->setPreferences(preferences_);
    playbackComponent_->setPreferences(preferences_.playback);

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

void MainWindow::copyWindowGeometryAndStateToPreferences()
{
    preferences_.windowGeometry = saveGeometry();
    preferences_.windowState = saveState();
}

void MainWindow::timerEvent(QTimerEvent *)
{
    if (inputController_.blocked())
        return;

    sharedMemory_->lock();
    const char command = *(const char *)sharedMemory_->constData();
    if (command != 0)
        *(char *)sharedMemory_->data() = 0;
    sharedMemory_->unlock();

    if (command != 0) {
        switch (command) {
            case 'P':
                actions_->playback.play->trigger();
                break;
            case 'S':
                actions_->playback.stop->trigger();
                break;
            case 'N':
                actions_->playback.next->trigger();
                break;
            case 'A':
                actions_->playback.playAll->trigger();
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
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "Entered MainWindow::closeEvent." << std::endl;
# endif
    if (inputController_.blocked())
        return;

    if (preferences_.notificationAreaIcon &&
            preferences_.closeToNotificationArea && ! quitState_) {
        hide();
        event->ignore();
    }
    else {
        if (isPreferencesWindowOpen_)
            preferencesWindow_->close();
        if (! playlistComponent_->quit()) {
            quitState_ = false;
            event->ignore();
            return;
        }

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
        std::cout << "MainWindow::closeEvent: quit is inevitable." << std::endl;
# endif

        inputController_.blockInput(true);
        copyWindowGeometryAndStateToPreferences();
        preferences_.playback.history.currentIndex =
            playbackComponent_->currentHistoryEntryIndex();
        if (preferences_ != savedPreferences_) {
            handlePreferencesErrors([this] {
                preferences_.save(preferencesFilename);
            }, savePreferencesErrorPrefix);
        }
        playbackComponent_->quit();
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
    /// TODO: make quitState_ of enum type: noQuit, quitting, quitted.
    /// if (quitState_ != quitted) { ... }
    if (! quitState_) {
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
        std::cout << "Urgent quit was requested."
                  " MainWindow::closeEvent was bypassed." << std::endl;
# endif

        quitState_ = true;
        if (isPreferencesWindowOpen_)
            preferencesWindow_->close();
        copyWindowGeometryAndStateToPreferences();
        preferences_.playback.history.currentIndex =
            playbackComponent_->currentHistoryEntryIndex();
        if (preferences_ != savedPreferences_) {
            handlePreferencesErrors([this] {
                preferences_.save(preferencesFilename);
            }, savePreferencesErrorPrefix, true);
        }

        QCoreApplication::quit();
    }
}
