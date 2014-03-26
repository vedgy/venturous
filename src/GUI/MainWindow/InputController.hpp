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

# ifndef VENTUROUS_INPUT_CONTROLLER_HPP
# define VENTUROUS_INPUT_CONTROLLER_HPP

# include <QString>
# include <QMessageBox>


class InputController
{
public:
    /// @brief Blocks/unblocks most of non-GUI user input (for example Ventool
    /// commands).
    /// Reacting to QCoreApplication::aboutToQuit - immediate quit - is allowed.
    /// Reacting to MediaPayer::FinishedSlot is also allowed.
    /// @param block If true, input will be blocked; if false - unblocked.
    virtual void blockInput(bool block) = 0;

    /// @brief Shows message box and handles input blocking/unblocking.
    /// @param title Title of the message box.
    /// @param text Text of the message box.
    /// @param buttons Buttons to be displayed in the message box.
    /// @param defaultButton Button, which will be set as default and escape
    /// button of the message box.
    /// @param icon Message box's icon.
    /// @return If message box was destroyed too early, QMessageBox::NoButton.
    /// In this case it is advised to return from calling method immediately,
    /// because *everything* is probably already destroyed.
    /// If message box was not destroyed beforehand, returns button, selected by
    /// user.
    virtual int showMessage(
        const QString & title, const QString & text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton,
        QMessageBox::Icon icon = QMessageBox::Critical) = 0;

protected:
    ~InputController() = default;
};

# endif // VENTUROUS_INPUT_CONTROLLER_HPP
