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
# include "Actions.hpp"
# include "Preferences.hpp"

# include <VenturousCore/ItemTree.hpp>
# include <VenturousCore/MediaPlayer.hpp>

# include <QtGlobal>
# include <QString>
# include <QObject>

# include <vector>
# include <string>


QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QMainWindow)

class PlaybackComponent : public QObject
{
    Q_OBJECT
public:
    typedef std::vector<std::string> ItemCollection;

    /// NOTE: mainWindow, tree and actions must remain valid
    /// throughout this PlaybackComponent's lifetime.
    explicit PlaybackComponent(QMainWindow & mainWindow,
                               const ItemTree::Tree & tree,
                               const Actions::Playback & actions,
                               const Preferences::Playback &,
                               const std::string & preferencesDir);

    void setPreferences(const Preferences::Playback &);

    bool isPlayerRunning() const { return isPlayerRunning_; }

    void play(const ItemCollection &);

    /// @brief Should be called before normal quit. Can block execution.
    void quit();

    /// @brief Should be called before urgent quit. Does not block execution.
    void quitWithoutBlocking();

public slots:
    /// TODO: remove after allowing multiple items selection in TreeWidget.
    void onItemActivated(QString absolutePath);

signals:
    /// isRunning is always equal to isPlayerRunning().
    void playerStateChanged(bool isRunning);

private:
    void setPreferencesExceptHistory(const Preferences::Playback &);

    void onPlayerFinished(bool crashExit, int exitCode,
                          std::vector<std::string> missingFilesAndDirs);

    /// @brief Must be called after played item is changed.
    /// @param item Absolute path to played item. Must be passed if and only if
    /// exactly one item is played.
    void playedItemChanged(const std::string & item = std::string());

    /// @brief If isPlayerRunning_ != isRunning, sets isPlayerRunning_ to
    /// isRunning and emits playerStateChanged(isRunning).
    void setPlayerState(bool isRunning);

    QMainWindow & mainWindow_;
    const ItemTree::Tree & tree_;
    const Actions::Playback & actions_;
    const std::string historyFilename_;

    /// TODO: use.
    bool nextFromHistory_;
    bool isPlayerRunning_ = false;

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
