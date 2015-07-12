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

# include "Actions.hpp"

# include "Icons.hpp"

# include <QAction>
# include <QKeySequence>


Actions::Actions(const Icons::Theme & theme)
    : file(theme), playback(theme), playlist(theme), help(theme),
      addingPolicy(theme)
{
}


Actions::File::File(const Icons::Theme & theme)
    : preferences(new QAction(theme.preferences(),
                              tr("&Preferences ..."), this)),
    /// TODO: add icon (places/folder).
    preferencesDirectory(new QAction(tr("Preferences &directory"), this)),
    quit(new QAction(theme.quit(), tr("&Quit"), this))
{
    preferences->setIconText("Pf");
    preferences->setToolTip(tr("Show preferences window"));
    preferencesDirectory->setIconText("PD");
    preferencesDirectory->setToolTip(
        tr("Show preferences directory in the file manager"));
    quit->setIconText("Q");
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
}

Actions::File::~File() = default;


Actions::Playback::Playback(const Icons::Theme & theme)
    : play(new QAction(theme.playbackPlay(), tr("&Play"), this)),
      pause(new QAction(theme.playbackPause(), tr("Pa&use"), this)),
      stop(new QAction(theme.playbackStop(), tr("&Stop"), this)),
      previous(new QAction(theme.playbackPrevious(), tr("Pre&vious"), this)),
      replayLast(new QAction(theme.playbackReplayLast(),
                             tr("Replay &last"), this)),
      nextFromHistory(new QAction(theme.playbackNextFromHistory(),
                                  tr("Next &from history"), this)),
      nextRandom(new QAction(theme.playbackNextRandom(),
                             tr("Next &random"), this)),
      next(new QAction(theme.playbackNext(), tr("&Next"), this)),
      playAll(new QAction(theme.playbackPlayAll(), tr("Play &all"), this)),
      showExternalPlayerWindow(new QAction(theme.audioPlayerShow(),
                                           tr("&Show window"), this)),
      hideExternalPlayerWindow(new QAction(theme.audioPlayerHide(),
                                           tr("&Hide window"), this)),
      setExternalPlayerOptions(new QAction(tr("Set &essential options"), this)),
      updateStatus(new QAction(theme.viewRefresh(),
                               tr("Up&date status"), this)),
      importHistory(new QAction(theme.load(), tr("&Import ..."), this)),
      exportHistory(new QAction(theme.saveAs(), tr("&Export ..."), this)),
      clearHistory(new QAction(theme.clear(), tr("&Clear"), this))
{
    play->setIconText("Pl");
    play->setShortcut(Qt::CTRL | Qt::Key_P);
    pause->setIconText("Pa");
    pause->setShortcut(Qt::CTRL | Qt::Key_U);
    stop->setIconText("St");
    stop->setShortcut(Qt::CTRL | Qt::Key_S);

    previous->setIconText("Pr");
    previous->setShortcut(Qt::CTRL | Qt::Key_V);
    replayLast->setIconText("RL");

    nextFromHistory->setIconText("NH");
    nextRandom->setIconText("NR");
    next->setIconText("Ne");
    next->setShortcut(Qt::CTRL | Qt::Key_N);

    playAll->setIconText("PA");

    {
        const QString externalPlayerWindow = tr(" external player window");
        showExternalPlayerWindow->setIconText("SPW");
        showExternalPlayerWindow->setToolTip(tr("Show") + externalPlayerWindow);
        hideExternalPlayerWindow->setIconText("HPW");
        hideExternalPlayerWindow->setToolTip(tr("Hide") + externalPlayerWindow);
    }
    setExternalPlayerOptions->setIconText("SPO");
    setExternalPlayerOptions->setToolTip(
        tr("Set the necessary external player options"));
    updateStatus->setIconText("US");
    updateStatus->setToolTip(tr("Query playback status of external player"));

    {
        const QString historyName = tr(" history");
        importHistory->setIconText("Im");
        importHistory->setToolTip(tr("Import") + historyName);
        exportHistory->setIconText("Ex");
        exportHistory->setToolTip(tr("Export") + historyName);
        clearHistory->setIconText("Cl");
        clearHistory->setToolTip(tr("Clear") + historyName);
    }
}

Actions::Playback::~Playback() = default;


Actions::Playlist::Playlist(const Icons::Theme & theme)
    : editMode(new QAction(theme.edit(), tr("&Edit mode"), this)),
      applyChanges(new QAction(theme.apply(), tr("Apply changes"), this)),
      cancelChanges(new QAction(theme.cancel(), tr("Cancel changes"), this)),
      addFiles(new QAction(theme.add(), tr("Add &files ..."), this)),
      addDirectory(new QAction(theme.addDir(), tr("Add &directory ..."), this)),
      cleanUp(new QAction(theme.cleanUp(), tr("Clean &up"), this)),
      clear(new QAction(theme.clear(), tr("&Clear"), this)),
      restorePrevious(new QAction(
                          theme.revert(), tr("&Restore previous"), this)),
      load(new QAction(theme.load(), tr("&Load ..."), this)),
      saveAs(new QAction(theme.saveAs(), tr("&Save as ..."), this))
{
    const QString playlistName = tr(" playlist");
    editMode->setCheckable(true);
    editMode->setIconText("EM");
    applyChanges->setIconText("AC");
    cancelChanges->setIconText("CC");
    addFiles->setIconText("AF");
    addDirectory->setIconText("AD");

    cleanUp->setIconText("CU");
    cleanUp->setToolTip(tr("Clean%1 up").arg(playlistName));
    clear->setIconText("Cl");
    clear->setToolTip(tr("Clear") + playlistName);
    restorePrevious->setIconText("RP");
    restorePrevious->setToolTip(tr("Restore previous") + playlistName);
    load->setIconText("Lo");
    load->setToolTip(tr("Load") + playlistName);
    saveAs->setIconText("SA");
    saveAs->setToolTip(tr("Save%1 as").arg(playlistName));
}

Actions::Playlist::~Playlist() = default;


Actions::Help::Help(const Icons::Theme & theme)
    : help(new QAction(theme.help(), tr("&Help ..."), this)),
      about(new QAction(theme.about(), tr("&About ..."), this))
{
    help->setIconText("He");
    help->setShortcuts(QKeySequence::HelpContents);
    about->setIconText("Ab");
}

Actions::Help::~Help() = default;


Actions::AddingPolicy::AddingPolicy(const Icons::Theme & theme)
    : audioFile(new QAction(theme.audioFile(),
                            tr("Consider adding &files"), this)),
    mediaDir(new QAction(theme.mediaDir(),
                         tr("Consider adding media &directories"), this)),
    bothAudioFile(new QAction(theme.bothAudioFile(),
                              tr("If both, add files"), this)),
    bothMediaDir(new QAction(theme.bothMediaDir(),
                             tr("If both, add media directories"), this))
{
    audioFile->setCheckable(true);
    audioFile->setIconText("cAF");
    mediaDir->setCheckable(true);
    mediaDir->setIconText("cAD");
    bothAudioFile->setCheckable(true);
    bothAudioFile->setIconText("bAF");
    bothMediaDir->setCheckable(true);
    bothMediaDir->setIconText("bAD");
}

Actions::AddingPolicy::~AddingPolicy() = default;

void Actions::AddingPolicy::primaryActionChanged()
{
    const bool both = audioFile->isChecked() && mediaDir->isChecked();
    bothAudioFile->setEnabled(both);
    bothMediaDir->setEnabled(both);
}
