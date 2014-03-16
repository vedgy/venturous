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

# include "TreeWidget.hpp"
# include "Preferences.hpp"

# include <VenturousCore/ItemTree.hpp>
# include <VenturousCore/MediaPlayer.hpp>

# include <QtGlobal>
# include <QString>
# include <QMainWindow>
# include <QMenuBar>
# include <QToolBar>
# include <QSystemTrayIcon>

# include <string>
# include <stdexcept>
# include <memory>


class Actions;
class PreferencesWindow;
QT_FORWARD_DECLARE_CLASS(QSharedMemory)
QT_FORWARD_DECLARE_CLASS(QLabel)

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string & sWhat) : std::runtime_error(sWhat) {}
    };

    static const std::string itemsFilename;

    explicit MainWindow(std::unique_ptr<QSharedMemory> sharedMemory,
                        QWidget * parent = nullptr, Qt::WindowFlags flags = 0);

    ~MainWindow() override;

private:
    static const QString preferencesFilename;
    static const QString saveErrorPrefix;

    void initPreferences();
    void initTree();
    void connectSlots();

    /// @brief Shows QMessageBox with given errorMessage.
    /// @param suffix If it is not empty, ('\n' + suffix) is appended to default
    /// error message.
    void showLoadingPlaylistErrorMessage(const std::string & errorMessage,
                                         const QString & suffix = QString());

    /// Must be called after itemTree_.itemCount() or isPlayerRunning_ change.
    void setWindowTitle();

    void onPlayerFinished(bool crashExit, int exitCode,
                          std::vector<std::string> missingFilesAndDirs);

    void showNotificationAreaIcon();
    void hideNotificationAreaIcon();

    /// @brief Unhides, unminimizes and activates window.
    void showWindowProperly();
    /// @brief If notification area icon is enabled, hides window; otherwise,
    /// minimizes window.
    void hideWindowProperly();

    /// @brief Must be called after player.isRunning() changes.
    void playerStateChanged(bool isRunning);

    /// Must be called after switching edit mode.
    void updateActionsState();
    /// WARNING: call this method only in edit mode.
    /// @return (* temporaryTree_ == itemTree_).
    bool noChanges() const;
    void enterEditMode();
    /// Use this method if noChanges() was just checked and equal to false.
    /// @return true if changes were saved successfully.
    bool applyChangesChanged();
    /// Use this method if noChanges() was just checked.
    /// @param noChanges Set this parameter to noChanges().
    void cancelChanges(bool noChanges);
    bool enterAskEditMode();
    bool leaveAskEditMode();
    bool leaveAskChangedEditMode();
    bool leaveAskUnchangedEditMode();
    bool ensureAskInEditMode();
    bool ensureAskOutOfEditMode();

    /// @brief Saves temporaryTree_ to itemsFilename.
    /// @return true on success, false on failure (error).
    bool saveTemporaryTree() const;


    /// @brief Calls ensureAskInEditMode(). If edit mode is set, calls
    /// filenameGetter() and if acquired filename is not empty, tries to load
    /// temporaryTree_ from it.
    /// @tparam FilenameGetter Must be a callable object,
    /// which returns [const] QString [&[&]].
    template <typename FilenameGetter>
    void loadTemporaryPlaylist(FilenameGetter filenameGetter);

    /// @brief Calls callable object f, catches and handles QtUtilities::Error.
    /// @return true on success, false on failure (error).
    template <typename F>
    bool handlePreferencesErrors(F f, const QString & errorPrefix);


    /// @brief Must be called after preferences_ are changed.
    void preferencesChanged();

    /// @brief Must be called after played item is changed.
    /// @param playedItem Must satisfy Preferences::PlayedItem requirements.
    /// @param itemAbsolutePath Path to played item. Must be passed
    /// if and only if exactly one item is played.
    void playedItemChanged(int playedItem,
                           const QString & itemAbsolutePath = QString());

    void copyWindowGeometryAndStateToPreferences();

    void timerEvent(QTimerEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void closeEvent(QCloseEvent *) override;

    /// Is used to receive Ventool commands and ensure single Venturous
    /// instance.
    std::unique_ptr<QSharedMemory> sharedMemory_;

    ItemTree::Tree itemTree_;
    std::unique_ptr<ItemTree::Tree> temporaryTree_;
    ItemTree::RandomItemChooser randomItemChooser_;
    MediaPlayer mediaPlayer_;

    std::unique_ptr<Actions> actions_;
    QMenuBar menuBar_;
    QToolBar toolBar_;
    TreeWidget treeWidget_;
    QLabel * const lastPlayedItemLabel;

    Preferences savedPreferences_, preferences_;
    PreferencesWindow * preferencesWindow_;
    QSystemTrayIcon * notificationAreaIcon_ = nullptr;

    bool isPlayerRunning_ = false;
    int ventoolCheckInterval_ = 0;
    int timerIdentifier_ = 0;
    bool isPreferencesWindowOpen_ = false;
    bool quitState_ = false;

private slots:
    void onNotificationAreaIconActivated(QSystemTrayIcon::ActivationReason);

    void onPreferencesUpdated();
    void onPreferencesActivated();
    void onFileQuit();

    void onItemActivated(QString absolutePath);
    void playbackPlay();
    void playbackStop();
    void playbackNext();
    void onPlayAll();

    void onEditModeStateChanged();
    void applyChanges();
    void cancelChanges();

    void onAddFiles();
    void onAddDirectory();
    void onCleanUp();
    void onClear();
    void onRestorePrevious();

    void onLoad();
    void onSaveAs();

    void onHelpHelp();
    void onHelpAbout();

    void onAudioFileStateChanged();
    void onMediaDirStateChanged();
    void onBothAudioFileStateChanged();
    void onBothMediaDirStateChanged();

    void onAboutToQuit();
};

# endif // VENTUROUS_MAIN_WINDOW_HPP
