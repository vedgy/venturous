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

# include "WindowUtilities.hpp"

# include <QPointer>
# include <QWidget>
# include <QMessageBox>


namespace WindowUtilities
{
void showWindow(QWidget & window)
{
    const Qt::WindowStates state = window.windowState();
    if (state & Qt::WindowMinimized) {
        if (state & Qt::WindowMaximized)
            window.showMaximized();
        else
            window.showNormal();
    }
    else
        window.show();
}

void showAndActivateWindow(QWidget & window)
{
    showWindow(window);
    window.activateWindow();
}

}


WindowInputController::WindowInputController(QWidget & window) : window_(window)
{
}

void WindowInputController::blockInput(const bool block)
{
    if (blocked_ != block) {
        blocked_ = block;
        if (block)
            WindowUtilities::showWindow(window_);
    }
}

int WindowInputController::showMessage(
    const QString & title, const QString & text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton, QMessageBox::Icon icon)
{
    blockInput(true);

    QPointer<QMessageBox> messageBox = new QMessageBox(
        icon, title, text, buttons, & window_);
    messageBox->setDefaultButton(defaultButton);
    messageBox->setEscapeButton(defaultButton);
    messageBox->setAttribute(Qt::WA_DeleteOnClose, false);

    const auto selectedButton = messageBox->exec();
    if (messageBox == nullptr)
        return QMessageBox::NoButton; // window_ is already destroyed!

    messageBox->deleteLater();
    blockInput(false);
    return selectedButton;
}
