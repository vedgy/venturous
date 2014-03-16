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

# include "MainWindow.hpp"

# include "Icons.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtCoreUtilities/String.hpp>

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
            "index.html", APPLICATION_NAME ".png"
        }
    };
    const QString errorTitle = APPLICATION_NAME + tr(" help");
    const QString dirName = APPLICATION_NAME;

    QDir dir = QDir::temp();
    const QString dirErrorMessage =
        tr("Could not %3 %1 directory in %2.").arg(dirName, dir.absolutePath());

    if (! dir.exists(dirName) && ! dir.mkdir(dirName)) {
        QMessageBox::critical(this, errorTitle,
                              dirErrorMessage.arg(tr("create")));
        return;
    }
    if (! dir.cd(dirName)) {
        QMessageBox::critical(this, errorTitle,
                              dirErrorMessage.arg(tr("enter")));
        return;
    }

    for (const QString & filename : filenames) {
        if (! dir.exists(filename)) {
            const QString absoluteName = dir.filePath(filename);
            if (! QFile::copy(Icons::getAbsolutePath(filename), absoluteName)) {
                QMessageBox::critical(
                    this, errorTitle,
                    tr("Could not create file %1.").arg(absoluteName));
                return;
            }
        }
    }
    QDesktopServices::openUrl(QUrl(dir.filePath(filenames.front()),
                                   QUrl::StrictMode));
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(
        this, tr("About ") + APPLICATION_NAME,
        "<b>" APPLICATION_NAME "</b> " + tr("version") + " <b>" +
        QCoreApplication::applicationVersion() + "</b>" +
        tr(" - wrapper for %1, which manages playlist and queue.").arg(
            QtUtilities::toQString(MediaPlayer::playerName())) +
        QString::fromUtf8(
            "<br><br>© 2014 Igor Kushnir &lt;igorkuo AT Google mail&gt;.<br>") +
        "License: <a href='http://www.gnu.org/copyleft/gpl.html'>GNU GPL v3</a>"
        " or later.<br><br>" +
        tr("Report bugs and request features at ") +
        "<a href='https://github.com/vedgy/venturous/issues'>"
        "Venturous GitHub repository</a>.");
}
