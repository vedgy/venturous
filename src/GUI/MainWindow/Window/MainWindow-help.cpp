/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015, 2019 Igor Kushnir <igorkuo AT Google mail>

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

# include "Icons.hpp"

# include <QtGlobal>
# include <QString>
# include <QDir>
# include <QFile>
# include <QUrl>
# include <QObject>
# include <QCoreApplication>
# include <QMessageBox>
# include <QDesktopServices>

# include <array>


namespace
{
namespace ApplicationInfo
{
constexpr const char * sectionSeparator()
{
    return "<br><br>";
}

QString applicationVersion()
{
    return "<b>" APPLICATION_NAME "</b> " + QObject::tr("version")
           + " <b>" + QCoreApplication::applicationVersion() + "</b>"
           + QObject::tr(" - random playback manager.");
}

QString copyrightAndLicense()
{
    return QString::fromUtf8(
               "© 2014, 2015, 2017, 2019"
               " Igor Kushnir &lt;igorkuo AT Google mail&gt;.<br>")
           + "License: <a href='http://www.gnu.org/copyleft/gpl.html'>"
           "GNU GPL v3 or later</a>.";
}

QString compilerAndQtVersions()
{
    return QObject::tr("Compiled with ") +
# ifdef __VERSION__
#   if defined(__GNUG__) && ! defined(__clang__) && ! defined(__INTEL_COMPILER)
           "GCC "
#   endif
           __VERSION__
# else // __VERSION__
           QObject::tr("unknown compiler");
# endif // __VERSION__
           + QObject::tr(" and Qt version ") + QT_VERSION_STR ".<br>"
           + QObject::tr("Currently using Qt shared libraries version ")
           + qVersion() + '.';
}

QString webLink()
{
    return QObject::tr("Report bugs and request features at ") +
           "<a href='https://github.com/vedgy/venturous/issues'>"
           "Venturous GitHub repository</a>.";
}

} // END namespace ApplicationInfo
} // END unnamed namespace

void MainWindow::onHelpHelp()
{
    /// Names in Venturous resources.
    const std::array<QString, 2> filenames = {{
            "user-guide.html", ICON_NAME ".png"
        }
    };
    const QString errorTitle = APPLICATION_NAME + tr(" help");
    const QString dirName =
        ICON_NAME "-v" + QCoreApplication::applicationVersion();

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
    const QString absoluteName = dir.filePath(filenames.front());
    if (! QDesktopServices::openUrl(QUrl::fromLocalFile(absoluteName))) {
        inputController_.showMessage(errorTitle,
                                     tr("Could not open file ") + absoluteName);
    }
}

void MainWindow::onHelpAbout()
{
    inputController_.blockInput(true);
    using namespace ApplicationInfo;
    QMessageBox::about(
        this, tr("About ") + APPLICATION_NAME,
        applicationVersion() + sectionSeparator() + copyrightAndLicense()
        + sectionSeparator() + compilerAndQtVersions()
        + sectionSeparator() + webLink());

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "\"About\" message box was closed." << std::endl;
# endif
    inputController_.blockInput(false);
}
