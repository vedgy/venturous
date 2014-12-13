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

# include "PreferencesWindow.hpp"
# include "Icons.hpp"

# include <QtWidgetsUtilities/Miscellaneous.hpp>
# include <QtWidgetsUtilities/HandleErrors.hpp>

# include <QtCoreUtilities/Error.hpp>

# include <CommonUtilities/ExceptionsToStderr.hpp>

# include <QString>
# include <QFileInfo>

# include <cassert>


PreferencesComponent::PreferencesComponent(
    QtUtilities::Widgets::InputController & inputController,
    const QString & preferencesDir, bool & cancelled)
    : preferences(), inputController_(inputController),
      preferencesFilename_(preferencesDir + APPLICATION_NAME ".xml"),
      savedPreferences_(preferences)
{
# ifdef DEBUG_VENTUROUS_PREFERENCES_COMPONENT
    std::cout << "preferencesFilename_ = " <<
              QtUtilities::qStringToString(preferencesFilename_) << std::endl;
# endif

    if (QFileInfo(preferencesFilename_).isFile()) {
        if (handlePreferencesErrors([this] {
        savedPreferences_.load(preferencesFilename_);
        }, tr("Loading preferences failed"), false, & cancelled)) {
            preferences = savedPreferences_;
        }
        else
            savedPreferences_ = preferences;
    }
    else
        cancelled = false;
}

PreferencesComponent::~PreferencesComponent()
{
    CommonUtilities::exceptionsToStderr([this] {
        savePreferences(true);
    }, VENTUROUS_ERROR_PREFIX "In ~PreferencesComponent(): ");
}

void PreferencesComponent::setTheme(const Icons::Theme & theme)
{
    icons_.add = theme.add();
    icons_.remove = theme.remove();
    icons_.undo = theme.undo();
    icons_.revert = theme.revert();
}

void PreferencesComponent::showPreferencesWindow(QWidget * const parent)
{
    if (preferencesWindow_ == nullptr) {
        preferencesWindow_.reset(
            new PreferencesWindow(preferences, icons_, parent));
        preferencesWindow_->setUiPreferences();
        connect(preferencesWindow_.get(), SIGNAL(preferencesUpdated()),
                SLOT(onPreferencesUpdated()));
        preferencesWindow_->show();
    }
    else
        QtUtilities::Widgets::showAndActivateWindow(preferencesWindow_.get());
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



bool PreferencesComponent::handlePreferencesErrors(
    Function f, const QString & errorPrefix, const bool silentMode,
    bool * const cancelled)
{
    QtUtilities::Widgets::HandleErrors handleErrors {
        [&]() -> QString {
            try {
                f();
                return QString();
            }
            catch (const QtUtilities::Error & error) {
                return errorPrefix + ": " + QString::fromUtf8(error.what());
            }
        }
    };
    return silentMode ? handleErrors.nonBlocking()
           : handleErrors.blocking(inputController_, tr("Preferences error"),
                                   cancelled);
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
