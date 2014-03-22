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

# include "Preferences.hpp"

# include <VenturousCore/ItemTree-inl.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QAction>
# include <QMessageBox>
# include <QDockWidget>
# include <QLabel>
# include <QStatusBar>
# include <QMainWindow>

# include <utility>
# include <functional>
# include <vector>
# include <string>


namespace
{
QString historyWindowName() { return QObject::tr("History"); }

/// @brief Shows error message and asks user if playback should be continued.
/// @param parent Is used as an owner of message box.
/// @param title Title of the message box.
/// @param errorMessage Message to be displayed before question.
/// @return true if playback should be continued.
bool criticalContinuePlaybackQuestion(
    QWidget * parent, const QString & title, const QString & errorMessage)
{
    const auto selectedButton =
        QMessageBox::critical(
            parent, title, errorMessage + QObject::tr("\n\tContinue playback?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    return selectedButton == QMessageBox::Yes;
}

}



PlaybackComponent::PlaybackComponent(
    QMainWindow & mainWindow, const ItemTree::Tree & tree,
    const Actions::Playback & actions,
    const Preferences::Playback & preferences,
    const std::string & preferencesDir)
    : mainWindow_(mainWindow), tree_(tree), actions_(actions),
      historyFilename_(preferencesDir + "history"),
      lastPlayedItemLabel_(new QLabel(mainWindow.statusBar())),
      historyWidget_(
          std::bind(& PlaybackComponent::play, this, std::placeholders::_1),
          preferences.history)
{
    setPreferencesExceptHistory(preferences);
    {
        using namespace std::placeholders;
        mediaPlayer_.setFinishedSlot(
            std::bind(& PlaybackComponent::onPlayerFinished, this, _1, _2, _3));
    }

    mainWindow.statusBar()->addWidget(lastPlayedItemLabel_);

    // Do nothing in case of failure, because history is not very important
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
    connect(actions.next, SIGNAL(triggered(bool)), SLOT(playbackNext()));
    connect(actions.playAll, SIGNAL(triggered(bool)), SLOT(onPlayAll()));

    playedItemChanged(historyWidget_.current());

    typedef Preferences::Playback::StartupPolicy StartupPolicy;
    switch (preferences.startupPolicy) {
        case StartupPolicy::playbackNext:
            playbackNext();
            break;
        case StartupPolicy::playbackPlay:
            playbackPlay();
            break;
        case StartupPolicy::doNothing:
            break;
    }
}

void PlaybackComponent::setPreferences(
    const Preferences::Playback & preferences)
{
    historyWidget_.setPreferences(preferences.history);
    setPreferencesExceptHistory(preferences);
}

void PlaybackComponent::play(const PlaybackComponent::ItemCollection & items)
{
    mediaPlayer_.start(items);
    setPlayerState(true);
    playedItemChanged();
}

void PlaybackComponent::quit()
{
    while (! historyWidget_.save(historyFilename_)) {
        const auto selectedButton =
            QMessageBox::critical(& mainWindow_, historyWindowName(),
                                  tr("Saving history failed."),
                                  QMessageBox::Retry | QMessageBox::Cancel,
                                  QMessageBox::Cancel);
        if (selectedButton == QMessageBox::Cancel)
            break;
    }
}

void PlaybackComponent::quitWithoutBlocking()
{
    historyWidget_.save(historyFilename_);
}


void PlaybackComponent::onItemActivated(const QString absolutePath)
{
    const std::string item = QtUtilities::qStringToString(absolutePath);
    mediaPlayer_.start(item);
    setPlayerState(true);
    playedItemChanged(item);
}



void PlaybackComponent::setPreferencesExceptHistory(
    const Preferences::Playback & preferences)
{
    mediaPlayer_.setExternalPlayerTimeout(preferences.externalPlayerTimeout);
    mediaPlayer_.setAutoSetOptions(preferences.autoSetExternalPlayerOptions);
    nextFromHistory_ = preferences.nextFromHistory;
}

void PlaybackComponent::onPlayerFinished(
    const bool crashExit, const int exitCode,
    const std::vector<std::string> missingFilesAndDirs)
{
    if (crashExit &&
            ! criticalContinuePlaybackQuestion(
                & mainWindow_, tr("External player error"),
                tr("%1 crashed with exit code %2.")
                .arg(QtUtilities::toQString(MediaPlayer::playerName()))
                .arg(exitCode))) {
        setPlayerState(false);
        return;
    }

    if (! missingFilesAndDirs.empty()) {
        QString message = tr("No such files or directories (%1):")
                          .arg(missingFilesAndDirs.size());
        for (const std::string & s : missingFilesAndDirs)
            message += '\n' + QtUtilities::toQString(s) + ';';
        message[message.size() - 1] = '.';

        if (! criticalContinuePlaybackQuestion(
                    & mainWindow_, tr("Missing files or directories"),
                    message)) {
            setPlayerState(false);
            return;
        }
    }

    playbackNext();
}

void PlaybackComponent::playedItemChanged(const std::string & item)
{
    QString textPrefix = tr("Last played item: ");
    if (item.empty()) {
        historyWidget_.playedMultipleItems();
        lastPlayedItemLabel_->setText(std::move(textPrefix) + tr("<unknown>"));
        lastPlayedItemLabel_->setToolTip(tr("Unknown item(s)"));
    }
    else {
        historyWidget_.push(item);
        const QString qItem = QtUtilities::toQString(item);
        lastPlayedItemLabel_->setText(std::move(textPrefix) + qItem);
        lastPlayedItemLabel_->setToolTip(qItem);
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

void PlaybackComponent::playbackNext()
{
    if (tree_.itemCount() > 0) {
        const std::string item = randomItemChooser_.randomPath(tree_);
        mediaPlayer_.start(item);
        setPlayerState(true);
        playedItemChanged(item);
    }
    else
        playbackStop();
}

void PlaybackComponent::onPlayAll()
{
    play(tree_.getAllItems<std::vector<std::string>>());
}

