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

# include "RevertAndRestoreDefaultsTab.hpp"

# include <QString>
# include <QStringList>
# include <QIcon>
# include <QSpacerItem>
# include <QGridLayout>
# include <QLabel>
# include <QPushButton>


namespace
{
void addButton(const QIcon & icon, const QString & text,
               const QString & tooltip, QWidget * parent,
               const char * slot, QGridLayout * layout, int row)
{
    QPushButton * const button = new QPushButton(icon, text, parent);
    button->setToolTip(tooltip);
    parent->connect(button, SIGNAL(clicked(bool)), slot);
    layout->addWidget(button, row, 1);
}

}


RevertAndRestoreDefaultsTab::RevertAndRestoreDefaultsTab(
    const QStringList & tabNames, const QIcon & revertIcon,
    const QIcon & restoreDefaultsIcon,
    QWidget * const parent, const Qt::WindowFlags f) : QWidget(parent, f)
{
    QGridLayout * const layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("Target tab")), 0, 0);
    layout->addWidget(new QLabel(tr("Action")), 0, 1);

    tabComboBox.addItems(tabNames);
    tabComboBox.addItem("<all>");
    tabComboBox.setToolTip(tr("Clicking one of the buttons will apply\n"
                              "selected action to the specified tab."));
    layout->addWidget(& tabComboBox, 2, 0);

    addButton(revertIcon, tr("Revert"),
              tr("Revert changes that were just made."),
              this, SLOT(onRevertClicked()), layout, 1);
    addButton(restoreDefaultsIcon, tr("Restore defaults"),
              tr("Restore default values.\n"
                 "Note: changes caused by this action can be reverted."),
              this, SLOT(onRestoreDefaultsClicked()), layout, 3);

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding), 0, 2);
    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum,
                                    QSizePolicy::Expanding),
                    4, 0);
}


void RevertAndRestoreDefaultsTab::onRevertClicked()
{
    emit revertPreferences(tabComboBox.currentIndex());
}

void RevertAndRestoreDefaultsTab::onRestoreDefaultsClicked()
{
    emit restoreDefaultPreferences(tabComboBox.currentIndex());
}
