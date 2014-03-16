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

# include "Preferences.hpp"

# include <QtXmlUtilities/Shortcuts.hpp>

# include <VenturousCore/AddingItems.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QDomElement>
# include <QDomDocument>

# include <array>
# include <tuple>


namespace
{
const std::array<QString, 3> playedItemNames = {{
        QObject::tr("none"), QObject::tr("all"), QObject::tr("custom selection")
    }
};
constexpr int minPlayedItem =
    - int(std::tuple_size<decltype(playedItemNames)>::value - 1);

void assertValidPlayedItem(int playedItem)
{
    if (playedItem < minPlayedItem) {
        throw QtUtilities::Error(
            QObject::tr("played item is less than minimum allowed value "
                        "(%1 < %2).").arg(playedItem).arg(minPlayedItem));
    }
}

namespace Names
{
const QString root = APPLICATION_NAME,
              externalPlayerTimeout = "ExternalPlayerTimeout",
              autoSetExternalPlayerOptions = "AutoSetExternalPlayerOptions",
              alwaysUseFallbackIcons = "AlwaysUseFallbackIcons",
              notificationAreaIcon = "NotificationAreaIcon",
              startToNotificationArea = "StartToNotificationArea",
              closeToNotificationArea = "CloseToNotificationArea",
              startupPolicy = "StartupPolicy",
              treeAutoUnfoldedLevels = "TreeAutoUnfoldedLevels",
              treeAutoCleanup = "TreeAutoCleanup",
              savePreferencesToDiskImmediately =
                  "SavePreferencesToDiskImmediately",
                  ventoolCheckInterval = "VentoolCheckInterval",

                  lastPlayedItem = "LastPlayedItem",
                  preferencesWindowGeometry = "PreferencesWindowGeometry",
                  windowGeometry = "WindowGeometry",
                  windowState = "WindowState";

namespace AddingItemsPolicy
{
const QString localRoot = "AddingItemsPolicy",
              filePatterns = "FilePatterns",
              mediaDirFilePatterns = "MediaDirFilePatterns",
              pattern = "pattern",
              addFiles = "AddFiles",
              addMediaDirs = "AddMediaDirs",
              ifBothAddFiles = "IfBothAddFiles",
              ifBothAddMediaDirs = "IfBothAddMediaDirs";

}

}


void appendQStringList(QDomDocument & doc, QDomElement & e,
                       const QString & localRootTagName,
                       const QStringList & list,
                       const QString & itemTagName)
{
    QDomElement localRoot = doc.createElement(localRootTagName);
    for (const QString & s : list) {
        localRoot.appendChild(
            QtUtilities::Xml::createElement(doc, itemTagName, s));
    }
    e.appendChild(localRoot);
}

void appendAddingItemsPolicy(QDomDocument & doc, QDomElement & root,
                             const AddingItems::Policy & policy)
{
    using namespace QtUtilities::Xml;
    using namespace Names::AddingItemsPolicy;
    QDomElement e = doc.createElement(localRoot);

    appendQStringList(doc, e, filePatterns, policy.filePatterns, pattern);
    appendQStringList(doc, e, mediaDirFilePatterns,
                      policy.mediaDirFilePatterns, pattern);
    e.appendChild(createElement(doc, addFiles, policy.addFiles));
    e.appendChild(createElement(doc, addMediaDirs, policy.addMediaDirs));
    e.appendChild(createElement(doc, ifBothAddFiles, policy.ifBothAddFiles));
    e.appendChild(createElement(doc, ifBothAddMediaDirs,
                                policy.ifBothAddMediaDirs));

    root.appendChild(e);
}


void loadQStringList(const QDomElement & e, const QString & localRootTagName,
                     QStringList & list,  const QString & itemTagName)
{
    using namespace QtUtilities::Xml;
    const QDomElement localRoot = getUniqueChild(e, localRootTagName);
    if (! localRoot.isNull()) {
        list = getChildren<QStringList>(localRoot, itemTagName,
        [](const QDomElement & de) {
            return de.text();
        });
    }
}

void loadAddingItemsPolicy(const QDomElement & root,
                           AddingItems::Policy & policy)
{
    using namespace QtUtilities::Xml;
    using namespace Names::AddingItemsPolicy;
    const QDomElement e = getUniqueChild(root, localRoot);

    loadQStringList(e, filePatterns, policy.filePatterns, pattern);
    loadQStringList(e, mediaDirFilePatterns, policy.mediaDirFilePatterns,
                    pattern);
    copyUniqueChildsTextTo(e, addFiles, policy.addFiles);
    copyUniqueChildsTextTo(e, addMediaDirs, policy.addMediaDirs);
    copyUniqueChildsTextTo(e, ifBothAddFiles, policy.ifBothAddFiles);
    copyUniqueChildsTextTo(e, ifBothAddMediaDirs, policy.ifBothAddMediaDirs);
}


template <typename Number>
void checkMaxValue(const QString & name, Number value, Number maxValue)
{
    if (value > maxValue) {
        throw QtUtilities::StringError(
            name +
            QObject::tr(" is greater than maximum allowed value (%1 > %2).").
            arg(value).arg(maxValue));
    }
}

template <typename Number>
bool copyUniqueChildsTextTo(const QDomElement & e, const QString & tagName,
                            Number & destination, Number maxValue)
{
    if (QtUtilities::Xml::copyUniqueChildsTextTo(e, tagName, destination)) {
        checkMaxValue(tagName, destination, maxValue);
        return true;
    }
    return false;
}

}


