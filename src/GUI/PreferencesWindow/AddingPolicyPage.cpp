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
# include <QFrame>
# include <QCheckBox>


namespace
{
QVBoxLayout * createFrame(QLayout * parentLayout,
                          QFrame ** createdFrame = nullptr)
{
    QFrame * const frame = new QFrame;
    parentLayout->addWidget(frame);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    frame->setLineWidth(2);
    if (createdFrame != nullptr)
        * createdFrame = frame;
    return new QVBoxLayout(frame);
}

void addIfBothSubOption(
    QLayout * layout, QCheckBox & subOption, const QString & itemType)
{
    subOption.setText(QObject::tr("If both are present, add %1").arg(itemType));
    subOption.setToolTip(
        QObject::tr("This option is considered if files that match file "
                    "patterns\nare found in media directory.\n"
                    "If checked, %1 will be added to playlist.").arg(itemType));
    layout->addWidget(& subOption);
}

}


AddingPolicyPage::AddingPolicyPage(
    QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f), addFilesCheckBox(tr("Add files")),
      addMediaDirsCheckBox(tr("Add media dirs"))
{
    QVBoxLayout * const mainLayout = new QVBoxLayout(this);
    {
        const QString tooltip =
            tr("If checked, %1\nare considered playable items.");

        QVBoxLayout * const primaryLayout =
            createFrame(mainLayout, & primaryFrame);
        addFilesCheckBox.setToolTip(
            tooltip.arg(tr("files that match file patterns")));
        primaryLayout->addWidget(& addFilesCheckBox);

        addMediaDirsCheckBox.setToolTip(tooltip.arg(tr("media directories")));
        primaryLayout->addWidget(& addMediaDirsCheckBox);

        onPrimaryCheckBoxToggled();
        for (QCheckBox * checkBox :
                { & addFilesCheckBox, & addMediaDirsCheckBox }) {
            connect(checkBox, SIGNAL(toggled(bool)),
                    SLOT(onPrimaryCheckBoxToggled()));
        }
    }
    {
        QHBoxLayout * secondaryLayout = new QHBoxLayout;
        secondaryLayout->addSpacing(30);
        mainLayout->addLayout(secondaryLayout);

        QVBoxLayout * const subOptionLayout = createFrame(secondaryLayout);
        addIfBothSubOption(subOptionLayout, ifBothAddFilesCheckBox,
                           tr("files"));
        addIfBothSubOption(subOptionLayout, ifBothAddMediaDirsCheckBox,
                           tr("media dirs"));
    }
    mainLayout->addStretch();
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
    primaryFrame->setFrameShadow(
        bothPrimaryChecked ? QFrame::Sunken : QFrame::Raised);
    ifBothAddFilesCheckBox.setEnabled(bothPrimaryChecked);
    ifBothAddMediaDirsCheckBox.setEnabled(bothPrimaryChecked);
}
