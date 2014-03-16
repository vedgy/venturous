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

# include "MainWindow.hpp"

# include "Preferences.hpp"

# include <VenturousCore/ItemTree-inl.hpp>

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QMessageBox>

# include <vector>
# include <string>


namespace
{
/// @brief Shows error message and asks user if playback should be continued.
/// @param parent Is used as an owner of message box.
/// @param title Title of the message box.
/// @param errorMessage Message to be displayed before question.
/// @return true if playback should be continued.
bool criticalContinuePlaybackQuestion(
    QWidget * parent, const QString & title, const QString & errorMessage)
{
    const auto selectedButton =
        QMessageBox::critical(
            parent, title, errorMessage + QObject::tr("\n\tContinue playback?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    return selectedButton == QMessageBox::Yes;
}

}


void MainWindow::onPlayerFinished(
    const bool crashExit, const int exitCode,
    const std::vector<std::string> missingFilesAndDirs)
{
    if (crashExit &&
            ! criticalContinuePlaybackQuestion(
                this, tr("External player error"),
                tr("%1 crashed with exit code %2.")
                .arg(QtUtilities::toQString(MediaPlayer::playerName()))
                .arg(exitCode))) {
        return;
    }

    if (! missingFilesAndDirs.empty()) {
        QString message = tr("No such files or directories (%1):")
                          .arg(missingFilesAndDirs.size());
        for (const std::string & s : missingFilesAndDirs)
            message += '\n' + QtUtilities::toQString(s) + ';';
        message[message.size() - 1] = '.';

        if (! criticalContinuePlaybackQuestion(
                    this, tr("Missing files or directories"), message)) {
            return;
        }
    }

    playbackNext();
}


void MainWindow::onItemActivated(const QString absolutePath)
{
    mediaPlayer_.start(QtUtilities::qStringToString(absolutePath));
    playerStateChanged(true);
    playedItemChanged(Preferences::PlayedItem::customSelection, absolutePath);
}

void MainWindow::playbackPlay()
{
    mediaPlayer_.start();
    playerStateChanged(true);
}

void MainWindow::playbackStop()
{
    mediaPlayer_.quit();
    playerStateChanged(false);
}

void MainWindow::playbackNext()
{
    if (itemTree_.itemCount() > 0) {
        const int itemId = randomItemChooser_.randomItemId(itemTree_);
        const std::string absolutePath = itemTree_.getItemAbsolutePath(itemId);
        mediaPlayer_.start(absolutePath);
        playerStateChanged(true);
        playedItemChanged(itemId + 1, QtUtilities::toQString(absolutePath));
    }
    else {
        mediaPlayer_.quit();
        playerStateChanged(false);
    }
}

void MainWindow::onPlayAll()
{
    mediaPlayer_.start(itemTree_.getAllItems<std::vector<std::string>>());
    playerStateChanged(true);
    playedItemChanged(Preferences::PlayedItem::all);
}
