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

# ifndef VENTUROUS_PREFERENCES_HPP
# define VENTUROUS_PREFERENCES_HPP

# include <VenturousCore/AddingItems.hpp>

# include <QByteArray>
# include <QString>

# include <cstddef>
# include <cstdint>
# include <algorithm>
# include <limits>


/// Manages preferences. Saves and loads them in XML format.
/// Throws QtUtilities::Error or its descendant if error occurs.
class Preferences
{
public:
    struct Playback {
        struct History {
            static constexpr std::size_t maxMaxSize =
                sizeof(std::size_t) > 2 && sizeof(int) > 2 ? 999 * 1001
                : std::min<std::uintmax_t>(
                    std::numeric_limits<std::size_t>::max(),
                    std::numeric_limits<int>::max()) - 1;

            static constexpr unsigned maxNHiddenDirs = 99;
            static constexpr int multipleItemsIndex = -1;

            explicit History();


            std::size_t maxSize;
            bool copyPlayedEntryToTop;
            bool saveToDiskImmediately;
            unsigned nHiddenDirs;

            /// Internal option.
            int currentIndex;
        }
        history;

        typedef unsigned char StartupPolicyUnderlyingType;
        enum class StartupPolicy : StartupPolicyUnderlyingType
        {
            doNothing, playbackPlay, playbackNext
        };

        static constexpr StartupPolicyUnderlyingType maxStartupPolicy = 2;

        explicit Playback();


        bool autoSetExternalPlayerOptions;
        bool nextFromHistory;
        StartupPolicy startupPolicy;
    }
    playback;


    static constexpr unsigned maxVentoolCheckInterval = 9999;


    explicit Preferences();

    /// @brief Saves preferences to file filename.
    void save(const QString & filename) const;

    /// @brief Loads preferences from file filename.
    /// NOTE: in case of failure (throwing Error) state of this becomes
    /// undefined.
    void load(const QString & filename);


    AddingItems::Policy addingPolicy;
    bool alwaysUseFallbackIcons;
    bool notificationAreaIcon, startToNotificationArea, closeToNotificationArea;
    unsigned char treeAutoUnfoldedLevels;
    bool treeAutoCleanup;
    bool savePreferencesToDiskImmediately;
    unsigned ventoolCheckInterval;

    /// Internal options follow.
    QByteArray preferencesWindowGeometry;
    QByteArray windowGeometry;
    QByteArray windowState;
};


bool operator == (const Preferences::Playback::History & lhs,
                  const Preferences::Playback::History & rhs);

inline bool operator != (const Preferences::Playback::History & lhs,
                         const Preferences::Playback::History & rhs)
{
    return !(lhs == rhs);
}

bool operator == (const Preferences::Playback & lhs,
                  const Preferences::Playback & rhs);

inline bool operator != (const Preferences::Playback & lhs,
                         const Preferences::Playback & rhs)
{
    return !(lhs == rhs);
}

bool operator == (const Preferences & lhs, const Preferences & rhs);

inline bool operator != (const Preferences & lhs, const Preferences & rhs)
{
    return !(lhs == rhs);
}

# endif // VENTUROUS_PREFERENCES_HPP