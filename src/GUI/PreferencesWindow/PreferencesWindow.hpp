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

# ifndef VENTUROUS_PREFERENCES_WINDOW_HPP
# define VENTUROUS_PREFERENCES_WINDOW_HPP

# include <QIcon>
# include <QTabWidget>

# include <array>


class PreferencesPage;
class Preferences;

/// Allows user to change preferences. Never blocks execution.
class PreferencesWindow : public QTabWidget
{
    Q_OBJECT
public:
    struct Icons {
        QIcon add, remove, undo, revert;
    };

    explicit PreferencesWindow(Preferences & preferences, const Icons & icons,
                               QWidget * parent = nullptr);

    /// @brief Configures UI according to preferences_.
    void setUiPreferences();

signals:
    /// @brief Is emitted after preferences_ value is updated (from closeEvent).
    /// NOTE: execution may be blocked by signal receiver.
    void preferencesUpdated();

private:
    /// @brief Calls setUiPreferences(source) for all tabs_.
    void setUiPreferences(const Preferences & source);
    /// @brief Sets UI preferences of specified tab from source.
    /// If (tab == tabs_.size()), calls setUiPreferences(source).
    void setUiPreferences(int tab, const Preferences & source);

    void keyPressEvent(QKeyEvent *) override;
    void closeEvent(QCloseEvent *) override;

    Preferences & preferences_;

    const std::array<PreferencesPage *, 4> tabs_;

private slots:
    /// @brief Calls setUiPreferences(tab, preferences_).
    void revertPreferences(int tab);
    /// @brief Calls setUiPreferences(tab, Preferences()).
    void restoreDefaultPreferences(int tab);
};

# endif // VENTUROUS_PREFERENCES_WINDOW_HPP
