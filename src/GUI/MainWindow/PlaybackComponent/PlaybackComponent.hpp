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

# include "HistoryWidget.hpp"
# include "CommonTypes.hpp"
# include "Actions.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtGlobal>
# include <QObject>

# include <string>
# include <memory>


class Preferences;
namespace QtUtilities
{
namespace Widgets
{
class InputController;
}
}
QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QStringList)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QMainWindow)

/// WARNING: each method can block execution if not stated otherwise.
class PlaybackComponent : public QObject
{
    Q_OBJECT
public:
    /// @param cancelled Is set to true if user has cancelled launching
    /// application (because of error); is set to false otherwise.
    /// NOTE: mainWindow, actions, inputController and preferences must remain
    /// valid throughout this PlaybackComponent's lifetime.
    explicit PlaybackComponent(
        QMainWindow & mainWindow, const Actions::Playback & actions,
        QtUtilities::Widgets::InputController & inputController,
        const Preferences & preferences, const std::string & preferencesDir,
        bool & cancelled);
    /// NOTE: does not block execution.
    ~PlaybackComponent();

    void setPreferences(const Preferences &);

    /// NOTE: does not block execution.
    MediaPlayer::Status status() const noexcept { return status_; }
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
    /// status is always equal to status().
    /// WARNING: signal receiver must not block execution.
    void statusChanged(MediaPlayer::Status status);

private:
    using Status = MediaPlayer::Status;

    /// @brief Applies everything except history settings from preferences.
    /// @param cancelled Makes a difference only in case of error.
    /// If not nullptr, cancelling operation is allowed.
    /// *cancelled is set to true if user cancels the operation; is set to false
    /// otherwise (success or ignoring error).
    void setPreferencesExceptHistory(const Preferences & preferences,
                                     bool * cancelled = nullptr);

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

    /// @brief If status_ != status, sets status_ to status and performs other
    /// accompanying actions.
    /// NOTE: does not block execution.
    void setStatus(Status status);

    /// @brief Should be called after successful setting HistoryWidget current
    /// entry via previous(), current() or next(), when ready to block
    /// execution.
    void checkHistoryWidgetChanges();

    /// @brief Saves history if (! isHistorySaved_).
    /// @param noBlocking If true, function does not block execution; otherwise
    /// it blocks in case of error.
    void saveHistory(bool noBlocking = false);

    void timerEvent(QTimerEvent *) override;


    QMainWindow & mainWindow_;
    const Actions::Playback & actions_;
    QtUtilities::Widgets::InputController & inputController_;
    const std::string historyFilename_;

    int statusUpdateInterval_ = 0;
    bool saveHistoryToDiskImmediately_;
    bool desktopNotifications_;
    Status status_ = Status::stopped;
    int timerIdentifier_ = 0;
    bool isHistorySaved_ = true;

    unsigned playerId_;
    std::unique_ptr<MediaPlayer> mediaPlayer_;

    std::unique_ptr<QLabel> lastPlayedItemLabel_;
    HistoryWidget historyWidget_;

private slots:
    void onPlayerFinished(bool crashExit, int exitCode, QStringList errors,
                          QStringList missingFilesAndDirs);
    void onPlayerError(QString errorMessage);

    /// @brief Checks external player playback status and calls setStatus().
    void updateStatus();

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

    void showExternalPlayerWindow();
    void hideExternalPlayerWindow();

    void importHistory();
    void exportHistory();
};

# endif // VENTUROUS_PLAYBACK_COMPONENT_HPP
