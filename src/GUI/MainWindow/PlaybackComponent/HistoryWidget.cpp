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

# include "CommonTypes.hpp"
# include "Preferences.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QFont>
# include <QListWidgetItem>
# include <QKeyEvent>

# include <cstddef>
# include <cassert>
# include <utility>
# include <vector>
# include <string>


namespace
{
/// @param absolutePath Non-empty absolute path.
/// @return absolutePath without first nHiddenDirs directories. If nHiddenDirs
/// exceeds number of directories in absolutePath, filename is returned.
QString getShortenedPath(const QString & absolutePath, unsigned nHiddenDirs)
{
    assert(! absolutePath.isEmpty());
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


HistoryWidget::HistoryWidget(PlayExistingEntry playExistingEntry,
                             CommonTypes::PlayItems playItems,
                             const Preferences::Playback::History & preferences,
                             QWidget * const parent)
    : QListWidget(parent), playExistingEntry_(std::move(playExistingEntry)),
      playItems_(std::move(playItems)),
      copyPlayedEntryToTop_(preferences.copyPlayedEntryToTop),
      nHiddenDirs_(preferences.nHiddenDirs),
      currentEntryIndex_(preferences.currentIndex)
{
    history_.setMaxSize(preferences.maxSize);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(this, SIGNAL(itemActivated(QListWidgetItem *)),
            SLOT(onUiItemActivated(QListWidgetItem *)));
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

QString HistoryWidget::currentAbsolute() const
{
    const QListWidgetItem * const it = item(currentEntryIndex_);
    if (it == nullptr)
        return QString();
    return it->toolTip();
}

QString HistoryWidget::currentShortened() const
{
    const QListWidgetItem * const it = item(currentEntryIndex_);
    if (it == nullptr)
        return QString();
    return it->text();
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
    QListWidgetItem * const it = item(currentEntryIndex_);
    if (it != nullptr) {
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

void HistoryWidget::keyPressEvent(QKeyEvent * const event)
{
    assert(history_.items().size() == std::size_t(count()));
    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            const auto items = selectedItems();
            if (! items.empty()) {
                if (items.size() == 1)
                    onUiItemActivated(items.back());
                else {
                    std::vector<std::string> entries;
                    entries.reserve(items.size());
                    for (const QListWidgetItem * const item : items)
                        entries.emplace_back(history_.items()[row(item)]);
                    playItems_(std::move(entries));
                }
            }
        }
        break;
        case Qt::Key_Delete: {
            const auto items = selectedItems();
            if (! items.empty()) {
                const QListWidgetItem * currentItem = item(currentEntryIndex_);
                if (currentItem != nullptr && currentItem->isSelected()) {
                    currentItem = nullptr;
                    currentEntryIndex_ = -1;
                }

                std::vector<std::size_t> indices;
                indices.reserve(items.size());
                for (const QListWidgetItem * const item : items)
                    indices.emplace_back(row(item));
                history_.remove(indices);
                for (const QListWidgetItem * const item : items)
                    delete item;

                if (currentItem != nullptr)
                    currentEntryIndex_ = row(currentItem);
            }
        }
        break;
        default:
            QListWidget::keyPressEvent(event);
    }
}

void HistoryWidget::onUiItemActivated(QListWidgetItem * const item)
{
    if (copyPlayedEntryToTop_)
        playItems_(CommonTypes::ItemCollection { history_.items()[row(item)] });
    else {
        const int index = row(item);
        setCurrentEntry(index);
        playExistingEntry_(history_.items()[index]);
    }
}
