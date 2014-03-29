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

# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
# include <iostream>
# endif


# include "MainWindow.hpp"

# include "PlaybackComponent.hpp"
# include "PlaylistComponent.hpp"
# include "PreferencesWindow.hpp"
# include "Icons.hpp"
# include "Actions.hpp"

# include <QtCoreUtilities/String.hpp>

# include <QString>
# include <QObject>
# include <QSharedMemory>
# include <QDir>
# include <QFile>
# include <QCoreApplication>
# include <QAction>
# include <QMenu>
# include <QMenuBar>
# include <QMessageBox>
# include <QSizePolicy>
# include <QStatusBar>
# include <QLabel>

# include <cassert>
# include <utility>
# include <functional>
# include <memory>


namespace
{
QString constructPreferencesDirName()
{
    QString result(PREFERENCES_DIR);
    if (! result.isEmpty() && result[0] == '~')
        result.replace(0, 1, QDir::homePath());
    return result;
}
const QString preferencesDir = constructPreferencesDirName();


void initMenuBar(QMenuBar & menuBar, const Actions & actions)
{
    QMenu * const file = menuBar.addMenu(QObject::tr("&File"));
    file->addActions( { actions.file.preferences, actions.file.quit });

    const Actions::Playback & pbA = actions.playback;
    QMenu * const playback = menuBar.addMenu(QObject::tr("Play&back"));
    playback->addActions( { pbA.play, pbA.stop, pbA.next, pbA.playAll });
    playback->insertSeparator(pbA.playAll);

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

    QMenu * const help = menuBar.addMenu(QObject::tr("&Help"));
    help->addActions( { actions.help.help, actions.help.about });
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
    toolBar.addActions( { pbA.play, pbA.stop, pbA.next });
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


MainWindow::MainWindow(std::unique_ptr<QSharedMemory> sharedMemory,
                       QWidget * const parent, const Qt::WindowFlags flags)
    : QMainWindow(parent, flags), sharedMemory_(std::move(sharedMemory)),
      toolBar_("Toolbar"), inputController_(* this)
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "PREFERENCES_DIR = " << PREFERENCES_DIR << std::endl;
    std::cout << "preferencesFilename = " <<
              QtUtilities::qStringToString(preferencesFilename) << std::endl;
# endif

    assert(sharedMemory_ && "Valid shared memory expected, nullptr found.");

    initPreferences();
    {
        std::unique_ptr<const Icons::Theme> theme(
            new Icons::Theme(preferences_.alwaysUseFallbackIcons));

        actions_.reset(new Actions(* theme));
        setWindowIcon(theme->venturous());
        preferencesWindow_ = new PreferencesWindow(
            preferences_, actions_->playlist.addFiles->icon(), this);
    }

    initMenuBar(menuBar_, * actions_);
    setMenuBar(& menuBar_);

    toolBar_.setObjectName(toolBar_.windowTitle());
    initToolBar(toolBar_, * actions_);
    addToolBar(& toolBar_);

    const std::string preferencesDirString =
        QtUtilities::qStringToString(preferencesDir);

    playlistComponent_.reset(
        new PlaylistComponent(* this, actions_->playlist, inputController_,
                              preferences_, preferencesDirString));

    playbackComponent_.reset(
        new PlaybackComponent(* this, playlistComponent_->tree(),
                              actions_->playback, inputController_,
                              preferences_.playback, preferencesDirString));

    playlistComponent_->setPlayItems(
    [this](CommonTypes::ItemCollection items) {
        playbackComponent_->play(std::move(items));
    });

    connectSlots();

    onPlayerStateChanged(playbackComponent_->isPlayerRunning());
    /// FIXME: not all preferences should be reset.
    preferencesChanged();

    restoreGeometry(preferences_.windowGeometry);
    restoreState(preferences_.windowState);

    if (!(preferences_.notificationAreaIcon &&
            preferences_.startToNotificationArea)) {
        show();
    }
}

MainWindow::~MainWindow()
{
# ifdef DEBUG_VENTUROUS_MAIN_WINDOW
    std::cout << "Entered MainWindow destructor." << std::endl;
# endif
}


const QString MainWindow::preferencesFilename =
    preferencesDir + APPLICATION_NAME ".xml";
const QString MainWindow::savePreferencesErrorPrefix =
    QObject::tr("Saving preferences failed");


void MainWindow::initPreferences()
{
    if (QFile::exists(preferencesFilename)) {
        if (handlePreferencesErrors([this] {
        savedPreferences_.load(preferencesFilename);
        }, tr("Loading preferences failed"))) {
            preferences_ = savedPreferences_;
        }
        else
            savedPreferences_ = preferences_;
    }
}

void MainWindow::connectSlots()
{
    connect(preferencesWindow_, SIGNAL(preferencesUpdated()),
            SLOT(onPreferencesUpdated()));
    connect(actions_->file.preferences, SIGNAL(triggered(bool)),
            SLOT(onPreferencesActivated()));
    connect(actions_->file.quit, SIGNAL(triggered(bool)), SLOT(onFileQuit()));

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

    connect(playlistComponent_.get(), SIGNAL(treeChanged()),
            SLOT(setWindowTitle()));

    connect(playlistComponent_.get(), SIGNAL(itemActivated(QString)),
            playbackComponent_.get(), SLOT(onItemActivated(QString)));

    connect(playbackComponent_.get(), SIGNAL(playerStateChanged(bool)),
            SLOT(onPlayerStateChanged(bool)));

    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
            SLOT(onAboutToQuit()));
}
