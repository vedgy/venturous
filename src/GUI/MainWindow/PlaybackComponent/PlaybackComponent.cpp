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

# include "PlaybackComponent.hpp"

# include "CommonTypes.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtWidgetsUtilities/InputController.hpp>
# include <QtWidgetsUtilities/HandleErrors.hpp>

# include <QtCoreUtilities/String.hpp>

# include <CommonUtilities/ExceptionsToStderr.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QTimer>
# include <QFileInfo>
# include <QProcess>
# include <QAction>
# include <QLabel>
# include <QMessageBox>
# include <QFileDialog>
# include <QDockWidget>
# include <QStatusBar>
# include <QMainWindow>

# include <utility>
# include <functional>
# include <tuple>
# include <string>


namespace
{
inline QString historyWindowName() { return QObject::tr("History"); }

inline QString externalPlayerErrors(bool plural = true)
{
    return QObject::tr("External player ") +
           (plural ? QObject::tr("errors") : QObject::tr("error"));
}

}



PlaybackComponent::PlaybackComponent(
    QMainWindow & mainWindow, const Actions::Playback & actions,
    QtUtilities::Widgets::InputController & inputController,
    const Preferences & preferences, const std::string & preferencesDir,
    bool & cancelled)
    : mainWindow_(mainWindow), actions_(actions),
      inputController_(inputController),
      historyFilename_(preferencesDir + "history"),
      playerId_(unsigned(GetMediaPlayer::playerList().size())),
      historyWidget_(preferences.customActions,
                     std::bind(& PlaybackComponent::playFromHistory, this,
                               std::placeholders::_1),
    [this](CommonTypes::ItemCollection items) { play(std::move(items)); },
preferences.playback.history)
{
    setPreferencesExceptHistory(preferences, & cancelled);
    if (cancelled)
        return;

    if (QFileInfo(QtUtilities::toQString(historyFilename_)).isFile()) {
        /// WARNING: repeated execution blocking is possible here!
        QtUtilities::Widgets::HandleErrors {
            [&] {
                return historyWidget_.load(historyFilename_) ?
                QString() : tr("Loading history failed.");
            }
        } .blocking(inputController_, historyWindowName(), & cancelled);
        if (cancelled)
            return;
    }

    {
        QDockWidget * const dockWidget =
            new QDockWidget(historyWindowName(), & mainWindow);
        dockWidget->setObjectName(dockWidget->windowTitle());
        dockWidget->setWidget(& historyWidget_);
        mainWindow.addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
    }

    connect(actions.play, SIGNAL(triggered(bool)), SLOT(playbackPlay()));
    connect(actions.pause, SIGNAL(triggered(bool)), SLOT(playbackPause()));
    connect(actions.stop, SIGNAL(triggered(bool)), SLOT(playbackStop()));
    connect(actions.previous, SIGNAL(triggered(bool)),
            SLOT(playbackPrevious()));
    connect(actions.replayLast, SIGNAL(triggered(bool)),
            SLOT(playbackReplayLast()));
    connect(actions.nextFromHistory, SIGNAL(triggered(bool)),
            SLOT(playbackNextFromHistory()));

    connect(actions.showExternalPlayerWindow, SIGNAL(triggered(bool)),
            SLOT(showExternalPlayerWindow()));
    connect(actions.hideExternalPlayerWindow, SIGNAL(triggered(bool)),
            SLOT(hideExternalPlayerWindow()));
    connect(actions.updateStatus, SIGNAL(triggered(bool)),
            SLOT(updateStatus()));

    connect(actions.importHistory, SIGNAL(triggered(bool)),
            SLOT(importHistory()));
    connect(actions.exportHistory, SIGNAL(triggered(bool)),
            SLOT(exportHistory()));

    historyWidget_.connect(actions.clearHistory, SIGNAL(triggered(bool)),
                           SLOT(clearHistory()));

    connect(& historyWidget_, SIGNAL(historyChanged()),
            SLOT(onHistoryChanged()));
}

PlaybackComponent::~PlaybackComponent()
{
    CommonUtilities::exceptionsToStderr([this] {
        saveHistory(true);
    }, VENTUROUS_ERROR_PREFIX "In ~PlaybackComponent(): ");
}

void PlaybackComponent::setPreferences(const Preferences & preferences)
{
    historyWidget_.setPreferences(preferences.playback.history);
    setPreferencesExceptHistory(preferences);
}

void PlaybackComponent::play(std::string item)
{
    if (! mediaPlayer_->start(item))
        return;
    historyWidget_.push(std::move(item));
    onHistoryChanged();
    resetLastPlayedItem();
    setStatus(Status::playing);
}

