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

# ifndef VENTUROUS_CUSTOM_ACTIONS_PAGE_HPP
# define VENTUROUS_CUSTOM_ACTIONS_PAGE_HPP

# include "PreferencesPage.hpp"

# include <QFrame>
# include <QTableWidget>

# include <memory>


class Preferences;

class CustomActionsPage : public PreferencesPage
{
    Q_OBJECT
public:
    explicit CustomActionsPage(QWidget * parent = nullptr,
                               Qt::WindowFlags f = 0);

    void setUiPreferences(const Preferences & source) override;
    void writeUiPreferencesTo(Preferences & destination) const override;

private:
    void insertRow(int row);


    QTableWidget table_;
    std::unique_ptr<QFrame> helpFrame_;

private slots:
    void addRow();
    void insertRow();
    void removeSelectedRows();
    void moveSelectedRowsUp();
    void moveSelectedRowsDown();
    void onCellChanged(int row, int column);
    void onShowHelpToggled();
};

# endif // VENTUROUS_CUSTOM_ACTIONS_PAGE_HPP
