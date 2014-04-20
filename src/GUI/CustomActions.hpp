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
# include <QObject>

# include <vector>


namespace CustomActions
{
/// @brief Atcion is displayed if (enabled && number_of_args >= minArgN &&
/// (maxArgN == -1 || number_of_args <= maxArgN) &&
/// (type == anyItem || all_arg's_types match type)).
struct Action {
    typedef unsigned char TypeUnderlyingType;
    enum class Type : TypeUnderlyingType
    {
        file = 0, directory, anyItem
    };
    static constexpr TypeUnderlyingType maxType = 2;
    static constexpr int minMinArgN = 0, maxMinArgN = 99,
                         minMaxArgN = -1, maxMaxArgN = maxMinArgN;

    /// Is displayed in the menu.
    QString text;
    /// Is executed if this action is triggered. '?' symbols are replaced with
    /// absolute paths to items; "??" is replaced with "?".
    QString command;
    int minArgN, maxArgN;
    Type type;
    /// Is not used by CustomActions.
    /// Can be used for detailed action description.
    QString comment;
    bool enabled;
};

bool operator == (const Action & lhs, const Action & rhs);

inline bool operator != (const Action & lhs, const Action & rhs)
{
    return !(lhs == rhs);
}

typedef std::vector<Action> Actions;

/// @brief Selects displayable elements of actions and shows popup menu with
/// them at the specified position.\n
/// Executes appropriate command if user selects some action.
/// @param commonItemPrefix The beginning of absolute path that is common for
/// all items.
/// @param itemNames Collection of individual item's names with
/// commonItemPrefix omitted.
/// @return true if popup menu was shown, false if there were no displayable
/// actions.
bool showMenu(const Actions & actions, QString commonItemPrefix,
              std::vector<QString> itemNames, const QPoint & position);

inline QString noActionsMessage()
{
    return QObject::tr("No custom actions are enabled for selected item(s).");
}

}

# endif // VENTUROUS_CUSTOM_ACTIONS_HPP
