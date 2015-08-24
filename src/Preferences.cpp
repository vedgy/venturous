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

# include "Preferences.hpp"

# include "CustomActions.hpp"
# include "FilePattern.hpp"

# include <VenturousCore/AddingItems.hpp>
# include <VenturousCore/MediaPlayer.hpp>

# include <QtXmlUtilities/ReadingShortcuts.hpp>
# include <QtXmlUtilities/WritingShortcuts.hpp>

# include <CommonUtilities/FunctionConstant.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QDomElement>
# include <QDomDocument>

# include <cstddef>
# include <utility>


namespace
{
namespace Names
{
# define VENTUROUS_preferences_string_constant(name, value) \
    NAMESPACE_FUNCTION_CONSTANT(QString, name, value)

namespace Playback
{
namespace History
{
VENTUROUS_preferences_string_constant(localRoot, "History")
VENTUROUS_preferences_string_constant(maxSize, "MaxSize")
VENTUROUS_preferences_string_constant(copyPlayedEntryToTop,
                                      "CopyPlayedEntryToTop")
VENTUROUS_preferences_string_constant(saveToDiskImmediately,
                                      "SaveToDiskImmediately")
VENTUROUS_preferences_string_constant(nHiddenDirs, "HiddenDirsNumber")
VENTUROUS_preferences_string_constant(currentIndex, "CurrentIndex")
} // END namespace History

VENTUROUS_preferences_string_constant(localRoot, "Playback")
VENTUROUS_preferences_string_constant(playerId, "PlayerID")
VENTUROUS_preferences_string_constant(autoSetExternalPlayerOptions,
                                      "AutoSetExternalPlayerOptions")
VENTUROUS_preferences_string_constant(autoHideExternalPlayerWindow,
                                      "AutoHideExternalPlayerWindow")
VENTUROUS_preferences_string_constant(exitExternalPlayerOnQuit,
                                      "ExitExternalPlayerOnQuit")
VENTUROUS_preferences_string_constant(statusUpdateInterval,
                                      "StatusUpdateInterval")
VENTUROUS_preferences_string_constant(nextFromHistory, "NextFromHistory")
VENTUROUS_preferences_string_constant(desktopNotifications,
                                      "DesktopNotifications")
VENTUROUS_preferences_string_constant(startupPolicy, "StartupPolicy")
} // END namespace Playback

namespace AddingItems
{
VENTUROUS_preferences_string_constant(localRoot, "AddingItems")
VENTUROUS_preferences_string_constant(filePatterns, "FilePatterns")
VENTUROUS_preferences_string_constant(mediaDirFilePatterns,
                                      "MediaDirFilePatterns")
VENTUROUS_preferences_string_constant(pattern, "pattern")
VENTUROUS_preferences_string_constant(enabled, "enabled")
VENTUROUS_preferences_string_constant(policy, "Policy")
VENTUROUS_preferences_string_constant(addFiles, "AddFiles")
VENTUROUS_preferences_string_constant(addMediaDirs, "AddMediaDirs")
VENTUROUS_preferences_string_constant(ifBothAddFiles, "IfBothAddFiles")
VENTUROUS_preferences_string_constant(ifBothAddMediaDirs, "IfBothAddMediaDirs")
} // END namespace AddingItems

namespace CustomActions
{
VENTUROUS_preferences_string_constant(localRoot, "CustomActions")
VENTUROUS_preferences_string_constant(action, "action")
VENTUROUS_preferences_string_constant(enabled, "Enabled")
VENTUROUS_preferences_string_constant(text, "Text")
VENTUROUS_preferences_string_constant(command, "Command")
VENTUROUS_preferences_string_constant(minArgN, "MinArgN")
VENTUROUS_preferences_string_constant(maxArgN, "MaxArgN")
VENTUROUS_preferences_string_constant(type, "Type")
VENTUROUS_preferences_string_constant(comment, "Comment")
} // END namespace CustomActions

VENTUROUS_preferences_string_constant(root, APPLICATION_NAME)
VENTUROUS_preferences_string_constant(alwaysUseFallbackIcons,
                                      "AlwaysUseFallbackIcons")
VENTUROUS_preferences_string_constant(notificationAreaIcon,
                                      "NotificationAreaIcon")
VENTUROUS_preferences_string_constant(startToNotificationArea,
                                      "StartToNotificationArea")
VENTUROUS_preferences_string_constant(closeToNotificationArea,
                                      "CloseToNotificationArea")
VENTUROUS_preferences_string_constant(statusBar, "StatusBar")
VENTUROUS_preferences_string_constant(treeAutoUnfoldedLevels,
                                      "TreeAutoUnfoldedLevels")
VENTUROUS_preferences_string_constant(treeAutoCleanup, "TreeAutoCleanup")
VENTUROUS_preferences_string_constant(savePreferencesToDiskImmediately,
                                      "SavePreferencesToDiskImmediately")
VENTUROUS_preferences_string_constant(ventoolCheckInterval,
                                      "VentoolCheckInterval")

VENTUROUS_preferences_string_constant(preferencesWindowGeometry,
                                      "PreferencesWindowGeometry")
VENTUROUS_preferences_string_constant(windowGeometry, "WindowGeometry")
VENTUROUS_preferences_string_constant(windowState, "WindowState")

# undef VENTUROUS_preferences_string_constant
} // END namespace Names


typedef QtUtilities::XmlWriting::Element XmlElement;

void appendHistory(XmlElement & parent,
                   const Preferences::Playback::History & history)
{
    using namespace Names::Playback::History;
    XmlElement e = parent.appendElement(localRoot());

    e.appendChild(maxSize(), history.maxSize);
    e.appendChild(copyPlayedEntryToTop(), history.copyPlayedEntryToTop);
    e.appendChild(saveToDiskImmediately(), history.saveToDiskImmediately);
    e.appendChild(nHiddenDirs(), history.nHiddenDirs);
    e.appendChild(currentIndex(), history.currentIndex);
}

void appendPlayback(XmlElement & parent, const Preferences::Playback & playback)
{
    using namespace Names::Playback;
    XmlElement e = parent.appendElement(localRoot());

    e.appendChild(playerId(), playback.playerId);

    e.appendChild(autoSetExternalPlayerOptions(),
                  playback.autoSetExternalPlayerOptions);
    e.appendChild(autoHideExternalPlayerWindow(),
                  playback.autoHideExternalPlayerWindow);
    e.appendChild(exitExternalPlayerOnQuit(),
                  playback.exitExternalPlayerOnQuit);

    e.appendChild(statusUpdateInterval(), playback.statusUpdateInterval);
    e.appendChild(nextFromHistory(), playback.nextFromHistory);
    e.appendChild(desktopNotifications(), playback.desktopNotifications);
    e.appendChild(startupPolicy(),
                  static_cast <
                  Preferences::Playback::StartupPolicyUnderlyingType >(
                      playback.startupPolicy));

    appendHistory(e, playback.history);
}

void appendFilePatternList(XmlElement & parent, const QString & listName,
                           const FilePatternList & list)
{
    XmlElement e = parent.appendElement(listName);
    for (const FilePattern & p : list) {
        using namespace Names::AddingItems;
        e.appendChildWithAttribute(pattern(), p.pattern, enabled(), p.enabled);
    }
}

void appendAddingPolicy(XmlElement & parent, const AddingItems::Policy & p)
{
    using namespace Names::AddingItems;
    XmlElement e = parent.appendElement(policy());
    e.appendChild(addFiles(), p.addFiles);
    e.appendChild(addMediaDirs(), p.addMediaDirs);
    e.appendChild(ifBothAddFiles(), p.ifBothAddFiles);
    e.appendChild(ifBothAddMediaDirs(), p.ifBothAddMediaDirs);
}

void appendAddingItems(XmlElement & parent,
                       const Preferences::AddingPatterns & patterns,
                       const AddingItems::Policy & policy)
{
    using namespace Names::AddingItems;
    XmlElement base = parent.appendElement(localRoot());
    appendFilePatternList(base, filePatterns(), patterns.filePatterns);
    appendFilePatternList(base, mediaDirFilePatterns(),
                          patterns.mediaDirFilePatterns);
    appendAddingPolicy(base, policy);
}

void appendCustomActions(XmlElement & parent,
                         const CustomActions::Actions & actions)
{
    using namespace Names::CustomActions;
    XmlElement base = parent.appendElement(localRoot());

    for (const CustomActions::Action & a : actions) {
        XmlElement e = base.appendElement(action());
        e.appendChild(enabled(), a.enabled);
        e.appendChild(text(), a.text);
        e.appendChild(command(), a.command);
        e.appendChild(minArgN(), a.minArgN);
        e.appendChild(maxArgN(), a.maxArgN);
        e.appendChild(type(),
                      static_cast<CustomActions::Action::TypeUnderlyingType>(
                          a.type));
        e.appendChild(comment(), a.comment);
    }
}


void loadHistory(const QDomElement & parent,
                 Preferences::Playback::History & history)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::Playback::History;
    typedef Preferences::Playback::History H;

