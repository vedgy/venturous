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

# ifdef DEBUG_VENTUROUS_PREFERENCES_COMPONENT
# include <QtCoreUtilities/String.hpp>
# include <iostream>
# endif

# include "PreferencesComponent.hpp"

# include "InputController.hpp"
# include "WindowUtilities.hpp"
# include "PreferencesWindow.hpp"

# include <QString>
# include <QFile>

# include <cassert>


PreferencesComponent::PreferencesComponent(
    InputController & inputController,
    const QString & preferencesDir)
    : inputController_(inputController),
      preferencesFilename_(preferencesDir + APPLICATION_NAME ".xml")
{
# ifdef DEBUG_VENTUROUS_PREFERENCES_COMPONENT
    std::cout << "preferencesFilename_ = " <<
              QtUtilities::qStringToString(preferencesFilename_) << std::endl;
# endif

    if (QFile::exists(preferencesFilename_)) {
        if (handlePreferencesErrors([this] {
        savedPreferences_.load(preferencesFilename_);
        }, tr("Loading preferences failed"))) {
            preferences = savedPreferences_;
        }
        else
            savedPreferences_ = preferences;
    }
}

PreferencesComponent::~PreferencesComponent()
{
    savePreferences(true);
}

void PreferencesComponent::showPreferencesWindow(
    const QIcon & addIcon, QWidget * const parent)
{
    if (preferencesWindow_ == nullptr) {
        preferencesWindow_.reset(
            new PreferencesWindow(preferences, addIcon, parent));
        preferencesWindow_->setUiPreferences();
        connect(preferencesWindow_.get(), SIGNAL(preferencesUpdated()),
                SLOT(onPreferencesUpdated()));
        preferencesWindow_->show();
    }
    else
        WindowUtilities::showAndActivateWindow(* preferencesWindow_);
}

void PreferencesComponent::closePreferencesWindow()
{
    if (preferencesWindow_ != nullptr)
        preferencesWindow_->close();
}

void PreferencesComponent::quit()
{
    closePreferencesWindow();
    savePreferences();
}



void PreferencesComponent::savePreferences(const bool silentMode)
{
    if (! silentMode)
        emit aboutToSave();
    if (preferences != savedPreferences_) {
        if (handlePreferencesErrors([this] {
        preferences.save(preferencesFilename_);
        }, tr("Saving preferences failed"), silentMode)) {
            savedPreferences_ = preferences;
        }
    }
}


void PreferencesComponent::onPreferencesUpdated()
{
    if (preferences.savePreferencesToDiskImmediately)
        savePreferences();
    assert(preferencesWindow_ != nullptr);
    preferencesWindow_.release()->deleteLater();
    /// WARNING: repeated execution blocking is possible here!
    emit preferencesChanged();
}
