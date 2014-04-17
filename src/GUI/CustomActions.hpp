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

# ifndef VENTUROUS_CUSTOM_ACTIONS_HPP
# define VENTUROUS_CUSTOM_ACTIONS_HPP

# include <QPoint>
# include <QString>
# include <QStringList>

# include <vector>


namespace CustomActions
{
struct Action {
    typedef unsigned char TypeUnderlyingType;
    enum class Type : TypeUnderlyingType
    {
        file = 0, directory, anyItem
    };
    static constexpr TypeUnderlyingType maxType = 2;
    static constexpr int minMinArgN = 0, maxMinArgN = 99,
                         minMaxArgN = -1, maxMaxArgN = maxMinArgN;

    QString text, command;
    int minArgN, maxArgN;
    Type type;
    QString comment;
    bool enabled;
};

bool operator == (const Action & lhs, const Action & rhs);

inline bool operator != (const Action & lhs, const Action & rhs)
{
    return !(lhs == rhs);
}

typedef std::vector<Action> Actions;

void showMenu(const Actions & actions, QStringList arguments,
              const QPoint & position);

}

# endif // VENTUROUS_CUSTOM_ACTIONS_HPP