constexpr unsigned Preferences::maxExternalPlayerTimeout;
constexpr Preferences::StartupPolicyUnderlyingType
Preferences::maxStartupPolicy;
constexpr unsigned Preferences::maxVentoolCheckInterval;

constexpr int Preferences::PlayedItem::none, Preferences::PlayedItem::all,
          Preferences::PlayedItem::customSelection;

QString Preferences::PlayedItem::toQString(const int playedItem)
{
    assertValidPlayedItem(playedItem);

    return playedItem > 0 ? QString::number(playedItem) :
           playedItemNames[- playedItem];
}


Preferences::Preferences(const unsigned externalPlayerTimeout,
                         const bool autoSetExternalPlayerOptions,
                         const bool alwaysUseFallbackIcons,
                         const bool notificationAreaIcon,
                         const bool startToNotificationArea,
                         const bool closeToNotificationArea,
                         const StartupPolicy startupPolicy,
                         const unsigned char treeAutoUnfoldedLevels,
                         const bool treeAutoCleanup,
                         const bool savePreferencesToDiskImmediately,
                         const unsigned ventoolCheckInterval)
    : externalPlayerTimeout(externalPlayerTimeout),
      autoSetExternalPlayerOptions(autoSetExternalPlayerOptions),
      alwaysUseFallbackIcons(alwaysUseFallbackIcons),
      notificationAreaIcon(notificationAreaIcon),
      startToNotificationArea(startToNotificationArea),
      closeToNotificationArea(closeToNotificationArea),
      startupPolicy(startupPolicy),
      treeAutoUnfoldedLevels(treeAutoUnfoldedLevels),
      treeAutoCleanup(treeAutoCleanup),
      savePreferencesToDiskImmediately(savePreferencesToDiskImmediately),
      ventoolCheckInterval(ventoolCheckInterval)
{
    checkMaxValue(Names::externalPlayerTimeout, externalPlayerTimeout,
                  maxExternalPlayerTimeout);
    checkMaxValue(Names::ventoolCheckInterval, ventoolCheckInterval,
                  maxVentoolCheckInterval);
}


void Preferences::save(const QString & filename) const
{
    using namespace QtUtilities::Xml;
    QDomDocument doc = createDocument();
    QDomElement root = createRoot(doc, Names::root);

    root.appendChild(createElement(doc, Names::externalPlayerTimeout,
                                   externalPlayerTimeout));
    root.appendChild(createElement(doc, Names::autoSetExternalPlayerOptions,
                                   autoSetExternalPlayerOptions));
    appendAddingItemsPolicy(doc, root, addingPolicy);

    root.appendChild(createElement(doc, Names::alwaysUseFallbackIcons,
                                   alwaysUseFallbackIcons));

    root.appendChild(createElement(doc, Names::notificationAreaIcon,
                                   notificationAreaIcon));
    root.appendChild(createElement(doc, Names::startToNotificationArea,
                                   startToNotificationArea));
    root.appendChild(createElement(doc, Names::closeToNotificationArea,
                                   closeToNotificationArea));

    root.appendChild(createElement(doc, Names::startupPolicy,
                                   static_cast<StartupPolicyUnderlyingType>(
                                       startupPolicy)));

    root.appendChild(createElement(doc, Names::treeAutoUnfoldedLevels,
                                   treeAutoUnfoldedLevels));
    root.appendChild(createElement(doc, Names::treeAutoCleanup,
                                   treeAutoCleanup));

    root.appendChild(createElement(doc, Names::savePreferencesToDiskImmediately,
                                   savePreferencesToDiskImmediately));
    root.appendChild(createElement(doc, Names::ventoolCheckInterval,
                                   ventoolCheckInterval));


    root.appendChild(createElement(doc, Names::lastPlayedItem, lastPlayedItem));

    root.appendChild(
        createElementFromByteArray(doc, Names::preferencesWindowGeometry,
                                   preferencesWindowGeometry));
    root.appendChild(
        createElementFromByteArray(doc, Names::windowGeometry, windowGeometry));
    root.appendChild(
        createElementFromByteArray(doc, Names::windowState, windowState));

    QtUtilities::Xml::save(doc, filename);
}


