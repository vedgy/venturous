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
# include "CustomActions.hpp"
# include "Preferences.hpp"

# include <QtCoreUtilities/String.hpp>
# include <QtCoreUtilities/Miscellaneous.hpp>

# include <QPoint>
# include <QList>
# include <QString>
# include <QStringList>
# include <QFont>
# include <QListWidgetItem>
# include <QKeyEvent>
# include <QContextMenuEvent>

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
/// If nHiddenDirs < 0, -nHiddenDirs last components of path are returned.
/// For instance, if nHiddenDirs == -2,
/// "<directory that contains file>/filename" is returned.
/// If -nHiddenDirs exceeds number of components in absolutePath, absolutePath
/// is returned.
QString getShortenedPath(const QString & absolutePath, int nHiddenDirs)
{
    assert(! absolutePath.isEmpty());
    int index;
    if (nHiddenDirs < 0) {
        index = 0;
        while (++nHiddenDirs <= 0) {
            index = absolutePath.lastIndexOf('/', index - 1);
            if (index <= 0)
                break; // not found or found root symbol.
        }
        ++index;
    }
    else {
        // skipping first symbol due to ItemTree's path specifics.
        index = 1;
        while (--nHiddenDirs >= 0) {
            const int nextIndex = absolutePath.indexOf('/', index) + 1;
            if (nextIndex == 0)
                break; // not found.
            index = nextIndex;
        }
    }

    if (index <= 1)
        return absolutePath;
    return absolutePath.mid(index);
}

inline void setShortenedTooltipToText(QListWidgetItem * item, int nHiddenDirs)
{
    assert(item != nullptr);
    item->setText(getShortenedPath(item->toolTip(), nHiddenDirs));
}

}


HistoryWidget::HistoryWidget(const CustomActions::Actions & customActions,
                             PlayExistingEntry playExistingEntry,
                             CommonTypes::PlayItems playItems,
                             const Preferences::Playback::History & preferences,
                             QWidget * const parent)
    : QListWidget(parent), customActions_(customActions),
      playExistingEntry_(std::move(playExistingEntry)),
      playItems_(std::move(playItems)),
      copyPlayedEntryToTop_(preferences.copyPlayedEntryToTop),
      nHiddenDirs_(preferences.nHiddenDirs),
      currentEntryIndex_(preferences.currentIndex), tooltipShower_(this)
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
        const int maxSize = static_cast<int>(preferences.maxSize);
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
    QtUtilities::makePathTo(filename);
    return history_.save(filename);
}

std::string HistoryWidget::current()
{
    return setCurrentEntry(currentEntryIndex_);
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
    {
        const int nItems = count();
        if (std::size_t(nItems) > history_.maxSize())
            delete takeItem(nItems - 1);
    }
    scrollToTop();
    assert(history_.items().size() == std::size_t(count()));
    currentEntryIndex_ = 0;
    emphasizeCurrentEntry();
}

void HistoryWidget::playedMultipleItems()
{
    silentlySetCurrentEntry(Preferences::Playback::History::multipleItemsIndex);
}


void HistoryWidget::clearHistory()
{
    if (! history_.items().empty()) {
        history_.clear();
        QListWidget::clear();
        emit historyChanged();
    }
}



std::string HistoryWidget::entryAt(const int index) const
{
    if (index >= 0) {
        const std::size_t i = std::size_t(index);
        if (i < history_.items().size())
            return history_.items()[i];
    }
    return std::string();
}

std::string HistoryWidget::setCurrentEntry(const int index)
{
    std::string entry = entryAt(index);
    if (! entry.empty()) {
        if (copyPlayedEntryToTop_)
            push(entry);
        else
            silentlySetCurrentEntry(index);
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

void HistoryWidget::silentlySetCurrentEntry(const int index)
{
    if (currentEntryIndex_ != index) {
        emphasizeCurrentEntry(false);
        currentEntryIndex_ = index;
        emphasizeCurrentEntry();
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
        case Qt::Key_Enter:
            playSelectedItems();
            break;
        case Qt::Key_Delete:
            removeSelectedItems();
            break;
        case Qt::Key_Insert: {
            const auto items = selectedItems();
            if (! items.empty()) {
                if (items.size() == 1)
                    silentlySetCurrentEntry(row(items.back()));
                else
                    playedMultipleItems();
            }
            break;
        }
        case Qt::Key_Backspace:
            playedMultipleItems();
            break;
        default:
            QListWidget::keyPressEvent(event);
    }
}

void HistoryWidget::playSelectedItems()
{
    const auto items = selectedItems();
    if (! items.empty()) {
        if (items.size() == 1)
            onUiItemActivated(items.back());
        else {
            std::vector<std::string> entries;
            entries.reserve(std::size_t(items.size()));
            for (const QListWidgetItem * const item : items)
                entries.emplace_back(history_.items()[std::size_t(row(item))]);
            playItems_(std::move(entries));
        }
    }
}

void HistoryWidget::removeSelectedItems()
{
    const auto items = selectedItems();
    if (! items.empty()) {
        const QListWidgetItem * currentItem = item(currentEntryIndex_);
        if (currentItem != nullptr && currentItem->isSelected()) {
            currentItem = nullptr;
            currentEntryIndex_ =
                Preferences::Playback::History::multipleItemsIndex;
        }

        std::vector<std::size_t> indices;
        indices.reserve(std::size_t(items.size()));
        for (const QListWidgetItem * const item : items)
            indices.emplace_back(row(item));
        history_.remove(indices);
        for (const QListWidgetItem * const item : items)
            delete item;

        if (currentItem != nullptr)
            currentEntryIndex_ = row(currentItem);

        emit historyChanged();
    }
}

void HistoryWidget::contextMenuEvent(QContextMenuEvent * const event)
{
    const QPoint position = event->globalPos();
    QString commonPrefix;
    QStringList itemNames;
    {
        const auto items = selectedItems();
        if (! items.empty()) {
            itemNames.reserve(items.size());

            const auto appendFilenameAndReturnDir =
            [& itemNames](const QListWidgetItem * item) {
                QString absolutePath = item->toolTip();
                int i = absolutePath.lastIndexOf('/');
                if (i <= 0) { // Root is not considered a separate dir.
                    itemNames << std::move(absolutePath);
                    return QString();
                }
                ++i;
                itemNames << absolutePath.mid(i);
                return absolutePath.left(i);
            };

            commonPrefix = appendFilenameAndReturnDir(items.front());
            for (int i = 1; i < items.size(); ++i) {
                if (appendFilenameAndReturnDir(items[i]) != commonPrefix) {
                    tooltipShower_.show(
                        position,
                        tr("Custom actions are enabled only if all "
                           "selected items are siblings (are located in the "
                           "same directory)."));
                    return;
                }
            }
        }
    }
    CustomActions::showMenu(
        customActions_, std::move(commonPrefix), std::move(itemNames),
        position, tooltipShower_);
}


void HistoryWidget::onUiItemActivated(QListWidgetItem * const item)
{
    assert(item != nullptr);
    playExistingEntry_(setCurrentEntry(row(item)));
    if (isHistoryChangedBySettingCurrentEntry())
        emit historyChanged();
}