    const QDomElement e = getUniqueChild(parent, localRoot());
    if (e.isNull())
        return;
    copyUniqueChildsTextToMax(e, maxSize(), history.maxSize, H::maxMaxSize);
    copyUniqueChildsTextTo(e, copyPlayedEntryToTop(),
                           history.copyPlayedEntryToTop);
    copyUniqueChildsTextTo(e, saveToDiskImmediately(),
                           history.saveToDiskImmediately);
    copyUniqueChildsTextToRange(e, nHiddenDirs(), history.nHiddenDirs,
                                H::minNHiddenDirs, H::maxNHiddenDirs);
    copyUniqueChildsTextToRange(e, currentIndex(), history.currentIndex,
                                H::multipleItemsIndex, int(H::maxMaxSize));
}

void loadPlayback(const QDomElement & parent, Preferences::Playback & playback)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::Playback;
    typedef Preferences::Playback P;

    const QDomElement e = getUniqueChild(parent, localRoot());
    if (e.isNull())
        return;

    copyUniqueChildsTextToMax(
        e, playerId(), playback.playerId,
        static_cast<unsigned>(GetMediaPlayer::playerList().size() - 1));

    copyUniqueChildsTextTo(e, autoSetExternalPlayerOptions(),
                           playback.autoSetExternalPlayerOptions);
    copyUniqueChildsTextTo(e, autoHideExternalPlayerWindow(),
                           playback.autoHideExternalPlayerWindow);
    copyUniqueChildsTextTo(e, exitExternalPlayerOnQuit(),
                           playback.exitExternalPlayerOnQuit);

    copyUniqueChildsTextToRange0Allowed(e, statusUpdateInterval(),
                                        playback.statusUpdateInterval,
                                        P::minStatusUpdateInterval,
                                        P::maxStatusUpdateInterval);
    copyUniqueChildsTextTo(e, nextFromHistory(), playback.nextFromHistory);
    copyUniqueChildsTextTo(e, desktopNotifications(),
                           playback.desktopNotifications);
    {
        P::StartupPolicyUnderlyingType p;
        if (copyUniqueChildsTextToMax(e, startupPolicy(),
                                      p, P::maxStartupPolicy)) {
            playback.startupPolicy = static_cast<P::StartupPolicy>(p);
        }
    }

    loadHistory(e, playback.history);
}

