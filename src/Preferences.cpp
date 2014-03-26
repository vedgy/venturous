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

# include <VenturousCore/AddingItems.hpp>

# include <QtXmlUtilities/Shortcuts.hpp>

# include <QtCoreUtilities/Validation.hpp>

# include <QString>
# include <QStringList>
# include <QDomElement>
# include <QDomDocument>


namespace
{
template <typename Number>
bool copyUniqueChildsTextToMax(const QDomElement & e, const QString & tagName,
                               Number & destination, Number maxValue)
{
    if (QtUtilities::Xml::copyUniqueChildsTextTo(e, tagName, destination)) {
        QtUtilities::checkMaxValue(tagName, destination, maxValue);
        return true;
    }
    return false;
}


namespace Names
{
namespace Playback
{
namespace History
{
const QString localRoot = "History",
              maxSize = "MaxSize",
              copyPlayedEntryToTop = "CopyPlayedEntryToTop",
              saveToDiskImmediately = "SaveToDiskImmediately",
              nHiddenDirs = "HiddenDirsNumber",
              currentIndex = "CurrentIndex";
}

const QString localRoot = "Playback",
              autoSetExternalPlayerOptions = "AutoSetExternalPlayerOptions",
              nextFromHistory = "NextFromHistory",
              startupPolicy = "StartupPolicy";
}

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

const QString root = APPLICATION_NAME,
              alwaysUseFallbackIcons = "AlwaysUseFallbackIcons",
              notificationAreaIcon = "NotificationAreaIcon",
              startToNotificationArea = "StartToNotificationArea",
              closeToNotificationArea = "CloseToNotificationArea",
              treeAutoUnfoldedLevels = "TreeAutoUnfoldedLevels",
              treeAutoCleanup = "TreeAutoCleanup",
              savePreferencesToDiskImmediately =
                  "SavePreferencesToDiskImmediately",
                  ventoolCheckInterval = "VentoolCheckInterval",

