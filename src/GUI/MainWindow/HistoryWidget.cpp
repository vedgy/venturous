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

# include "HistoryWidget.hpp"

# include "Preferences.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QFont>
# include <QListWidgetItem>

# include <cstddef>
# include <cassert>
# include <utility>
# include <string>


namespace
{
/// TODO: make publicly available for use in status bar.
/// @return absolutePath without first nHiddenDirs directories. If nHiddenDirs
/// exceeds number of directories in absolutePath, filename is returned.
QString getShortenedPath(const QString & absolutePath, unsigned nHiddenDirs)
{
    if (nHiddenDirs == 0)
        return absolutePath;
    // skipping first symbol due to ItemTree's path specifics.
    int index = 1;
    do {
        const int nextIndex = absolutePath.indexOf('/', index) + 1;
        if (nextIndex == 0)
            break; // not found.
        index = nextIndex;
    }
    while (--nHiddenDirs > 0);

    return absolutePath.mid(index);
}

void setShortenedTooltipToText(QListWidgetItem * item, unsigned nHiddenDirs)
{
    assert(item != nullptr);
    item->setText(getShortenedPath(item->toolTip(), nHiddenDirs));
}

}


HistoryWidget::HistoryWidget(PlayCommand playCommand,
                             const Preferences::Playback::History & preferences,
                             QWidget * const parent)
    : QListWidget(parent), playCommand_(std::move(playCommand)),
      copyPlayedEntryToTop_(preferences.copyPlayedEntryToTop),
      saveToDiskImmediately_(preferences.saveToDiskImmediately),
      nHiddenDirs_(preferences.nHiddenDirs),
      currentEntryIndex_(preferences.currentIndex)
{
    history_.setMaxSize(preferences.maxSize);
}

void HistoryWidget::setPreferences(
    const Preferences::Playback::History & preferences)
{
    if (history_.maxSize() != preferences.maxSize) {
        history_.setMaxSize(preferences.maxSize);
        const int maxSize = preferences.maxSize;
        for (int i = count() - 1; i >= maxSize; --i)
            delete takeItem(i);
    }
    copyPlayedEntryToTop_ = preferences.copyPlayedEntryToTop;
    saveToDiskImmediately_ = preferences.saveToDiskImmediately;
    if (nHiddenDirs_ != preferences.nHiddenDirs) {
        nHiddenDirs_ = preferences.nHiddenDirs;
        resetAllItemsText();
    }
}

bool HistoryWidget::load(const std::string & filename)
{
    QListWidget::clear();
    if (history_.load(filename)) {
        for (const std::string & entry : history_.items()) {
            QListWidgetItem * const item = new QListWidgetItem;
            item->setToolTip(QtUtilities::toQString(entry));
            addItem(item);
        }
        resetAllItemsText();
        emphasizeCurrentEntry();
        return true;
    }
    history_.clear();
    return false;
}

bool HistoryWidget::save(const std::string & filename) const
{
    return history_.save(filename);
}

void HistoryWidget::push(std::string entry)
{
    if (history_.maxSize() == 0)
        return;
    history_.push(std::move(entry));
    emphasizeCurrentEntry(false);
    {
        QListWidgetItem * const item = new QListWidgetItem;
        item->setToolTip(QtUtilities::toQString(history_.items().front()));
        setShortenedTooltipToText(item, nHiddenDirs_);
        insertItem(0, item);
    }
    if (std::size_t(count()) > history_.maxSize())
        delete takeItem(count() - 1);
    scrollToTop();
    assert(history_.items().size() == std::size_t(count()));
    currentEntryIndex_ = 0;
    emphasizeCurrentEntry();
}

void HistoryWidget::playedMultipleItems()
{
    emphasizeCurrentEntry(false);
    currentEntryIndex_ = Preferences::Playback::History::multipleItemsIndex;
}


void HistoryWidget::clearHistory()
{
    history_.clear();
    QListWidget::clear();
}



std::string HistoryWidget::entryAt(const int index) const
{
    if (index >= 0) {
        const std::size_t i = index;
        if (i < history_.items().size())
            return history_.items()[i];
    }
    return std::string();
}

std::string HistoryWidget::setCurrentEntry(const int index)
{
    const std::string entry = entryAt(index);
    if (! entry.empty()) {
        emphasizeCurrentEntry(false);
        currentEntryIndex_ = index;
        emphasizeCurrentEntry();
    }
    return entry;
}

void HistoryWidget::emphasizeCurrentEntry(const bool emphasized)
{
    if (currentEntryIndex_ >= 0 && currentEntryIndex_ < count()) {
        QListWidgetItem * const it = item(currentEntryIndex_);
        QFont font = it->font();
        font.setBold(emphasized);
        it->setFont(font);
    }
}

void HistoryWidget::resetAllItemsText()
{
    for (int i = count() - 1; i >= 0; --i)
        setShortenedTooltipToText(item(i), nHiddenDirs_);
}
