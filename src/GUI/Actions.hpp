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

# ifndef VENTUROUS_ACTIONS_HPP
# define VENTUROUS_ACTIONS_HPP

# include <QtGlobal>
# include <QObject>


namespace Icons
{
class Theme;
}
QT_FORWARD_DECLARE_CLASS(QAction)

/// NOTE: each action's parent is the class that contains it.
/// For example playAll's parent is playback.
struct Actions {
    explicit Actions(const Icons::Theme & theme);

    class File : public QObject
    {
    public:
        explicit File(const Icons::Theme & theme);
        QAction * preferences, * quit;
    } file;

    class Playback : public QObject
    {
    public:
        explicit Playback(const Icons::Theme & theme);
        QAction * play, * stop, * previous, * replayLast,
                * nextFromHistory, * nextRandom, * next, * playAll;
    } playback;

    class Playlist : public QObject
    {
    public:
        explicit Playlist(const Icons::Theme & theme);
        QAction * editMode, * applyChanges, * cancelChanges;
        QAction * addFiles, * addDirectory, * cleanUp, * clear,
                * restorePrevious;
        QAction * load, * saveAs;
    } playlist;

    class Help : public QObject
    {
    public:
        explicit Help(const Icons::Theme & theme);
        QAction * help, * about;
    } help;

    class AddingPolicy : public QObject
    {
    public:
        explicit AddingPolicy(const Icons::Theme & theme);
        QAction * audioFile, * mediaDir, * bothAudioFile, * bothMediaDir;

        /// @brief Must be called after audioFile's or mediaDir's checkState
        /// changes.
        void primaryActionChanged();
    } addingPolicy;
};

# endif // VENTUROUS_ACTIONS_HPP