                  preferencesWindowGeometry = "PreferencesWindowGeometry",
                  windowGeometry = "WindowGeometry",
                  windowState = "WindowState";
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

void appendHistory(QDomDocument & doc, QDomElement & root,
                   const Preferences::Playback::History & history)
{
    using namespace QtUtilities::Xml;
    using namespace Names::Playback::History;
    QDomElement e = doc.createElement(localRoot);

    e.appendChild(createElement(doc, maxSize, history.maxSize));
    e.appendChild(createElement(doc, copyPlayedEntryToTop,
                                history.copyPlayedEntryToTop));
    e.appendChild(createElement(doc, saveToDiskImmediately,
                                history.saveToDiskImmediately));
    e.appendChild(createElement(doc, nHiddenDirs, history.nHiddenDirs));
    e.appendChild(createElement(doc, currentIndex, history.currentIndex));

    root.appendChild(e);
}

void appendPlayback(QDomDocument & doc, QDomElement & root,
                    const Preferences::Playback & playback)
{
    using namespace QtUtilities::Xml;
    using namespace Names::Playback;
    QDomElement e = doc.createElement(localRoot);

    e.appendChild(createElement(doc, autoSetExternalPlayerOptions,
                                playback.autoSetExternalPlayerOptions));

    e.appendChild(createElement(doc, nextFromHistory,
                                playback.nextFromHistory));
    e.appendChild(
        createElement(
            doc, startupPolicy,
            static_cast<Preferences::Playback::StartupPolicyUnderlyingType>(
                playback.startupPolicy)));

    appendHistory(doc, e, playback.history);

    root.appendChild(e);
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

void loadHistory(const QDomElement & root,
                 Preferences::Playback::History & history)
{
    using namespace QtUtilities::Xml;
    using namespace Names::Playback::History;
    typedef Preferences::Playback::History H;

    const QDomElement e = getUniqueChild(root, localRoot);

    copyUniqueChildsTextToMax(e, maxSize, history.maxSize, H::maxMaxSize);
    copyUniqueChildsTextTo(e, copyPlayedEntryToTop,
                           history.copyPlayedEntryToTop);
    copyUniqueChildsTextTo(e, saveToDiskImmediately,
                           history.saveToDiskImmediately);
    copyUniqueChildsTextToMax(e, nHiddenDirs,
                              history.nHiddenDirs, H::maxNHiddenDirs);
    copyUniqueChildsTextTo(e, currentIndex, history.currentIndex);
    QtUtilities::checkRange(currentIndex, history.currentIndex,
                            H::multipleItemsIndex, int(H::maxMaxSize));
}

void loadPlayback(const QDomElement & root, Preferences::Playback & playback)
{
    using namespace QtUtilities::Xml;
    using namespace Names::Playback;
    typedef Preferences::Playback P;

    const QDomElement e = getUniqueChild(root, localRoot);

    copyUniqueChildsTextTo(e, autoSetExternalPlayerOptions,
                           playback.autoSetExternalPlayerOptions);

    copyUniqueChildsTextTo(e, nextFromHistory, playback.nextFromHistory);
    {
        P::StartupPolicyUnderlyingType p;
        if (copyUniqueChildsTextToMax(e, startupPolicy, p, P::maxStartupPolicy))
            playback.startupPolicy = static_cast<P::StartupPolicy>(p);
    }

    loadHistory(e, playback.history);
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

}


constexpr std::size_t Preferences::Playback::History::maxMaxSize;
constexpr unsigned Preferences::Playback::History::maxNHiddenDirs;
constexpr int Preferences::Playback::History::multipleItemsIndex;

Preferences::Playback::History::History()
    : maxSize(100), copyPlayedEntryToTop(false), saveToDiskImmediately(false),
      nHiddenDirs(3), currentIndex(0)
{
}


constexpr Preferences::Playback::StartupPolicyUnderlyingType
Preferences::Playback::maxStartupPolicy;

Preferences::Playback::Playback()
    : autoSetExternalPlayerOptions(true), nextFromHistory(false),
      startupPolicy(StartupPolicy::doNothing)
{
}


constexpr unsigned Preferences::maxVentoolCheckInterval;

Preferences::Preferences()
    : alwaysUseFallbackIcons(false),
      notificationAreaIcon(false),
      startToNotificationArea(false),
      closeToNotificationArea(true),
      treeAutoUnfoldedLevels(5),
      treeAutoCleanup(false),
      savePreferencesToDiskImmediately(false),
      ventoolCheckInterval(1000)
{
}

void Preferences::save(const QString & filename) const
{
    using namespace QtUtilities::Xml;
    QDomDocument doc = createDocument();
    QDomElement root = createRoot(doc, Names::root);

    appendPlayback(doc, root, playback);
    appendAddingItemsPolicy(doc, root, addingPolicy);

    root.appendChild(createElement(doc, Names::alwaysUseFallbackIcons,
                                   alwaysUseFallbackIcons));

    root.appendChild(createElement(doc, Names::notificationAreaIcon,
                                   notificationAreaIcon));
    root.appendChild(createElement(doc, Names::startToNotificationArea,
                                   startToNotificationArea));
    root.appendChild(createElement(doc, Names::closeToNotificationArea,
                                   closeToNotificationArea));

    root.appendChild(createElement(doc, Names::treeAutoUnfoldedLevels,
                                   treeAutoUnfoldedLevels));
    root.appendChild(createElement(doc, Names::treeAutoCleanup,
                                   treeAutoCleanup));

    root.appendChild(createElement(doc, Names::savePreferencesToDiskImmediately,
                                   savePreferencesToDiskImmediately));
    root.appendChild(createElement(doc, Names::ventoolCheckInterval,
                                   ventoolCheckInterval));


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

    loadPlayback(root, playback);
    loadAddingItemsPolicy(root, addingPolicy);

    copyUniqueChildsTextTo(root, Names::alwaysUseFallbackIcons,
                           alwaysUseFallbackIcons);

    copyUniqueChildsTextTo(root, Names::notificationAreaIcon,
                           notificationAreaIcon);
    copyUniqueChildsTextTo(root, Names::startToNotificationArea,
                           startToNotificationArea);
    copyUniqueChildsTextTo(root, Names::closeToNotificationArea,
                           closeToNotificationArea);

    copyUniqueChildsTextTo(root, Names::treeAutoUnfoldedLevels,
                           treeAutoUnfoldedLevels);
    copyUniqueChildsTextTo(root, Names::treeAutoCleanup, treeAutoCleanup);

    copyUniqueChildsTextTo(root, Names::savePreferencesToDiskImmediately,
                           savePreferencesToDiskImmediately);
    copyUniqueChildsTextToMax(root, Names::ventoolCheckInterval,
                              ventoolCheckInterval, maxVentoolCheckInterval);


    copyUniqueChildsTextToByteArray(
        root, Names::preferencesWindowGeometry, preferencesWindowGeometry);
    copyUniqueChildsTextToByteArray(
        root, Names::windowGeometry, windowGeometry);
    copyUniqueChildsTextToByteArray(root, Names::windowState, windowState);
}


bool operator == (const Preferences::Playback::History & lhs,
                  const Preferences::Playback::History & rhs)
{
    return lhs.maxSize == rhs.maxSize &&
           lhs.copyPlayedEntryToTop == rhs.copyPlayedEntryToTop &&
           lhs.saveToDiskImmediately == rhs.saveToDiskImmediately &&
           lhs.nHiddenDirs == rhs.nHiddenDirs &&
           lhs.currentIndex == rhs.currentIndex;
}

bool operator == (const Preferences::Playback & lhs,
                  const Preferences::Playback & rhs)
{
    return lhs.history == rhs.history &&
           lhs.autoSetExternalPlayerOptions == rhs.autoSetExternalPlayerOptions
           && lhs.nextFromHistory == rhs.nextFromHistory &&
           lhs.startupPolicy == rhs.startupPolicy;
}

bool operator == (const Preferences & lhs, const Preferences & rhs)
{
    return lhs.playback == rhs.playback &&
           lhs.addingPolicy == rhs.addingPolicy &&

           lhs.alwaysUseFallbackIcons == rhs.alwaysUseFallbackIcons &&

           lhs.notificationAreaIcon == rhs.notificationAreaIcon &&
           lhs.startToNotificationArea == rhs.startToNotificationArea &&
           lhs.closeToNotificationArea == rhs.closeToNotificationArea &&

           lhs.treeAutoUnfoldedLevels == rhs.treeAutoUnfoldedLevels &&
           lhs.treeAutoCleanup == rhs.treeAutoCleanup &&

           lhs.savePreferencesToDiskImmediately ==
           rhs.savePreferencesToDiskImmediately &&
           lhs.ventoolCheckInterval == rhs.ventoolCheckInterval &&

           lhs.preferencesWindowGeometry == rhs.preferencesWindowGeometry &&
           lhs.windowGeometry == rhs.windowGeometry &&
           lhs.windowState == rhs.windowState;
}
