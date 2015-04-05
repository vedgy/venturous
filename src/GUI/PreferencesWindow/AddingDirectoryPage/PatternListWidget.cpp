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

# include "PatternListWidget.hpp"

# include "FilePattern.hpp"

# include <QList>
# include <QString>
# include <QVariant>
# include <QListWidgetItem>
# include <QListWidget>
# include <QKeyEvent>

# include <cstddef>
# include <cassert>
# include <utility>
# include <algorithm>


namespace
{
using PatternSet = PatternListWidget::PatternSet;
}
Q_DECLARE_METATYPE(PatternSet::iterator)

namespace
{
inline void setChecked(QListWidgetItem * item, bool checked)
{
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

inline void appendPatternToList(FilePatternList & patternList,
                                const QListWidgetItem * item)
{
    patternList.push_back( { item->text(),
                             item->checkState() == Qt::Checked
                           });
}

inline PatternSet::iterator getIterator(const QListWidgetItem * item)
{
    return item->data(Qt::UserRole).value<PatternSet::iterator>();
}

inline void setIterator(QListWidgetItem * item, PatternSet::iterator iterator)
{
    item->setData(Qt::UserRole, QVariant::fromValue(std::move(iterator)));
}

} // END unnamed namespace


PatternListWidget::PatternListWidget(QWidget * const parent) :
    QListWidget(parent), tooltipShower_(this)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(this, SIGNAL(itemChanged(QListWidgetItem *)),
            SLOT(onItemChanged(QListWidgetItem *)));
}

void PatternListWidget::setUiPatterns(const FilePatternList & patterns)
{
    clear();
    patternSet_.clear();
    addUniquePatterns(patterns);
}

FilePatternList PatternListWidget::getUiPatterns() const
{
    FilePatternList result;
    const int nItems = count();
    result.reserve(std::size_t(nItems));
    for (int i = 0; i < nItems; ++i)
        appendPatternToList(result, item(i));
    return result;
}

void PatternListWidget::addPatterns(const FilePatternList & patterns)
{
    clearSelection();
    addUniquePatterns(patterns);
    scrollToBottom();
}

FilePatternList PatternListWidget::getSelectedPatterns() const
{
    FilePatternList result;
    const auto selected = selectedItems();
    result.reserve(std::size_t(selected.size()));
    for (const QListWidgetItem * item : selected)
        appendPatternToList(result, item);
    return result;
}


void PatternListWidget::addPattern()
{
    clearSelection();
    QListWidgetItem * item;
    const QString newItemText = tr("<enter new pattern>");
    if (patternSet_.count(newItemText) == 1) {
        // Reuse the existing pattern with text=newItemText.
        const auto items = findItems(newItemText, Qt::MatchExactly);
        assert(items.size() == 1);
        item = items.back();
    }
    else {
        // Create a new pattern with text=newItemText.
        const PatternSet::iterator iterator =
            patternSet_.insert(newItemText).first;
        item = addPatternToUiList(newItemText, true, iterator);
    }
    scrollToItem(item);
    editItem(item);
}


void PatternListWidget::addUniquePattern(const FilePattern & pattern)
{
    const auto pair = patternSet_.insert(pattern.pattern);
    if (pair.second)
        addPatternToUiList(pattern.pattern, pattern.enabled, pair.first);
}

void PatternListWidget::addUniquePatterns(const FilePatternList & patterns)
{
    for (const FilePattern & p : patterns)
        addUniquePattern(p);
}

QListWidgetItem * PatternListWidget::addPatternToUiList(
    const QString & pattern, const bool checked,
    const PatternSet::iterator iterator)
{
    QListWidgetItem * const item = new QListWidgetItem(pattern);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                   Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
    setChecked(item, checked);
    setIterator(item, iterator);
    addItem(item);
    return item;
}

void PatternListWidget::keyPressEvent(QKeyEvent * const event)
{
    switch (event->key()) {
        case Qt::Key_Delete:
            applyToSelectedItems([this](QListWidgetItem * item) {
                patternSet_.erase(getIterator(item));
                delete item;
            });
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

template <typename ItemUser>
void PatternListWidget::applyToSelectedItems(ItemUser itemUser)
{
    const auto selected = selectedItems();
    std::for_each(selected.begin(), selected.end(), std::move(itemUser));
}


void PatternListWidget::onItemChanged(QListWidgetItem * const item)
{
    const auto iterator = getIterator(item);
    const auto handleInvalidPattern = [&](QString && errorMessage) {
        tooltipShower_.show(mapToGlobal(QPoint {}), std::move(errorMessage));
        item->setText(*iterator);
    };

    QString newText = item->text();
    if (newText.isEmpty()) {
        handleInvalidPattern(tr("Pattern can not be empty."));
        return;
    }
    if (newText != *iterator) { // item's text was changed.
        const auto pair = patternSet_.insert(std::move(newText));
        if (! pair.second) {
            handleInvalidPattern(
                tr("Entered pattern is already present in the list."));
            return;
        }
        patternSet_.erase(iterator);
        setIterator(item, pair.first);
    }
}
