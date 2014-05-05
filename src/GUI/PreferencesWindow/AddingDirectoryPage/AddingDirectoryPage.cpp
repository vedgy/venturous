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

# include "AddingDirectoryPage.hpp"

# include "Icons.hpp"
# include "Preferences.hpp"

# include <QSize>
# include <QString>
# include <QIcon>
# include <QVBoxLayout>
# include <QGridLayout>
# include <QLabel>
# include <QPushButton>


AddingDirectoryPage::AddingDirectoryPage(const QIcon & addIcon,
        QWidget * const parent, const Qt::WindowFlags f)
    : PreferencesPage(parent, f)
{
    QVBoxLayout * const mainLayout = new QVBoxLayout(this);

    addingPolicyFrame_.setFrameStyle(QFrame::Panel | QFrame::Raised);
    addingPolicyFrame_.setLineWidth(2);
    mainLayout->addWidget(& addingPolicyFrame_);
    mainLayout->addSpacing(10);
    {
        QGridLayout * const layout = new QGridLayout;

        const QString filesMatchPatterns =
            tr("Files that match checked patterns\n");
        const QString copySelectedPatterns =
            tr("Copy selected patterns from %1 to %2 column.");
        const QString left = tr("left"), right = tr("right");

        filePatternList_.setToolTip(filesMatchPatterns +
                                    tr("can be added as playable items."));
        addColumn(layout, 0, tr("File patterns"), filePatternList_, addIcon);

        addCopyButton(layout, 2, "CopyRight",
                      copySelectedPatterns.arg(left, right), SLOT(copyRight()));

        addCopyButton(layout, 4, "CopyLeft",
                      copySelectedPatterns.arg(right, left), SLOT(copyLeft()));

        mediaDirFilePatternList_.setToolTip(
            filesMatchPatterns +
            tr("mark their parent directory as media dir."));
        addColumn(layout, 2, tr("Media directory file patterns"),
                  mediaDirFilePatternList_, addIcon);

        layout->setRowStretch(1, 3);
        layout->setRowStretch(3, 1);
        layout->setRowStretch(5, 3);

        mainLayout->addLayout(layout);
    }
}

void AddingDirectoryPage::setUiPreferences(const Preferences & source)
{
    addingPolicyFrame_.setUiPreferences(source.addingPolicy);
    filePatternList_.setUiPatterns(source.addingPolicy.filePatterns);
    mediaDirFilePatternList_.setUiPatterns(
        source.addingPolicy.mediaDirFilePatterns);
}

void AddingDirectoryPage::writeUiPreferencesTo(Preferences & destination) const
{
    addingPolicyFrame_.writeUiPreferencesTo(destination.addingPolicy);
    destination.addingPolicy.filePatterns = filePatternList_.getUiPatterns();
    destination.addingPolicy.mediaDirFilePatterns =
        mediaDirFilePatternList_.getUiPatterns();
}


void AddingDirectoryPage::addColumn(
    QGridLayout * const layout, const int column,
    const QString & caption, PatternListWidget & patternList,
    const QIcon & addIcon)
{
    layout->addWidget(new QLabel(caption, this), 0, column);
    layout->addWidget(& patternList, 1, column, 5, 1);

    QPushButton * const addButton =
        new QPushButton(addIcon, tr("Add pattern"), this);
    patternList.connect(addButton, SIGNAL(clicked(bool)),
                        SLOT(addUnknownPattern()));
    layout->addWidget(addButton, 6, column);
}

void AddingDirectoryPage::addCopyButton(
    QGridLayout * const layout, const int row, const QString & iconName,
    const QString & tooltip, const char * const slot)
{
    QPushButton * const button = new QPushButton(
        QIcon(Icons::getAbsolutePath(iconName)), QString(), this);
    button->setIconSize(QSize(33, 15));
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked(bool)), slot);

    layout->addWidget(button, row, 1);
}


void AddingDirectoryPage::copyLeft()
{
    filePatternList_.addUnknownPatterns(
        mediaDirFilePatternList_.getSelectedUnknownPatterns());
}

void AddingDirectoryPage::copyRight()
{
    mediaDirFilePatternList_.addUnknownPatterns(
        filePatternList_.getSelectedUnknownPatterns());
}
