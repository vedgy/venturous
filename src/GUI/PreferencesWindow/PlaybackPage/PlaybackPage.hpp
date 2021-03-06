/*
 This file is part of Venturous.
 Copyright (C) 2014, 2019 Igor Kushnir <igorkuo AT Google mail>

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

# ifndef VENTUROUS_PLAYBACK_PAGE_HPP
# define VENTUROUS_PLAYBACK_PAGE_HPP

# include "HistoryPreferencesFrame.hpp"
# include "PreferencesPage.hpp"

# include <QLabel>
# include <QSpinBox>
# include <QDoubleSpinBox>
# include <QCheckBox>
# include <QComboBox>


class PlaybackPage : public PreferencesPage
{
    Q_OBJECT
public:
    explicit PlaybackPage(QWidget * parent = nullptr, Qt::WindowFlags f = 0);

    void setUiPreferences(const Preferences & source) override;
    void writeUiPreferencesTo(Preferences & destination) const override;

private:
    QComboBox playerIdComboBox;
    QCheckBox autoSetOptionsCheckBox;
    QCheckBox autoHideWindowCheckBox;
    QCheckBox exitPlayerOnQuitCheckBox;
    QCheckBox statusUpdateCheckBox;
    QLabel statusUpdateWarningLabel;
    QDoubleSpinBox statusUpdateSpinBox;
    QCheckBox nextFromHistoryCheckBox;
    QSpinBox skipRecentHistoryItemCountSpinBox;
    QCheckBox desktopNotificationsCheckBox;
    QComboBox startupPolicyComboBox;
    HistoryPreferencesFrame historyFrame_;

private slots:
    void onPlayerIdChanged(int newId);
    void onStatusUpdateToggled(bool checked);
};

# endif // VENTUROUS_PLAYBACK_PAGE_HPP
