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

# include "Preferences.hpp"

# include <QFont>
# include <QVBoxLayout>
# include <QFormLayout>
# include <QLabel>


HistoryPreferencesFrame::HistoryPreferencesFrame(
    QWidget * const parent, const Qt::WindowFlags f) : QFrame(parent, f)
{
    QVBoxLayout * const mainLayout = new QVBoxLayout(this);
    {
        QLabel * const captionLabel = new QLabel("<b>History</b>", this);
        QFont font = captionLabel->font();
        font.setPointSize(font.pointSize() + 2);
        captionLabel->setFont(font);
        mainLayout->addWidget(captionLabel, 0, Qt::AlignCenter);
    }

    QFormLayout * const layout = new QFormLayout();
    mainLayout->addLayout(layout);

    typedef Preferences::Playback::History History;

    maxSizeSpinBox.setRange(0, History::maxMaxSize);
    maxSizeSpinBox.setToolTip(
        tr("Maximum number of entries in history.\n"
           "The oldest entry is removed when this limit is reached."));
    layout->addRow(tr("Maximum size"), & maxSizeSpinBox);

    copyPlayedEntryToTopCheckBox.setToolTip(
        tr("If checked, entry, which was played from history, is copied to the "
           "history top.\n"
           "Otherwise, history pointer is moved to custom-played entry."));
    layout->addRow(tr("Copy played entry to top"),
                   & copyPlayedEntryToTopCheckBox);

    saveToDiskImmediatelyCheckBox.setToolTip(
        tr("If checked, history will be saved to disk each time new item "
           "is added, otherwise - before application quit only."));
    layout->addRow(tr("Save to disk immediately"),
                   & saveToDiskImmediatelyCheckBox);

    nHiddenDirsSpinBox.setRange(0, History::maxNHiddenDirs);
    nHiddenDirsSpinBox.setToolTip(
        tr("Number of directories in item path to be hidden in history window "
           "and status bar.\n"
           "Absolute path to item is always present in tooltip."));
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
