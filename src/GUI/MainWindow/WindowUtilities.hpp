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

# ifndef VENTUROUS_WINDOW_UTILITIES_HPP
# define VENTUROUS_WINDOW_UTILITIES_HPP

# include "InputController.hpp"

# include <QWidget>


namespace WindowUtilities
{
void showWindow(QWidget & window);
void showAndActivateWindow(QWidget & window);
}

class WindowInputController : public InputController
{
public:
    /// @param window Window to be automatically shown when blocking and to be
    /// used as message box parent.
    explicit WindowInputController(QWidget & window);

    bool blocked() const { return blocked_; }

    /// @brief Calls showWindow(window_) if block == true.
    /// Sets blocked_ to block.
    void blockInput(bool block) override;

    int showMessage(
        const QString & title, const QString & text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton,
        QMessageBox::Icon icon = QMessageBox::Critical) override;

private:
    QWidget & window_;
    /// Blocked input flag.
    bool blocked_ = false;
};

# endif // VENTUROUS_WINDOW_UTILITIES_HPP
