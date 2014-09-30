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

# ifdef DEBUG_VENTUROUS_TREE_WIDGET
# include <QtCoreUtilities/String.hpp>
# include <QList>
# include <string>
# include <iostream>
# endif


# include "TreeWidget.hpp"

# include "CommonTypes.hpp"
# include "CustomActions.hpp"

# include <VenturousCore/ItemTree-inl.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QPoint>
# include <QList>
# include <QString>
# include <QStringList>
# include <QColor>
# include <QPalette>
# include <QTreeWidgetItem>
# include <QHeaderView>
# include <QKeyEvent>
# include <QContextMenuEvent>

# include <cassert>
# include <utility>
# include <algorithm>
# include <vector>
# include <deque>
# include <string>


namespace
{
inline bool isChecked(const QTreeWidgetItem * item)
{
    return item->checkState(0) == Qt::Checked;
}

inline void setChecked(QTreeWidgetItem * item, bool checked)
{
    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
}

inline QString itemText(const QTreeWidgetItem * item)
{
    return item->text(0);
}

inline QString itemTooltip(const QTreeWidgetItem * item)
{
    return item->toolTip(0);
}

inline QString itemPath(const QTreeWidgetItem * item)
{
    return itemTooltip(item) + itemText(item);
}

/// @brief Sets an appropriate tooltip for item with the specified parent.
void setToolTip(const QTreeWidgetItem * parent, QTreeWidgetItem * item)
{
    // Tooltip consists of absolute path to item (item's name is not included).
    item->setToolTip(0, itemPath(parent) + '/');
}
void setToolTip(const QTreeWidget *, QTreeWidgetItem * item)
{
    // Absolute path is empty for the top-level item.
    item->setToolTip(0, "");
}

/// @brief Creates structure that matches node's structure
/// and places it under parent as a root.
template <class Parent>
void createTreeWidgetItem(
    Parent * parent, const ItemTree::Node & node)
{
    QTreeWidgetItem * const item = new QTreeWidgetItem(
        parent, { QtUtilities::toQString(node.name()) });
    setToolTip(parent, item);
    setChecked(item, node.isPlayable());

    for (const ItemTree::Node & child : node.children())
        createTreeWidgetItem(item, child);
}

constexpr Qt::ItemFlags readOnlyFlags() noexcept {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

constexpr Qt::ItemFlags editableFlags() noexcept {
    return readOnlyFlags() | Qt::ItemIsUserCheckable;
}

/// @brief Sets flags for specified item and all its descendants.
void recursivelySetFlags(QTreeWidgetItem * item, Qt::ItemFlags flags)
{
    item->setFlags(flags);
    for (int i = item->childCount() - 1; i >= 0; --i)
        recursivelySetFlags(item->child(i), flags);
}

/// @brief Removes all selected descendants of item and corresponding
/// descendants of node.
void removeSelectedItems(QTreeWidgetItem * item, ItemTree::Node & node)
{
    std::vector<ItemTree::Node> & children = node.children();
    assert(int(children.size()) == item->childCount());

    for (int i = item->childCount() - 1; i >= 0; --i) {
        QTreeWidgetItem * child = item->child(i);
        if (child->isSelected()) {
            children.erase(children.begin() + i);
            delete child;
        }
        else
            removeSelectedItems(child, children[std::size_t(i)]);
    }
}

typedef std::deque<QString> PlayablePaths;
/// @brief Appends absolute paths of item's playable descendants
/// (including item) that are under selection to playablePaths.
/// @param item Top-level item of the tree.
/// @param wasSelected Must be false for top-level items.
void getPathsToPlay(const QTreeWidgetItem * item, PlayablePaths & playablePaths,
                    bool wasSelected = false)
{
    if (item->isSelected())
        wasSelected = true;
    if (wasSelected && isChecked(item))
        playablePaths.emplace_back(itemPath(item));

    const int nChildren = item->childCount();
    for (int i = 0; i < nChildren; ++i)
        getPathsToPlay(item->child(i), playablePaths, wasSelected);
}


# ifdef DEBUG_VENTUROUS_TREE_WIDGET
void printMessageAndSelectedItemsSize(const std::string & message, int size)
{
    std::cout << "TreeWidget: " << message << ". "
              << size << " item" << (size == 1 ? " is" : "s are")
              << " selected." << std::endl;
}
# endif

}


TreeWidget::Error::~Error() noexcept = default;

TreeWidget::TreeWidget(const ItemTree::Tree & itemTree,
                       const std::unique_ptr<ItemTree::Tree> & temporaryTree,
                       const CustomActions::Actions & customActions,
                       CommonTypes::PlayItems playItems,
                       QWidget * const parent)
    : QTreeWidget(parent), itemTree_(itemTree), temporaryTree_(temporaryTree),
      customActions_(customActions), playItems_(std::move(playItems)),
      tooltipShower_(this)
{
    setColumnCount(1);
    header()->close();
    header()->
# if QT_VERSION >= 0x050000
    setSectionResizeMode
# else
    setResizeMode
# endif
    (QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            SLOT(onUiItemActivated(QTreeWidgetItem *)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            SLOT(onUiItemChanged(QTreeWidgetItem *)));

    updateTree();
}

void TreeWidget::updateTree()
{
    updateTree(editMode_ ? *temporaryTree_ : itemTree_);
}

void TreeWidget::setEditMode(const bool on, const bool updateTree)
{
    if (on == editMode_)
        return;
    editMode_ = on;
    if (updateTree)
        this->updateTree();
    else
        setUiEditMode();
}

void TreeWidget::setAutoUnfoldedLevels(const int autoUnfoldedLevels)
{
    if (autoUnfoldedLevels < 0)
        throw Error("negative autoUnfoldedLevels value.");
    if (autoUnfoldedLevels != autoUnfoldedLevels_) {
        autoUnfoldedLevels_ = autoUnfoldedLevels;
        autoUnfold();
    }
}


void TreeWidget::updateTree(const ItemTree::Tree & itemTree)
{
    const bool blocked = blockSignals(true);
    clear();
    for (const ItemTree::Node & topLevelNode : itemTree.topLevelNodes())
        createTreeWidgetItem(this, topLevelNode);
    setUiEditMode();
    autoUnfold();
    blockSignals(blocked);
}

void TreeWidget::setUiEditMode()
{
    const bool blocked = blockSignals(true);
    const bool visible = isVisible();
    if (visible)
        hide(); // Dramatically improves performance for large tree.

    int hue;
    Qt::ItemFlags flags;
    if (editMode_) {
        hue = 280;
        flags = editableFlags();
    }
    else {
        hue = 57;
        flags = readOnlyFlags();
    }

    {
        QPalette p = palette();
        constexpr int minLightness = 20, maxLightness = 247;
        const int lightness = std::max(std::min(p.base().color().lightness(),
                                                maxLightness), minLightness);
        constexpr int saturation = 255;
        p.setColor(QPalette::Base, QColor::fromHsl(hue, saturation, lightness));
        setPalette(p);
    }
    for (int i = topLevelItemCount() - 1; i >= 0; --i)
        recursivelySetFlags(topLevelItem(i), flags);

    if (visible)
        show();
    setFocus();
    blockSignals(blocked);
}

void TreeWidget::autoUnfold()
{
    if (autoUnfoldedLevels_ == 0)
        collapseAll();
    else
        expandToDepth(autoUnfoldedLevels_ - 1);
}

void TreeWidget::keyPressEvent(QKeyEvent * const event)
{
    const int key = event->key();
    if (editMode_) {
        switch (key) {
            case Qt::Key_Delete:
                onDelete();
                return;
            case Qt::Key_Insert:
                applyToSelectedItems([](QTreeWidgetItem * item) {
                    setChecked(item, true);
                });
                return;
            case Qt::Key_Backspace:
                applyToSelectedItems([](QTreeWidgetItem * item) {
                    setChecked(item, false);
                });
                return;
            case Qt::Key_Space:
                applyToSelectedItems([](QTreeWidgetItem * item) {
                    setChecked(item, ! isChecked(item));
                });
                return;
        }
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        onEnter();
        return;
    }
    // Asterisk: expands all children of the current item (if present).
    // Below is a WORKAROUND for Qt performance issue, which doesn't change
    // program behaviour but dramatically improves Asterisk key performance
    // if at least one child of current item is visible.
    if (key == Qt::Key_Asterisk)
        currentItem()->setExpanded(false);
    QTreeView::keyPressEvent(event);
}

void TreeWidget::onDelete()
{
# ifdef DEBUG_VENTUROUS_TREE_WIDGET
    printMessageAndSelectedItemsSize("Delete pressed", selectedItems().size());
# endif
    assertValidTemporaryTree();
    std::vector<ItemTree::Node> & nodes = temporaryTree_->topLevelNodes();
    assert(int(nodes.size()) == topLevelItemCount());

    for (int i = topLevelItemCount() - 1; i >= 0; --i) {
        QTreeWidgetItem * item = topLevelItem(i);
        if (item->isSelected()) {
            nodes.erase(nodes.begin() + i);
            delete item;
        }
        else
            removeSelectedItems(item, nodes[std::size_t(i)]);
    }
}

void TreeWidget::onEnter()
{
# ifdef DEBUG_VENTUROUS_TREE_WIDGET
    printMessageAndSelectedItemsSize("Enter pressed", selectedItems().size());
# endif
    PlayablePaths paths;
    const int nItems = topLevelItemCount();
    for (int i = 0; i < nItems; ++i)
        getPathsToPlay(topLevelItem(i), paths);
    if (! paths.empty()) {
        CommonTypes::ItemCollection items(paths.size());
        std::transform(paths.cbegin(), paths.cend(), items.begin(),
                       QtUtilities::qStringToString);
        playItems_(items);
    }
}

void TreeWidget::contextMenuEvent(QContextMenuEvent * const event)
{
# ifdef DEBUG_VENTUROUS_TREE_WIDGET
    printMessageAndSelectedItemsSize("context menu requested",
                                     selectedItems().size());
# endif
    const QPoint position = event->globalPos();
    QString commonPrefix;
    QStringList itemNames;
    {
        const auto selected = selectedItems();
        if (! selected.empty()) {
            const QTreeWidgetItem * const parent = selected.front()->parent();
            for (int i = 1; i < selected.size(); ++i) {
                if (selected[i]->parent() != parent) {
                    tooltipShower_.show(
                        position,
                        tr("Custom actions are enabled only if all "
                           "selected items are siblings (share the same "
                           "parent tree node)."));
                    return;
                }
            }
            commonPrefix = itemTooltip(selected.front());
            itemNames.reserve(selected.size());
            for (const QTreeWidgetItem * item : selected)
                itemNames << itemText(item);
        }
    }
    CustomActions::showMenu(
        customActions_, std::move(commonPrefix), std::move(itemNames),
        position, tooltipShower_);
}

template <typename ItemUser>
void TreeWidget::applyToSelectedItems(ItemUser f)
{
    hide(); // Improves performance for large tree.
    const auto selected = selectedItems();
    std::for_each(selected.begin(), selected.end(), f);
    show();
    setFocus();
}


void TreeWidget::onUiItemActivated(QTreeWidgetItem * const item)
{
    if (editMode_ || ! isChecked(item))
        return;
    playItems_( { QtUtilities::qStringToString(itemPath(item)) });
}

void TreeWidget::onUiItemChanged(QTreeWidgetItem * item)
{
    /// NOTE: the only allowed change is toggling checkbox in edit mode.
    /// So this change is assumed below.
# ifdef DEBUG_VENTUROUS_TREE_WIDGET
    std::cout << "TreeWidget: UI item's checkState changed: "
              << QtUtilities::qStringToString(itemText(item)) << std::endl;
# endif

    const bool checked = isChecked(item);
    std::deque<std::string> path;
    do {
        path.emplace_front(QtUtilities::qStringToString(itemText(item)));
        item = item->parent();
    }
    while (item != nullptr);

    assertValidTemporaryTree();
    ItemTree::Node * const node = temporaryTree_->descendant(
                                      path.cbegin(), path.cend());
    if (node == nullptr)
        throw Error("could not find specified item in temporaryTree_.");
    if (checked == node->isPlayable()) {
        throw Error("desynchronization of QTreeWidget and temporaryTree_ "
                    "was detected.");
    }
    node->setPlayable(checked);
}
