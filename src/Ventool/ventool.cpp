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

# include "SharedMemory.hpp"

# include <QSharedMemory>

# include <string>
# include <iostream>
# include <iomanip>


namespace
{
namespace Command
{
constexpr const char * play() noexcept { return "play"; }
constexpr const char * pause() noexcept { return "pause"; }
constexpr const char * stop() noexcept { return "stop"; }
constexpr const char * previous() noexcept { return "previous"; }
constexpr const char * replayLast() noexcept { return "replay-last"; }
constexpr const char * nextFromHistory() noexcept {
    return "next-from-history";
}
constexpr const char * nextRandom() noexcept { return "next-random"; }
constexpr const char * next() noexcept { return "next"; }
constexpr const char * playAll() noexcept { return "play-all"; }
constexpr const char * showExternal() noexcept { return "show-external"; }
constexpr const char * hideExternal() noexcept { return "hide-external"; }
constexpr const char * updateStatus() noexcept { return "update-status"; }
constexpr const char * show() noexcept { return "show"; }
constexpr const char * hide() noexcept { return "hide"; }
constexpr const char * quit() noexcept { return "quit"; }

constexpr const char * help() noexcept { return "help"; }

}

void printCommand(const char * command, const char * description)
{
    std::cout << "    " << std::left << std::setw(20) << command
              << "- " << description << '\n';
}

void printHelp()
{
    std::cout << TOOL_EXECUTABLE " - sends commands to running "
              APPLICATION_NAME " instance.\n";
    std::cout << "Usage: " TOOL_EXECUTABLE " <command>\n"
              "where <command> is on of the following:\n";
    using namespace Command;
    printCommand(play(), "starts playback");
    printCommand(pause(), "pauses playback");
    printCommand(stop(), "exits external player");
    printCommand(previous(), "plays previous item from history");
    printCommand(replayLast(), "plays current item from history");
    printCommand(nextFromHistory(), "plays next item from history");
    printCommand(nextRandom(), "plays random item");
    printCommand(next(), "plays next item");
    printCommand(playAll(), "plays all playable items");
    printCommand(showExternal(), "shows external player");
    printCommand(hideExternal(), "hides external player");
    printCommand(updateStatus(), "updates playback status");
    printCommand(show(), "shows " APPLICATION_NAME " window");
    printCommand(hide(), "hides " APPLICATION_NAME " window");
    printCommand(quit(), "quits " APPLICATION_NAME);
    printCommand(help(), "prints [this] help message");
    std::cout << "\nCommands may be prefixed with \"--\" "
              "(GNU-style long-options) or not, your choice.\n\n";
}

void printError(const std::string & error)
{
    std::cerr << TOOL_EXECUTABLE ": " << error << "." << std::endl;
}

void printErrorAndHelp(const std::string & error)
{
    printError(error);
    printHelp();
}

}


int main(int argc, char * argv[])
{
    using namespace SharedMemory;
    char symbol = Symbol::noCommand();
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
            if (arg.size() == 2)
                break; // "--" -> ignore the rest.
            arg.erase(0, 2);
        }
        using namespace Command;
        if (arg == play())
            symbol = Symbol::play();
        else if (arg == pause())
            symbol = Symbol::pause();
        else if (arg == stop())
            symbol = Symbol::stop();
        else if (arg == previous())
            symbol = Symbol::previous();
        else if (arg == replayLast())
            symbol = Symbol::replayLast();
        else if (arg == nextFromHistory())
            symbol = Symbol::nextFromHistory();
        else if (arg == nextRandom())
            symbol = Symbol::nextRandom();
        else if (arg == next())
            symbol = Symbol::next();
        else if (arg == playAll())
            symbol = Symbol::playAll();
        else if (arg == showExternal())
            symbol = Symbol::showExternal();
        else if (arg == hideExternal())
            symbol = Symbol::hideExternal();
        else if (arg == updateStatus())
            symbol = Symbol::updateStatus();
        else if (arg == show())
            symbol = Symbol::show();
        else if (arg == hide())
            symbol = Symbol::hide();
        else if (arg == quit())
            symbol = Symbol::quit();
        else if (arg == help()) {
            printHelp();
            return 1;
        }
        else {
            printErrorAndHelp("unknown command \"" + arg + '"');
            return 3;
        }

        if (i != 1) {
            printErrorAndHelp(
                "more than one command was specified. This is not allowed");
            return 4;
        }
    }
    if (symbol == Symbol::noCommand()) {
        printErrorAndHelp("command was expected");
        return 2;
    }

    QSharedMemory shared(key());
    if (shared.attach(QSharedMemory::ReadWrite)) {
        shared.lock();
        *(char *)shared.data() = symbol;
        shared.unlock();
    }
    else {
        printError(APPLICATION_NAME " is not running");
        return 7;
    }
}
