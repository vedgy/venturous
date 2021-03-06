/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015 Igor Kushnir <igorkuo AT Google mail>

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

# include <VenturousCore/MediaPlayer.hpp>

# include <QtWidgetsUtilities/WindowInputController.hpp>

# include <QtGlobal>
# include <QMenuBar>
# include <QToolBar>
# include <QMainWindow>
# include <QSystemTrayIcon>

# include <memory>


class PlaylistComponent;
class PlaybackComponent;
class PreferencesComponent;
struct Actions;
QT_FORWARD_DECLARE_CLASS(QSharedMemory)
QT_FORWARD_DECLARE_CLASS(QSessionManager)

/// WARNING: each method can block execution if not stated otherwise.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /// @param sharedMemory Must be valid and attached to.
    /// @param cancelled Is set to true if user has cancelled launching
    /// application (because of error); is set to false otherwise.
    explicit MainWindow(
        std::unique_ptr<QSharedMemory> sharedMemory, bool & cancelled,
        QWidget * parent = nullptr, Qt::WindowFlags flags = 0);
    /// NOTE: does not block execution.
    ~MainWindow();

private:
    enum class CommitDataState : unsigned char { none, commited, cancelled };

    /// @return Path to Venturous preferences.
    static QString getPreferencesDirName();

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

    /// @brief Should be called before normal quit.
    /// @return true if quit is allowed, false if user cancelled it.
    bool quit();

    void timerEvent(QTimerEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void closeEvent(QCloseEvent *) override;


    /// Is used to receive Ventool commands and ensure single Venturous
    /// instance.
    std::unique_ptr<QSharedMemory> sharedMemory_;

    std::unique_ptr<Actions> actions_;
    QMenuBar menuBar_;
    QToolBar toolBar_;

    /// Blocks/unblocks most of non-GUI user input (Ventool commands for
    /// example).
    /// Reacting to QCoreApplication::aboutToQuit and
    /// QApplication::commitDataRequest signals is allowed.
    /// Reacting to MediaPayer::FinishedSlot is also allowed.
    QtUtilities::Widgets::WindowInputController inputController_;

    std::unique_ptr<PreferencesComponent> preferencesComponent_;
    std::unique_ptr<PlaybackComponent> playbackComponent_;
    std::unique_ptr<PlaylistComponent> playlistComponent_;

    std::unique_ptr<QSystemTrayIcon> notificationAreaIcon_;

    int ventoolCheckInterval_ = 0;
    int timerIdentifier_ = 0;
    CommitDataState commitDataState_ = CommitDataState::none;

private slots:
    /// NOTE: does not block execution.
    void copyInternalOptionsToPreferences();
    void onPreferencesChanged();
    /// @brief Sets appropriate window title.
    /// Must be called after playbackComponent_->status() or
    /// playlistComponent_->itemCount()
    /// or playlistComponent_->editMode() change.
    /// NOTE: does not block execution.
    void setWindowTitle();
    /// NOTE: does not block execution.
    void onPlayerStatusChanged(MediaPlayer::Status newStatus);
    /// NOTE: does not block execution.
    void onNotificationAreaIconActivated(QSystemTrayIcon::ActivationReason);

    /// NOTE: does not block execution.
    void onFilePreferences();
    void onFilePreferencesDirectory();
    void onFileQuit();
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
    void resetCommitDataState();

    void onCommitDataRequest(QSessionManager & manager);

    /// NOTE: does not block execution.
    void onAboutToQuit();
};

# endif // VENTUROUS_MAIN_WINDOW_HPP
