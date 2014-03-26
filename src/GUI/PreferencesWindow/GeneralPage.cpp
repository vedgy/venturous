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

# include <QString>
# include <QObject>
# include <QCheckBox>
# include <QSpacerItem>
# include <QLayout>
# include <QHBoxLayout>
# include <QFormLayout>

# include <limits>


namespace
{
void addSpacing(QLayout * layout, int height = 10)
{
    layout->addItem(new QSpacerItem(1, height));
}

void addNotificationAreaSubOption(
    QFormLayout * layout, QCheckBox & subOption,
    const QString & toolTip, const QString & labelPrefix)
{
    subOption.setToolTip(toolTip);

    QHBoxLayout * rowLayout = new QHBoxLayout;
    rowLayout->addSpacing(30);
    rowLayout->addWidget(& subOption);
    layout->addRow(
        "    " + labelPrefix +
        QObject::tr(" %1 to notification area").arg(APPLICATION_NAME),
        rowLayout);
}

}

GeneralPage::GeneralPage(QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QFormLayout * const layout = new QFormLayout(this);

    alwaysUseFallbackIconsCheckBox.setToolTip(tr("If checked, fallback "
            "(application default) icons will always be used.\n"
            "Otherwise, icons from system theme will be preferred.\n"
            "Changing this option will take effect after restarting the "
            "application."));
    layout->addRow(tr("Always use fallback icons"),
                   & alwaysUseFallbackIconsCheckBox);

    addSpacing(layout);


    notificationAreaIconCheckBox.setToolTip(
        tr("Show %1 icon in notification area.").arg(APPLICATION_NAME));
    layout->addRow(tr("Notification area icon"),
                   & notificationAreaIconCheckBox);

    onNotificationAreaIconCheckBoxToggled(
        notificationAreaIconCheckBox.isChecked());
    connect(& notificationAreaIconCheckBox, SIGNAL(toggled(bool)),
            SLOT(onNotificationAreaIconCheckBoxToggled(bool)));

    addNotificationAreaSubOption(
        layout, startToNotificationAreaCheckBox,
        tr("If checked, %1 window is hidden on start.").arg(APPLICATION_NAME),
        tr("Start"));

    addNotificationAreaSubOption(
        layout, closeToNotificationAreaCheckBox,
        tr("If checked, %1 window is closed to notification area.\n"
           "Otherwise, closing is equialent to quitting.").arg(
            APPLICATION_NAME),
        tr("Close"));

    addSpacing(layout);


    treeAutoUnfoldedLevelsSpinBox.setRange(
        0, std::numeric_limits < decltype(
            Preferences::treeAutoUnfoldedLevels) >::max());
    treeAutoUnfoldedLevelsSpinBox.setToolTip(tr("Number of items in playlist "
            "tree, which will be unfolded by default."));
    layout->addRow(tr("Auto unfolded levels in the tree"),
                   & treeAutoUnfoldedLevelsSpinBox);

    treeAutoCleanupCheckBox.setToolTip(
        tr("If checked, non-playable nodes with no playable descendants will be"
           " removed from playlist tree automatically after applying changes.")
    );
    layout->addRow(tr("Automatically clean up tree"),
                   & treeAutoCleanupCheckBox);

    addSpacing(layout);


    saveToDiskImmediatelyCheckBox.setToolTip(
        tr("If checked, settings will be saved to disk immediately after "
           "closing preferences window, otherwise - "
           "before application quit only."));
    layout->addRow(tr("Save preferences to disk immediately"),
                   & saveToDiskImmediatelyCheckBox);

    checkIntervalSpinBox.setRange(0, Preferences::maxVentoolCheckInterval);
    checkIntervalSpinBox.setSingleStep(10);
    checkIntervalSpinBox.setToolTip(
        tr("Time interval between subsequent checks for %1 commands.\n"
           "Shorter interval makes reactions to %1 commands more prompt.\n"
           "Longer interval may slightly improve performance.\n"
           "If set to 0, %1 commands will have no effect.").arg(TOOL_NAME));
    checkIntervalSpinBox.setSuffix(tr("ms"));
    layout->addRow(TOOL_NAME + tr(" check interval"),
                   & checkIntervalSpinBox);
}

void GeneralPage::setUiPreferences(const Preferences & source)
{
    alwaysUseFallbackIconsCheckBox.setChecked(source.alwaysUseFallbackIcons);

    notificationAreaIconCheckBox.setChecked(source.notificationAreaIcon);
    startToNotificationAreaCheckBox.setChecked(source.startToNotificationArea);
    closeToNotificationAreaCheckBox.setChecked(source.closeToNotificationArea);

    treeAutoUnfoldedLevelsSpinBox.setValue(source.treeAutoUnfoldedLevels);
    treeAutoCleanupCheckBox.setChecked(source.treeAutoCleanup);

    saveToDiskImmediatelyCheckBox.setChecked(
        source.savePreferencesToDiskImmediately);
    checkIntervalSpinBox.setValue(source.ventoolCheckInterval);
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

    destination.treeAutoUnfoldedLevels = treeAutoUnfoldedLevelsSpinBox.value();
    destination.treeAutoCleanup = treeAutoCleanupCheckBox.isChecked();

    destination.savePreferencesToDiskImmediately =
        saveToDiskImmediatelyCheckBox.isChecked();
    destination.ventoolCheckInterval = checkIntervalSpinBox.value();
}


void GeneralPage::onNotificationAreaIconCheckBoxToggled(
    const bool checked)
{
    startToNotificationAreaCheckBox.setEnabled(checked);
    closeToNotificationAreaCheckBox.setEnabled(checked);
}
