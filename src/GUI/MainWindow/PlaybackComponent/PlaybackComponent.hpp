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
# include "Preferences.hpp"

# include <VenturousCore/ItemTree.hpp>
# include <VenturousCore/MediaPlayer.hpp>

# include <QtGlobal>
# include <QString>
# include <QObject>

# include <vector>
# include <string>


class InputController;
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QMainWindow)

class PlaybackComponent : public QObject
{
    Q_OBJECT
public:
    /// NOTE: mainWindow, tree, actions and inputController must remain valid
    /// throughout this PlaybackComponent's lifetime.
    explicit PlaybackComponent(QMainWindow & mainWindow,
                               const ItemTree::Tree & tree,
                               const Actions::Playback & actions,
                               InputController & inputController,
                               const Preferences::Playback &,
                               const std::string & preferencesDir);

    ~PlaybackComponent();

    void setPreferences(const Preferences::Playback &);

    bool isPlayerRunning() const { return isPlayerRunning_; }

    int currentHistoryEntryIndex() const {
        return historyWidget_.currentEntryIndex();
    }

    /// @brief Starts playing item and pushes it to history.
    void play(std::string item);

    /// @brief Starts playing items and adjusts history.
    void play(CommonTypes::ItemCollection items);

    /// @brief Should be called before normal quit. Can block execution.
    void quit();

public slots:
    /// TODO: remove after allowing multiple items selection in TreeWidget.
    void onItemActivated(QString absolutePath);

signals:
    /// isPlayerRunning is always equal to isPlayerRunning().
    void playerStateChanged(bool isPlayerRunning);

private:
    void setPreferencesExceptHistory(const Preferences::Playback &);

    void onPlayerFinished(bool crashExit, int exitCode,
                          std::vector<std::string> missingFilesAndDirs);

    /// @brief Shows error message and asks user if playback should be
    /// continued.
    /// @param title Title of the message box.
    /// @param errorMessage Message to be displayed before question.
    /// @return true if playback should be continued.
    /// WARNING: if this method returns false, return from calling method
    /// immediately, because this object could be already destroyed.
    bool criticalContinuePlaybackQuestion(
        const QString & title, const QString & errorMessage);

    /// @brief Starts playing entry. Does not call historyWidget_.push().
    void playFromHistory(std::string entry);

    /// @brief Must be called after current history entry is changed.
    void currentHistoryEntryChanged();

    /// @brief If isPlayerRunning_ != isRunning, sets isPlayerRunning_ to
    /// isRunning and performs other accompanying actions.
    void setPlayerState(bool isRunning);

    /// @brief Saves history if (! isHistorySaved_). Can block execution.
    void saveHistory();

    /// @brief Pushes item to history; handles saveHistoryToDiskImmediately_
    /// and isHistorySaved_.
    void pushToHistory(std::string item);


    QMainWindow & mainWindow_;
    const ItemTree::Tree & tree_;
    const Actions::Playback & actions_;
    InputController & inputController_;
    const std::string historyFilename_;

    bool saveHistoryToDiskImmediately_;
    bool nextFromHistory_;
    bool isPlayerRunning_ = false;
    bool isHistorySaved_ = true;

    ItemTree::RandomItemChooser randomItemChooser_;
    MediaPlayer mediaPlayer_;

    QLabel * const lastPlayedItemLabel_;
    HistoryWidget historyWidget_;

private slots:
    void playbackPlay();
    void playbackStop();
    void playbackNext();
    void onPlayAll();
};

# endif // VENTUROUS_PLAYBACK_COMPONENT_HPP
