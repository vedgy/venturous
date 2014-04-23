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

# include "GuiUtilities.hpp"
# include "CustomActions.hpp"
# include "Preferences.hpp"

# include <QString>
# include <QStringList>
# include <QObject>
# include <QModelIndex>
# include <QTableWidgetItem>
# include <QItemSelectionModel>
# include <QHBoxLayout>
# include <QVBoxLayout>
# include <QFrame>
# include <QLabel>
# include <QPushButton>
# include <QSpinBox>
# include <QCheckBox>
# include <QComboBox>
# include <QTableWidget>
# include <QKeyEvent>

# include <cstddef>
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

void setRow(QTableWidget & table, int row, const CustomActions::Action & action)
{
    typedef CustomActions::Action A;
    setText(table, row, 0, action.text);
    setText(table, row, 1, action.command);
    setInt(table, row, 2, action.minArgN, A::minMinArgN, A::maxMinArgN);
    setInt(table, row, 3, action.maxArgN, A::minMaxArgN, A::maxMaxArgN);
    {
        QComboBox * const c = new QComboBox;
        c->addItems( { QObject::tr("File"), QObject::tr("Directory"),
                       QObject::tr("Any item")
                     });
        c->setCurrentIndex(static_cast<int>(action.type));
        table.setCellWidget(row, 4, c);
    }
    setText(table, row, 5, action.comment);
    setBool(table, row, 6, action.enabled);
}

void addLabel(const QString & text, QWidget * parent, QVBoxLayout * layout)
{
    QLabel * const label = new QLabel(text, parent);
    label->setWordWrap(true);
    layout->addWidget(label);
}

void addButton(const QString & text, const QString & tooltip, QWidget * parent,
               const char * slot, QHBoxLayout * layout)
{
    QPushButton * const button = new QPushButton(text, parent);
    button->setToolTip(tooltip);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    parent->connect(button, SIGNAL(clicked(bool)), slot);
    layout->addWidget(button);
}

}


CustomActionsPage::CustomActionsPage(
    QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    table_.setSelectionBehavior(QAbstractItemView::SelectRows);
    table_.setColumnCount(7);
    table_.setHorizontalHeaderLabels( {
        tr("Text"), tr("Command"), tr("Min"), tr("Max"), tr("Type"),
        tr("Comment"), tr("Enabled")
    });
    connect(& table_, SIGNAL(cellChanged(int, int)),
            SLOT(onCellChanged(int, int)));

    QVBoxLayout * const layout = new QVBoxLayout(this);
    layout->addWidget(& table_);

    {
        QHBoxLayout * const actionsLayout = new QHBoxLayout;
        addButton(tr("Add row"), tr("Add row at the bottom of the table."),
                  this, SLOT(addRow()), actionsLayout);
        addButton(tr("Insert row"),
                  tr("Insert a new row before the first selected row\n"
                     "or at the bottom of the table if nothing is selected."),
                  this, SLOT(insertRow()), actionsLayout);
        addButton(tr("Remove selected"),
                  tr("Remove selected rows from the table."),
                  this, SLOT(removeSelectedRows()), actionsLayout);
        QCheckBox * const helpCheckBox = new QCheckBox(tr("Show help"), this);
        connect(helpCheckBox, SIGNAL(toggled(bool)), SLOT(onShowHelpToggled()));
        actionsLayout->addWidget(helpCheckBox);

        layout->addLayout(actionsLayout);
    }
}

void CustomActionsPage::setUiPreferences(const Preferences & source)
{
    const CustomActions::Actions & actions = source.customActions;
    table_.setRowCount(actions.size());
    for (std::size_t i = 0; i < actions.size(); ++i)
        setRow(table_, i, actions[i]);
    table_.resizeColumnsToContents();
}

void CustomActionsPage::writeUiPreferencesTo(Preferences & destination) const
{
    CustomActions::Actions & actions = destination.customActions;
    actions.resize(table_.rowCount());
    for (std::size_t i = 0; i < actions.size(); ++i) {
        CustomActions::Action & a = actions[i];
        const int row = int(i);
        a.text = table_.item(row, 0)->text();
        a.command = table_.item(row, 1)->text();
        a.minArgN =
            qobject_cast<QSpinBox *>(table_.cellWidget(row, 2))->value();
        a.maxArgN =
            qobject_cast<QSpinBox *>(table_.cellWidget(row, 3))->value();
        a.type = static_cast<CustomActions::Action::Type>(
                     qobject_cast<QComboBox *>(table_.cellWidget(row, 4))
                     ->currentIndex());
        a.comment = table_.item(row, 5)->text();
        a.enabled = table_.item(row, 6)->checkState() == Qt::Checked;
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
    const auto indices = table_.selectionModel()->selectedRows();
    if (indices.empty())
        return;

    std::vector<int> descendingSortedIndices(indices.size());
    std::transform(indices.begin(), indices.end(),
                   descendingSortedIndices.begin(),
                   std::bind(& QModelIndex::row, std::placeholders::_1));
    std::sort(descendingSortedIndices.begin(), descendingSortedIndices.end(),
              std::greater<int>());

    for (int index : descendingSortedIndices)
        table_.removeRow(index);
}

void CustomActionsPage::onCellChanged(const int row, const int column)
{
# ifdef DEBUG_VENTUROUS_CUSTOM_ACTIONS_PAGE
    std::cout << "Custom actions table: cell (" << row << ',' << column
              << ") changed." << std::endl;
# endif
    if (column == table_.columnCount() - 1) {
        const bool blocked = table_.blockSignals(true);
        const bool enabled =
            table_.item(row, column)->checkState() == Qt::Checked;
        for (int col = 0; col < column; ++col) {
            const auto item = table_.item(row, col);
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
            GuiUtilities::getCaptionLabel(tr("Columns description"),
                                          helpFrame_.get(), true, 0),
            0, Qt::AlignCenter);

        const auto al = [&](const QString & text) {
            addLabel(text, helpFrame_.get(), layout);
        };
        al(tr("1. <i>Text</i> is displayed in the menu."));
        al(tr("2. <i>Command</i> is executed if this action is triggered. "
              "'?' symbols are replaced with absolute paths to items; "
              "\"??\" is replaced with \"?\"."));
        al(tr("3,4. <i>Min</i>, <i>Max</i> - minimum and maximum number of "
              "selected items respectively. Custom action is available only if "
              "number of selected items is between <i>Min</i> and <i>Max</i>. "
              "<i>Max</i>=-1 means \"without upper bound\"."));
        al(tr("5. <i>Type</i> - allowed type of selected items."));
        al(tr("6. <i>Comment</i> - field that is not used by the program. "
              "It can be used to provide user with additional information "
              "about custom action."));
        al(tr("7. <i>Enabled</i>: disabled actions are not shown in menu."));

        this->layout()->addWidget(helpFrame_.get());
    }
    else
        helpFrame_.reset();
}
