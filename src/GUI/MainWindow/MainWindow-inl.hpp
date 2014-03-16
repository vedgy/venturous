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

# ifndef VENTUROUS_MAIN_WINDOW_INL_HPP
# define VENTUROUS_MAIN_WINDOW_INL_HPP

# include "MainWindow.hpp"

# include <QtCoreUtilities/Error.hpp>
# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QMessageBox>

# include <string>


template <typename FilenameGetter>
void MainWindow::loadTemporaryPlaylist(FilenameGetter filenameGetter)
{
    if (! ensureAskInEditMode())
        return;
    const QString file = filenameGetter();
    if (! file.isEmpty()) {
        const std::string errorMessage =
            temporaryTree_->load(QtUtilities::qStringToString(file));
        if (! errorMessage.empty()) {
            temporaryTree_->topLevelNodes().clear();
            showLoadingPlaylistErrorMessage(errorMessage);
        }
        treeWidget_.updateTree();
    }
}

template <typename F>
bool MainWindow::handlePreferencesErrors(F f, const QString & errorPrefix)
{
    while (true) {
        try {
            f();
            return true;
        }
        catch (const QtUtilities::Error & error) {
            const auto selectedButton =
                QMessageBox::critical(this, tr("Preferences error"),
                                      errorPrefix + ": " + error.message(),
                                      QMessageBox::Retry | QMessageBox::Cancel,
                                      QMessageBox::Cancel);

            if (selectedButton == QMessageBox::Cancel)
                return false;
        }
    }
}

# endif // VENTUROUS_MAIN_WINDOW_INL_HPP
