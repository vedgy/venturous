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

# include <VenturousCore/MediaPlayer.hpp>

# include <QString>
# include <QStringList>
# include <QSpacerItem>
# include <QFormLayout>
# include <QFrame>
# include <QComboBox>


namespace
{
inline void configure(QComboBox & combobox)
{
    combobox.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

}


PlaybackPage::PlaybackPage(QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QFormLayout * const layout = new QFormLayout(this);

    playerIdComboBox.addItems(GetMediaPlayer::playerList());
    playerIdComboBox.setToolTip(
        tr("Selected player will be used for playing items.\n"
           "It must be installed. Some of the player's settings\n"
           "will be adjusted for integration with %1.").arg(APPLICATION_NAME));
    configure(playerIdComboBox);
    layout->addRow(tr("External player"), & playerIdComboBox);
    onPlayerIdChanged(0);
    connect(& playerIdComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(onPlayerIdChanged(int)));


    autoSetOptionsCheckBox.setToolTip(
        tr("If checked, essential external player options\n"
           "are set each time player is launched.\n"
           "Otherwise, it is user's responsibility to ensure\n"
           "that external player options are correct."));
    layout->addRow(tr("Always set external player options"),
                   & autoSetOptionsCheckBox);

    autoHideWindowCheckBox.setToolTip(
        tr("If checked, external player window is hidden\n"
           "each time player is launched."));
    layout->addRow(tr("Always hide external player window"),
                   & autoHideWindowCheckBox);

    layout->addRow(tr("Exit external player on quit"),
                   & exitPlayerOnQuitCheckBox);


    nextFromHistoryCheckBox.setToolTip(
        tr("If checked, \"Next\" action first tries to get next item\n"
           "from history; if it is not available, plays random item.\n"
           "Otherwise, \"Next\" action always plays random item."));
    layout->addRow(tr("Next from history"), & nextFromHistoryCheckBox);

    desktopNotificationsCheckBox.setToolTip(tr(
            "If checked, desktop notifications will be shown when\n"
            "played item is changed (requires notify-send executable\n"
            "(from libnotify) in PATH)."));
    layout->addRow(tr("Desktop notifications"), & desktopNotificationsCheckBox);

    startupPolicyComboBox.addItems( {
        tr("<no action>"), tr("Start playback"), tr("Replay last item"),
        tr("Play next random item"), tr("Play next item")
    });
    startupPolicyComboBox.setToolTip(
        tr("Selected action will be executed on each %1 start.").arg(
            APPLICATION_NAME));
    configure(startupPolicyComboBox);
    layout->addRow(tr("Startup action"), & startupPolicyComboBox);

    layout->addItem(new QSpacerItem(1, 10));

    historyFrame_.setFrameStyle(QFrame::Panel | QFrame::Raised);
    historyFrame_.setLineWidth(2);
    layout->addRow(& historyFrame_);
}

void PlaybackPage::setUiPreferences(const Preferences & source)
{
    const Preferences::Playback & playback = source.playback;

    playerIdComboBox.setCurrentIndex(int(playback.playerId));
    autoSetOptionsCheckBox.setChecked(playback.autoSetExternalPlayerOptions);
    autoHideWindowCheckBox.setChecked(playback.autoHideExternalPlayerWindow);
    exitPlayerOnQuitCheckBox.setChecked(playback.exitExternalPlayerOnQuit);
    nextFromHistoryCheckBox.setChecked(playback.nextFromHistory);
    desktopNotificationsCheckBox.setChecked(playback.desktopNotifications);
    startupPolicyComboBox.setCurrentIndex(static_cast<int>(
            playback.startupPolicy));

    historyFrame_.setUiPreferences(playback.history);
}

void PlaybackPage::writeUiPreferencesTo(Preferences & destination) const
{
    Preferences::Playback & playback = destination.playback;

    playback.playerId = unsigned(playerIdComboBox.currentIndex());
    playback.autoSetExternalPlayerOptions = autoSetOptionsCheckBox.isChecked();
    playback.autoHideExternalPlayerWindow = autoHideWindowCheckBox.isChecked();
    playback.exitExternalPlayerOnQuit = exitPlayerOnQuitCheckBox.isChecked();
    playback.nextFromHistory = nextFromHistoryCheckBox.isChecked();
    playback.desktopNotifications = desktopNotificationsCheckBox.isChecked();
    playback.startupPolicy = static_cast<Preferences::Playback::StartupPolicy>(
                                 startupPolicyComboBox.currentIndex());

    historyFrame_.writeUiPreferencesTo(playback.history);
}


void PlaybackPage::onPlayerIdChanged(const int id)
{
    const bool enabled = GetMediaPlayer::isExternalPlayerProcessDetached(id);
    exitPlayerOnQuitCheckBox.setEnabled(enabled);
    exitPlayerOnQuitCheckBox.setToolTip(
        (enabled ?
         tr("If checked, external player is finished when\n"
            "%1 quits; otherwise it remains running.") :
         tr("Current external player must be finished when\n"
            "%1 quits. So this option has no effect.")
        ).arg(APPLICATION_NAME));

}
