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
# include "InputController.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
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
# include <string>
# include <iostream>


namespace
{
QString historyWindowName() { return QObject::tr("History"); }
QString externalPlayerError() { return QObject::tr("External player error"); }

}



PlaybackComponent::PlaybackComponent(
    QMainWindow & mainWindow, const Actions::Playback & actions,
    InputController & inputController, const Preferences & preferences,
    const std::string & preferencesDir)
    : mainWindow_(mainWindow), actions_(actions),
      inputController_(inputController),
      historyFilename_(preferencesDir + "history"),
      historyWidget_(preferences.customActions,
                     std::bind(& PlaybackComponent::playFromHistory, this,
                               std::placeholders::_1),
    [this](CommonTypes::ItemCollection items) { play(std::move(items)); },
preferences.playback.history)
{
    setPreferencesExceptHistory(preferences);
    {
        using namespace std::placeholders;
        mediaPlayer_.setFinishedSlot(
            std::bind(& PlaybackComponent::onPlayerFinished, this, _1, _2, _3));
        mediaPlayer_.setErrorSlot(
            std::bind(& PlaybackComponent::onPlayerError, this, _1));
    }

    // Do nothing in case of failure because history is not very important
    // and file may not exist.
    historyWidget_.load(historyFilename_);
    {
        QDockWidget * const dockWidget =
            new QDockWidget(historyWindowName(), & mainWindow);
        dockWidget->setObjectName(dockWidget->windowTitle());
        dockWidget->setWidget(& historyWidget_);
        mainWindow.addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
    }

    connect(actions.play, SIGNAL(triggered(bool)), SLOT(playbackPlay()));
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
    if (! isHistorySaved_) {
        if (! historyWidget_.save(historyFilename_)) {
            std::cerr << VENTUROUS_ERROR_PREFIX "Saving history failed."
            << std::endl;
        }
    }
}

void PlaybackComponent::setPreferences(const Preferences & preferences)
{
    historyWidget_.setPreferences(preferences.playback.history);
    setPreferencesExceptHistory(preferences);
}

void PlaybackComponent::play(std::string item)
{
    if (! mediaPlayer_.start(item))
        return;
    historyWidget_.push(std::move(item));
    onHistoryChanged();
    resetLastPlayedItem();
    setPlayerState(true);
}

void PlaybackComponent::play(CommonTypes::ItemCollection items)
{
    if (items.size() == 1)
        play(std::move(items.back()));
    else {
        if (! mediaPlayer_.start(std::move(items)))
            return;
        historyWidget_.playedMultipleItems();
        resetLastPlayedItem();
        setPlayerState(true);
    }
}

bool PlaybackComponent::playNextFromHistory()
{
    return playFromHistoryIfNotEmpty(historyWidget_.next());
}

void PlaybackComponent::quit()
{
    if (isPlayerRunning_)
        playbackStop();
    saveHistory();
}



void PlaybackComponent::setPreferencesExceptHistory(
    const Preferences & preferences)
{
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
    const Preferences::Playback & pb = preferences.playback;
    mediaPlayer_.setAutoSetOptions(pb.autoSetExternalPlayerOptions);
    mediaPlayer_.setAutoHideWindow(pb.autoHideExternalPlayerWindow);
    desktopNotifications_ = pb.desktopNotifications;
    saveHistoryToDiskImmediately_ = pb.history.saveToDiskImmediately;
    if (saveHistoryToDiskImmediately_)
        saveHistory();
}

void PlaybackComponent::onPlayerFinished(
    const bool crashExit, const int exitCode,
    std::vector<std::string> missingFilesAndDirs)
{
    if (crashExit || exitCode != 0) {
        QString message =
            tr("%1 %2 with exit code %3.")
            .arg(QtUtilities::toQString(MediaPlayer::playerName()),
                 crashExit ? tr("crashed") : tr("exited"))
            .arg(exitCode);
        if (! criticalContinuePlaybackQuestion(
        externalPlayerError(), std::move(message))) {
            return;
        }
    }

    if (! missingFilesAndDirs.empty()) {
        QString message = tr("No such files or directories (%1):")
                          .arg(missingFilesAndDirs.size());
        for (const std::string & s : missingFilesAndDirs)
            message += '\n' + QtUtilities::toQString(s) + ';';
        message[message.size() - 1] = '.';

        if (! criticalContinuePlaybackQuestion(
        tr("Missing files or directories"), std::move(message))) {
            return;
        }
    }

    actions_.next->trigger();
}

void PlaybackComponent::onPlayerError(std::string errorMessage)
{
    setPlayerState(false);
    inputController_.showMessage(
        externalPlayerError(), QtUtilities::toQString(
            MediaPlayer::playerName() + ": " + std::move(errorMessage)));
}

bool PlaybackComponent::criticalContinuePlaybackQuestion(
    const QString & title, const QString & errorMessage)
{
    setPlayerState(false);
    const auto selectedButton =
        inputController_.showMessage(
            title, errorMessage + tr("\n\tContinue playback?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return selectedButton == QMessageBox::Yes;
}

void PlaybackComponent::playFromHistory(const std::string entry)
{
    if (! mediaPlayer_.start(entry))
        return;
    resetLastPlayedItem();
    setPlayerState(true);
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

void PlaybackComponent::setPlayerState(const bool isRunning)
{
    if (isPlayerRunning_ != isRunning) {
        isPlayerRunning_ = isRunning;
        actions_.play->setEnabled(! isRunning);
        emit playerStateChanged(isRunning);
    }
}

void PlaybackComponent::checkHistoryWidgetChanges()
{
    if (historyWidget_.isHistoryChangedBySettingCurrentEntry())
        onHistoryChanged();
}

void PlaybackComponent::saveHistory()
{
    if (! isHistorySaved_) {
        while (! historyWidget_.save(historyFilename_)) {
            const auto selectedButton =
                inputController_.showMessage(
                    historyWindowName(), tr("Saving history failed."),
                    QMessageBox::Retry | QMessageBox::Ignore,
                    QMessageBox::Ignore);
            if (selectedButton != QMessageBox::Retry)
                return;
        }
        isHistorySaved_ = true;
    }
}


void PlaybackComponent::onHistoryChanged()
{
    isHistorySaved_ = false;
    if (saveHistoryToDiskImmediately_)
        saveHistory();
}

void PlaybackComponent::playbackPlay()
{
    if (mediaPlayer_.start())
        setPlayerState(true);
}

void PlaybackComponent::playbackStop()
{
    mediaPlayer_.quit();
    setPlayerState(false);
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
    MediaPlayer::setPlayerWindowVisible(true);
}

void PlaybackComponent::hideExternalPlayerWindow()
{
    MediaPlayer::setPlayerWindowVisible(false);
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
