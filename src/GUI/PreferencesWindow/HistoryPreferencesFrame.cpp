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

# include "HistoryPreferencesFrame.hpp"

# include "GuiUtilities.hpp"
# include "Preferences.hpp"

# include <QVBoxLayout>
# include <QFormLayout>
# include <QLabel>


HistoryPreferencesFrame::HistoryPreferencesFrame(
    QWidget * const parent, const Qt::WindowFlags f) : QFrame(parent, f)
{
    QVBoxLayout * const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(GuiUtilities::getCaptionLabel(tr("History"), this),
                          0, Qt::AlignCenter);

    QFormLayout * const layout = new QFormLayout();
    mainLayout->addLayout(layout);

    typedef Preferences::Playback::History History;

    maxSizeSpinBox.setRange(0, History::maxMaxSize);
    maxSizeSpinBox.setSingleStep(10);
    maxSizeSpinBox.setToolTip(
        tr("Maximum number of entries in history.\n"
           "The oldest entry is removed when this limit is reached."));
    maxSizeSpinBox.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addRow(tr("Maximum size"), & maxSizeSpinBox);

    copyPlayedEntryToTopCheckBox.setToolTip(
        tr("If checked, entry that was played from history\n"
           "is copied to the history top.\n"
           "Otherwise, history pointer is moved to custom-played entry."));
    layout->addRow(tr("Copy played entry to top"),
                   & copyPlayedEntryToTopCheckBox);

    saveToDiskImmediatelyCheckBox.setToolTip(
        tr("If checked, history will be saved to disk each time new item\n"
           "is added, otherwise - before application quit only."));
    layout->addRow(tr("Save to disk immediately"),
                   & saveToDiskImmediatelyCheckBox);

    nHiddenDirsSpinBox.setRange(0, History::maxNHiddenDirs);
    nHiddenDirsSpinBox.setToolTip(
        tr("Number of directories in item path to be hidden\n"
           "in history window and status bar.\n"
           "Absolute path to item is always present in tooltip."));
    nHiddenDirsSpinBox.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addRow(tr("Number of hidden directories"), & nHiddenDirsSpinBox);
}

void HistoryPreferencesFrame::setUiPreferences(
    const Preferences::Playback::History & source)
{
    maxSizeSpinBox.setValue(source.maxSize);
    copyPlayedEntryToTopCheckBox.setChecked(source.copyPlayedEntryToTop);
    saveToDiskImmediatelyCheckBox.setChecked(source.saveToDiskImmediately);
    nHiddenDirsSpinBox.setValue(source.nHiddenDirs);
}

void HistoryPreferencesFrame::writeUiPreferencesTo(
    Preferences::Playback::History & destination) const
{
    destination.maxSize = maxSizeSpinBox.value();
    destination.copyPlayedEntryToTop = copyPlayedEntryToTopCheckBox.isChecked();
    destination.saveToDiskImmediately =
        saveToDiskImmediatelyCheckBox.isChecked();
    destination.nHiddenDirs = nHiddenDirsSpinBox.value();
}