void addToPatternList(FilePatternList & patternList, QStringList && stringList,
                      bool enabled)
{
    for (QString & pattern : stringList)
        patternList.push_back( { std::move(pattern), enabled });
}

Preferences::AddingPatterns defaultAddingPatterns()
{
    Preferences::AddingPatterns patterns;
    {
        QStringList metadata = AddingItems::allMetadataPatterns();
        QStringList audio = AddingItems::allAudioPatterns();
        FilePatternList & patternList = patterns.filePatterns;
        patternList.reserve(std::size_t(metadata.size() + audio.size()));
        addToPatternList(patternList, std::move(metadata), true);
        addToPatternList(patternList, std::move(audio), false);
    }
    {
        QStringList audio = AddingItems::allAudioPatterns();
        FilePatternList & patternList = patterns.mediaDirFilePatterns;
        patternList.reserve(std::size_t(audio.size()));
        addToPatternList(patternList, std::move(audio), true);
    }
    return patterns;
}

FilePattern domElementToFilePattern(const QDomElement & e)
{
    FilePattern pattern { e.text(), true };
    QtUtilities::XmlReading::copyElementsAttributeTo(
        e, Names::AddingItems::enabled(), pattern.enabled);
    return pattern;
}

void loadFilePatternList(const QDomElement & parent, const QString & listName,
                         FilePatternList & list)
{
    using namespace QtUtilities::XmlReading;
    const QDomElement e = getUniqueChild(parent, listName);
    if (e.isNull())
        return;
    list = getChildren<FilePatternList>(e, Names::AddingItems::pattern(),
                                        domElementToFilePattern);
}

