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

# ifndef VENTUROUS_TREE_WIDGET_HPP
# define VENTUROUS_TREE_WIDGET_HPP

# include <QString>
# include <QTreeWidget>

# include <string>
# include <stdexcept>
# include <memory>


namespace ItemTree
{
class Tree;
}

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string & sWhat) : std::runtime_error(sWhat) {}
    };

    /// NOTE: itemTree and temporaryTree must remain valid
    /// throughout this TreeWidget's lifetime.
    explicit TreeWidget(const ItemTree::Tree & itemTree,
                        const std::unique_ptr<ItemTree::Tree> & temporaryTree,
                        QWidget * parent = nullptr);

    /// @brief Updates visual representation to match:
    /// itemTree_ if edit mode is off, temporaryTree_ if edit mode is on.
    /// Must be called after changing itemTree_ or *temporaryTree_ externally;
    /// or after switching edit mode if additionally
    /// itemTree_ != *temporaryTree_.
    void updateTree();

    /// @brief Determines if TreeWidget can be edited by user.
    /// @param on If true, editing is allowed.
    /// @param updateTree If true and additionally edit mode was actually
    /// changed, tree would be updated acccording to new active tree state.
    /// NOTE: assuming that edit mode is actually changed:
    /// setEditMode(on, true) is equialent to, but faster than
    /// setEditMode(on, false); updateTree().
    void setEditMode(bool on, bool updateTree);

    bool editMode() const { return editMode_; }


    /// @brief Determines number of items (>= 0) in tree
    /// that will be unfolded by default.
    void setAutoUnfoldedLevels(int autoUnfoldedLevels);

    int getAutoUnfoldedLevels() const { return autoUnfoldedLevels_; }

    void assertValidTemporaryTree() const {
        if (! temporaryTree_)
            throw Error("valid temporaryTree_ expected, nullptr found.");
    }

signals:
    /// @brief Is emitted after user activates some item in this TreeWidget.
    /// @param absolutePath Path to activated item.
    /// NOTE: execution may be blocked by signal receiver.
    void itemActivated(QString absolutePath);

private:
    /// @brief Updates visual representation to match itemTree.
    void updateTree(const ItemTree::Tree & itemTree);

    /// @brief Sets appropriate options for QTreeWidget and QTreeWidgetItems
    /// based on editMode_.
    void setUiEditMode();

    /// @brief Unfolds or folds items, depending on autoUnfoldedLevels_.
    void autoUnfold();

    void keyPressEvent(QKeyEvent *) override;

    const ItemTree::Tree & itemTree_;
    const std::unique_ptr<ItemTree::Tree> & temporaryTree_;
    bool editMode_ = false;
    int autoUnfoldedLevels_ = 9;

private slots:
    void onUiItemActivated(QTreeWidgetItem * item);
    void onUiItemChanged(QTreeWidgetItem * item);
};

# endif // VENTUROUS_TREE_WIDGET_HPP