## Introduction

<b>Venturous</b> is a random playback manager, which uses media player's
command line interface.
Currently only Audacious media player is supported, but adding support for
other players should be quite simple.

Venturous is built around one feature, which is missing in most (maybe
even all) media players: user-defined items for random playback. This
feature is especially useful for grouping separate tracks that are
actually parts of one composition. For example parts of classical music
compositions (movements) are often distributed as separate tracks. It is
usually desirable to listen to movements in succession and in a proper order.

Venturous has hierarchical playlist, which reflects file system. The playlist
is protected from accidental changes.
Venturous also features playback history, which allows user to identify
previously played items and play them again.

## Build requirements

GNU/Linux or MS Windows (might also work in OS X, not tested).

### External Dependencies
* git
* wget OR curl OR inkscape
* cmake (2.8 or later)
* make
* g++ (4.7 or later)
* qt4 OR qt5 (Core, XML and GUI/Widgets modules)

## How to build and install

1. Clone this repository, e.g.:

        git clone https://github.com/vedgy/venturous.git

2. Enter newly created directory, e.g.:

        cd venturous

3. If you want to install latest stable version, execute the following command:

        git checkout tags/v0.9.1
If you prefer latest development version (can be unstable), just skip this step.

4. Run configuration script:

        ./update_and_configure_submodules

5. Follow instructions that appear at the end of
`update_and_configure_submodules` output.

### Troubleshooting
If errors appear after running `update_and_configure_submodules`, this probably
means that some dependency is not satisfied. Check this.

Also errors appear in the following cases:
* qt4 is not installed. If you have qt5 installed and want to use it, pass
"qt5" argument to `update_and_configure_submodules`.
* Neither wget nor curl is installed. If you have inkscape installed and want
to use it for icon generation, pass "generate-png" argument to
`update_and_configure_submodules`.

## Runtime requirements
* Audacious version 3.4 or newer.
* audacious and audtool executables must be present in PATH.
    There are two problems with this requirement in MS Windows:
    * Audacious bin directory is not added in PATH environment
    variable automatically. User has to manually add it there.
    * audtool is usually unavailable in Windows at all. With audtool
    missing only basic Venturous features work.

## How to uninstall
Enter `venturous/build` directory, acquire root privileges (`sudo` or `su`)
and run

    xargs rm < install_manifest.txt

## License

Copyright (C) 2014 Igor Kushnir <igorkuo AT Google mail>

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
