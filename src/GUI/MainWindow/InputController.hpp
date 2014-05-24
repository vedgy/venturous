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
# include <QStringList>
# include <QMessageBox>
# include <QFileDialog>

# include <cassert>


class InputController
{
public:
    InputController() = default;

    /// @brief Blocks/unblocks most of non-GUI user input (Ventool commands
    /// for example).
    /// Reacting to QCoreApplication::aboutToQuit - immediate quit - is allowed.
    /// Reacting to MediaPayer::FinishedSlot is also allowed.
    /// @param block If true, input will be blocked; if false - unblocked.
    /// WARNING: recursive blocking and redundant unblocking are forbidden.
    virtual void blockInput(bool block) = 0;

    /// @brief Shows message box and handles input blocking/unblocking.
    /// @param title Title of the message box.
    /// @param text Text of the message box.
    /// @param buttons Buttons to be displayed in the message box.
    /// @param defaultButton Button that will be set as default and escape
    /// button of the message box unless it is equal to QMessageBox::NoButton.
    /// @param icon Message box's icon.
    /// @return Button that was selected by user. If QMessageBox::NoButton is
    /// returned, it means that message box was closed abnormally. It is advised
    /// to return from calling function without blocking execution in this case.
    int showMessage(
        const QString & title, const QString & text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
        QMessageBox::Icon icon = QMessageBox::Critical) {
        return showMessageImplementation(
                   title, text, buttons, defaultButton, icon);
    }

    /// @brief Asks user for file or directory.
    /// @param caption Caption of the file dialog.
    /// @return File or directory name, selected by user. If no item was
    /// selected, that is, operation was cancelled, empty string.
    /// WARNING: never pass fileMode equal to QFileDialog::ExistingFiles.
    QString getFileOrDirName(const QString & caption,
                             QFileDialog::AcceptMode acceptMode,
                             QFileDialog::FileMode fileMode) {
        assert(fileMode != QFileDialog::ExistingFiles);
        const QStringList names = getFileOrDirNames(
                                      caption, acceptMode, fileMode);
        assert(names.size() <= 1 &&
               "More than one name must never appear here!");
        return names.empty() ? QString() : names.back();
    }

    /// @brief Asks user for one or more files to be opened for reading.
    /// @param caption Caption of the file dialog.
    /// @return List of file names, selected by user. If no item was
    /// selected, that is, operation was cancelled, empty list.
    QStringList getOpenFileNames(const QString & caption) {
        return getFileOrDirNames(caption, QFileDialog::AcceptOpen,
                                 QFileDialog::ExistingFiles);
    }

protected:
    ~InputController() = default;

    virtual int showMessageImplementation(
        const QString & title, const QString & text,
        QMessageBox::StandardButtons buttons,
        QMessageBox::StandardButton defaultButton, QMessageBox::Icon icon) = 0;

    virtual QStringList getFileOrDirNames(const QString & caption,
                                          QFileDialog::AcceptMode acceptMode,
                                          QFileDialog::FileMode fileMode) = 0;

private:
    Q_DISABLE_COPY(InputController)
};

# endif // VENTUROUS_INPUT_CONTROLLER_HPP
