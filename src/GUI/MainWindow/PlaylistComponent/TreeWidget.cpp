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
# include <iostream>
# endif


# include "TreeWidget.hpp"

# include <VenturousCore/ItemTree-inl.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QStringList>
# include <QTreeWidgetItem>
# include <QHeaderView>
# include <QKeyEvent>

# include <cassert>
# include <utility>
# include <deque>
# include <string>


namespace
{
bool isChecked(const QTreeWidgetItem * item)
{
    return item->checkState(0) == Qt::Checked;
}

/// @brief Creates structure, which matches node's structure,
/// and places it under parent as a root.
template <class Parent>
void createTreeWidgetItem(
    Parent * parent, const ItemTree::Node & node)
{
    QTreeWidgetItem * const item = new QTreeWidgetItem(
        parent, { QtUtilities::toQString(node.name()) });
    item->setCheckState(0, node.isPlayable() ? Qt::Checked : Qt::Unchecked);

    for (const ItemTree::Node & child : node.children())
        createTreeWidgetItem(item, child);
}

const Qt::ItemFlags readOnlyFlags = Qt::ItemIsEnabled,
                    editableFlags = readOnlyFlags | Qt::ItemIsSelectable |
                                    Qt::ItemIsUserCheckable;

/// @brief Sets flags for specified item and all its descendants.
/// @param editable If true, editableFlags are set, otherwise, variation of
/// readOnlyFlags is set.
void recursivelySetEditable(QTreeWidgetItem * item, bool editable)
{
    Qt::ItemFlags flags;
    if (editable)
        flags = editableFlags;
    else {
        flags = readOnlyFlags;
        if (isChecked(item))
            flags |= Qt::ItemIsSelectable;
    }
    item->setFlags(flags);

    for (int i = item->childCount() - 1; i >= 0; --i)
        recursivelySetEditable(item->child(i), editable);
}

/// @brief Recursively unfolds or folds descendants of item.
/// @param depth Maximum level to unfold. If (depth == 1), only one item would
/// be unfolded. All items below depth will be folded.
void setExpanded(QTreeWidgetItem * item, int depth)
{
    if (depth > 0) {
        --depth;
        item->setExpanded(true);
    }
    else
        item->setExpanded(false);

    for (int i = item->childCount() - 1; i >= 0; --i)
        setExpanded(item->child(i), depth);

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
            removeSelectedItems(child, children[i]);
    }
}

}


TreeWidget::TreeWidget(const ItemTree::Tree & itemTree,
                       const std::unique_ptr<ItemTree::Tree> & temporaryTree,
                       QWidget * const parent)
    : QTreeWidget(parent), itemTree_(itemTree), temporaryTree_(temporaryTree)
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
    setSelectionMode(editMode_ ? ExtendedSelection : SingleSelection);
    for (int i = topLevelItemCount() - 1; i >= 0; --i)
        recursivelySetEditable(topLevelItem(i), editMode_);
    blockSignals(blocked);
}

void TreeWidget::autoUnfold()
{
    for (int i = topLevelItemCount() - 1; i >= 0; --i)
        ::setExpanded(topLevelItem(i), autoUnfoldedLevels_);
}

void TreeWidget::keyPressEvent(QKeyEvent * const event)
{
    if (editMode_ && event->key() == Qt::Key_Delete) {
# ifdef DEBUG_VENTUROUS_TREE_WIDGET
        std::cout << "Delete pressed." << std::endl;
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
                removeSelectedItems(item, nodes[i]);
        }
    }
    else
        QTreeView::keyPressEvent(event);
}


void TreeWidget::onUiItemActivated(QTreeWidgetItem * item)
{
    /// TODO: add multiple item selection possibility (add all in playlist).
    /// Make 2 options:
    /// 1) play all selected items (no matter playable or not);
    /// 2) play all [playable] Items - children of each selected item.
    if (editMode_ || ! isChecked(item))
        return;
    QString absolutePath;
    do {
        absolutePath.prepend('/' + item->text(0));
        item = item->parent();
    }
    while (item != nullptr);
    absolutePath.remove(0, 1); // remove extra '/'.
    emit itemActivated(std::move(absolutePath));
}

void TreeWidget::onUiItemChanged(QTreeWidgetItem * item)
{
    /// NOTE: the only allowed change is toggling checkbox in edit mode.
    /// So this change is assumed below.
# ifdef DEBUG_VENTUROUS_TREE_WIDGET
    std::cout << "Ui item's checkState changed: "
              << QtUtilities::qStringToString(item->text(0)) << std::endl;
# endif

    const bool checked = isChecked(item);
    std::deque<std::string> path;
    do {
        path.emplace_front(QtUtilities::qStringToString(item->text(0)));
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