void Preferences::load(const QString & filename)
{
    using namespace QtUtilities::Xml;
    const QDomElement root = loadRoot(filename, Names::root);

    copyUniqueChildsTextTo(root, Names::externalPlayerTimeout,
                           externalPlayerTimeout, maxExternalPlayerTimeout);
    copyUniqueChildsTextTo(root, Names::autoSetExternalPlayerOptions,
                           autoSetExternalPlayerOptions);
    loadAddingItemsPolicy(root, addingPolicy);

    copyUniqueChildsTextTo(root, Names::alwaysUseFallbackIcons,
                           alwaysUseFallbackIcons);

    copyUniqueChildsTextTo(root, Names::notificationAreaIcon,
                           notificationAreaIcon);
    copyUniqueChildsTextTo(root, Names::startToNotificationArea,
                           startToNotificationArea);
    copyUniqueChildsTextTo(root, Names::closeToNotificationArea,
                           closeToNotificationArea);

    {
        StartupPolicyUnderlyingType p;
        if (copyUniqueChildsTextTo(
                    root, Names::startupPolicy, p, maxStartupPolicy)) {
            startupPolicy = static_cast<StartupPolicy>(p);
        }
    }

    copyUniqueChildsTextTo(root, Names::treeAutoUnfoldedLevels,
                           treeAutoUnfoldedLevels);
    copyUniqueChildsTextTo(root, Names::treeAutoCleanup, treeAutoCleanup);

    copyUniqueChildsTextTo(root, Names::savePreferencesToDiskImmediately,
                           savePreferencesToDiskImmediately);
    copyUniqueChildsTextTo(root, Names::ventoolCheckInterval,
                           ventoolCheckInterval, maxVentoolCheckInterval);


    copyUniqueChildsTextTo(root, Names::lastPlayedItem, lastPlayedItem);
    assertValidPlayedItem(lastPlayedItem);

    copyUniqueChildsTextToByteArray(
        root, Names::preferencesWindowGeometry, preferencesWindowGeometry);
    copyUniqueChildsTextToByteArray(
        root, Names::windowGeometry, windowGeometry);
    copyUniqueChildsTextToByteArray(root, Names::windowState, windowState);
}



bool operator == (const Preferences & lhs, const Preferences & rhs)
{
    return
        lhs.externalPlayerTimeout == rhs.externalPlayerTimeout &&
        lhs.autoSetExternalPlayerOptions == rhs.autoSetExternalPlayerOptions &&
        lhs.addingPolicy == rhs.addingPolicy &&
        lhs.alwaysUseFallbackIcons == rhs.alwaysUseFallbackIcons &&
        lhs.notificationAreaIcon == rhs.notificationAreaIcon &&
        lhs.startToNotificationArea == rhs.startToNotificationArea &&
        lhs.closeToNotificationArea == rhs.closeToNotificationArea &&
        lhs.startupPolicy == rhs.startupPolicy &&
        lhs.treeAutoUnfoldedLevels == rhs.treeAutoUnfoldedLevels &&
        lhs.treeAutoCleanup == rhs.treeAutoCleanup &&
        lhs.savePreferencesToDiskImmediately ==
        rhs.savePreferencesToDiskImmediately &&
        lhs.ventoolCheckInterval == rhs.ventoolCheckInterval &&

        lhs.lastPlayedItem == rhs.lastPlayedItem &&
        lhs.preferencesWindowGeometry == rhs.preferencesWindowGeometry &&
        lhs.windowGeometry == rhs.windowGeometry &&
        lhs.windowState == rhs.windowState;
}
