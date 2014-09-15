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

# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS_PAGE
# include <iostream>
# endif


# include "CustomActionsPage.hpp"

# include "CustomActions.hpp"
# include "Preferences.hpp"

# include <QtWidgetsUtilities/Miscellaneous.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QModelIndex>
# include <QAbstractItemModel>
# include <QIcon>
# include <QTableWidgetItem>
# include <QItemSelectionRange>
# include <QItemSelection>
# include <QItemSelectionModel>
# include <QHBoxLayout>
# include <QVBoxLayout>
# include <QFrame>
# include <QLabel>
# include <QPushButton>
# include <QToolButton>
# include <QSpinBox>
# include <QCheckBox>
# include <QComboBox>
# include <QTableWidget>

# include <cstddef>
# include <cassert>
# include <utility>
# include <functional>
# include <algorithm>
# include <vector>


namespace
{
void setText(QTableWidget & table, int row, int column, const QString & text)
{
    QTableWidgetItem * const item = new QTableWidgetItem(text);
    table.setItem(row, column, item);
}

void setBool(QTableWidget & table, int row, int column, bool value)
{
    QTableWidgetItem * const item = new QTableWidgetItem;
    item->setCheckState(value ? Qt::Checked : Qt::Unchecked);
    table.setItem(row, column, item);
}

void setInt(QTableWidget & table, int row, int column,
            int value, int minValue, int maxValue)
{
    QSpinBox * const spinBox = new QSpinBox;
    spinBox->setRange(minValue, maxValue);
    spinBox->setValue(value);
    table.setCellWidget(row, column, spinBox);
}

namespace Columns
{
constexpr int enabled = 0, text = enabled + 1, command = text + 1,
              minArgN = 3, maxArgN = minArgN + 1, type = maxArgN + 1,
              comment = type + 1, n = 7;
}

struct CellWidgetValues {
    int minArgN, maxArgN, type;
};

void setCellWidgetColumns(QTableWidget & table, int row,
                          const CellWidgetValues & cellWidgetValues)
{
    typedef CustomActions::Action A;
    setInt(table, row, Columns::minArgN, cellWidgetValues.minArgN,
           A::minMinArgN, A::maxMinArgN);
    setInt(table, row, Columns::maxArgN, cellWidgetValues.maxArgN,
           A::minMaxArgN, A::maxMaxArgN);
    {
        QComboBox * const c = new QComboBox;
        c->addItems( { QObject::tr("File"), QObject::tr("Directory"),
                       QObject::tr("Any item")
                     });
        c->setCurrentIndex(cellWidgetValues.type);
        table.setCellWidget(row, Columns::type, c);
    }
}

void setRow(QTableWidget & table, int row, const CustomActions::Action & action)
{
    setText(table, row, Columns::text, action.text);
    setText(table, row, Columns::command, action.command);
    setCellWidgetColumns(table, row, { action.minArgN, action.maxArgN,
                                       static_cast<int>(action.type)
                                     });
    setText(table, row, Columns::comment, action.comment);
    /// NOTE: "enabled" column must be set last.
    setBool(table, row, Columns::enabled, action.enabled);
}

void addLabel(const QString & text, QWidget * parent, QVBoxLayout * layout)
{
    QLabel * const label = new QLabel(text, parent);
    label->setWordWrap(true);
    layout->addWidget(label);
}

void addButton(const QIcon & icon, const QString & text,
               const QString & tooltip, QWidget * parent,
               const char * slot, QHBoxLayout * layout)
{
    QPushButton * const button = new QPushButton(icon, text, parent);
    button->setToolTip(tooltip);
    QtUtilities::Widgets::setFixedSizePolicy(button);
    parent->connect(button, SIGNAL(clicked(bool)), slot);
    layout->addWidget(button);
}

void addArrowButton(Qt::ArrowType type, QWidget * parent, const char * slot,
                    QHBoxLayout * layout)
{
    QToolButton * const button = new QToolButton(parent);
    button->setArrowType(type);
    button->setToolTip(QObject::tr("Move selected rows %1.").
                       arg(type == Qt::UpArrow ? QObject::tr("up")
                           : QObject::tr("down")));
    parent->connect(button, SIGNAL(clicked(bool)), slot);
    layout->addWidget(button);
}

typedef std::vector<int> TableIndices;

template <typename Compare = std::less<int>>
TableIndices getSortedSelected(const QTableWidget & table,
                               Compare compare = Compare())
{
    const auto indices = table.selectionModel()->selectedRows();
    TableIndices sortedIndices(std::size_t(indices.size()));
    std::transform(indices.begin(), indices.end(), sortedIndices.begin(),
                   std::bind(& QModelIndex::row, std::placeholders::_1));
    std::sort(sortedIndices.begin(), sortedIndices.end(), compare);
    return sortedIndices;
}

CellWidgetValues getCellWidgetValues(const QTableWidget & table, int row)
{
    return {
        qobject_cast<QSpinBox *>(
            table.cellWidget(row, Columns::minArgN))->value(),
        qobject_cast<QSpinBox *>(
            table.cellWidget(row, Columns::maxArgN))->value(),
        qobject_cast<QComboBox *>(
            table.cellWidget(row, Columns::type))->currentIndex()
    };
}

void moveRow(QTableWidget & table, int from, int to)
{
    setCellWidgetColumns(table, to, getCellWidgetValues(table, from));
    /// NOTE: columns traversal order is important, because "enabled" column
    /// must be set last.
    assert(Columns::enabled == 0);
    for (int col = Columns::n - 1; col >= 0; --col) {
        QTableWidgetItem * const item = table.takeItem(from, col);
        if (item != nullptr)
            table.setItem(to, col, item);
    }
}

void selectRows(QTableWidget & table, const TableIndices & indices)
{
    const QAbstractItemModel & model = * table.model();
    QItemSelection selection;
    selection.reserve(int(indices.size()));
    for (int i : indices) {
        selection.push_back( { model.index(i, 0),
                               model.index(i, Columns::n - 1)
                             });
    }
    table.selectionModel()->select(selection, QItemSelectionModel::Select);
}

}


