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


/// Manages preferences. Saves and loads them in XML format.
/// Throws QtUtilities::Error or its descendant if error occurs.
class Preferences
{
public:
    typedef unsigned char StartupPolicyUnderlyingType;
    enum class StartupPolicy : StartupPolicyUnderlyingType
    {
        doNothing, playbackPlay, playbackNext
    };

    static constexpr unsigned maxExternalPlayerTimeout = 9999;
    static constexpr StartupPolicyUnderlyingType maxStartupPolicy = 2;
    static constexpr unsigned maxVentoolCheckInterval = 9999;

    struct PlayedItem {
        /// playedItem > 0 corresponds to (itemId = playedItem - 1)
        /// for ItemTree::Tree::getItemAbsolutePath().
        static constexpr int none = 0, all = -1, customSelection = -2;

        static QString toQString(int playedItem);
    };


    explicit Preferences(unsigned externalPlayerTimeout,
                         bool autoSetExternalPlayerOptions,
                         bool alwaysUseFallbackIcons,
                         bool notificationAreaIcon,
                         bool startToNotificationArea,
                         bool closeToNotificationArea,
                         StartupPolicy startupPolicy,
                         unsigned char treeAutoUnfoldedLevels,
                         bool treeAutoCleanup,
                         bool savePreferencesToDiskImmediately,
                         unsigned ventoolCheckInterval);

    /// @brief Saves preferences to file filename.
    void save(const QString & filename) const;

    /// @brief Loads preferences from file filename.
    /// NOTE: in case of failure (throwing Error) state of this becomes
    /// undefined.
    void load(const QString & filename);

    unsigned externalPlayerTimeout;
    bool autoSetExternalPlayerOptions;
    AddingItems::Policy addingPolicy;
    bool alwaysUseFallbackIcons;
    bool notificationAreaIcon, startToNotificationArea, closeToNotificationArea;
    StartupPolicy startupPolicy;
    unsigned char treeAutoUnfoldedLevels;
    bool treeAutoCleanup;
    bool savePreferencesToDiskImmediately;
    unsigned ventoolCheckInterval;

    /// Internal options follow.

    /// Requirements are in struct PlayedItem.
    int lastPlayedItem = PlayedItem::none;
    QByteArray preferencesWindowGeometry;
    QByteArray windowGeometry;
    QByteArray windowState;
};

bool operator == (const Preferences & lhs, const Preferences & rhs);

inline bool operator != (const Preferences & lhs, const Preferences & rhs)
{
    return !(lhs == rhs);
}

# endif // VENTUROUS_PREFERENCES_HPP