void PlaybackComponent::play(CommonTypes::ItemCollection items)
{
    if (items.size() == 1)
        play(std::move(items.back()));
    else {
        if (! mediaPlayer_->start(std::move(items)))
            return;
        historyWidget_.playedMultipleItems();
        resetLastPlayedItem();
        setStatus(Status::playing);
    }
}

bool PlaybackComponent::playNextFromHistory()
{
    return playFromHistoryIfNotEmpty(historyWidget_.next());
}

void PlaybackComponent::quit()
{
    saveHistory();
}



void PlaybackComponent::setPreferencesExceptHistory(
    const Preferences & preferences, bool * const cancelled)
{
    const Preferences::Playback & pb = preferences.playback;

    if (mediaPlayer_ != nullptr)
        mediaPlayer_->setExitExternalPlayerOnQuit(pb.exitExternalPlayerOnQuit);
    if (pb.playerId != playerId_) {
        if (mediaPlayer_ == nullptr)
            setStatus(Status::stopped);
        else
            playbackStop();
        playerId_ = pb.playerId;

        QtUtilities::Widgets::HandleErrors {
            [&] {
                QStringList errors;
                std::tie(mediaPlayer_, errors) =
                GetMediaPlayer::instance(int(playerId_));
                return errors.empty() ? QString() : errors.join("\n");
            }
        } .blocking(inputController_, externalPlayerErrors(), cancelled);
        if (cancelled != nullptr && * cancelled)
            return;

        connect(mediaPlayer_.get(),
                SIGNAL(finished(bool, int, QStringList, QStringList)),
                SLOT(onPlayerFinished(bool, int, QStringList, QStringList)));
        connect(mediaPlayer_.get(), SIGNAL(error(QString)),
                SLOT(onPlayerError(QString)));
        mediaPlayer_->setExitExternalPlayerOnQuit(pb.exitExternalPlayerOnQuit);
    }
    else if (cancelled != nullptr)
        * cancelled = false;

    mediaPlayer_->setAutoSetOptions(pb.autoSetExternalPlayerOptions);
    mediaPlayer_->setAutoHideWindow(pb.autoHideExternalPlayerWindow);
    if (int(pb.statusUpdateInterval) != statusUpdateInterval_) {
        statusUpdateInterval_ = int(pb.statusUpdateInterval);
        killTimer(timerIdentifier_);
        if (statusUpdateInterval_ != 0)
            timerIdentifier_ = startTimer(statusUpdateInterval_);
    }
    desktopNotifications_ = pb.desktopNotifications;
    saveHistoryToDiskImmediately_ = pb.history.saveToDiskImmediately;

    if (preferences.statusBar) {
        if (lastPlayedItemLabel_ == nullptr) {
            lastPlayedItemLabel_.reset(new QLabel);
            mainWindow_.statusBar()->addWidget(lastPlayedItemLabel_.get());
            resetLastPlayedItem(false);
        }
    }
    else {
        if (lastPlayedItemLabel_ != nullptr) {
            lastPlayedItemLabel_.reset();
            delete mainWindow_.statusBar();
        }
    }

    if (saveHistoryToDiskImmediately_) {
        /// WARNING: repeated execution blocking is possible here!
        saveHistory();
    }
}

void PlaybackComponent::playFromHistory(const std::string entry)
{
    if (! mediaPlayer_->start(entry))
        return;
    resetLastPlayedItem();
    setStatus(Status::playing);
}

bool PlaybackComponent::playFromHistoryIfNotEmpty(std::string entry)
{
    if (entry.empty())
        return false;
    playFromHistory(std::move(entry));
    checkHistoryWidgetChanges();
    return true;
}

void PlaybackComponent::resetLastPlayedItem(const bool playbackStarted)
{
    if (playbackStarted && desktopNotifications_) {
        QString summary = historyWidget_.currentShortened();
        if (summary.isEmpty()) {
            summary = historyWidget_.maxSize() == 0  ?
                      tr("<unknown item(s)>") : tr("<multiple items>");
        }
        // replace special notify-send characters.
        summary.replace('&', "&amp;");
        summary.replace('<', "&lt;");
        summary.replace('\\', "&#92;");
        QProcess::startDetached("notify-send",
        { "-a", APPLICATION_NAME, "-i", ICON_NAME, "|>", summary });
    }

    if (lastPlayedItemLabel_ == nullptr)
        return;
    QString textPrefix = tr("Last played item: ");
    const QString entry = historyWidget_.currentAbsolute();
    if (entry.isEmpty()) {
        lastPlayedItemLabel_->setText(std::move(textPrefix) + tr("<unknown>"));
        lastPlayedItemLabel_->setToolTip(
            tr("Unknown item(s).\n"
               "This means that multiple items were played or that last played "
               "item was removed from history."));
    }
    else {
        lastPlayedItemLabel_->setText(std::move(textPrefix) +
                                      historyWidget_.currentShortened());
        lastPlayedItemLabel_->setToolTip(entry);
    }
}

