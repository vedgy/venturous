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

# ifndef VENTUROUS_PREFERENCES_PAGE_HPP
# define VENTUROUS_PREFERENCES_PAGE_HPP

# include <QWidget>


class Preferences;

class PreferencesPage : public QWidget
{
public:
    explicit PreferencesPage(QWidget * parent = nullptr,
                             Qt::WindowFlags f = 0) : QWidget(parent, f) {}

    virtual ~PreferencesPage() {}

    /// @brief Sets UI values from source.
    virtual void setUiPreferences(const Preferences & source) = 0;

    /// @brief Copies preferenses that are handled by this widget,
    /// from UI to destination.
    virtual void writeUiPreferencesTo(Preferences & destination) const = 0;
};

# endif // VENTUROUS_PREFERENCES_PAGE_HPP
