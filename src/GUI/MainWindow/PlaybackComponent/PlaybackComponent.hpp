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

# ifndef VENTUROUS_PLAYBACK_COMPONENT_HPP
# define VENTUROUS_PLAYBACK_COMPONENT_HPP

# include "CommonTypes.hpp"
# include "HistoryWidget.hpp"
# include "Actions.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtGlobal>
# include <QObject>

# include <vector>
# include <string>
# include <memory>


class InputController;
class Preferences;
QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QMainWindow)

/// WARNING: each method can block execution if not stated otherwise.
class PlaybackComponent : public QObject
{
    Q_OBJECT
public:
    /// NOTE: mainWindow, actions and inputController must remain valid
    /// throughout this PlaybackComponent's lifetime.
    /// NOTE: does not block execution.
    explicit PlaybackComponent(QMainWindow & mainWindow,
                               const Actions::Playback & actions,
                               InputController & inputController,
                               const Preferences &,
                               const std::string & preferencesDir);
    /// NOTE: does not block execution.
    ~PlaybackComponent();

    void setPreferences(const Preferences &);

    /// NOTE: does not block execution.
    bool isPlayerRunning() const { return isPlayerRunning_; }
    /// NOTE: does not block execution.
    int currentHistoryEntryIndex() const {
        return historyWidget_.currentEntryIndex();
    }

    /// @brief Starts playing item and pushes it to history.
    void play(std::string item);
    /// @brief Starts playing items and adjusts history.
    void play(CommonTypes::ItemCollection items);

    /// @brief If there is next item in history, starts playing it and returns
    /// true. Otherwise, does not block execution and returns false.
    bool playNextFromHistory();

    /// @brief Should be called before normal quit.
    void quit();

signals:
    /// isPlayerRunning is always equal to isPlayerRunning().
    /// WARNING: signal receiver must not block execution.
    void playerStateChanged(bool isPlayerRunning);

private:
    void setPreferencesExceptHistory(const Preferences &);

    void onPlayerFinished(bool crashExit, int exitCode,
                          std::vector<std::string> missingFilesAndDirs);
    void onPlayerError(std::string errorMessage);

    /// @brief Shows error message and asks user if playback should be
    /// continued.
    /// @param title Title of the message box.
    /// @param errorMessage Message to be displayed before question.
    /// @return true if playback should be continued.
    bool criticalContinuePlaybackQuestion(
        const QString & title, const QString & errorMessage);

    /// @brief Starts playing entry. Does not call historyWidget_.push().
    /// NOTE: does not block execution.
    void playFromHistory(std::string entry);

    /// @brief If entry.empty(), does not block execution and returns false.
    /// Otherwise, plays entry from history, calls checkHistoryWidgetChanges()
    /// and returns true.
    bool playFromHistoryIfNotEmpty(std::string entry);

    /// @brief Must be called after current history entry is changed.
    /// @param playbackStarted true if method is called as a consequence of
    /// playing item(s), false otherwise.
    /// NOTE: does not block execution.
    void resetLastPlayedItem(bool playbackStarted = true);

    /// @brief If isPlayerRunning_ != isRunning, sets isPlayerRunning_ to
    /// isRunning and performs other accompanying actions.
    /// NOTE: does not block execution.
    void setPlayerState(bool isRunning);

    /// @brief Should be called after successful setting HistoryWidget current
    /// entry via previous(), current() or next(), when ready to block
    /// execution.
    void checkHistoryWidgetChanges();

    /// @brief Saves history if (! isHistorySaved_).
    void saveHistory();


    QMainWindow & mainWindow_;
    const Actions::Playback & actions_;
    InputController & inputController_;
    const std::string historyFilename_;

    bool saveHistoryToDiskImmediately_;
    bool desktopNotifications_;
    bool isPlayerRunning_ = false;
    bool isHistorySaved_ = true;

    MediaPlayer mediaPlayer_;

    std::unique_ptr<QLabel> lastPlayedItemLabel_;
    HistoryWidget historyWidget_;

private slots:
    /// @brief Sets isHistorySaved_ to false, handles
    /// saveHistoryToDiskImmediately_.
    void onHistoryChanged();

    /// NOTE: does not block execution.
    void playbackPlay();
    /// NOTE: does not block execution.
    void playbackStop();
    void playbackPrevious();
    void playbackReplayLast();
    void playbackNextFromHistory();

    void importHistory();
    void exportHistory();
};

# endif // VENTUROUS_PLAYBACK_COMPONENT_HPP
