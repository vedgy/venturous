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

# include "PatternListWidget.hpp"

# include <VenturousCore/AddingItems.hpp>

# include <QString>
# include <QStringList>
# include <QListWidgetItem>
# include <QListWidget>
# include <QKeyEvent>

# include <cstddef>
# include <algorithm>
# include <set>


namespace
{
const Qt::ItemFlags captionItemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable,
                    knownItemFlags = captionItemFlags | Qt::ItemIsUserCheckable,
                    unknownItemFlags = knownItemFlags | Qt::ItemIsEditable;

void addCaption(QListWidget * listWidget, const QString & text)
{
    QListWidgetItem * const item = new QListWidgetItem(text);
    item->setBackgroundColor(Qt::cyan);
    item->setFlags(captionItemFlags);
    listWidget->addItem(item);
}

QListWidgetItem * addUnknownPattern(QListWidget * listWidget,
                                    const QString & pattern, bool checked)
{
    QListWidgetItem * const item = new QListWidgetItem(pattern);
    item->setFlags(unknownItemFlags);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    listWidget->addItem(item);
    return item;
}

bool isNonCaptionItem(const QListWidgetItem * item)
{
    return item->flags() != captionItemFlags;
}

bool isUnknownItem(const QListWidgetItem * item)
{
    return item->flags() == unknownItemFlags;
}

}


PatternListWidget::PatternListWidget(QWidget * const parent) :
    QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    knownPatterns_.reserve(AddingItems::allMetaDataPatterns.size() +
                           AddingItems::allAudioPatterns.size());

    captionRows_[0] = count();
    addCaption(this, tr("Meta data patterns"));
    addKnownPatterns(AddingItems::allMetaDataPatterns);

    captionRows_[1] = count();
    addCaption(this, tr("Audio patterns"));
    addKnownPatterns(AddingItems::allAudioPatterns);

    std::sort(knownPatterns_.begin(), knownPatterns_.end());

    captionRows_[2] = count();
    addCaption(this, tr("Other patterns"));
}

void PatternListWidget::setUiPatterns(const QStringList & patterns)
{
    removeAllUnknownPatterns();
    std::for_each(knownPatterns_.cbegin(), knownPatterns_.cend(),
    [](const KnownPattern & kp) {
        kp.item->setCheckState(Qt::Unchecked);
    });

    for (const QString & pattern : patterns) {
        const auto range = std::equal_range(
                               knownPatterns_.cbegin(), knownPatterns_.cend(),
                               KnownPattern(pattern, nullptr));
        if (range.first == range.second)
            ::addUnknownPattern(this, pattern, true);
        else
            range.first->item->setCheckState(Qt::Checked);
    }
}

QStringList PatternListWidget::getUiPatterns() const
{
    QStringList result;
    std::set<QString> included;

    const int nItems = count();
    for (int i = 0; i < nItems; ++i) {
        const QListWidgetItem * const current = item(i);
        if (current->checkState() == Qt::Checked) {
            const auto p = included.insert(current->text());
            if (p.second)
                result << * p.first;
        }
    }
    return result;
}

void PatternListWidget::addUnknownPatterns(const QStringList & unknownPatterns)
{
    for (const QString & pattern : unknownPatterns)
        ::addUnknownPattern(this, pattern, false);
    scrollToBottom();
}

QStringList PatternListWidget::getSelectedUnknownPatterns() const
{
    QStringList result;
    if (item(captionRows_.back())->isSelected()) {
        const int startItem = captionRows_.back() + 1;
        const int nItems = count();
        result.reserve(nItems - startItem);
        for (int i = startItem; i < nItems; ++i)
            result << item(i)->text();
    }
    else {
        const auto selected = selectedItems();

        for (const QListWidgetItem * item : selected) {
            if (isUnknownItem(item))
                result << item->text();
        }
    }
    return result;
}


void PatternListWidget::addUnknownPattern()
{
    unselectAllItems();
    QListWidgetItem * const item =
        ::addUnknownPattern(this, tr("<pattern>"), true);
    scrollToItem(item);
    editItem(item);
}


PatternListWidget::KnownPattern::KnownPattern(
    const QString & pattern, QListWidgetItem * const item)
    : pattern(pattern), item(item)
{
}

PatternListWidget::KnownPattern::KnownPattern(const QString & pattern)
    : pattern(pattern), item(new QListWidgetItem(pattern))
{
    item->setFlags(knownItemFlags);
}


void PatternListWidget::unselectAllItems()
{
    const auto selected = selectedItems();
    std::for_each(selected.begin(), selected.end(),
    [](QListWidgetItem * const item) {
        item->setSelected(false);
    });
}

void PatternListWidget::removeAllUnknownPatterns()
{
    for (int i = count() - 1; i > captionRows_.back(); --i)
        delete takeItem(i);
}

void PatternListWidget::addKnownPatterns(const QStringList & patterns)
{
    for (const QString & pattern : patterns) {
        knownPatterns_.emplace_back(pattern);
        addItem(knownPatterns_.back().item);
    }
}

void PatternListWidget::keyPressEvent(QKeyEvent * const event)
{
    switch (event->key()) {
        case Qt::Key_Delete:
            removeSelectedUnknownItems();
            break;
        case Qt::Key_Insert:
            applyToSelectedItems([](QListWidgetItem * item) {
                item->setCheckState(Qt::Checked);
            });
            break;
        case Qt::Key_Backspace:
            applyToSelectedItems([](QListWidgetItem * item) {
                item->setCheckState(Qt::Unchecked);
            });
            break;
        case Qt::Key_Space:
            applyToSelectedItems([](QListWidgetItem * item) {
                item->setCheckState(item->checkState() == Qt::Checked ?
                                    Qt::Unchecked : Qt::Checked);
            });
            break;
        default:
            QListWidget::keyPressEvent(event);
    }
}

void PatternListWidget::removeSelectedUnknownItems()
{
    if (item(captionRows_.back())->isSelected()) {
        removeAllUnknownPatterns();
        scrollToBottom();
        return;
    }

    const auto selected = selectedItems();
    bool itemRemoved = false;
    std::for_each(selected.begin(), selected.end(),
    [&](QListWidgetItem * const item) {
        if (isUnknownItem(item)) {
            delete item;
            itemRemoved = true;
        }
    });

    if (itemRemoved)
        scrollToBottom();
}

template <typename ItemUser>
void PatternListWidget::applyToSelectedItems(ItemUser f)
{
    std::set<QListWidgetItem *> selected;

    bool allItemsSelected = true;

    // Insert children of selected captions.
    for (std::size_t i = 0; i < captionRows_.size(); ++i) {
        if (item(captionRows_[i])->isSelected()) {
            const int end = (i + 1 == captionRows_.size() ?
                             count() : captionRows_[i + 1]);
            for (int row = captionRows_[i] + 1; row < end; ++row)
                selected.insert(item(row));
        }
        else
            allItemsSelected = false;
    }

    if (! allItemsSelected) {
        // Insert all non-caption selected items.
        const auto selectedRows = selectedItems();
        std::for_each(selectedRows.begin(), selectedRows.end(),
        [&](QListWidgetItem * const item) {
            if (isNonCaptionItem(item))
                selected.insert(item);
        });
    }

    std::for_each(selected.cbegin(), selected.cend(),
    [& f](QListWidgetItem * const item) {
        f(item);
    });
}
