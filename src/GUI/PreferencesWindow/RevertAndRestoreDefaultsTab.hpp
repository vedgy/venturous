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

# ifndef VENTUROUS_REVERT_AND_RESTORE_DEFAULTS_TAB_HPP
# define VENTUROUS_REVERT_AND_RESTORE_DEFAULTS_TAB_HPP

# include <QtGlobal>
# include <QWidget>
# include <QComboBox>


QT_FORWARD_DECLARE_CLASS(QStringList)
QT_FORWARD_DECLARE_CLASS(QIcon)

class RevertAndRestoreDefaultsTab : public QWidget
{
    Q_OBJECT
public:
    explicit RevertAndRestoreDefaultsTab(
        const QStringList & tabNames, const QIcon & revertIcon,
        const QIcon & restoreDefaultsIcon,
        QWidget * parent = nullptr, Qt::WindowFlags f = 0);

signals:
    /// @param tab Specifies tab, for which preferences must be restored.
    /// If (tab == tabNames.size()), all tabs must be restored.
    void revertPreferences(int tab);
    /// @param tab Specifies tab, for which preferences must be set from
    /// defaults.
    /// If (tab == tabNames.size()), all tabs must be set from defaults.
    void restoreDefaultPreferences(int tab);

private:
    QComboBox tabComboBox;

private slots:
    void onRevertClicked();
    void onRestoreDefaultsClicked();
};

# endif // VENTUROUS_REVERT_AND_RESTORE_DEFAULTS_TAB_HPP
