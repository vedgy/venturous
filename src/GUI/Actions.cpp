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
    : preferences(new QAction(
                      theme.preferences(), tr("&Preferences ..."), this)),
    quit(new QAction(theme.quit(), tr("&Quit"), this))
{
    preferences->setIconText("Pf");
    quit->setIconText("Q");
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
}


Actions::Playback::Playback(const Icons::Theme & theme)
    : play(new QAction(theme.playbackPlay(), tr("&Play"), this)),
      stop(new QAction(theme.playbackStop(), tr("&Stop"), this)),
      next(new QAction(theme.playbackNext(), tr("&Next"), this)),
      playAll(new QAction(theme.playbackPlayAll(), tr("Play &all"), this))
{
    play->setIconText("Pl");
    play->setShortcut(Qt::CTRL | Qt::Key_P);
    stop->setIconText("St");
    stop->setShortcut(Qt::CTRL | Qt::Key_S);
    next->setIconText("Nx");
    next->setShortcut(Qt::CTRL | Qt::Key_N);
    playAll->setIconText("PA");
}


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
    editMode->setCheckable(true);
    editMode->setIconText("EM");
    applyChanges->setIconText("AC");
    cancelChanges->setIconText("CC");
    addFiles->setIconText("AF");
    addDirectory->setIconText("AD");
    cleanUp->setIconText("CU");
    clear->setIconText("Cl");
    restorePrevious->setIconText("RP");
    load->setIconText("Lo");
    saveAs->setIconText("SA");
}


Actions::Help::Help(const Icons::Theme & theme)
    : help(new QAction(theme.help(), tr("&Help ..."), this)),
      about(new QAction(theme.about(), tr("&About ..."), this))
{
    help->setIconText("He");
    help->setShortcuts(QKeySequence::HelpContents);
    about->setIconText("Ab");
}


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

void Actions::AddingPolicy::primaryActionChanged()
{
    const bool both = audioFile->isChecked() && mediaDir->isChecked();
    bothAudioFile->setEnabled(both);
    bothMediaDir->setEnabled(both);
}