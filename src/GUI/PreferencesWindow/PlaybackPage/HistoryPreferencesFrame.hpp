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

# ifndef VENTUROUS_HISTORY_PREFERENCES_FRAME_HPP
# define VENTUROUS_HISTORY_PREFERENCES_FRAME_HPP

# include "Preferences.hpp"

# include <QFrame>
# include <QSpinBox>
# include <QCheckBox>


class HistoryPreferencesFrame : public QFrame
{
public:
    explicit HistoryPreferencesFrame(QWidget * parent = nullptr,
                                     Qt::WindowFlags f = 0);

    void setUiPreferences(const Preferences::Playback::History & source);
    void writeUiPreferencesTo(
        Preferences::Playback::History & destination) const;

private:
    QSpinBox maxSizeSpinBox;
    QCheckBox copyPlayedEntryToTopCheckBox;
    QCheckBox saveToDiskImmediatelyCheckBox;
    QSpinBox nHiddenDirsSpinBox;
};

# endif // VENTUROUS_HISTORY_PREFERENCES_FRAME_HPP
