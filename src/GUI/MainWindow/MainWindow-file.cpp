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

# include "WindowUtilities.hpp"
# include "PlaybackComponent.hpp"
# include "PreferencesWindow.hpp"


void MainWindow::onPreferencesUpdated()
{
    if (preferences_.savePreferencesToDiskImmediately &&
            preferences_ != savedPreferences_) {
        copyWindowGeometryAndStateToPreferences();
        preferences_.playback.history.currentIndex =
            playbackComponent_->currentHistoryEntryIndex();
        if (handlePreferencesErrors([this] {
        preferences_.save(preferencesFilename);
        }, savePreferencesErrorPrefix)) {
            savedPreferences_ = preferences_;
        }
    }
    isPreferencesWindowOpen_ = false;
    preferencesChanged();
}

void MainWindow::onPreferencesActivated()
{
    if (isPreferencesWindowOpen_) {
        WindowUtilities::showAndActivateWindow(* preferencesWindow_);
        return;
    }
    preferencesWindow_->setUiPreferences();
    isPreferencesWindowOpen_ = true;
    preferencesWindow_->show();
}

void MainWindow::onFileQuit()
{
    quitState_ = true;
    close();
}
