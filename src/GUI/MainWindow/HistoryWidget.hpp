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

# include "Preferences.hpp"

# include <VenturousCore/History.hpp>

# include <QListWidget>

# include <functional>
# include <vector>
# include <string>


class HistoryWidget : public QListWidget
{
    Q_OBJECT
public:
    /// TODO: add another command with history index to allow honoring
    /// copyPlayedEntryToTop_.
    typedef std::vector<std::string> ItemCollection;
    typedef std::function<void(const ItemCollection &)> PlayCommand;

    /// @param playCommand Method, which starts playing items from
    /// ItemCollection parameter in external player.
    explicit HistoryWidget(PlayCommand playCommand,
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


    int currentEntryIndex() const { return currentEntryIndex_; }

    std::string previous() { return setCurrentEntry(currentEntryIndex_ + 1); }
    std::string current() const { return entryAt(currentEntryIndex_); }
    std::string next() { return setCurrentEntry(currentEntryIndex_ - 1); }

    /// @brief Adds entry to the history and sets currentEntryIndex_ to 0.
    void push(std::string entry);

    /// @brief Sets currentEntryIndex_ to
    /// Preferences::Playback::History::multipleItemsIndex.
    void playedMultipleItems();

public slots:
    void clearHistory();

private:
    /// @return Entry path at specified index or empty string if there is no
    /// such entry.
    std::string entryAt(int index) const;
    /// @brief If there is an entry at specified index, points
    /// currentEntryIndex_ to it and returns its path,
    /// otherwise returns empty string.
    std::string setCurrentEntry(int index);

    /// @brief emphasizes/deemphasizes current entry if it exists.
    void emphasizeCurrentEntry(bool emphasized = true);

    /// @brief Resets text of all items based on tooltip (absolute path is
    /// stored in each item's tooltip) and nHiddenDirs_.
    void resetAllItemsText();

    const PlayCommand playCommand_;
    History history_;

    bool copyPlayedEntryToTop_;
    bool saveToDiskImmediately_;
    unsigned nHiddenDirs_;
    /// Identical to Preferences::Playback::History::currentIndex, but is
    /// changed more frequently.
    /// If currentEntryIndex_ >= history_.items().size(), it means that
    /// maxSize() was changed and current item is unknown. In this case
    /// previous() and current() would definitely return empty string, but
    /// next() could return history_.items().back() if
    /// currentIndex_ == history_.items().size().
    int currentEntryIndex_;
};

# endif // VENTUROUS_HISTORY_WIDGET_HPP