void loadAddingPolicy(const QDomElement & parent, AddingItems::Policy & p)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::AddingItems;
    const QDomElement e = getUniqueChild(parent, policy());
    if (e.isNull())
        return;
    copyUniqueChildsTextTo(e, addFiles(), p.addFiles);
    copyUniqueChildsTextTo(e, addMediaDirs(), p.addMediaDirs);
    copyUniqueChildsTextTo(e, ifBothAddFiles(), p.ifBothAddFiles);
    copyUniqueChildsTextTo(e, ifBothAddMediaDirs(), p.ifBothAddMediaDirs);
}

void loadAddingItems(const QDomElement & parent,
                     Preferences::AddingPatterns & patterns,
                     AddingItems::Policy & policy)
{
    using namespace Names::AddingItems;
    const QDomElement e = QtUtilities::XmlReading::getUniqueChild(
                              parent, localRoot());
    if (e.isNull())
        return;
    loadFilePatternList(e, filePatterns(), patterns.filePatterns);
    loadFilePatternList(e, mediaDirFilePatterns(),
                        patterns.mediaDirFilePatterns);
    loadAddingPolicy(e, policy);
}

CustomActions::Actions defaultCustomActions()
{
    const QString mustBeInstalled = QObject::tr(" must be installed.");
    const QString xdgUtilsMustBeInstalled = "xdg-utils" + mustBeInstalled;
    const QString replaceCommandWith =
        QObject::tr("Replace \"%1\" with your preferred %2.");
    const QString defaultFileManager = "thunar", defaultTextEditor = "mousepad";
    CustomActions::Actions actions {
        CustomActions::Action {
            QObject::tr("Open with default application"), "xdg-open ?",
            1, 1, CustomActions::Action::Type::anyItem, true,
            xdgUtilsMustBeInstalled
        },
        CustomActions::Action {
            QObject::tr("Open containing directory"), "xdg-open @",
            1, -1, CustomActions::Action::Type::anyItem, true,
            xdgUtilsMustBeInstalled
        },
        CustomActions::Action {
            QObject::tr("Open in file manager"), defaultFileManager + " ?",
            0, -1, CustomActions::Action::Type::anyItem, false,
            replaceCommandWith.arg("thunar", QObject::tr("file manager"))
        },
        CustomActions::Action {
            QObject::tr("Open in VLC"), "vlc ?",
            0, -1, CustomActions::Action::Type::anyItem, false,
            "VLC" + mustBeInstalled
        },
        CustomActions::Action {
            QObject::tr("View/edit text file"), defaultTextEditor + " ?",
            0, -1, CustomActions::Action::Type::file, false,
            replaceCommandWith.arg("mousepad", QObject::tr("text editor"))
        },
        CustomActions::Action {
            QObject::tr("Move to music trash"), "mv ? ~/Music/trash/",
            1, -1, CustomActions::Action::Type::anyItem, false,
            QObject::tr("~/Music/trash directory must exist.")
        },
        CustomActions::Action {
            QObject::tr("Move to Trash"), "trash-put ?",
            1, -1, CustomActions::Action::Type::anyItem, false,
            "trash-cli" + mustBeInstalled
        }
    };

# ifdef WIN32_DEFAULT_CUSTOM_ACTIONS
    {
        CustomActions::Action & defaultApplication = actions[0];
        defaultApplication.enabled = false;

        CustomActions::Action & containingDirectory = actions[1];
        containingDirectory.command = "explorer @";
        containingDirectory.comment.clear();

        CustomActions::Action & fileManager = actions[2];
        fileManager.command = "explorer ?";
        fileManager.maxArgN = 1;
        fileManager.enabled = true;
        fileManager.comment.clear();

        CustomActions::Action & textEditor = actions[4];
        textEditor.command = "notepad ?";
        textEditor.maxArgN = 1;
        textEditor.comment.clear();
    }
# endif

    return actions;
}

