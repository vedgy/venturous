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

# include "PreferencesWindow.hpp"

# include "GeneralPage.hpp"
# include "PlaybackPage.hpp"
# include "AddingDirectoryPage.hpp"
# include "CustomActionsPage.hpp"
# include "Preferences.hpp"

# include <QString>
# include <QWidget>
# include <QScrollArea>
# include <QTabWidget>
# include <QKeyEvent>

# include <functional>
# include <algorithm>


namespace
{
void addScrollableTab(QTabWidget * parent,
                      QWidget * widget, const QString & name)
{
    QScrollArea * const scrollArea = new QScrollArea(parent);
    scrollArea->setWidget(widget);
    scrollArea->setWidgetResizable(true);
    parent->addTab(scrollArea, name);
}

}


PreferencesWindow::PreferencesWindow(
    Preferences & preferences, const Icons & icons, QWidget * const parent)
    : QTabWidget(parent), preferences_(preferences),
      tabs_ {{
        new GeneralPage, new PlaybackPage, new AddingDirectoryPage(icons.add),
        new CustomActionsPage(icons.add, icons.remove)
    }
} {
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModal);
    setMinimumSize(200, 200);
    setWindowTitle(tr("%1 preferences").arg(APPLICATION_NAME));
    addScrollableTab(this, tabs_[0], tr("&General"));
    addScrollableTab(this, tabs_[1], tr("&Playback"));
    addScrollableTab(this, tabs_[2], tr("&Adding directory"));
    addScrollableTab(this, tabs_[3], tr("&Custom actions"));
}


void PreferencesWindow::setUiPreferences()
{
    std::for_each(tabs_.cbegin(), tabs_.cend(),
                  std::bind(& PreferencesPage::setUiPreferences,
                            std::placeholders::_1, std::cref(preferences_)));
    restoreGeometry(preferences_.preferencesWindowGeometry);
}


void PreferencesWindow::keyPressEvent(QKeyEvent * const event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QTabWidget::keyPressEvent(event);
}

void PreferencesWindow::closeEvent(QCloseEvent *)
{
    std::for_each(tabs_.cbegin(), tabs_.cend(),
                  std::bind(& PreferencesPage::writeUiPreferencesTo,
                            std::placeholders::_1, std::ref(preferences_)));
    preferences_.preferencesWindowGeometry = saveGeometry();
    emit preferencesUpdated();
}
