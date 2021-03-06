/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015 Igor Kushnir <igorkuo AT Google mail>

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

# ifdef DEBUG_VENTUROUS_ICONS
# include <QtCoreUtilities/String.hpp>
# include <algorithm>
# include <iostream>
# endif // DEBUG_VENTUROUS_ICONS


# ifdef EMBEDDED_ICONS
# include <QFileInfo>
# endif // EMBEDDED_ICONS

# include "Icons.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QSize>
# include <QList>
# include <QString>
# include <QStringList>
# include <QFile>
# include <QTextStream>
# include <QIcon>

# include <cstddef>
# include <utility>
# include <array>
# include <tuple>


namespace
{
constexpr std::size_t playbackStartIndex = 3, playlistStartIndex = 15,
                      helpStartIndex = 27, addingPolicyStartIndex = 29;

inline bool isEmpty(const QIcon & icon)
{
    return icon.availableSizes().empty();
}

struct IconName {
    QString context, name;

    IconName() = default;

    explicit IconName(const QString & line) {
        const QStringList parts = line.split('/', QString::KeepEmptyParts);
        if (parts.size() != 2)
            throw Icons::Theme::Error("invalid line format in icon list.");
        context = std::move(parts.front());
        name = std::move(parts.back());
    }
};
template <std::size_t nIcons>
using IconNames = std::array<IconName, nIcons>;

template <std::size_t nIcons>
IconNames<nIcons> getNames()
{
    IconNames<nIcons> names;
    std::size_t index = 0;
    const QString filename = Icons::getAbsolutePath("icon_list");
    QFile source(filename);
    if (! source.open(QFile::ReadOnly)) {
        throw Icons::Theme::Error(
            "could not open " + QtUtilities::qStringToString(filename) +
            " for reading.");
    }

    QTextStream stream(& source);
    QString line;
    do {
        line = stream.readLine().trimmed();
        if (! line.isEmpty()) {
            if (index == names.size())
                throw Icons::Theme::Error("too many icons.");
            names[index] = IconName(line);
            ++index;
        }
    }
    while (! line.isNull());

    if (index != names.size())
        throw Icons::Theme::Error("not enough icons.");
    return names;
}

# ifdef EMBEDDED_ICONS
# ifdef DEBUG_VENTUROUS_ICONS
void printInfo(const QIcon & icon)
{
    QList<QSize> availableSizes = icon.availableSizes();
    if (availableSizes.empty())
        std::cout << "icons not found (or found svg only)." << std::endl;
    else {
        std::sort(availableSizes.begin(), availableSizes.end(),
        [](const QSize & lhs, const QSize & rhs) {
            return lhs.width() < rhs.width();
        });

        std::cout << "available icon sizes are:";
        for (int i = 0; i < availableSizes.size(); ++i) {
            if (i != 0)
                std::cout << ',';
            const QSize & size = availableSizes[i];
            std::cout << ' ' << size.width();
            if (size.width() != size.height())
                std::cout << 'x' << size.height();
        }
        std::cout << '.' << std::endl;
    }
}
# endif // DEBUG_VENTUROUS_ICONS
# endif // EMBEDDED_ICONS

} // END unnamed namespace


