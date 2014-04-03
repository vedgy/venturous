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

# include "AddingPolicyPage.hpp"

# include "Preferences.hpp"

# include <QString>
# include <QObject>
# include <QHBoxLayout>
# include <QVBoxLayout>


namespace
{
void addIfBothSubOption(
    QVBoxLayout * layout, QCheckBox & subOption, const QString & itemType)
{
    subOption.setText(QObject::tr("If both are present, add %1").arg(itemType));
    subOption.setToolTip(
        QObject::tr("This option is considered if files that match file "
                    "patterns are found in media directory.\n"
                    "If checked, %1 will be added to playlist.").arg(itemType));

    QHBoxLayout * rowLayout = new QHBoxLayout;
    rowLayout->addSpacing(30);
    rowLayout->addWidget(& subOption);

    layout->addLayout(rowLayout);
}

}


AddingPolicyPage::AddingPolicyPage(
    QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f), addFilesCheckBox(tr("Add files")),
      addMediaDirsCheckBox(tr("Add media dirs"))
{
    QVBoxLayout * const layout = new QVBoxLayout(this);

    addFilesCheckBox.setToolTip(tr("If checked, files that match file "
                                   "patterns are considered playable items."));
    layout->addWidget(& addFilesCheckBox);

    addMediaDirsCheckBox.setToolTip(tr("If checked, media directories are "
                                       "considered playable items."));
    layout->addWidget(& addMediaDirsCheckBox);

    onPrimaryCheckBoxToggled();
    for (QCheckBox * checkBox :
            { & addFilesCheckBox, & addMediaDirsCheckBox }) {
        connect(checkBox, SIGNAL(toggled(bool)),
                SLOT(onPrimaryCheckBoxToggled()));
    }

    addIfBothSubOption(layout, ifBothAddFilesCheckBox, tr("files"));
    addIfBothSubOption(layout, ifBothAddMediaDirsCheckBox, tr("media dirs"));
    layout->addStretch();
}

void AddingPolicyPage::setUiPreferences(const Preferences & source)
{
    const AddingItems::Policy & policy = source.addingPolicy;
    addFilesCheckBox.setChecked(policy.addFiles);
    addMediaDirsCheckBox.setChecked(policy.addMediaDirs);
    ifBothAddFilesCheckBox.setChecked(policy.ifBothAddFiles);
    ifBothAddMediaDirsCheckBox.setChecked(policy.ifBothAddMediaDirs);
}

void AddingPolicyPage::writeUiPreferencesTo(Preferences & destination) const
{
    AddingItems::Policy & policy = destination.addingPolicy;
    policy.addFiles = addFilesCheckBox.isChecked();
    policy.addMediaDirs = addMediaDirsCheckBox.isChecked();
    policy.ifBothAddFiles = ifBothAddFilesCheckBox.isChecked();
    policy.ifBothAddMediaDirs = ifBothAddMediaDirsCheckBox.isChecked();
}


void AddingPolicyPage::onPrimaryCheckBoxToggled()
{
    const bool bothPrimaryChecked = addFilesCheckBox.isChecked() &&
                                    addMediaDirsCheckBox.isChecked();
    ifBothAddFilesCheckBox.setEnabled(bothPrimaryChecked);
    ifBothAddMediaDirsCheckBox.setEnabled(bothPrimaryChecked);
}
