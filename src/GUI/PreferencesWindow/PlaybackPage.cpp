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

# include "PlaybackPage.hpp"

# include "Preferences.hpp"

# include <QStringList>
# include <QSpacerItem>
# include <QFormLayout>
# include <QFrame>


PlaybackPage::PlaybackPage(QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QFormLayout * const layout = new QFormLayout(this);

    autoSetOptionsCheckBox.setToolTip(
        tr("If checked, recommended external player options are set each time "
           "player is launched.\n"
           "Otherwise, it is user's responsibility to ensure that "
           "external player options are correct."));
    layout->addRow(tr("Always set external player options"),
                   & autoSetOptionsCheckBox);

    nextFromHistoryCheckBox.setToolTip(
        tr("If checked, \"Next\" action first tries to get next item from "
           "history, if it is not available, plays random item.\n"
           "Otherwise, \"Next\" action always plays random item."));
    layout->addRow(tr("Next from history"), & nextFromHistoryCheckBox);

    startupPolicyComboBox.addItems( {
        tr("<no action>"), tr("Start playback"), tr("Play next item")
    });
    startupPolicyComboBox.setToolTip(
        tr("Selected action will be executed on each %1 start.").arg(
            APPLICATION_NAME));
    layout->addRow(tr("Startup action"), & startupPolicyComboBox);

    layout->addItem(new QSpacerItem(1, 10));

    historyFrame_.setFrameStyle(QFrame::Panel | QFrame::Raised);
    historyFrame_.setLineWidth(2);
    layout->addRow(& historyFrame_);
}

void PlaybackPage::setUiPreferences(const Preferences & source)
{
    const Preferences::Playback & playback = source.playback;

    autoSetOptionsCheckBox.setChecked(playback.autoSetExternalPlayerOptions);
    nextFromHistoryCheckBox.setChecked(playback.nextFromHistory);
    startupPolicyComboBox.setCurrentIndex(static_cast<int>(
            playback.startupPolicy));

    historyFrame_.setUiPreferences(playback.history);
}

void PlaybackPage::writeUiPreferencesTo(Preferences & destination) const
{
    Preferences::Playback & playback = destination.playback;

    playback.autoSetExternalPlayerOptions = autoSetOptionsCheckBox.isChecked();
    playback.nextFromHistory = nextFromHistoryCheckBox.isChecked();
    playback.startupPolicy = static_cast<Preferences::Playback::StartupPolicy>(
                                 startupPolicyComboBox.currentIndex());

    historyFrame_.writeUiPreferencesTo(playback.history);
}