namespace Icons
{
Theme::Error::~Error() noexcept = default;


Theme::Theme(const bool alwaysUseFallbackIcons)
{
    constexpr std::size_t nIcons = std::tuple_size<decltype(icons_)>::value;
    const IconNames<nIcons> names = getNames<nIcons>();

    const QString themeName = "SimpleFugue";
    if (alwaysUseFallbackIcons)
        QIcon::setThemeName(themeName);
    for (std::size_t i = 0; i < icons_.size(); ++i)
        icons_[i] = QIcon::fromTheme(names[i].name);

# ifdef EMBEDDED_ICONS
    const std::array<int, 7> sizes {{ 16, 22, 24, 32, 48, 96, 256 }};
    const QString prefix = QString(":/icons/%1/").arg(themeName);
    for (std::size_t i = 0; i < icons_.size(); ++i) {
# ifdef DEBUG_VENTUROUS_ICONS
        std::cout << QtUtilities::qStringToString(names[i].name) << " - ";
# endif
        if (isEmpty(icons_[i])) {
            const QString contextAndName =
                '/' + names[i].context + '/' + names[i].name;
            for (int size : sizes) {
                const QString filename = prefix + QString("%1x%1").arg(size) +
                                         contextAndName + ".png";
                if (QFileInfo(filename).isFile())
                    icons_[i].addFile(filename, QSize(size, size));
            }
            const QString scalableIcon = prefix + "scalable" +
                                         contextAndName + ".svg";
            if (QFileInfo(scalableIcon).isFile())
                icons_[i].addFile(scalableIcon);

# ifdef DEBUG_VENTUROUS_ICONS
            std::cout << "using fallback icons; ";
# endif
        }
# ifdef DEBUG_VENTUROUS_ICONS
        printInfo(icons_[i]);
# endif
    }
# endif // EMBEDDED_ICONS

    if (isEmpty(icons_.front())) {
# ifdef DEBUG_VENTUROUS_ICONS
        std::cout << "Using application icon from " APPLICATION_NAME
                  " resources." << std::endl;
# endif
        icons_.front().addFile(getAbsolutePath(ICON_NAME ".png"));
    }
}


const QIcon & Theme::venturous() const
{
    return icons_[0];
}

const QIcon & Theme::preferences() const
{
    return icons_[1];
}

const QIcon & Theme::quit() const
{
    return icons_[2];
}


const QIcon & Theme::playbackPlay() const
{
    return icons_[playbackStartIndex];
}

const QIcon & Theme::playbackPause() const
{
    return icons_[playbackStartIndex + 1];
}

const QIcon & Theme::playbackStop() const
{
    return icons_[playbackStartIndex + 2];
}

const QIcon & Theme::playbackPrevious() const
{
    return icons_[playbackStartIndex + 3];
}

const QIcon & Theme::playbackReplayLast() const
{
    return icons_[playbackStartIndex + 4];
}

const QIcon & Theme::playbackNextFromHistory() const
{
    return icons_[playbackStartIndex + 5];
}

const QIcon & Theme::playbackNextRandom() const
{
    return icons_[playbackStartIndex + 6];
}

const QIcon & Theme::playbackNext() const
{
    return icons_[playbackStartIndex + 7];
}

const QIcon & Theme::playbackPlayAll() const
{
    return icons_[playbackStartIndex + 8];
}

const QIcon & Theme::audioPlayerShow() const
{
    return icons_[playbackStartIndex + 9];
}

const QIcon & Theme::audioPlayerHide() const
{
    return icons_[playbackStartIndex + 10];
}

const QIcon & Theme::viewRefresh() const
{
    return icons_[playbackStartIndex + 11];
}


const QIcon & Theme::edit() const
{
    return icons_[playlistStartIndex];
}

const QIcon & Theme::apply() const
{
    return icons_[playlistStartIndex + 1];
}

const QIcon & Theme::cancel() const
{
    return icons_[playlistStartIndex + 2];
}

const QIcon & Theme::add() const
{
    return icons_[playlistStartIndex + 3];
}

const QIcon & Theme::remove() const
{
    return icons_[playlistStartIndex + 4];
}

const QIcon & Theme::addDir() const
{
    return icons_[playlistStartIndex + 5];
}

const QIcon & Theme::cleanUp() const
{
    return icons_[playlistStartIndex + 6];
}

const QIcon & Theme::clear() const
{
    return icons_[playlistStartIndex + 7];
}

const QIcon & Theme::undo() const
{
    return icons_[playlistStartIndex + 8];
}

const QIcon & Theme::revert() const
{
    return icons_[playlistStartIndex + 9];
}

const QIcon & Theme::load() const
{
    return icons_[playlistStartIndex + 10];
}

const QIcon & Theme::saveAs() const
{
    return icons_[playlistStartIndex + 11];
}


const QIcon & Theme::help() const
{
    return icons_[helpStartIndex];
}

const QIcon & Theme::about() const
{
    return icons_[helpStartIndex + 1];
}


const QIcon & Theme::audioFile() const
{
    return icons_[addingPolicyStartIndex];
}

const QIcon & Theme::mediaDir() const
{
    return icons_[addingPolicyStartIndex + 1];
}

const QIcon & Theme::bothAudioFile() const
{
    return icons_[addingPolicyStartIndex + 2];
}

const QIcon & Theme::bothMediaDir() const
{
    return icons_[addingPolicyStartIndex + 3];
}

} // END namespace Icons
