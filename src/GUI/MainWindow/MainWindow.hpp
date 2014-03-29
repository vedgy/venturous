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
# include "Preferences.hpp"

# include <QtCoreUtilities/Error.hpp>
# include <QtCoreUtilities/String.hpp>

# include <QtGlobal>
# include <QString>
# include <QMessageBox>
# include <QMenuBar>
# include <QToolBar>
# include <QMainWindow>
# include <QSystemTrayIcon>

# include <memory>
# include <iostream>


class Actions;
class PreferencesWindow;
class PlaylistComponent;
class PlaybackComponent;
QT_FORWARD_DECLARE_CLASS(QSharedMemory)

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(std::unique_ptr<QSharedMemory> sharedMemory,
                        QWidget * parent = nullptr, Qt::WindowFlags flags = 0);

    ~MainWindow();

private:
    static const QString preferencesFilename;
    static const QString savePreferencesErrorPrefix;

    void initPreferences();
    void connectSlots();

    void showNotificationAreaIcon();
    void hideNotificationAreaIcon();

    /// @brief Unhides, unminimizes and activates window.
    void showWindowProperly();
    /// @brief If notification area icon is enabled, hides window; otherwise,
    /// minimizes window.
    void hideWindowProperly();

    /// @brief Calls callable object f, catches and handles QtUtilities::Error.
    /// @param errorPrefix Text, which will be displayed before
    /// QtUtilities::Error::message().
    /// @param silentMode Makes a difference only in case of error.
    /// If true, error message is printed to stderr and method returns false.
    /// Otherwise, execution is blocked and user is allowed to retry operation.
    /// @return true on success, false on failure (error).
    template <typename F>
    bool handlePreferencesErrors(F f, const QString & errorPrefix,
                                 bool silentMode = false);

    /// @brief Must be called after preferences_ are changed.
    void preferencesChanged();

    void copyWindowGeometryAndStateToPreferences();

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
    std::unique_ptr<PlaylistComponent> playlistComponent_;
    std::unique_ptr<PlaybackComponent> playbackComponent_;

    Preferences savedPreferences_, preferences_;
    PreferencesWindow * preferencesWindow_;
    QSystemTrayIcon * notificationAreaIcon_ = nullptr;

    int ventoolCheckInterval_ = 0;
    int timerIdentifier_ = 0;
    bool isPreferencesWindowOpen_ = false;
    bool quitState_ = false;

private slots:
    /// Must be called after itemTree_.itemCount() or isPlayerRunning_ change.
    void setWindowTitle();

    void onPlayerStateChanged(bool isPlayerRunning);

    void onNotificationAreaIconActivated(QSystemTrayIcon::ActivationReason);

    void onPreferencesUpdated();
    void onPreferencesActivated();
    void onFileQuit();

    void onHelpHelp();
    void onHelpAbout();

    void onAudioFileStateChanged();
    void onMediaDirStateChanged();
    void onBothAudioFileStateChanged();
    void onBothMediaDirStateChanged();

    void onAboutToQuit();
};

template <typename F>
bool MainWindow::handlePreferencesErrors(
    F f, const QString & errorPrefix, const bool silentMode)
{
    while (true) {
        try {
            f();
            return true;
        }
        catch (const QtUtilities::Error & error) {
            const QString message = errorPrefix + ": " + error.message();
            if (silentMode) {
                std::cerr << QtUtilities::qStringToString(message) << std::endl;
                return false;
            }
            const auto selectedButton =
                QMessageBox::critical(this, tr("Preferences error"), message,
                                      QMessageBox::Retry | QMessageBox::Cancel,
                                      QMessageBox::Cancel);

            if (selectedButton != QMessageBox::Retry)
                return false;
        }
    }
}

# endif // VENTUROUS_MAIN_WINDOW_HPP
