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

# ifndef VENTUROUS_MAIN_WINDOW_HPP
# define VENTUROUS_MAIN_WINDOW_HPP

# include "WindowUtilities.hpp"

# include <QtGlobal>
# include <QMenuBar>
# include <QToolBar>
# include <QMainWindow>
# include <QSystemTrayIcon>

# include <memory>


class PlaylistComponent;
class PlaybackComponent;
class PreferencesComponent;
class Actions;
QT_FORWARD_DECLARE_CLASS(QSharedMemory)

/// WARNING: each method can block execution if not stated otherwise.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /// @param sharedMemory Must be valid and attached to.
    explicit MainWindow(std::unique_ptr<QSharedMemory> sharedMemory,
                        QWidget * parent = nullptr, Qt::WindowFlags flags = 0);
    /// NOTE: does not block execution.
    ~MainWindow();

private:
    /// @brief Updates state according to preferencesComponent_->preferences
    /// but does not set preferences to components.
    /// NOTE: does not block execution.
    void setPreferencesNoComponents();

    /// @brief Unhides, unminimizes and activates window.
    /// NOTE: does not block execution.
    void showWindowProperly();
    /// @brief If notification area icon is enabled, hides window; otherwise,
    /// minimizes window.
    /// NOTE: does not block execution.
    void hideWindowProperly();

    /// NOTE: does not block execution.
    void showNotificationAreaIcon();
    /// NOTE: does not block execution.
    void hideNotificationAreaIcon();

    void timerEvent(QTimerEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void closeEvent(QCloseEvent *) override;

    /// Is used to receive Ventool commands and ensure single Venturous
    /// instance.
    std::unique_ptr<QSharedMemory> sharedMemory_;

    std::unique_ptr<Actions> actions_;
    QMenuBar menuBar_;
    QToolBar toolBar_;

    WindowInputController inputController_;
    std::unique_ptr<PreferencesComponent> preferencesComponent_;
    std::unique_ptr<PlaybackComponent> playbackComponent_;
    std::unique_ptr<PlaylistComponent> playlistComponent_;

    std::unique_ptr<QSystemTrayIcon> notificationAreaIcon_;

    int ventoolCheckInterval_ = 0;
    int timerIdentifier_ = 0;
    bool quitState_ = false;

private slots:
    /// NOTE: does not block execution.
    void copyInternalOptionsToPreferences();
    void onPreferencesChanged();
    /// @brief Sets appropriate window title.\n
    /// Must be called after playbackComponent_->isPlayerRunning() or
    /// playlistComponent_->itemCount()
    /// or playlistComponent_->editMode() change.
    /// NOTE: does not block execution.
    void setWindowTitle();
    /// NOTE: does not block execution.
    void onPlayerStateChanged(bool isPlayerRunning);
    /// NOTE: does not block execution.
    void onNotificationAreaIconActivated(QSystemTrayIcon::ActivationReason);

    void onFileQuit();
    /// NOTE: does not block execution.
    void onFilePreferences();
    void onPlaybackNext();
    void onHelpHelp();
    void onHelpAbout();

    /// NOTE: does not block execution.
    void onAudioFileStateChanged();
    /// NOTE: does not block execution.
    void onMediaDirStateChanged();
    /// NOTE: does not block execution.
    void onBothAudioFileStateChanged();
    /// NOTE: does not block execution.
    void onBothMediaDirStateChanged();

    /// NOTE: does not block execution.
    void onAboutToQuit();
};

# endif // VENTUROUS_MAIN_WINDOW_HPP
