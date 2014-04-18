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

# include "InputController.hpp"
# include "WindowUtilities.hpp"
# include "PlaylistComponent.hpp"
# include "PlaybackComponent.hpp"
# include "PreferencesComponent.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <QString>
# include <QObject>
# include <QSharedMemory>
# include <QCoreApplication>
# include <QIcon>
# include <QAction>
# include <QMenu>
# include <QKeyEvent>
# include <QCloseEvent>
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



MainWindow::~MainWindow()
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "Entered MainWindow destructor." << std::endl;
# endif
}



void MainWindow::setPreferencesNoComponents()
{
    const Preferences & preferences = preferencesComponent_->preferences;

    if (preferences.notificationAreaIcon)
        showNotificationAreaIcon();
    else
        hideNotificationAreaIcon();

    if (int(preferences.ventoolCheckInterval) != ventoolCheckInterval_) {
        ventoolCheckInterval_ = int(preferences.ventoolCheckInterval);
        killTimer(timerIdentifier_);
        if (ventoolCheckInterval_ != 0)
            timerIdentifier_ = startTimer(ventoolCheckInterval_);
    }

    Actions::AddingPolicy & dest = actions_->addingPolicy;
    const AddingItems::Policy & source = preferences.addingPolicy;
    dest.audioFile->setChecked(source.addFiles);
    dest.mediaDir->setChecked(source.addMediaDirs);
    dest.bothAudioFile->setChecked(source.ifBothAddFiles);
    dest.bothMediaDir->setChecked(source.ifBothAddMediaDirs);
    dest.primaryActionChanged();
}

void MainWindow::showWindowProperly()
{
    WindowUtilities::showAndActivateWindow(* this);
}

void MainWindow::hideWindowProperly()
{
    if (preferencesComponent_->preferences.notificationAreaIcon)
        hide();
    else
        showMinimized();
}

void MainWindow::showNotificationAreaIcon()
{
    if (notificationAreaIcon_ != nullptr)
        return;
    QMenu * const iconMenu = new QMenu(APPLICATION_NAME, this);
    iconMenu->addActions( { actions_->playback.play, actions_->playback.stop,
                            actions_->playback.previous,
                            actions_->playback.next,
                            actions_->file.preferences, actions_->file.quit
                          });
    iconMenu->insertSeparator(actions_->file.preferences);

    notificationAreaIcon_.reset(new QSystemTrayIcon(windowIcon(), this));
    notificationAreaIcon_->setContextMenu(iconMenu);
    connect(notificationAreaIcon_.get(),
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT(onNotificationAreaIconActivated(
                     QSystemTrayIcon::ActivationReason)));

    notificationAreaIcon_->setToolTip(
        getIconToolTip(playbackComponent_->isPlayerRunning()));

    notificationAreaIcon_->show();
}

void MainWindow::hideNotificationAreaIcon()
{
    if (notificationAreaIcon_ != nullptr) {
        notificationAreaIcon_->contextMenu()->deleteLater();
        notificationAreaIcon_.release()->deleteLater();
        show();
    }
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
        const Actions::Playback & pb = actions_->playback;
        switch (command) {
            case 'P':
                pb.play->trigger();
                break;
            case 'S':
                pb.stop->trigger();
                break;
            case 'V':
                pb.previous->trigger();
                break;
            case 'L':
                pb.replayLast->trigger();
                break;
            case 'T':
                pb.nextFromHistory->trigger();
                break;
            case 'R':
                pb.nextRandom->trigger();
                break;
            case 'N':
                onPlaybackNext();
                break;
            case 'A':
                pb.playAll->trigger();
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
                inputController_.showMessage(TOOL_NAME + tr(" command"),
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
    if (inputController_.blocked()) {
        quitState_ = false;
        event->ignore();
        return;
    }

    const Preferences & preferences = preferencesComponent_->preferences;
    if (preferences.notificationAreaIcon &&
            preferences.closeToNotificationArea && ! quitState_) {
        hide();
        event->ignore();
    }
    else {
        preferencesComponent_->closePreferencesWindow();

        /// WARNING: repeated execution blocking is possible here!
        if (! playlistComponent_->quit()) {
            quitState_ = false;
            event->ignore();
            return;
        }
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
        std::cout << "MainWindow::closeEvent: quit is inevitable." << std::endl;
# endif
        playbackComponent_->quit();
        /// WARNING: repeated execution blocking is possible here!
        preferencesComponent_->quit();

        QCoreApplication::quit();
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
        std::cout << "MainWindow::closeEvent: finished." << std::endl;
# endif
    }
}


void MainWindow::copyInternalOptionsToPreferences()
{
    Preferences & preferences = preferencesComponent_->preferences;
    preferences.playback.history.currentIndex =
        playbackComponent_->currentHistoryEntryIndex();
    preferences.windowGeometry = saveGeometry();
    preferences.windowState = saveState();
}

void MainWindow::onPreferencesChanged()
{
    setPreferencesNoComponents();
    const Preferences & preferences = preferencesComponent_->preferences;
    playlistComponent_->setPreferences(preferences);
    playbackComponent_->setPreferences(preferences);
}

void MainWindow::setWindowTitle()
{
    const int nItems = playlistComponent_->itemCount();
    QMainWindow::setWindowTitle(
        APPLICATION_NAME + tr(" - %1 playable %2 - %3%4").
        arg(nItems).arg(nItems == 1 ? tr("item") : tr("items")).
        arg(getStatus(playbackComponent_->isPlayerRunning()),
            playlistComponent_->editMode() ?
            tr(" (apply changes to use updated playlist)") : ""));
}

void MainWindow::onPlayerStateChanged(const bool isPlayerRunning)
{
    setWindowTitle();
    if (notificationAreaIcon_ != nullptr)
        notificationAreaIcon_->setToolTip(getIconToolTip(isPlayerRunning));
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

void MainWindow::onFileQuit()
{
    quitState_ = true;
    close();
}

void MainWindow::onFilePreferences()
{
    preferencesComponent_->showPreferencesWindow(
        actions_->playlist.addFiles->icon(), this);
}

void MainWindow::onPlaybackNext()
{
    if (preferencesComponent_->preferences.playback.nextFromHistory) {
        if (playbackComponent_->playNextFromHistory())
            return;
    }
    if (! playlistComponent_->playRandomItem())
        actions_->playback.stop->trigger();
}

void MainWindow::onAudioFileStateChanged()
{
    preferencesComponent_->preferences.addingPolicy.addFiles =
        actions_->addingPolicy.audioFile->isChecked();
    actions_->addingPolicy.primaryActionChanged();
}

void MainWindow::onMediaDirStateChanged()
{
    preferencesComponent_->preferences.addingPolicy.addMediaDirs =
        actions_->addingPolicy.mediaDir->isChecked();
    actions_->addingPolicy.primaryActionChanged();
}

void MainWindow::onBothAudioFileStateChanged()
{
    preferencesComponent_->preferences.addingPolicy.ifBothAddFiles =
        actions_->addingPolicy.bothAudioFile->isChecked();
}

void MainWindow::onBothMediaDirStateChanged()
{
    preferencesComponent_->preferences.addingPolicy.ifBothAddMediaDirs =
        actions_->addingPolicy.bothMediaDir->isChecked();
}

void MainWindow::onAboutToQuit()
{
    // If the last component is not initialized, it means that MainWindow
    // constructor has not yet returned. Calling
    // copyInternalOptionsToPreferences() is not needed and can be dangerous
    // in this case.
    if (playlistComponent_ != nullptr)
        copyInternalOptionsToPreferences();
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "About to quit..." << std::endl;
# endif
}
