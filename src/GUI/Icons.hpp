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

# ifndef VENTUROUS_ICONS_HPP
# define VENTUROUS_ICONS_HPP

# include <QString>
# include <QIcon>

# include <array>
# include <string>
# include <stdexcept>


namespace Icons
{
/// @return Absolute path to file with specified name in Venturous resources.
inline QString getAbsolutePath(const QString & filename)
{
    return ":/" APPLICATION_NAME "/" + filename;
}


class Theme
{
public:
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string & sWhat) : std::runtime_error(sWhat) {}
    };

    /// @param alwaysUseFallbackIcons If false, system icon theme will be
    /// preferred. If true, icons from system theme will never be used.
    explicit Theme(bool alwaysUseFallbackIcons = false);

    const QIcon & venturous() const;

    const QIcon & preferences() const;
    const QIcon & quit() const;

    const QIcon & playbackPlay() const;
    const QIcon & playbackStop() const;
    const QIcon & playbackPrevious() const;
    const QIcon & playbackReplayLast() const;
    const QIcon & playbackNextFromHistory() const;
    const QIcon & playbackNextRandom() const;
    const QIcon & playbackNext() const;
    const QIcon & playbackPlayAll() const;

    const QIcon & edit() const;
    const QIcon & apply() const;
    const QIcon & cancel() const;
    const QIcon & add() const;
    const QIcon & addDir() const;
    const QIcon & cleanUp() const;
    const QIcon & clear() const;
    const QIcon & revert() const;
    const QIcon & load() const;
    const QIcon & saveAs() const;

    const QIcon & about() const;
    const QIcon & help() const;

    const QIcon & audioFile() const;
    const QIcon & mediaDir() const;
    const QIcon & bothAudioFile() const;
    const QIcon & bothMediaDir() const;

private:
    std::array<QIcon, 27> icons_;
};

}

# endif // VENTUROUS_ICONS_HPP
