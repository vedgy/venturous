/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015 Igor Kushnir <igorkuo AT Google mail>

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

# include <QtGlobal>
# include <QString>

# include <vector>


namespace QtUtilities
{
namespace Widgets
{
class TooltipShower;
}
}
QT_FORWARD_DECLARE_CLASS(QPoint)
QT_FORWARD_DECLARE_CLASS(QStringList)

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

    static Action getEmpty() {
        return {
            QString(), QString(), 1, 1, Action::Type::anyItem, false, QString()
        };
    }


    /// Is displayed in the menu.
    QString text;
    /// Is executed if this action is triggered. '?' symbol is replaced with
    /// the list of absolute paths to items; '@' symbol is replaced with
    /// commonItemPrefix; there are some other quoting/escaping rules that
    /// mimic Bash shell syntax.
    QString command;
    int minArgN, maxArgN;
    Type type;
    bool enabled;
    /// Is not used by CustomActions.
    /// Can be used for detailed action description.
    QString comment;
};

bool operator == (const Action & lhs, const Action & rhs);

inline bool operator != (const Action & lhs, const Action & rhs)
{
    return !(lhs == rhs);
}

typedef std::vector<Action> Actions;

/// @brief Selects displayable elements of actions and shows popup menu with
/// them at the specified position.
/// Executes appropriate command if user selects some action.
/// @param commonItemPrefix The beginning of absolute path that is common for
/// all items.
/// @param itemNames Collection of individual item's names with
/// commonItemPrefix omitted.
/// @return true if popup menu was shown, false if there were no displayable
/// actions.
bool showMenu(const Actions & actions, QString commonItemPrefix,
              QStringList itemNames, const QPoint & position);
/// @brief Calls showMenu(actions, commonItemPrefix, itemNames, position).
/// If it returns false, uses tooltipShower to show appropriate tooltip at
/// the same position.
void showMenu(const Actions & actions, QString commonItemPrefix,
              QStringList itemNames, const QPoint & position,
              QtUtilities::Widgets::TooltipShower & tooltipShower);

}

# endif // VENTUROUS_CUSTOM_ACTIONS_HPP