CustomActions::Action domElementToCustomAction(const QDomElement & e)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::CustomActions;
    using CustomActions::Action;
    Action a = Action::getEmpty();
    copyUniqueChildsTextTo(e, enabled(), a.enabled);
    copyUniqueChildsTextTo(e, text(), a.text);
    copyUniqueChildsTextTo(e, command(), a.command);
    copyUniqueChildsTextToRange(e, minArgN(), a.minArgN,
                                Action::minMinArgN, Action::maxMinArgN);
    copyUniqueChildsTextToRange(e, maxArgN(), a.maxArgN,
                                Action::minMaxArgN, Action::maxMaxArgN);
    {
        Action::TypeUnderlyingType t;
        if (copyUniqueChildsTextToMax(e, type(), t, Action::maxType))
            a.type = static_cast<Action::Type>(t);
    }
    copyUniqueChildsTextTo(e, comment(), a.comment);
    return a;
}

void loadCustomActions(const QDomElement & parent,
                       CustomActions::Actions & actions)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::CustomActions;
    const QDomElement e = getUniqueChild(parent, localRoot());
    if (e.isNull())
        return;
    actions = getChildren<CustomActions::Actions>(
                  e, action(), domElementToCustomAction);
}

} // END unnamed namespace


constexpr std::size_t Preferences::Playback::History::maxMaxSize;
constexpr int Preferences::Playback::History::minNHiddenDirs;
constexpr int Preferences::Playback::History::maxNHiddenDirs;
constexpr int Preferences::Playback::History::multipleItemsIndex;

Preferences::Playback::History::History()
    : maxSize(100), copyPlayedEntryToTop(false), saveToDiskImmediately(false),
      nHiddenDirs(-2), currentIndex(0)
{}


constexpr unsigned Preferences::Playback::minStatusUpdateInterval;
constexpr unsigned Preferences::Playback::defaultStatusUpdateInterval;
constexpr unsigned Preferences::Playback::maxStatusUpdateInterval;
constexpr Preferences::Playback::StartupPolicyUnderlyingType
Preferences::Playback::maxStartupPolicy;

Preferences::Playback::Playback()
    : playerId(0), autoSetExternalPlayerOptions(true),
      autoHideExternalPlayerWindow(false), exitExternalPlayerOnQuit(true),
      statusUpdateInterval(0), nextFromHistory(false),
      desktopNotifications(true), startupPolicy(StartupPolicy::doNothing)
{}


AddingItems::Patterns Preferences::AddingPatterns::enabledPatternLists() const
{
    return {
        getEnabledFilePatterns(filePatterns),
        getEnabledFilePatterns(mediaDirFilePatterns)
    };
}


constexpr unsigned Preferences::minVentoolCheckInterval;
constexpr unsigned Preferences::defaultVentoolCheckInterval;
constexpr unsigned Preferences::maxVentoolCheckInterval;

Preferences::Preferences()
    : addingPatterns(defaultAddingPatterns()),
      addingPolicy(),
      alwaysUseFallbackIcons(false),
      notificationAreaIcon(false),
      startToNotificationArea(false),
      closeToNotificationArea(false),
      statusBar(false),
      treeAutoUnfoldedLevels(5),
      treeAutoCleanup(false),
      savePreferencesToDiskImmediately(false),
      ventoolCheckInterval(defaultVentoolCheckInterval),
      customActions(defaultCustomActions())
{}

void Preferences::save(const QString & filename) const
{
    using namespace QtUtilities::XmlWriting;
    Document doc(Names::root());
    Element & root = doc.root;

    root.appendChild(Names::alwaysUseFallbackIcons(), alwaysUseFallbackIcons);

    root.appendChild(Names::notificationAreaIcon(), notificationAreaIcon);
    root.appendChild(Names::startToNotificationArea(), startToNotificationArea);
    root.appendChild(Names::closeToNotificationArea(), closeToNotificationArea);

    root.appendChild(Names::statusBar(), statusBar);

    root.appendChild(Names::treeAutoUnfoldedLevels(), treeAutoUnfoldedLevels);
    root.appendChild(Names::treeAutoCleanup(), treeAutoCleanup);

    root.appendChild(Names::savePreferencesToDiskImmediately(),
                     savePreferencesToDiskImmediately);
    root.appendChild(Names::ventoolCheckInterval(), ventoolCheckInterval);

    appendPlayback(root, playback);
    appendAddingItems(root, addingPatterns, addingPolicy);
    appendCustomActions(root, customActions);


    root.appendChildByteArray(Names::preferencesWindowGeometry(),
                              preferencesWindowGeometry);
    root.appendChildByteArray(Names::windowGeometry(), windowGeometry);
    root.appendChildByteArray(Names::windowState(), windowState);

    doc.save(filename);
}


