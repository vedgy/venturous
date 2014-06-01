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

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
# include <iostream>
# endif


# include "MainWindow.hpp"

# include "InputController.hpp"
# include "Icons.hpp"

# include <QString>
# include <QDir>
# include <QFile>
# include <QUrl>
# include <QCoreApplication>
# include <QMessageBox>
# include <QDesktopServices>

# include <array>


void MainWindow::onHelpHelp()
{
    /// Names in Venturous resources.
    const std::array<QString, 2> filenames = {{
            "user-guide.html", APPLICATION_NAME ".png"
        }
    };
    const QString errorTitle = APPLICATION_NAME + tr(" help");
    const QString dirName =
        APPLICATION_NAME "-v" + QCoreApplication::applicationVersion();

    QDir dir = QDir::temp();
    const QString dirErrorMessage =
        tr("Could not %3 %1 directory in %2.").arg(dirName, dir.absolutePath());

    if (! dir.exists(dirName) && ! dir.mkdir(dirName)) {
        inputController_.showMessage(errorTitle,
                                     dirErrorMessage.arg(tr("create")));
        return;
    }
    if (! dir.cd(dirName)) {
        inputController_.showMessage(errorTitle,
                                     dirErrorMessage.arg(tr("enter")));
        return;
    }

    for (const QString & filename : filenames) {
        if (! dir.exists(filename)) {
            const QString absoluteName = dir.filePath(filename);
            if (! QFile::copy(Icons::getAbsolutePath(filename), absoluteName)) {
                inputController_.showMessage(
                    errorTitle,
                    tr("Could not create file %1.").arg(absoluteName));
                return;
            }
        }
    }
    const QString url = dir.filePath(filenames.front());
    if (! QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode))) {
        inputController_.showMessage(errorTitle,
                                     tr("Could not open URL: ") + url);
    }
}

void MainWindow::onHelpAbout()
{
    inputController_.blockInput(true);
    QMessageBox::about(
        this, tr("About ") + APPLICATION_NAME,
        "<b>" APPLICATION_NAME "</b> " + tr("version") + " <b>" +
        QCoreApplication::applicationVersion() + "</b>" +
        tr(" - random playback manager.") +
        QString::fromUtf8(
            "<br><br>© 2014 Igor Kushnir &lt;igorkuo AT Google mail&gt;.<br>") +
        "License: <a href='http://www.gnu.org/copyleft/gpl.html'>GNU GPL v3</a>"
        " or later.<br><br>" +
        tr("Report bugs and request features at ") +
        "<a href='https://github.com/vedgy/venturous/issues'>"
        "Venturous GitHub repository</a>.");

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "\"About\" message box was closed." << std::endl;
# endif
    inputController_.blockInput(false);
}
