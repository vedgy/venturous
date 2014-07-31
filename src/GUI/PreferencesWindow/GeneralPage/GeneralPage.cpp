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

# include "GeneralPage.hpp"

# include "Preferences.hpp"

# include <QtWidgetsUtilities/Miscellaneous.hpp>

# include <QString>
# include <QObject>
# include <QFormLayout>
# include <QCheckBox>

# include <limits>


namespace
{
void addNotificationAreaSubOption(
    QFormLayout * layout, QCheckBox & subOption,
    const QString & tooltip, const QString & labelPrefix)
{
    subOption.setToolTip(tooltip);
    QtUtilities::Widgets::addSubWidget(
        layout, labelPrefix + QObject::tr(" %1 to notification area").arg(
            APPLICATION_NAME),
        & subOption);
}

constexpr double msInS = 1E3;
constexpr int lgMsInS = 3;

}

GeneralPage::GeneralPage(QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QFormLayout * const layout = new QFormLayout(this);

    alwaysUseFallbackIconsCheckBox.setToolTip(tr("If checked, fallback "
            "(application default) icons would always be used;\n"
            "otherwise, icons from system theme would be preferred.\n"
            "Changing this option will take effect after restarting the "
            "application."));
    layout->addRow(tr("Always use fallback icons"),
                   & alwaysUseFallbackIconsCheckBox);

    QtUtilities::Widgets::addSpacing(layout);


    notificationAreaIconCheckBox.setToolTip(
        tr("Show %1 icon in notification area.").arg(APPLICATION_NAME));
    onNotificationAreaIconToggled(notificationAreaIconCheckBox.isChecked());
    connect(& notificationAreaIconCheckBox, SIGNAL(toggled(bool)),
            SLOT(onNotificationAreaIconToggled(bool)));
    layout->addRow(tr("Notification area icon"),
                   & notificationAreaIconCheckBox);

    addNotificationAreaSubOption(
        layout, startToNotificationAreaCheckBox,
        tr("If checked, %1 window is hidden on start.").arg(APPLICATION_NAME),
        tr("Start"));

    addNotificationAreaSubOption(
        layout, closeToNotificationAreaCheckBox,
        tr("If checked, %1 window is closed to notification area;\n"
           "otherwise, closing is equialent to quitting.").arg(
            APPLICATION_NAME),
        tr("Close"));

    QtUtilities::Widgets::addSpacing(layout);


    statusBarCheckBox.setToolTip(
        tr("Display status bar with the last played item.\n"
           "Note: this information is usually available in history window."));
    layout->addRow(tr("Status bar"), & statusBarCheckBox);

    QtUtilities::Widgets::addSpacing(layout);


    treeAutoUnfoldedLevelsSpinBox.setRange(
        0, std::numeric_limits < decltype(
            Preferences::treeAutoUnfoldedLevels) >::max());
    treeAutoUnfoldedLevelsSpinBox.setToolTip(tr("Number of items in playlist "
            "tree that would be unfolded by default."));
    QtUtilities::Widgets::setFixedSizePolicy(& treeAutoUnfoldedLevelsSpinBox);
    layout->addRow(tr("Auto unfolded levels in playlist tree"),
                   & treeAutoUnfoldedLevelsSpinBox);

    treeAutoCleanupCheckBox.setToolTip(
        tr("If checked, non-playable nodes with no playable descendants would\n"
           "be removed from playlist tree automatically after applying "
           "changes."));
    layout->addRow(tr("Automatically clean up playlist tree"),
                   & treeAutoCleanupCheckBox);

    QtUtilities::Widgets::addSpacing(layout);


    saveToDiskImmediatelyCheckBox.setToolTip(
        tr("If checked, settings would be saved to disk\n"
           "immediately after closing preferences window;\n"
           "otherwise - before application quit only."));
    layout->addRow(tr("Save preferences to disk immediately"),
                   & saveToDiskImmediatelyCheckBox);

    QtUtilities::Widgets::addSpacing(layout);


    checkIntervalCheckBox.setToolTip(
        tr("If checked, %1 commands are enabled.\n"
           "Switching this option off is not recommended\n"
           "because %1 provides a command line interface,\n"
           "can be used for creating global shortcuts,\n"
           "is REQUIRED for detached players.").arg(TOOL_NAME));
    onCheckIntervalToggled(checkIntervalCheckBox.isChecked());
    connect(& checkIntervalCheckBox, SIGNAL(toggled(bool)),
            SLOT(onCheckIntervalToggled(bool)));
    layout->addRow(tr("Enable %1").arg(TOOL_NAME), & checkIntervalCheckBox);

    {
        using P = Preferences;
        checkIntervalSpinBox.setDecimals(lgMsInS);
        checkIntervalSpinBox.setRange(P::minVentoolCheckInterval / msInS,
                                      P::maxVentoolCheckInterval / msInS);
        checkIntervalSpinBox.setValue(P::defaultVentoolCheckInterval / msInS);
    }
    checkIntervalSpinBox.setSingleStep(0.1);
    checkIntervalSpinBox.setToolTip(
        tr("Time interval between subsequent checks for %1 commands.\n"
           "Shorter interval makes reactions to %1 commands more prompt.\n"
           "Longer interval may slightly improve performance.").arg(TOOL_NAME));
    checkIntervalSpinBox.setSuffix("s");
    QtUtilities::Widgets::setFixedSizePolicy(& checkIntervalSpinBox);
    QtUtilities::Widgets::addSubWidget(layout,
                                       TOOL_NAME + tr(" check interval"),
                                       & checkIntervalSpinBox);
}

