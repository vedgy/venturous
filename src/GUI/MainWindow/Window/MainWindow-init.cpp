/*
 This file is part of Venturous.
 Copyright (C) 2014, 2015, 2019 Igor Kushnir <igorkuo AT Google mail>

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

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
# include <iostream>
# endif


# include "MainWindow.hpp"

# include "PlaylistComponent.hpp"
# include "PlaybackComponent.hpp"
# include "PreferencesComponent.hpp"
# include "Icons.hpp"
# include "Actions.hpp"
# include "Preferences.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QSharedMemory>
# include <QCoreApplication>
# include <QAction>
# include <QSizePolicy>
# include <QLabel>
# include <QMenu>
# include <QMenuBar>
# include <QToolBar>
# include <QApplication>

# include <cassert>
# include <utility>
# include <functional>
# include <memory>


namespace
{
void initMenuBar(QMenuBar & menuBar, const Actions & actions)
{
    {
        QMenu * const file = menuBar.addMenu(QObject::tr("&File"));
        file->addActions( { actions.file.preferences,
                            actions.file.preferencesDirectory, actions.file.quit
                          });
    }
    {
        const Actions::Playback & pbA = actions.playback;
        QMenu * const playback = menuBar.addMenu(QObject::tr("Play&back"));
        playback->addActions( { pbA.play, pbA.pause, pbA.stop, pbA.previous,
                                pbA.replayLast
                              });
        playback->insertSeparator(pbA.previous);
        playback->addSeparator();
        playback->addActions( { pbA.nextFromHistory, pbA.nextRandom,
                                pbA.next
                              });
        playback->addSeparator();
        playback->addAction(pbA.playAll);
        playback->addSeparator();
        {
            QMenu * const externalPlayer =
                playback->addMenu(QObject::tr("&External player"));
            externalPlayer->addActions( { pbA.showExternalPlayerWindow,
                                          pbA.hideExternalPlayerWindow,
                                          pbA.setExternalPlayerOptions,
                                          pbA.updateStatus
                                        });
        }
        {
            QMenu * const history = playback->addMenu(QObject::tr("&History"));
            history->addActions( { pbA.importHistory, pbA.exportHistory });
            history->addSeparator();
            history->addAction(pbA.clearHistory);
        }
    }
    {
        const Actions::Playlist & plA = actions.playlist;
        QMenu * const playlist = menuBar.addMenu(QObject::tr("Play&list"));
        playlist->addActions( {
            plA.editMode, plA.applyChanges, plA.cancelChanges
        });
        playlist->addSeparator();
        playlist->addActions( {
            plA.addFiles, plA.addDirectory, plA.cleanUp, plA.clear,
            plA.restorePrevious
        });
        playlist->addSeparator();
        playlist->addActions( { plA.load, plA.saveAs });
    }
    {
        QMenu * const help = menuBar.addMenu(QObject::tr("&Help"));
        help->addActions( { actions.help.help, actions.help.about });
    }
}

void addStretch(QToolBar & toolBar)
{
    QLabel * const stretch = new QLabel("__");
    stretch->setAlignment(Qt::AlignCenter);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar.addWidget(stretch);
}

void initToolBar(QToolBar & toolBar, const Actions & actions)
{
    const Actions::Playback & pbA = actions.playback;
    toolBar.addActions( { pbA.play, pbA.pause, pbA.stop, pbA.previous,
                          pbA.next
                        });
    addStretch(toolBar);

    toolBar.addActions( { pbA.showExternalPlayerWindow,
                          pbA.hideExternalPlayerWindow
                        });
    addStretch(toolBar);

    toolBar.addAction(actions.help.help);
    toolBar.addAction(actions.file.preferences);
    addStretch(toolBar);


    const Actions::Playlist & plA = actions.playlist;

    toolBar.addActions( { plA.load, plA.saveAs });
    addStretch(toolBar);

    toolBar.addActions( { plA.editMode, plA.addFiles, plA.addDirectory });
    toolBar.insertSeparator(plA.addDirectory);

    const Actions::AddingPolicy & apA = actions.addingPolicy;
    toolBar.addActions( { apA.audioFile, apA.mediaDir,
                          apA.bothAudioFile, apA.bothMediaDir
                        });
}

}


MainWindow::MainWindow(
    std::unique_ptr<QSharedMemory> sharedMemory, bool & cancelled,
    QWidget * const parent, const Qt::WindowFlags flags)
    : QMainWindow(parent, flags), sharedMemory_(std::move(sharedMemory)),
      toolBar_(tr("Toolbar")), inputController_(this)
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "PREFERENCES_DIR = " << PREFERENCES_DIR << std::endl;
# endif
    assert(sharedMemory_ && "Valid shared memory expected, nullptr found.");

    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
            SLOT(onAboutToQuit()));
    connect(qApp, SIGNAL(commitDataRequest(QSessionManager &)),
            SLOT(onCommitDataRequest(QSessionManager &)));

    const QString preferencesDir = getPreferencesDirName();
    preferencesComponent_.reset(
        new PreferencesComponent(inputController_, preferencesDir, cancelled));
    if (cancelled)
        return;

    const Preferences & preferences = preferencesComponent_->preferences;
    {
        std::unique_ptr<const Icons::Theme> theme(
            new Icons::Theme(preferences.alwaysUseFallbackIcons));

        actions_.reset(new Actions(* theme));
        setWindowIcon(theme->venturous());
        preferencesComponent_->setTheme(* theme);
    }

    initMenuBar(menuBar_, * actions_);
    setMenuBar(& menuBar_);

    toolBar_.setObjectName(toolBar_.windowTitle());
    initToolBar(toolBar_, * actions_);
    addToolBar(& toolBar_);

    const std::string preferencesDirString =
        QtUtilities::qStringToString(preferencesDir);

    /// WARNING: repeated execution blocking is possible here!
    playbackComponent_.reset(
        new PlaybackComponent(* this, actions_->playback, inputController_,
                              preferences, preferencesDirString, cancelled));
    if (cancelled)
        return;
    /// WARNING: repeated execution blocking is possible here!
    playlistComponent_.reset(
        new PlaylistComponent(* this, * actions_, inputController_,
                              preferences,
                              std::bind(
                                  & PlaybackComponent::isRecentHistoryEntry,
                                  playbackComponent_.get(),
                                  std::placeholders::_1, std::placeholders::_2),
    [this](CommonTypes::ItemCollection items) {
        playbackComponent_->play(std::move(items));
    },
    preferencesDirString, cancelled));
    if (cancelled)
        return;


    connect(preferencesComponent_.get(), SIGNAL(aboutToSave()),
            SLOT(copyInternalOptionsToPreferences()));
    connect(preferencesComponent_.get(), SIGNAL(preferencesChanged()),
            SLOT(onPreferencesChanged()));

    connect(playbackComponent_.get(),
            SIGNAL(statusChanged(MediaPlayer::Status)),
            SLOT(onPlayerStatusChanged(MediaPlayer::Status)));

    connect(playlistComponent_.get(), SIGNAL(editModeChanged()),
            SLOT(setWindowTitle()));

    connect(actions_->file.preferences, SIGNAL(triggered(bool)),
            SLOT(onFilePreferences()));
    connect(actions_->file.preferencesDirectory, SIGNAL(triggered(bool)),
            SLOT(onFilePreferencesDirectory()));
    connect(actions_->file.quit, SIGNAL(triggered(bool)), SLOT(onFileQuit()));
    connect(actions_->playback.next, SIGNAL(triggered(bool)),
            SLOT(onPlaybackNext()));
    connect(actions_->help.help, SIGNAL(triggered(bool)), SLOT(onHelpHelp()));
    connect(actions_->help.about, SIGNAL(triggered(bool)), SLOT(onHelpAbout()));
    {
        const Actions::AddingPolicy & a = actions_->addingPolicy;
        connect(a.audioFile, SIGNAL(triggered(bool)),
                SLOT(onAudioFileStateChanged()));
        connect(a.mediaDir, SIGNAL(triggered(bool)),
                SLOT(onMediaDirStateChanged()));
        connect(a.bothAudioFile, SIGNAL(triggered(bool)),
                SLOT(onBothAudioFileStateChanged()));
        connect(a.bothMediaDir, SIGNAL(triggered(bool)),
                SLOT(onBothMediaDirStateChanged()));
    }

    setPreferencesNoComponents();
    onPlayerStatusChanged(playbackComponent_->status());

    restoreGeometry(preferences.windowGeometry);
    restoreState(preferences.windowState);
    if (!(preferences.notificationAreaIcon &&
            preferences.startToNotificationArea)) {
        show();
    }

    {
        typedef Preferences::Playback::StartupPolicy StartupPolicy;
        const Actions::Playback & pb = actions_->playback;
        /// WARNING: repeated execution blocking is possible here!
        switch (preferences.playback.startupPolicy) {
            case StartupPolicy::doNothing:
                break;
            case StartupPolicy::playbackPlay:
                pb.play->trigger();
                break;
            case StartupPolicy::playbackReplayLast:
                pb.replayLast->trigger();
                break;
            case StartupPolicy::playbackNextRandom:
                pb.nextRandom->trigger();
                break;
            case StartupPolicy::playbackNext:
                onPlaybackNext();
                break;
        }
    }
}
