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
# include "RevertAndRestoreDefaultsTab.hpp"
# include "Preferences.hpp"

# include <QString>
# include <QStringList>
# include <QWidget>
# include <QScrollArea>
# include <QTabWidget>
# include <QKeyEvent>

# include <cstddef>
# include <cassert>
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

    const QStringList tabNames { tr("General"), tr("Playback"),
              tr("Adding directory"), tr("Custom actions")
    };
    assert(int(tabs_.size()) == tabNames.size());
    for (std::size_t i = 0; i < tabs_.size(); ++i)
        addScrollableTab(this, tabs_[i], '&' + tabNames[int(i)]);

    RevertAndRestoreDefaultsTab * const revertTab =
        new RevertAndRestoreDefaultsTab(tabNames, icons.undo, icons.revert,
                                        this);
    connect(revertTab, SIGNAL(revertPreferences(int)),
            SLOT(revertPreferences(int)));
    connect(revertTab, SIGNAL(restoreDefaultPreferences(int)),
            SLOT(restoreDefaultPreferences(int)));
    addScrollableTab(this, revertTab, tr("&Revert/restore"));
}

void PreferencesWindow::setUiPreferences()
{
    setUiPreferences(preferences_);
    restoreGeometry(preferences_.preferencesWindowGeometry);
}



void PreferencesWindow::setUiPreferences(const Preferences & source)
{
    std::for_each(tabs_.cbegin(), tabs_.cend(),
                  std::bind(& PreferencesPage::setUiPreferences,
                            std::placeholders::_1, std::cref(source)));
}

void PreferencesWindow::setUiPreferences(
    const int tab, const Preferences & source)
{
    const std::size_t page = tab;
    if (page == tabs_.size())
        setUiPreferences(source);
    else
        tabs_[page]->setUiPreferences(source);
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


void PreferencesWindow::revertPreferences(const int tab)
{
    setUiPreferences(tab, preferences_);
}

void PreferencesWindow::restoreDefaultPreferences(const int tab)
{
    setUiPreferences(tab, Preferences());
}
