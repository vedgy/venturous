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

# ifndef VENTUROUS_PLAYLIST_COMPONENT_HPP
# define VENTUROUS_PLAYLIST_COMPONENT_HPP

# include "TreeWidget.hpp"
# include "CommonTypes.hpp"

# include <VenturousCore/ItemTree.hpp>

# include <QtGlobal>
# include <QString>
# include <QObject>

# include <functional>
# include <string>


struct Actions;
class Preferences;
namespace AddingItems
{
struct Policy;
}
namespace QtUtilities
{
namespace Widgets
{
class InputController;
}
}
QT_FORWARD_DECLARE_CLASS(QMainWindow)

/// WARNING: each method can block execution if not stated otherwise.
class PlaylistComponent : public QObject
{
    Q_OBJECT
public:
    /// @param cancelled Is set to true if user has cancelled launching
    /// application (because of error); is set to false otherwise.
    /// NOTE: mainWindow, actions, inputController and preferences must remain
    /// valid throughout this PlaylistComponent's lifetime.
    explicit PlaylistComponent(
        QMainWindow & mainWindow, const Actions & actions,
        QtUtilities::Widgets::InputController & inputController,
        const Preferences & preferences, CommonTypes::PlayItems playItems,
        const std::string & preferencesDir, bool & cancelled);
    /// NOTE: does not block execution.
    ~PlaylistComponent();
    /// NOTE: does not block execution.
    void setPreferences(const Preferences &);
    /// @return Number of playable items in playlist.
    /// NOTE: does not block execution.
    int itemCount() const { return itemTree_.itemCount(); }
    /// @return true if playlist is in edit mode, false otherwise.
    /// NOTE: does not block execution.
    bool editMode() const { return treeWidget_.editMode(); }

    /// @brief If itemTree_ is not empty, selects random item and starts
    /// playing it.
    /// @return true if playback was started, false otherwise
    bool playRandomItem();

    /// @brief Should be called before normal quit.
    /// @return true if quit is allowed, false if user cancelled it.
    bool quit();

signals:
    /// @brief Is emitted after playlist edit mode is changed.
    /// WARNING: signal receiver may not block execution.
    void editModeChanged();

private:
    using FilenameGetter = std::function<QString()>;

    /// Must be called after switching edit mode.
    /// NOTE: does not block execution.
    void updateActionsState();
    /// @return (* temporaryTree_ == itemTree_).
    /// WARNING: call this method only in edit mode.
    /// NOTE: does not block execution.
    bool noChanges() const;
    /// NOTE: does not block execution.
    void enterEditMode();

    /// Use this method if noChanges() was just checked and equal to false.
    /// @return true if changes were saved successfully.
    bool applyChangesChanged();
    /// Use this method if noChanges() was just checked.
    /// @param noChanges Set this parameter to noChanges().
    /// NOTE: does not block execution.
    void cancelChanges(bool noChanges);
    bool enterAskEditMode();
    bool leaveAskEditMode();
    bool leaveAskChangedEditMode();
    bool leaveAskUnchangedEditMode();
    bool ensureAskInEditMode();
    bool ensureAskOutOfEditMode();

    /// @brief Backs up current playlist file.
    /// @return true if backup was successful, false otherwise.
    /// NOTE: does not block execution.
    bool makeBackup() const;
    /// @brief Replaces current playlist file with backup copy.
    /// NOTE: does not block execution.
    void restoreBackup() const;

    /// @brief Saves temporaryTree_ to itemsFilename. Handles backing up.
    /// @param cancelled If equal to nullptr, function does not block.
    /// Otherwise, function blocks in case of error, *cancelled is set to true
    /// if operation was cancelled by user, to false otherwise.
    /// WARNING: call this method only in edit mode.
    void saveTemporaryTree(bool * cancelled = nullptr);

    /// @brief Calls ensureAskInEditMode(). If edit mode is set, calls
    /// filenameGetter() and if acquired filename is not empty, tries to load
    /// temporaryTree_ from it.
    void loadTemporaryPlaylist(FilenameGetter filenameGetter);


    const Actions & actions_;
    QtUtilities::Widgets::InputController & inputController_;
    const AddingItems::Policy & addingPolicy_;
    const CommonTypes::PlayItems playItems_;
    const std::string itemsFilename_;
    const QString qItemsFilename_;
    const QString qBackupItemsFilename_;

    bool treeAutoCleanup_;

    ItemTree::Tree itemTree_;
    std::unique_ptr<ItemTree::Tree> temporaryTree_;
    ItemTree::RandomItemChooser randomItemChooser_;

    TreeWidget treeWidget_;

private slots:
    void playbackNextRandom();
    void playbackPlayAll();

    void onEditModeStateChanged();
    void applyChanges();
    /// NOTE: does not block execution.
    void cancelChanges();

    void onAddFiles();
    void onAddDirectory();
    void onCleanUp();
    void onClear();
    void onRestorePrevious();

    void onLoad();
    void onSaveAs();
};

# endif // VENTUROUS_PLAYLIST_COMPONENT_HPP
