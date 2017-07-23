/*
 This file is part of Venturous.
 Copyright (C) 2014, 2017 Igor Kushnir <igorkuo AT Google mail>

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
# include <ios>
# include <iostream>
# endif


# include "MainWindow.hpp"

# include "PlaylistComponent.hpp"
# include "PlaybackComponent.hpp"
# include "PreferencesComponent.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"
# include "SharedMemory.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtWidgetsUtilities/Miscellaneous.hpp>

# include <QString>
# include <QTimer>
# include <QDir>
# include <QUrl>
# include <QSharedMemory>
# include <QCoreApplication>
# include <QIcon>
# include <QDesktopServices>
# include <QKeyEvent>
# include <QCloseEvent>
# include <QSessionManager>
# include <QAction>
# include <QMenu>
# include <QSystemTrayIcon>

# include <utility>


namespace
{
inline QString getIconTooltip(MediaPlayer::Status status)
{
    return APPLICATION_NAME " - " + MediaPlayer::toString(status);
}

}



MainWindow::~MainWindow()
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "Entered MainWindow destructor." << std::endl;
# endif
}



QString MainWindow::getPreferencesDirName()
{
    QString result(PREFERENCES_DIR);
    if (! result.isEmpty() && result[0] == '~')
        result.replace(0, 1, QDir::homePath());
    return result;
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
    QtUtilities::Widgets::showAndActivateWindow(this);
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
    {
        QMenu * const iconMenu = new QMenu(APPLICATION_NAME, this);
        const Actions::Playback & pb = actions_->playback;
        iconMenu->addActions( { pb.play, pb.pause, pb.stop, pb.previous,
                                pb.next
                              });
        iconMenu->addSeparator();
        {
            QMenu * const externalPlayerMenu =
                iconMenu->addMenu(tr("&External player"));
            externalPlayerMenu->addActions( { pb.showExternalPlayerWindow,
                                              pb.hideExternalPlayerWindow,
                                              pb.setExternalPlayerOptions,
                                              pb.updateStatus
                                            });
            iconMenu->addSeparator();
        }
        iconMenu->addActions( { actions_->file.preferences,
                                actions_->file.quit
                              });

        notificationAreaIcon_.reset(new QSystemTrayIcon(windowIcon(), this));
        notificationAreaIcon_->setContextMenu(iconMenu);
    }
    connect(notificationAreaIcon_.get(),
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT(onNotificationAreaIconActivated(
                     QSystemTrayIcon::ActivationReason)));

    notificationAreaIcon_->setToolTip(
        getIconTooltip(playbackComponent_->status()));

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

bool MainWindow::quit()
{
    preferencesComponent_->closePreferencesWindow();
    /// WARNING: repeated execution blocking is possible here!
    if (! playlistComponent_->quit())
        return false;
    /// WARNING: repeated execution blocking is possible here!
    playbackComponent_->quit();
    /// WARNING: repeated execution blocking is possible here!
    preferencesComponent_->quit();
    return true;
}

void MainWindow::timerEvent(QTimerEvent *)
{
    if (inputController_.blocked())
        return;
    using namespace SharedMemory;

    sharedMemory_->lock();
    const char command = *static_cast<const char *>(sharedMemory_->constData());
    if (command != Symbol::noCommand())
        *static_cast<char *>(sharedMemory_->data()) = Symbol::noCommand();
    sharedMemory_->unlock();

    if (command != Symbol::noCommand()) {
        const Actions::Playback & pb = actions_->playback;
        switch (command) {
            case Symbol::play():
                pb.play->trigger();
                break;
            case Symbol::pause():
                pb.pause->trigger();
                break;
            case Symbol::stop():
                pb.stop->trigger();
                break;
            case Symbol::previous():
                pb.previous->trigger();
                break;
            case Symbol::replayLast():
                pb.replayLast->trigger();
                break;
            case Symbol::nextFromHistory():
                pb.nextFromHistory->trigger();
                break;
            case Symbol::nextRandom():
                pb.nextRandom->trigger();
                break;
            case Symbol::next():
                onPlaybackNext();
                break;
            case Symbol::playAll():
                pb.playAll->trigger();
                break;
            case Symbol::showExternal():
                pb.showExternalPlayerWindow->trigger();
                break;
            case Symbol::hideExternal():
                pb.hideExternalPlayerWindow->trigger();
                break;
            case Symbol::setExternalOptions():
                pb.setExternalPlayerOptions->trigger();
                break;
            case Symbol::updateStatus():
                pb.updateStatus->trigger();
                break;
            case Symbol::show():
                showWindowProperly();
                break;
            case Symbol::hide():
                hideWindowProperly();
                break;
            case Symbol::quit():
                onFileQuit();
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
    std::cout << "Entered MainWindow::closeEvent()." << std::endl;
# endif
    if (commitDataState_ == CommitDataState::cancelled ||
            inputController_.blocked()) {
        event->ignore();
        return;
    }
    if (commitDataState_ == CommitDataState::commited)
        return;

    const Preferences & preferences = preferencesComponent_->preferences;
    if (! preferences.notificationAreaIcon ||
            ! preferences.closeToNotificationArea) {
        onFileQuit();
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
        arg(MediaPlayer::toString(playbackComponent_->status()),
            playlistComponent_->editMode() ?
            tr(" (apply changes to use updated playlist)") : ""));
}

void MainWindow::onPlayerStatusChanged(const MediaPlayer::Status newStatus)
{
    setWindowTitle();
    if (notificationAreaIcon_ != nullptr)
        notificationAreaIcon_->setToolTip(getIconTooltip(newStatus));
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

void MainWindow::onFilePreferences()
{
    preferencesComponent_->showPreferencesWindow(this);
}

void MainWindow::onFilePreferencesDirectory()
{
    QString directory = getPreferencesDirName();
    if (! QDesktopServices::openUrl(QUrl::fromLocalFile(directory))) {
        inputController_.showMessage(
            tr("Preferences directory error"),
            tr("Could not open directory ") + std::move(directory));
    }
}

void MainWindow::onFileQuit()
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "Entered MainWindow::onFileQuit()." << std::endl;
# endif
    if (quit())
        QCoreApplication::quit();
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

void MainWindow::resetCommitDataState()
{
    commitDataState_ = CommitDataState::none;
}

void MainWindow::onCommitDataRequest(QSessionManager & manager)
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "MainWindow::onCommitDataRequest()" << std::endl;
    std::cout << "Session manager allows interaction: " << std::boolalpha
              << manager.allowsInteraction() << std::endl;
    std::cout << "Session manager allows error interaction: " << std::boolalpha
              << manager.allowsErrorInteraction() << std::endl;
# endif

    commitDataState_ = CommitDataState::commited;
    if (manager.allowsErrorInteraction()) {
        if (inputController_.blocked() || ! quit()) {
            manager.cancel();
            commitDataState_ = CommitDataState::cancelled;
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
            std::cout << "MainWindow::onCommitDataRequest(): cancelled quit."
                      << std::endl;
# endif
        }
    }
    QTimer::singleShot(0, this, SLOT(resetCommitDataState()));
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "MainWindow::onCommitDataRequest(): finished." << std::endl;
# endif
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