void PlaybackComponent::setStatus(const Status status)
{
    if (status_ != status) {
        status_ = status;
        emit statusChanged(status_);
    }
}

void PlaybackComponent::checkHistoryWidgetChanges()
{
    if (historyWidget_.isHistoryChangedBySettingCurrentEntry())
        onHistoryChanged();
}

void PlaybackComponent::saveHistory(const bool noBlocking)
{
    if (isHistorySaved_)
        return;
    QtUtilities::Widgets::HandleErrors handleErrors {
        [&] {
            return historyWidget_.save(historyFilename_) ?
            QString() : tr("Saving history failed.");
        }
    };
    isHistorySaved_ = noBlocking ? handleErrors.nonBlocking() :
                      handleErrors.blocking(inputController_,
                                            historyWindowName());
}

void PlaybackComponent::timerEvent(QTimerEvent *)
{
    updateStatus();
}


void PlaybackComponent::onPlayerFinished(
    const bool crashExit, const int exitCode, const QStringList errors,
    const QStringList missingFilesAndDirs)
{
    QString errorMessage;
    if (crashExit)
        errorMessage = tr("crashed");
    else if (exitCode != 0)
        errorMessage = tr("finished with exit code %1").arg(exitCode);
    if (! errorMessage.isEmpty()) {
        errorMessage = mediaPlayer_->playerName() + ' ' +
                       errorMessage + ".\n\n";
    }

    if (! errors.empty())
        errorMessage += errors.join("\n") + "\n\n";
    if (! missingFilesAndDirs.empty()) {
        errorMessage += tr("No such files or directories (%1):")
                        .arg(missingFilesAndDirs.size()) + '\n' +
                        missingFilesAndDirs.join(";\n") + ".\n\n";
    }

    if (! errorMessage.isEmpty()) {
        errorMessage += '\t' + tr("Continue playback?");
        setStatus(Status::stopped);
        const auto selectedButton =
            inputController_.showMessage(externalPlayerErrors(), errorMessage,
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);
        if (selectedButton == QMessageBox::No)
            return;
    }
    actions_.next->trigger();
}

void PlaybackComponent::onPlayerError(QString errorMessage)
{
    inputController_.showMessage(
        externalPlayerErrors(false),
        mediaPlayer_->playerName() + ": " + std::move(errorMessage));
    updateStatus();
}

void PlaybackComponent::updateStatus()
{
    setStatus(mediaPlayer_->status());
}

void PlaybackComponent::onHistoryChanged()
{
    isHistorySaved_ = false;
    if (saveHistoryToDiskImmediately_)
        saveHistory();
}

void PlaybackComponent::playbackPlay()
{
    if (mediaPlayer_->start())
        setStatus(Status::playing);
}

void PlaybackComponent::playbackPause()
{
    mediaPlayer_->togglePause();
    QTimer::singleShot(1000, this, SLOT(updateStatus()));
}

void PlaybackComponent::playbackStop()
{
    mediaPlayer_->exit();
    setStatus(Status::stopped);
}

void PlaybackComponent::playbackPrevious()
{
    if (! playFromHistoryIfNotEmpty(historyWidget_.previous()))
        playbackStop();
}

void PlaybackComponent::playbackReplayLast()
{
    if (! playFromHistoryIfNotEmpty(historyWidget_.current()))
        playbackStop();
}

void PlaybackComponent::playbackNextFromHistory()
{
    if (! playNextFromHistory())
        playbackStop();
}

void PlaybackComponent::showExternalPlayerWindow()
{
    mediaPlayer_->setPlayerWindowVisible(true);
}

void PlaybackComponent::hideExternalPlayerWindow()
{
    mediaPlayer_->setPlayerWindowVisible(false);
}

void PlaybackComponent::importHistory()
{
    const QString file = inputController_.getFileOrDirName(tr("Import history"),
                         QFileDialog::AcceptOpen, QFileDialog::ExistingFile);
    if (! file.isEmpty()) {
        if (historyWidget_.load(QtUtilities::qStringToString(file)))
            historyWidget_.playedMultipleItems();
        else {
            inputController_.showMessage(
                tr("Importing history failed"),
                tr("Could not load history from specified file."));
        }
        onHistoryChanged();
    }
}

void PlaybackComponent::exportHistory()
{
    const QString file = inputController_.getFileOrDirName(
                             tr("Export history"), QFileDialog::AcceptSave,
                             QFileDialog::AnyFile);
    if (! file.isEmpty()) {
        if (! historyWidget_.save(QtUtilities::qStringToString(file))) {
            inputController_.showMessage(
                tr("Exporting history failed"),
                tr("Could not save history to specified file."));
        }
    }
}
