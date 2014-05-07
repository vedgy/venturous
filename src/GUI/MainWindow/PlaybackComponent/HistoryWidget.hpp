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

# ifndef VENTUROUS_HISTORY_WIDGET_HPP
# define VENTUROUS_HISTORY_WIDGET_HPP

# include "CommonTypes.hpp"
# include "GuiUtilities.hpp"
# include "CustomActions.hpp"
# include "Preferences.hpp"

# include <VenturousCore/History.hpp>

# include <QListWidget>

# include <functional>
# include <string>


class HistoryWidget : public QListWidget
{
    Q_OBJECT
public:
    typedef std::function<void(std::string)> PlayExistingEntry;

    /// @param playExistingEntry function that starts playing entry from
    /// history. It must not be pushed in history after playing.
    /// WARNING: playExistingEntry must not block execution.
    /// @param playItems Function that starts playing ItemCollection
    /// parameter. If item(s) actually get(s) played, HistoryWidget must be
    /// notified about this via push() or playedMultipleItems().
    /// NOTE: customActions must remain valid throughout this HistoryWidget's
    /// lifetime.
    explicit HistoryWidget(const CustomActions::Actions & customActions,
                           PlayExistingEntry playExistingEntry,
                           CommonTypes::PlayItems playItems,
                           const Preferences::Playback::History & preferences,
                           QWidget * parent = nullptr);

    void setPreferences(const Preferences::Playback::History & preferences);

    /// @brief Clears history and loads entries from file. Not more than
    /// maxSize() entries will be read.
    /// @return true if loading was successful.
    bool load(const std::string & filename);

    /// @brief Saves history to file.
    /// @return true if saving was successful.
    bool save(const std::string & filename) const;


    std::size_t maxSize() const { return history_.maxSize(); }
    int currentEntryIndex() const { return currentEntryIndex_; }

    std::string previous() { return setCurrentEntry(currentEntryIndex_ + 1); }
    /// @brief If currentEntryIndex_ points to existing entry, copies this entry
    /// to history top if (copyPlayedEntryToTop_ == true) and returns its path.
    /// Otherwise returns empty string.
    std::string current();
    std::string next() { return setCurrentEntry(currentEntryIndex_ - 1); }

    /// @brief Should be queried after successful setting current entry via
    /// previous(), current() or next().
    /// @return true if setting current entry changes history.
    bool isHistoryChangedBySettingCurrentEntry() const {
        return copyPlayedEntryToTop_;
    }

    /// @return Current history entry or empty string if there is no current
    /// entry.
    QString currentAbsolute() const;
    /// @return Shortened version of currentAbsolute(). This version is
    /// displayed in the list.
    QString currentShortened() const;

    /// @brief Adds entry to the history and sets currentEntryIndex_ to 0.
    void push(std::string entry);

    /// @brief Sets currentEntryIndex_ to
    /// Preferences::Playback::History::multipleItemsIndex.
    void playedMultipleItems();

public slots:
    /// WARNING: can block execution.
    void clearHistory();

signals:
    /// @brief Is emitted after history is changed via GUI (adding or manual
    /// removing items in list widget) or clearHistory() slot.
    /// NOTE: execution may be blocked by signal receiver.
    void historyChanged();

private:
    /// @return Entry path at specified index or empty string if there is no
    /// such entry.
    std::string entryAt(int index) const;
    /// @brief If there is an entry at specified index, makes it current
    /// (either by copying to top or just changing currentEntryIndex_) and
    /// returns its path, otherwise returns empty string.
    std::string setCurrentEntry(int index);

    /// @brief Emphasizes/deemphasizes current entry if it exists.
    void emphasizeCurrentEntry(bool emphasized = true);
    /// @brief Changes currentEntryIndex_ to index and handles emphasizing.
    void silentlySetCurrentEntry(int index);

    /// @brief Resets text of all items based on tooltip (absolute path is
    /// stored in each item's tooltip) and nHiddenDirs_.
    void resetAllItemsText();

    /// WARNING: can block execution.
    void keyPressEvent(QKeyEvent *) override;

    /// WARNING: can block execution.
    void playSelectedItems();
    /// WARNING: can block execution.
    void removeSelectedItems();

    void contextMenuEvent(QContextMenuEvent *) override;


    const CustomActions::Actions & customActions_;
    const PlayExistingEntry playExistingEntry_;
    const CommonTypes::PlayItems playItems_;
    History history_;

    bool copyPlayedEntryToTop_;
    int nHiddenDirs_;
    /// Identical to Preferences::Playback::History::currentIndex but is
    /// changed more frequently.
    /// If currentEntryIndex_ >= history_.items().size(), it means that
    /// maxSize() was changed and current item is unknown. In this case
    /// previous() and current() would definitely return empty string, but
    /// next() would return history_.items().back() if
    /// currentIndex_ == history_.items().size().
    int currentEntryIndex_;

    GuiUtilities::TooltipShower tooltipShower_;

private slots:
    /// WARNING: can block execution.
    void onUiItemActivated(QListWidgetItem * item);
};

# endif // VENTUROUS_HISTORY_WIDGET_HPP