CustomActionsPage::CustomActionsPage(
    const QIcon & addIcon, const QIcon & removeIcon,
    QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    table_.setSelectionBehavior(QAbstractItemView::SelectRows);
    table_.setColumnCount(Columns::n);
    table_.setHorizontalHeaderLabels( {
        tr("Enabled"), tr("Text"), tr("Command"), tr("Min"), tr("Max"),
        tr("Type"), tr("Comment")
    });
    connect(& table_, SIGNAL(cellChanged(int, int)),
            SLOT(onCellChanged(int, int)));

    QVBoxLayout * const layout = new QVBoxLayout(this);
    layout->addWidget(& table_);
    {
        QHBoxLayout * const actionsLayout = new QHBoxLayout;
        addButton(addIcon, tr("Add row"),
                  tr("Add row at the bottom of the table."),
                  this, SLOT(addRow()), actionsLayout);
        addButton(addIcon, tr("Insert row"),
                  tr("Insert a new row before the first selected row\n"
                     "or at the bottom of the table if nothing is selected."),
                  this, SLOT(insertRow()), actionsLayout);
        addButton(removeIcon, tr("Remove selected"),
                  tr("Remove selected rows from the table."),
                  this, SLOT(removeSelectedRows()), actionsLayout);

        addArrowButton(Qt::UpArrow, this, SLOT(moveSelectedRowsUp()),
                       actionsLayout);
        addArrowButton(Qt::DownArrow, this, SLOT(moveSelectedRowsDown()),
                       actionsLayout);

        QCheckBox * const helpCheckBox = new QCheckBox(tr("Show help"), this);
        helpCheckBox->setToolTip(tr("Show description of table columns."));
        connect(helpCheckBox, SIGNAL(toggled(bool)), SLOT(onShowHelpToggled()));
        actionsLayout->addWidget(helpCheckBox);

        layout->addLayout(actionsLayout);
    }
}

void CustomActionsPage::setUiPreferences(const Preferences & source)
{
    const CustomActions::Actions & actions = source.customActions;
    table_.setRowCount(static_cast<int>(actions.size()));
    for (std::size_t i = 0; i < actions.size(); ++i)
        setRow(table_, static_cast<int>(i), actions[i]);
    table_.resizeColumnsToContents();
}

void CustomActionsPage::writeUiPreferencesTo(Preferences & destination) const
{
    CustomActions::Actions & actions = destination.customActions;
    actions.resize(std::size_t(table_.rowCount()));
    for (std::size_t i = 0; i < actions.size(); ++i) {
        CustomActions::Action & a = actions[i];
        const int row = int(i);
        a.enabled = table_.item(row, Columns::enabled)->checkState() ==
                    Qt::Checked;
        a.text = table_.item(row, Columns::text)->text();
        a.command = table_.item(row, Columns::command)->text();
        {
            const CellWidgetValues values = getCellWidgetValues(table_, row);
            a.minArgN = values.minArgN;
            a.maxArgN = values.maxArgN;
            a.type = static_cast<CustomActions::Action::Type>(values.type);
        }
        a.comment = table_.item(row, Columns::comment)->text();
    }
}



void CustomActionsPage::insertRow(const int row)
{
    table_.insertRow(row);
    CustomActions::Action action = CustomActions::Action::getEmpty();
    action.enabled = true;
    setRow(table_, row, std::move(action));
    if (table_.rowCount() == 1)
        table_.resizeColumnsToContents();
}


