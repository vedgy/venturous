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

# ifndef VENTUROUS_ADDING_DIRECTORY_PAGE_HPP
# define VENTUROUS_ADDING_DIRECTORY_PAGE_HPP

# include "AddingPolicyFrame.hpp"
# include "PatternListWidget.hpp"
# include "PreferencesPage.hpp"

# include <QtGlobal>


class Preferences;
QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QIcon)
QT_FORWARD_DECLARE_CLASS(QGridLayout)

class AddingDirectoryPage : public PreferencesPage
{
    Q_OBJECT
public:
    explicit AddingDirectoryPage(const QIcon & addIcon,
                                 QWidget * parent = nullptr,
                                 Qt::WindowFlags f = 0);

    void setUiPreferences(const Preferences & source) override;
    void writeUiPreferencesTo(Preferences & destination) const override;

private:
    void addColumn(QGridLayout * layout, int column, const QString & caption,
                   PatternListWidget & patternList, const QIcon & addIcon);

    void addCopyButton(QGridLayout * layout, int row, const QString & iconName,
                       const QString & tooltip, const char * slot);

    AddingPolicyFrame addingPolicyFrame_;
    PatternListWidget filePatternList_, mediaDirFilePatternList_;

private slots:
    void copyLeft();
    void copyRight();
};

# endif // VENTUROUS_ADDING_DIRECTORY_PAGE_HPP