void GeneralPage::setUiPreferences(const Preferences & source)
{
    alwaysUseFallbackIconsCheckBox.setChecked(source.alwaysUseFallbackIcons);

    notificationAreaIconCheckBox.setChecked(source.notificationAreaIcon);
    startToNotificationAreaCheckBox.setChecked(source.startToNotificationArea);
    closeToNotificationAreaCheckBox.setChecked(source.closeToNotificationArea);

    statusBarCheckBox.setChecked(source.statusBar);

    treeAutoUnfoldedLevelsSpinBox.setValue(source.treeAutoUnfoldedLevels);
    treeAutoCleanupCheckBox.setChecked(source.treeAutoCleanup);

    saveToDiskImmediatelyCheckBox.setChecked(
        source.savePreferencesToDiskImmediately);

    {
        const int interval = int(source.ventoolCheckInterval);
        checkIntervalCheckBox.setChecked(interval != 0);
        if (interval != 0)
            checkIntervalSpinBox.setValue(interval / msInS);
    }
}

void GeneralPage::writeUiPreferencesTo(Preferences & destination) const
{
    destination.alwaysUseFallbackIcons =
        alwaysUseFallbackIconsCheckBox.isChecked();

    destination.notificationAreaIcon = notificationAreaIconCheckBox.isChecked();
    destination.startToNotificationArea =
        startToNotificationAreaCheckBox.isChecked();
    destination.closeToNotificationArea =
        closeToNotificationAreaCheckBox.isChecked();

    destination.statusBar = statusBarCheckBox.isChecked();

    destination.treeAutoUnfoldedLevels = treeAutoUnfoldedLevelsSpinBox.value();
    destination.treeAutoCleanup = treeAutoCleanupCheckBox.isChecked();

    destination.savePreferencesToDiskImmediately =
        saveToDiskImmediatelyCheckBox.isChecked();

    destination.ventoolCheckInterval =
        checkIntervalCheckBox.isChecked() ?
        unsigned(checkIntervalSpinBox.value() * msInS + 0.5) : 0u;
}


void GeneralPage::onNotificationAreaIconToggled(const bool checked)
{
    startToNotificationAreaCheckBox.setEnabled(checked);
    closeToNotificationAreaCheckBox.setEnabled(checked);
}

void GeneralPage::onCheckIntervalToggled(const bool checked)
{
    checkIntervalSpinBox.setEnabled(checked);
}
