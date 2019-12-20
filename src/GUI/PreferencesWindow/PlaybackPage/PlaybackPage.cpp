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

# include "PlaybackPage.hpp"

# include "Preferences.hpp"

# include <VenturousCore/MediaPlayer.hpp>

# include <QtWidgetsUtilities/Miscellaneous.hpp>

# include <QString>
# include <QStringList>
# include <QFormLayout>
# include <QFrame>


namespace
{
constexpr double msInS = 1E3;
constexpr int lgMsInS = 3;

}


PlaybackPage::PlaybackPage(QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QFormLayout * const layout = new QFormLayout(this);

    playerIdComboBox.addItems(GetMediaPlayer::playerList());
    playerIdComboBox.setToolTip(
        tr("Selected player will be used for playing items.\n"
           "It must be installed.\n"
           "Some of the player's settings may be adjusted\n"
           "automatically for integration with %1.").arg(APPLICATION_NAME));
    QtUtilities::Widgets::setFixedSizePolicy(& playerIdComboBox);
    onPlayerIdChanged(0);
    connect(& playerIdComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(onPlayerIdChanged(int)));
    layout->addRow(tr("External player"), & playerIdComboBox);

    autoSetOptionsCheckBox.setToolTip(
        tr("If checked, essential external player options\n"
           "would be set each time player is launched.\n"
           "Otherwise, it is user's responsibility to ensure\n"
           "that external player options are correct."));
    layout->addRow(tr("Always set external player options"),
                   & autoSetOptionsCheckBox);

    autoHideWindowCheckBox.setToolTip(
        tr("If checked, external player window would be hidden\n"
           "each time player is launched."));
    layout->addRow(tr("Always hide external player window"),
                   & autoHideWindowCheckBox);

    layout->addRow(tr("Exit external player on quit"),
                   & exitPlayerOnQuitCheckBox);


    statusUpdateCheckBox.setToolTip(
        tr("If checked, playback status would be updated\n"
           "at the specified interval."));
    layout->addRow(tr("Update status regularly"), & statusUpdateCheckBox);

    statusUpdateWarningLabel.setWordWrap(true);
    statusUpdateWarningLabel.setIndent(QtUtilities::Widgets::subWidgetIndent());
    statusUpdateWarningLabel.setText(
        "<font color='red'>"
        + tr("WARNING: regular updating playback status increases\n"
             "%1 CPU usage in idle state significantly.").
        arg(APPLICATION_NAME) + "</font>");

    {
        using P = Preferences::Playback;
        statusUpdateSpinBox.setDecimals(lgMsInS);
        statusUpdateSpinBox.setRange(P::minStatusUpdateInterval / msInS,
                                     P::maxStatusUpdateInterval / msInS);
        statusUpdateSpinBox.setValue(P::defaultStatusUpdateInterval / msInS);
    }
    statusUpdateSpinBox.setSingleStep(0.1);
    statusUpdateSpinBox.setToolTip(tr("Time interval at which status of the\n"
                                      "external player will be checked."));
    statusUpdateSpinBox.setSuffix("s");
    QtUtilities::Widgets::setFixedSizePolicy(& statusUpdateSpinBox);
    QtUtilities::Widgets::addSubWidget(layout, tr("Status update interval"),
                                       & statusUpdateSpinBox);

    onStatusUpdateToggled(statusUpdateCheckBox.isChecked());
    connect(& statusUpdateCheckBox, SIGNAL(toggled(bool)),
            SLOT(onStatusUpdateToggled(bool)));


    nextFromHistoryCheckBox.setToolTip(
        tr("If checked, \"Next\" action first tries to get next item\n"
           "from history; if it is not available, plays random item.\n"
           "Otherwise, \"Next\" action always plays random item."));
    layout->addRow(tr("Next from history"), & nextFromHistoryCheckBox);


    skipRecentHistoryItemCountSpinBox.setRange(
        0, Preferences::Playback::maxSkipRecentHistoryItemCount);
    skipRecentHistoryItemCountSpinBox.setToolTip(
        tr("The number of most recent items in history,\n"
           "which will not be selected for random playback."));
    QtUtilities::Widgets::setFixedSizePolicy(
        & skipRecentHistoryItemCountSpinBox);
    layout->addRow(tr("Skip recent history item count"),
                   & skipRecentHistoryItemCountSpinBox);


    desktopNotificationsCheckBox.setToolTip(tr(
            "If checked, desktop notifications would be shown after\n"
            "changing played item\n"
            "(requires notify-send executable (from libnotify) in PATH)."));
    layout->addRow(tr("Desktop notifications"), & desktopNotificationsCheckBox);


    startupPolicyComboBox.addItems( {
        tr("<no action>"), tr("Start playback"), tr("Replay last item"),
        tr("Play next random item"), tr("Play next item")
    });
    startupPolicyComboBox.setToolTip(
        tr("Selected action would be executed on each %1 start.").arg(
            APPLICATION_NAME));
    QtUtilities::Widgets::setFixedSizePolicy(& startupPolicyComboBox);
    layout->addRow(tr("Startup action"), & startupPolicyComboBox);

    QtUtilities::Widgets::addSpacing(layout);

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

    {
        const int interval = int(playback.statusUpdateInterval);
        statusUpdateCheckBox.setChecked(interval != 0);
        if (interval != 0)
            statusUpdateSpinBox.setValue(interval / msInS);
    }

    nextFromHistoryCheckBox.setChecked(playback.nextFromHistory);
    skipRecentHistoryItemCountSpinBox.setValue(
            static_cast<int>(playback.skipRecentHistoryItemCount));
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

    playback.statusUpdateInterval =
        statusUpdateCheckBox.isChecked() ?
        unsigned(statusUpdateSpinBox.value() * msInS + 0.5) : 0u;

    playback.nextFromHistory = nextFromHistoryCheckBox.isChecked();
    playback.skipRecentHistoryItemCount =
        static_cast<unsigned>(skipRecentHistoryItemCountSpinBox.value());
    playback.desktopNotifications = desktopNotificationsCheckBox.isChecked();
    playback.startupPolicy = static_cast<Preferences::Playback::StartupPolicy>(
                                 startupPolicyComboBox.currentIndex());

    historyFrame_.writeUiPreferencesTo(playback.history);
}


void PlaybackPage::onPlayerIdChanged(const int newId)
{
    const bool enabled = GetMediaPlayer::isExternalPlayerProcessDetached(newId);
    exitPlayerOnQuitCheckBox.setEnabled(enabled);
    exitPlayerOnQuitCheckBox.setToolTip(
        (enabled ?
         tr("If checked, external player would be exited when\n"
            "%1 quits; otherwise it would remain running.") :
         tr("Current external player must be finished when %1 quits.\n"
            "So this option has no effect.")
        ).arg(APPLICATION_NAME));
}

void PlaybackPage::onStatusUpdateToggled(const bool checked)
{
    statusUpdateWarningLabel.setVisible(checked);

    constexpr int row = 5;
    QFormLayout & layout = * static_cast<QFormLayout *>(this->layout());
    const bool warningInLayout =
        layout.itemAt(row, QFormLayout::SpanningRole) != nullptr;
    if (checked) {
        if (! warningInLayout)
            layout.insertRow(row, & statusUpdateWarningLabel);
    }
    else {
        if (warningInLayout)
            layout.removeWidget(& statusUpdateWarningLabel);
    }

    statusUpdateSpinBox.setEnabled(checked);
}
