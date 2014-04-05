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

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QAction>
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

}



PlaybackComponent::PlaybackComponent(
    QMainWindow & mainWindow, const Actions::Playback & actions,
    InputController & inputController,
    const Preferences::Playback & preferences,
    const std::string & preferencesDir)
    : actions_(actions), inputController_(inputController),
      historyFilename_(preferencesDir + "history"),
      historyWidget_(
          std::bind(& PlaybackComponent::playFromHistory, this,
                    std::placeholders::_1),
    [this](CommonTypes::ItemCollection items) { play(std::move(items)); },
preferences.history)
{
    setPreferencesExceptHistory(preferences);
    {
        using namespace std::placeholders;
        mediaPlayer_.setFinishedSlot(
            std::bind(& PlaybackComponent::onPlayerFinished, this, _1, _2, _3));
    }

    mainWindow.statusBar()->addWidget(& lastPlayedItemLabel_);

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

    connect(actions.importHistory, SIGNAL(triggered(bool)),
            SLOT(importHistory()));
    connect(actions.exportHistory, SIGNAL(triggered(bool)),
            SLOT(exportHistory()));
    historyWidget_.connect(actions.clearHistory, SIGNAL(triggered(bool)),
                           SLOT(clearHistory()));

    connect(& historyWidget_, SIGNAL(historyChanged()),
            SLOT(onHistoryChanged()));

    currentHistoryEntryChanged();
}

PlaybackComponent::~PlaybackComponent()
{
    if (! isHistorySaved_) {
        if (! historyWidget_.save(historyFilename_))
            std::cerr << ERROR_PREFIX "Saving history failed." << std::endl;
    }
}

void PlaybackComponent::setPreferences(
    const Preferences::Playback & preferences)
{
    historyWidget_.setPreferences(preferences.history);
    setPreferencesExceptHistory(preferences);
}

void PlaybackComponent::play(std::string item)
{
    mediaPlayer_.start(item);
    historyWidget_.push(std::move(item));
    onHistoryChanged();
    currentHistoryEntryChanged();
    setPlayerState(true);
}

void PlaybackComponent::play(CommonTypes::ItemCollection items)
{
    if (items.size() == 1)
        play(std::move(items.back()));
    else {
        mediaPlayer_.start(std::move(items));
        historyWidget_.playedMultipleItems();
        currentHistoryEntryChanged();
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


void PlaybackComponent::onItemActivated(const QString absolutePath)
{
    play(QtUtilities::qStringToString(absolutePath));
}



void PlaybackComponent::setPreferencesExceptHistory(
    const Preferences::Playback & preferences)
{
    mediaPlayer_.setAutoSetOptions(preferences.autoSetExternalPlayerOptions);
    saveHistoryToDiskImmediately_ = preferences.history.saveToDiskImmediately;
    if (saveHistoryToDiskImmediately_)
        saveHistory();
}

void PlaybackComponent::onPlayerFinished(
    const bool crashExit, const int exitCode,
    std::vector<std::string> missingFilesAndDirs)
{
    if (crashExit) {
        QString message =
            tr("%1 crashed with exit code %2.")
            .arg(QtUtilities::toQString(MediaPlayer::playerName()))
            .arg(exitCode);
        if (! criticalContinuePlaybackQuestion(
        tr("External player error"), std::move(message))) {
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

bool PlaybackComponent::criticalContinuePlaybackQuestion(
    const QString & title, const QString & errorMessage)
{
    setPlayerState(false);
    const auto selectedButton =
        inputController_.showMessage(
            title, errorMessage + QObject::tr("\n\tContinue playback?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return selectedButton == QMessageBox::Yes;
}

void PlaybackComponent::playFromHistory(const std::string entry)
{
    mediaPlayer_.start(entry);
    currentHistoryEntryChanged();
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

void PlaybackComponent::currentHistoryEntryChanged()
{
    QString textPrefix = tr("Last played item: ");
    const QString entry = historyWidget_.currentAbsolute();
    if (entry.isEmpty()) {
        lastPlayedItemLabel_.setText(std::move(textPrefix) + tr("<unknown>"));
        lastPlayedItemLabel_.setToolTip(
            tr("Unknown item(s).\n"
               "This means that multiple items were played or that last played "
               "item was removed from history."));
    }
    else {
        lastPlayedItemLabel_.setText(std::move(textPrefix) +
                                     historyWidget_.currentShortened());
        lastPlayedItemLabel_.setToolTip(entry);
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
    mediaPlayer_.start();
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
