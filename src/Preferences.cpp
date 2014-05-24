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

# include "CustomActions.hpp"

# include <VenturousCore/AddingItems.hpp>

# include <QtXmlUtilities/ReadingShortcuts.hpp>
# include <QtXmlUtilities/WritingShortcuts.hpp>

# include <QString>
# include <QStringList>
# include <QObject>
# include <QDomElement>
# include <QDomDocument>

# include <cstddef>


namespace
{
namespace Names
{
# define VENTUROUS_PREFERENCES_STRING_CONSTANT(NAME, VALUE) \
    inline const QString & NAME() {                         \
        static const QString value{ VALUE }; return value; }

namespace Playback
{
namespace History
{
VENTUROUS_PREFERENCES_STRING_CONSTANT(localRoot, "History")
VENTUROUS_PREFERENCES_STRING_CONSTANT(maxSize, "MaxSize")
VENTUROUS_PREFERENCES_STRING_CONSTANT(copyPlayedEntryToTop,
                                      "CopyPlayedEntryToTop")
VENTUROUS_PREFERENCES_STRING_CONSTANT(saveToDiskImmediately,
                                      "SaveToDiskImmediately")
VENTUROUS_PREFERENCES_STRING_CONSTANT(nHiddenDirs, "HiddenDirsNumber")
VENTUROUS_PREFERENCES_STRING_CONSTANT(currentIndex, "CurrentIndex")
}

VENTUROUS_PREFERENCES_STRING_CONSTANT(localRoot, "Playback")
VENTUROUS_PREFERENCES_STRING_CONSTANT(autoSetExternalPlayerOptions,
                                      "AutoSetExternalPlayerOptions")
VENTUROUS_PREFERENCES_STRING_CONSTANT(autoHideExternalPlayerWindow,
                                      "AutoHideExternalPlayerWindow")
VENTUROUS_PREFERENCES_STRING_CONSTANT(nextFromHistory, "NextFromHistory")
VENTUROUS_PREFERENCES_STRING_CONSTANT(desktopNotifications,
                                      "DesktopNotifications")
VENTUROUS_PREFERENCES_STRING_CONSTANT(startupPolicy, "StartupPolicy")
}

namespace AddingItemsPolicy
{
VENTUROUS_PREFERENCES_STRING_CONSTANT(localRoot, "AddingItemsPolicy")
VENTUROUS_PREFERENCES_STRING_CONSTANT(filePatterns, "FilePatterns")
VENTUROUS_PREFERENCES_STRING_CONSTANT(mediaDirFilePatterns,
                                      "MediaDirFilePatterns")
VENTUROUS_PREFERENCES_STRING_CONSTANT(pattern, "pattern")
VENTUROUS_PREFERENCES_STRING_CONSTANT(addFiles, "AddFiles")
VENTUROUS_PREFERENCES_STRING_CONSTANT(addMediaDirs, "AddMediaDirs")
VENTUROUS_PREFERENCES_STRING_CONSTANT(ifBothAddFiles, "IfBothAddFiles")
VENTUROUS_PREFERENCES_STRING_CONSTANT(ifBothAddMediaDirs, "IfBothAddMediaDirs")
}

namespace CustomActions
{
VENTUROUS_PREFERENCES_STRING_CONSTANT(localRoot, "CustomActions")
VENTUROUS_PREFERENCES_STRING_CONSTANT(action, "action")
VENTUROUS_PREFERENCES_STRING_CONSTANT(enabled, "Enabled")
VENTUROUS_PREFERENCES_STRING_CONSTANT(text, "Text")
VENTUROUS_PREFERENCES_STRING_CONSTANT(command, "Command")
VENTUROUS_PREFERENCES_STRING_CONSTANT(minArgN, "MinArgN")
VENTUROUS_PREFERENCES_STRING_CONSTANT(maxArgN, "MaxArgN")
VENTUROUS_PREFERENCES_STRING_CONSTANT(type, "Type")
VENTUROUS_PREFERENCES_STRING_CONSTANT(comment, "Comment")
}

VENTUROUS_PREFERENCES_STRING_CONSTANT(root, APPLICATION_NAME)
VENTUROUS_PREFERENCES_STRING_CONSTANT(alwaysUseFallbackIcons,
                                      "AlwaysUseFallbackIcons")
VENTUROUS_PREFERENCES_STRING_CONSTANT(notificationAreaIcon,
                                      "NotificationAreaIcon")
VENTUROUS_PREFERENCES_STRING_CONSTANT(startToNotificationArea,
                                      "StartToNotificationArea")
VENTUROUS_PREFERENCES_STRING_CONSTANT(closeToNotificationArea,
                                      "CloseToNotificationArea")
VENTUROUS_PREFERENCES_STRING_CONSTANT(statusBar, "StatusBar")
VENTUROUS_PREFERENCES_STRING_CONSTANT(treeAutoUnfoldedLevels,
                                      "TreeAutoUnfoldedLevels")
VENTUROUS_PREFERENCES_STRING_CONSTANT(treeAutoCleanup, "TreeAutoCleanup")
VENTUROUS_PREFERENCES_STRING_CONSTANT(savePreferencesToDiskImmediately,
                                      "SavePreferencesToDiskImmediately")
VENTUROUS_PREFERENCES_STRING_CONSTANT(ventoolCheckInterval,
                                      "VentoolCheckInterval")

VENTUROUS_PREFERENCES_STRING_CONSTANT(preferencesWindowGeometry,
                                      "PreferencesWindowGeometry")
VENTUROUS_PREFERENCES_STRING_CONSTANT(windowGeometry, "WindowGeometry")
VENTUROUS_PREFERENCES_STRING_CONSTANT(windowState, "WindowState")

# undef VENTUROUS_PREFERENCES_STRING_CONSTANT
}


typedef QtUtilities::XmlWriting::Element XmlElement;

void appendHistory(XmlElement & parent,
                   const Preferences::Playback::History & history)
{
    using namespace Names::Playback::History;
    XmlElement e = parent.appendChild(localRoot());

    e.appendChild(maxSize(), history.maxSize);
    e.appendChild(copyPlayedEntryToTop(), history.copyPlayedEntryToTop);
    e.appendChild(saveToDiskImmediately(), history.saveToDiskImmediately);
    e.appendChild(nHiddenDirs(), history.nHiddenDirs);
    e.appendChild(currentIndex(), history.currentIndex);
}

void appendPlayback(XmlElement & parent, const Preferences::Playback & playback)
{
    using namespace Names::Playback;
    XmlElement e = parent.appendChild(localRoot());

    e.appendChild(autoSetExternalPlayerOptions(),
                  playback.autoSetExternalPlayerOptions);
    e.appendChild(autoHideExternalPlayerWindow(),
                  playback.autoHideExternalPlayerWindow);

    e.appendChild(nextFromHistory(), playback.nextFromHistory);
    e.appendChild(desktopNotifications(), playback.desktopNotifications);
    e.appendChild(startupPolicy(),
                  static_cast <
                  Preferences::Playback::StartupPolicyUnderlyingType >(
                      playback.startupPolicy));

    appendHistory(e, playback.history);
}

void appendAddingItemsPolicy(XmlElement & parent,
                             const AddingItems::Policy & policy)
{
    using namespace Names::AddingItemsPolicy;
    XmlElement e = parent.appendChild(localRoot());

    e.appendQStringList(filePatterns(), pattern(), policy.filePatterns);
    e.appendQStringList(mediaDirFilePatterns(), pattern(),
                        policy.mediaDirFilePatterns);
    e.appendChild(addFiles(), policy.addFiles);
    e.appendChild(addMediaDirs(), policy.addMediaDirs);
    e.appendChild(ifBothAddFiles(), policy.ifBothAddFiles);
    e.appendChild(ifBothAddMediaDirs(), policy.ifBothAddMediaDirs);
}

void appendCustomActions(XmlElement & parent,
                         const CustomActions::Actions & actions)
{
    using namespace Names::CustomActions;
    XmlElement base = parent.appendChild(localRoot());

    for (const CustomActions::Action & a : actions) {
        XmlElement e = base.appendChild(action());
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

    copyUniqueChildsTextTo(e, autoSetExternalPlayerOptions(),
                           playback.autoSetExternalPlayerOptions);
    copyUniqueChildsTextTo(e, autoHideExternalPlayerWindow(),
                           playback.autoHideExternalPlayerWindow);

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

void loadAddingItemsPolicy(const QDomElement & parent,
                           AddingItems::Policy & policy)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::AddingItemsPolicy;
    const QDomElement e = getUniqueChild(parent, localRoot());

    copyQStringListTo(e, filePatterns(), pattern(), policy.filePatterns);
    copyQStringListTo(e, mediaDirFilePatterns(), pattern(),
                      policy.mediaDirFilePatterns);
    copyUniqueChildsTextTo(e, addFiles(), policy.addFiles);
    copyUniqueChildsTextTo(e, addMediaDirs(), policy.addMediaDirs);
    copyUniqueChildsTextTo(e, ifBothAddFiles(), policy.ifBothAddFiles);
    copyUniqueChildsTextTo(e, ifBothAddMediaDirs(), policy.ifBothAddMediaDirs);
}

CustomActions::Actions defaultCustomActions()
{
    const QString mustBeInstalled = QObject::tr(" must be installed.");
    CustomActions::Actions actions {
        CustomActions::Action {
            true, QObject::tr("Open in file manager"), "thunar ?", 0, -1,
            CustomActions::Action::Type::anyItem,
            "Thunar" + mustBeInstalled
        },
        CustomActions::Action {
            false, QObject::tr("Open in VLC"), "vlc ?", 0, -1,
            CustomActions::Action::Type::anyItem,
            "VLC" + mustBeInstalled
        },
        CustomActions::Action {
            false, QObject::tr("View/edit text file"), "mousepad ?", 0, -1,
            CustomActions::Action::Type::file,
            "Mousepad" + mustBeInstalled
        },
        CustomActions::Action {
            false, QObject::tr("Open containing directory"),
            R"(bash -c "thunar \"`dirname \"?\"`\"")", 1, 1,
            CustomActions::Action::Type::anyItem,
            "Thunar" + mustBeInstalled +
            QObject::tr(" Does not work correctly if path contains "
            "certain special symbols.")
        },
        CustomActions::Action {
            false, QObject::tr("Move to music trash"), "mv ? ~/Music/trash/",
            1, -1, CustomActions::Action::Type::anyItem,
            QObject::tr("~/Music/trash directory must exist.")
        },
        CustomActions::Action {
            false, QObject::tr("Move to Trash"), "trash-put ?", 1, -1,
            CustomActions::Action::Type::anyItem,
            "trash-cli" + mustBeInstalled
        }
    };

# ifdef WIN32_DEFAULT_CUSTOM_ACTIONS
    {
        CustomActions::Action & fileManager = actions[0];
        fileManager.command = "explorer ?";
        fileManager.maxArgN = 1;
        fileManager.comment.clear();

        CustomActions::Action & textEditor = actions[2];
        textEditor.command = "notepad ?";
        textEditor.maxArgN = 1;
        textEditor.comment.clear();
    }
# endif

    return actions;
}

void loadCustomActions(const QDomElement & parent,
                       CustomActions::Actions & actions)
{
    using namespace QtUtilities::XmlReading;
    using namespace Names::CustomActions;
    const QDomElement base = getUniqueChild(parent, localRoot());
    if (base.isNull())
        return;

    const auto collection = getChildren(base, action());
    typedef CustomActions::Action Action;
    actions.resize(collection.size(), Action::getEmpty());
    for (std::size_t i = 0; i < collection.size(); ++i) {
        const QDomElement & e = collection[i];
        Action & a = actions[i];

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
    }
}

}


