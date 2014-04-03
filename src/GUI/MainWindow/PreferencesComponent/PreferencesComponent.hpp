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

# ifndef VENTUROUS_PREFERENCES_COMPONENT_HPP
# define VENTUROUS_PREFERENCES_COMPONENT_HPP

# include "InputController.hpp"
# include "Preferences.hpp"

# include <QtCoreUtilities/Error.hpp>
# include <QtCoreUtilities/String.hpp>

# include <QtGlobal>
# include <QString>
# include <QObject>
# include <QMessageBox>

# include <memory>
# include <iostream>


class PreferencesWindow;
QT_FORWARD_DECLARE_CLASS(QIcon)
QT_FORWARD_DECLARE_CLASS(QWidget)

/// WARNING: each method can block execution if not stated otherwise.
class PreferencesComponent : public QObject
{
    Q_OBJECT
public:
    /// NOTE: inputController must remain valid throughout this
    /// PreferencesComponent's lifetime.
    explicit PreferencesComponent(InputController & inputController,
                                  const QString & preferencesDir);
    /// NOTE: does not block execution.
    ~PreferencesComponent();

    /// NOTE: does not block execution.
    void showPreferencesWindow(const QIcon & addIcon, QWidget * parent);

    /// @brief Closes preferences window if it is open.
    void closePreferencesWindow();

    /// @brief Should be called before normal quit.
    void quit();

    Preferences preferences;

signals:
    /// @brief Is emitted after preferences are changed.
    /// NOTE: execution may be blocked by signal receiver.
    void preferencesChanged();

    /// @brief Is emitted before saving preferences to disk unless saving is
    /// performed in destructor. Internal options should be updated via
    /// preferences field in receiving slot.
    /// WARNING: signal receiver must not block execution.
    void aboutToSave();

private:
    /// @brief Calls callable object f, catches and handles QtUtilities::Error.
    /// @param errorPrefix Text that will be displayed before
    /// QtUtilities::Error::message().
    /// @param silentMode Makes a difference only in case of error.
    /// If true, error message is printed to stderr and method returns false.
    /// Otherwise, execution is blocked and user is allowed to retry operation.
    /// @return true on success, false on failure (error).
    template <typename F>
    bool handlePreferencesErrors(F f, const QString & errorPrefix,
                                 bool silentMode = false);

    /// @brief Saves preferences to disk if (preferences != savedPreferences_).
    /// @param silentMode If true, aboutToSave() is not emitted and execution
    /// is not blocked.
    void savePreferences(bool silentMode = false);


    InputController & inputController_;
    const QString preferencesFilename_;

    Preferences savedPreferences_;
    std::unique_ptr<PreferencesWindow> preferencesWindow_;

private slots:
    void onPreferencesUpdated();
};


template <typename F>
bool PreferencesComponent::handlePreferencesErrors(
    F f, const QString & errorPrefix, const bool silentMode)
{
    while (true) {
        try {
            f();
            return true;
        }
        catch (const QtUtilities::Error & error) {
            const QString message = errorPrefix + ": " + error.message();
            if (silentMode) {
                std::cerr << ERROR_PREFIX <<
                          QtUtilities::qStringToString(message) << std::endl;
                return false;
            }
            const auto selectedButton =
                inputController_.showMessage(
                    tr("Preferences error"), message,
                    QMessageBox::Retry | QMessageBox::Ignore,
                    QMessageBox::Ignore);

            if (selectedButton != QMessageBox::Retry)
                return false;
        }
    }
}

# endif // VENTUROUS_PREFERENCES_COMPONENT_HPP
