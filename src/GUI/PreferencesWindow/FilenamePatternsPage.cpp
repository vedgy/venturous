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

# include "FilenamePatternsPage.hpp"

# include "Icons.hpp"
# include "Preferences.hpp"

# include <QSize>
# include <QString>
# include <QObject>
# include <QIcon>
# include <QGridLayout>
# include <QLabel>
# include <QPushButton>


FilenamePatternsPage::FilenamePatternsPage(const Icons::Theme & theme,
        QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QGridLayout * const layout = new QGridLayout(this);

    const QString filesMatchPatterns =
        QObject::tr("Files, which match checked patterns, ");
    const QString copySelectedPatterns =
        QObject::tr("Copy selected patterns from %1 to %2 column.");
    const QString left = QObject::tr("left"), right = QObject::tr("right");

    filePatternList_.setToolTip(filesMatchPatterns +
                                tr("can be added as playable items."));
    addColumn(layout, 0, tr("File patterns"), filePatternList_, theme);

    addCopyButton(layout, 2, "CopyRight", copySelectedPatterns.arg(left, right),
                  SLOT(copyRight()));

    addCopyButton(layout, 4, "CopyLeft", copySelectedPatterns.arg(right, left),
                  SLOT(copyLeft()));

    mediaDirFilePatternList_.setToolTip(
        filesMatchPatterns + tr("mark their parent directory as media dir."));
    addColumn(layout, 2, tr("Media directory file patterns"),
              mediaDirFilePatternList_, theme);

    layout->setRowStretch(1, 3);
    layout->setRowStretch(3, 1);
    layout->setRowStretch(5, 3);
}

void FilenamePatternsPage::setUiPreferences(const Preferences & source)
{
    filePatternList_.setUiPatterns(source.addingPolicy.filePatterns);
    mediaDirFilePatternList_.setUiPatterns(
        source.addingPolicy.mediaDirFilePatterns);
}

void FilenamePatternsPage::writeUiPreferencesTo(Preferences & destination) const
{
    destination.addingPolicy.filePatterns = filePatternList_.getUiPatterns();
    destination.addingPolicy.mediaDirFilePatterns =
        mediaDirFilePatternList_.getUiPatterns();
}


void FilenamePatternsPage::addColumn(
    QGridLayout * const layout, const int column,
    const QString & caption, PatternListWidget & patternList,
    const Icons::Theme & theme)
{
    layout->addWidget(new QLabel(caption, this), 0, column);
    layout->addWidget(& patternList, 1, column, 5, 1);

    QPushButton * const addButton =
        new QPushButton(theme.add(), QObject::tr("Add pattern"), this);
    patternList.connect(addButton, SIGNAL(clicked(bool)),
                        SLOT(addUnknownPattern()));
    layout->addWidget(addButton, 6, column);
}

void FilenamePatternsPage::addCopyButton(
    QGridLayout * const layout, const int row, const QString & iconName,
    const QString & toolTip, const char * const slot)
{
    QPushButton * const button = new QPushButton(
        QIcon(Icons::getAbsolutePath(iconName)), QString(), this);
    button->setIconSize(QSize(33, 15));
    button->setToolTip(toolTip);
    connect(button, SIGNAL(clicked(bool)), slot);

    layout->addWidget(button, row, 1);
}


void FilenamePatternsPage::copyLeft()
{
    filePatternList_.addUnknownPatterns(
        mediaDirFilePatternList_.getSelectedUnknownPatterns());
}

void FilenamePatternsPage::copyRight()
{
    mediaDirFilePatternList_.addUnknownPatterns(
        filePatternList_.getSelectedUnknownPatterns());
}
