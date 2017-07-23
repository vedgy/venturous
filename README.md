## Introduction

<b>Venturous</b> is a random playback manager, which uses media player's
command line interface.
Currently only *Audacious* media player is supported, but adding support for
other players should be quite simple.

*Venturous* is built around one feature, which is missing from most (maybe
even all) media players: user-defined items for random playback. This
feature is especially useful for grouping separate tracks that are
actually parts of one composition. For example parts of classical music
compositions (movements) are often distributed as separate tracks - either
separate audio files or separate entries in the *cue* file. It is
usually desirable to listen to the movements in succession
and in a proper order.

*Venturous* has a hierarchical playlist, which reflects a file system.
The playlist is protected from accidental changes.<br>
*Venturous* also features playback history, which allows user to identify
previously played items and play them again.<br>
Custom actions similar to those in *Thunar* file manager are available in
playlist's and history's context menus. Custom actions allow user to open
selected items in external application.
For example: open in file manager or in media player, edit text file,
move item(s) to trash.

## Build requirements

*GNU/Linux* or *MS Windows* (might also work in other systems - not tested).

### External dependencies
* *git*
* *wget* OR *curl* OR *inkscape*
* *cmake* (2.8 or later)
* *make*
* *g++* (4.7 or later) [recent versions of *clang++* also suffice but only with
    *qt5*]
* *qt4* OR *qt5* development libraries (*Core*, *XML* and *GUI*/*Widgets*
    modules)

## How to build and install

1. Clone this repository, e.g.:

        git clone https://github.com/vedgy/venturous.git

2. Enter newly created directory, e.g.:

        cd venturous

3. If you want to install the latest stable version, execute the following
command:

        git checkout tags/v1.3
If you prefer the latest development version (can be unstable), just skip this
step.

4. Run configuration script:

        ./update_submodules_and_configure

5. Follow instructions that appear at the end of
`update_submodules_and_configure` output.

### Troubleshooting
If errors appear after running `update_submodules_and_configure`, ensure
that all [build dependencies](#external-dependencies) are satisfied.

Errors also appear in the following case:
* Neither *wget* nor *curl* is installed. If you have *inkscape* installed and
want to use it for icon generation, pass *generate-png* argument to
`update_submodules_and_configure`.

## Runtime requirements
* <i>qt4</i> OR <i>qt5</i> shared libraries (<i>Core</i>, <i>XML</i>
    and <i>GUI</i>/<i>Widgets</i> modules).
* <i>Audacious</i> (managed Audacious mode in <i>Venturous</i> - which
    is <i>not</i> the default mode - requires Audacious 3.4 or later).
* <i>audacious</i> and <i>audtool</i> executables must be present in
    <i>PATH</i>. There are two problems with this requirement in MS Windows:
    * Audacious <i>bin</i> directory is not added in <i>PATH</i>
    environment variable automatically. User has to manually add it there.
    * <i>audtool</i> is usually unavailable in Windows at all.
    With <i>audtool</i> missing only basic <i>Venturous</i> features
    work. The biggest problem is that stopping playback does not work in
    this case, which prevents <i>Venturous</i> from normal functioning.
* <i>notify-send</i> from <i>libnotify</i> (optional - for desktop
    notifications).
* <i>xdg-open</i> from <i>xdg-utils</i> (optional - for some default
    custom actions on non-MS-Windows systems).
* <i>Song Change</i> Audacious plugin (optional - for detached
    Audacious mode, which is the default mode). This plugin is usually
    unavailable in MS Windows, so Windows users have to switch to the
    managed mode.

## How to uninstall
Enter `venturous/build` directory, acquire root privileges (`sudo` or `su`)
and run

    xargs rm < install_manifest.txt

## License

Copyright (C) 2014, 2015, 2017 Igor Kushnir <igorkuo AT Google mail>

Venturous is licensed under the <b>GNU GPLv3+</b> license,
a copy of which can be found in the `COPYING` file.

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