constexpr std::size_t Preferences::Playback::History::maxMaxSize;
constexpr int Preferences::Playback::History::minNHiddenDirs;
constexpr int Preferences::Playback::History::maxNHiddenDirs;
constexpr int Preferences::Playback::History::multipleItemsIndex;

Preferences::Playback::History::History()
    : maxSize(100), copyPlayedEntryToTop(false), saveToDiskImmediately(false),
      nHiddenDirs(-2), currentIndex(0)
{
}


constexpr Preferences::Playback::StartupPolicyUnderlyingType
Preferences::Playback::maxStartupPolicy;

Preferences::Playback::Playback()
    : autoSetExternalPlayerOptions(true), autoHideExternalPlayerWindow(false),
      nextFromHistory(false), desktopNotifications(true),
      startupPolicy(StartupPolicy::doNothing)
{
}


constexpr unsigned Preferences::maxVentoolCheckInterval;

Preferences::Preferences()
    : alwaysUseFallbackIcons(false),
      notificationAreaIcon(false),
      startToNotificationArea(false),
      closeToNotificationArea(false),
      statusBar(false),
      treeAutoUnfoldedLevels(5),
      treeAutoCleanup(false),
      savePreferencesToDiskImmediately(false),
      ventoolCheckInterval(1000),
      customActions(defaultCustomActions())
{
}

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
    appendAddingItemsPolicy(root, addingPolicy);
    appendCustomActions(root, customActions);


    root.appendByteArray(Names::preferencesWindowGeometry(),
                         preferencesWindowGeometry);
    root.appendByteArray(Names::windowGeometry(), windowGeometry);
    root.appendByteArray(Names::windowState(), windowState);

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
    copyUniqueChildsTextToMax(root, Names::ventoolCheckInterval(),
                              ventoolCheckInterval, maxVentoolCheckInterval);

    loadPlayback(root, playback);
    loadAddingItemsPolicy(root, addingPolicy);
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
           lhs.autoSetExternalPlayerOptions == rhs.autoSetExternalPlayerOptions
           &&
           lhs.autoHideExternalPlayerWindow == rhs.autoHideExternalPlayerWindow
           && lhs.nextFromHistory == rhs.nextFromHistory &&
           lhs.desktopNotifications == rhs.desktopNotifications &&
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