void CustomActionsPage::addRow()
{
    insertRow(table_.rowCount());
}

void CustomActionsPage::insertRow()
{
    int insertionRow = table_.rowCount();
    {
        const auto indices = table_.selectionModel()->selectedRows();
        for (const auto & index : indices) {
            const int row = index.row();
            if (row < insertionRow)
                insertionRow = row;
        }
    }
    insertRow(insertionRow);
}

void CustomActionsPage::removeSelectedRows()
{
    const TableIndices descendingSortedIndices =
        getSortedSelected(table_, std::greater<int>());
    for (int index : descendingSortedIndices)
        table_.removeRow(index);
}

void CustomActionsPage::moveSelectedRowsUp()
{
    TableIndices sortedIndices = getSortedSelected(table_);
    bool selectionChanged = false;
    for (std::size_t i = 0; i < sortedIndices.size(); ++i) {
        const int index = sortedIndices[i];
        if (index == int(i))
            continue; // Nowhere to move.
        table_.insertRow(index - 1);
        moveRow(table_, index + 1, index - 1);
        table_.removeRow(index + 1);
        --sortedIndices[i];
        selectionChanged = true;
    }
    if (selectionChanged)
        selectRows(table_, sortedIndices);
}

void CustomActionsPage::moveSelectedRowsDown()
{
    TableIndices descendingSortedIndices =
        getSortedSelected(table_, std::greater<int>());
    bool selectionChanged = false;
    for (std::size_t i = 0; i < descendingSortedIndices.size(); ++i) {
        const int index = descendingSortedIndices[i];
        if (table_.rowCount() - index == int(i + 1))
            continue; // Nowhere to move.
        table_.insertRow(index + 2);
        moveRow(table_, index, index + 2);
        table_.removeRow(index);
        ++descendingSortedIndices[i];
        selectionChanged = true;
    }
    if (selectionChanged)
        selectRows(table_, descendingSortedIndices);
}

void CustomActionsPage::onCellChanged(const int row, const int column)
{
# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS_PAGE
    std::cout << "Custom actions table: cell (" << row << ',' << column
              << ") changed." << std::endl;
# endif
    if (column == Columns::enabled) {
        const bool blocked = table_.blockSignals(true);
        const bool enabled =
            table_.item(row, column)->checkState() == Qt::Checked;
        assert(Columns::enabled == 0);
        for (int col = 1; col < Columns::n; ++col) {
            QTableWidgetItem * const item = table_.item(row, col);
            if (item == nullptr)
                table_.cellWidget(row, col)->setEnabled(enabled);
            else {
                const Qt::ItemFlags flags = item->flags();
                constexpr Qt::ItemFlag enabledFlag = Qt::ItemIsEnabled;
                if (bool(flags & enabledFlag) != enabled)
                    item->setFlags(flags ^ enabledFlag);
            }
        }
        table_.blockSignals(blocked);
    }
}

void CustomActionsPage::onShowHelpToggled()
{
    if (helpFrame_ == nullptr) {
        helpFrame_.reset(new QFrame);
        QVBoxLayout * const layout = new QVBoxLayout(helpFrame_.get());

        layout->addWidget(
            QtUtilities::Widgets::getCaption(tr("Columns description"),
                                             helpFrame_.get(), true, 0),
            0, Qt::AlignCenter);

        int number = 0;
        const auto al = [&](const QString & text) {
            addLabel(text.arg(++number), helpFrame_.get(), layout);
        };

        al(tr("%1. <i>Enabled</i>: disabled actions are not shown in menu."));
        al(tr("%1. <i>Text</i> is displayed in the menu."));
        al(tr("%1. <i>Command</i> is executed when this action is triggered. "
              "The following rules apply to <i>Command</i> text:<ul>"
              "<li>most Bash shell rules with respect to double and single "
              "quotes, backslash, whitespaces;</li>"
              "<li>'?' symbol is replaced with absolute paths to items unless "
              "escaped with '\\' or enclosed in single quotes;</li>"
              "<li>'~' symbol is replaced with current user's HOME directory."
              "</li></ul>"));
        al(tr("%1,%2. <i>Min</i>, <i>Max</i> - minimum and maximum number of "
              "selected items respectively. Custom action is available only if "
              "the number of selected items is between <i>Min</i> and "
              "<i>Max</i>. <i>Max</i>=-1 means \"without upper bound\".").arg(
               ++number));
        al(tr("%1. <i>Type</i> - allowed type of selected items."));
        al(tr("%1. <i>Comment</i> - field that is not used by the program. "
              "It can be used to provide user with additional information "
              "about custom action."));

        this->layout()->addWidget(helpFrame_.get());
    }
    else
        helpFrame_.reset();
}