void Preferences::load(const QString & filename)
{
    using namespace QtUtilities::XmlReading;
    const QDomElement root = loadRoot(filename, Names::root());
    if (root.isNull())
        return;

    copyUniqueChildsTextTo(root, Names::alwaysUseFallbackIcons(),
                           alwaysUseFallbackIcons);

    copyUniqueChildsTextTo(root, Names::notificationAreaIcon(),
                           notificationAreaIcon);
    copyUniqueChildsTextTo(root, Names::startToNotificationArea(),
                           startToNotificationArea);
    copyUniqueChildsTextTo(root, Names::closeToNotificationArea(),
                           closeToNotificationArea);

    copyUniqueChildsTextTo(root, Names::statusBar(), statusBar);

    copyUniqueChildsTextTo(root, Names::treeAutoUnfoldedLevels(),
                           treeAutoUnfoldedLevels);
    copyUniqueChildsTextTo(root, Names::treeAutoCleanup(), treeAutoCleanup);

    copyUniqueChildsTextTo(root, Names::savePreferencesToDiskImmediately(),
                           savePreferencesToDiskImmediately);
    copyUniqueChildsTextToRange0Allowed(root, Names::ventoolCheckInterval(),
                                        ventoolCheckInterval,
                                        minVentoolCheckInterval,
                                        maxVentoolCheckInterval);

    loadPlayback(root, playback);
    loadAddingItems(root, addingPatterns, addingPolicy);
    loadCustomActions(root, customActions);


    copyUniqueChildsTextToByteArray(
        root, Names::preferencesWindowGeometry(), preferencesWindowGeometry);
    copyUniqueChildsTextToByteArray(
        root, Names::windowGeometry(), windowGeometry);
    copyUniqueChildsTextToByteArray(root, Names::windowState(), windowState);
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
           lhs.playerId == rhs.playerId &&
           lhs.autoSetExternalPlayerOptions == rhs.autoSetExternalPlayerOptions
           &&
           lhs.autoHideExternalPlayerWindow == rhs.autoHideExternalPlayerWindow
           && lhs.exitExternalPlayerOnQuit == rhs.exitExternalPlayerOnQuit &&
           lhs.statusUpdateInterval == rhs.statusUpdateInterval &&
           lhs.nextFromHistory == rhs.nextFromHistory &&
           lhs.desktopNotifications == rhs.desktopNotifications &&
           lhs.startupPolicy == rhs.startupPolicy;
}

bool operator == (const Preferences::AddingPatterns & lhs,
                  const Preferences::AddingPatterns & rhs)
{
    return lhs.filePatterns == rhs.filePatterns &&
           lhs.mediaDirFilePatterns == rhs.mediaDirFilePatterns;
}

bool operator == (const Preferences & lhs, const Preferences & rhs)
{
    return lhs.playback == rhs.playback &&

           lhs.addingPatterns == rhs.addingPatterns &&
           lhs.addingPolicy == rhs.addingPolicy &&

           lhs.alwaysUseFallbackIcons == rhs.alwaysUseFallbackIcons &&

           lhs.notificationAreaIcon == rhs.notificationAreaIcon &&
           lhs.startToNotificationArea == rhs.startToNotificationArea &&
           lhs.closeToNotificationArea == rhs.closeToNotificationArea &&

           lhs.statusBar == rhs.statusBar &&

           lhs.treeAutoUnfoldedLevels == rhs.treeAutoUnfoldedLevels &&
           lhs.treeAutoCleanup == rhs.treeAutoCleanup &&

           lhs.savePreferencesToDiskImmediately ==
           rhs.savePreferencesToDiskImmediately &&
           lhs.ventoolCheckInterval == rhs.ventoolCheckInterval &&

           lhs.customActions == rhs.customActions &&

           lhs.preferencesWindowGeometry == rhs.preferencesWindowGeometry &&
           lhs.windowGeometry == rhs.windowGeometry &&
           lhs.windowState == rhs.windowState;
}
