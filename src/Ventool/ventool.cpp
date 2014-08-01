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

# include <QSharedMemory>

# include <cstddef>
# include <algorithm>
# include <array>
# include <tuple>
# include <string>
# include <iostream>


namespace
{
/// "help" must always be the last command here.
/// It is not mapped in commands array.
const std::array<std::string, 16> commandNames = {{
        "play", "pause", "stop", "previous", "replay-last", "next-from-history",
        "next-random", "next", "play-all", "show-external", "hide-external",
        "update-status", "show", "hide", "quit", "help"
    }
};
constexpr std::size_t helpCommand =
    std::tuple_size<decltype(commandNames)>::value - 1;

constexpr std::array<char, helpCommand> commands = {{
        'P', 'U', 'S', 'V', 'L', 'T', 'R', 'N', 'A', 'E', 'X',
        'D', 'W', 'H', 'Q'
    }
};

void printHelp()
{
    std::cout << TOOL_EXECUTABLE " - sends commands to running "
              APPLICATION_NAME " instance.\n";
    std::cout << "Usage: " TOOL_EXECUTABLE " ";
    for (std::size_t i = 0; i < commandNames.size(); ++i) {
        if (i != 0)
            std::cout << "|";
        std::cout << commandNames[i];
    }
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
    std::size_t command = commandNames.size();
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
            if (arg.size() == 2)
                break; // "--" -> ignore rest.
            arg.erase(0, 2);
        }
        const auto it = std::find(commandNames.begin(), commandNames.end(),
                                  arg);
        if (it == commandNames.end()) {
            printErrorAndHelp("unknown command \"" + arg + '"');
            return 3;
        }
        command = it - commandNames.begin();
        if (command == helpCommand) {
            printHelp();
            return 1;
        }
        if (i != 1) {
            printErrorAndHelp(
                "more than one command was specified. This is not allowed");
            return 4;
        }
    }
    if (command == commandNames.size()) {
        printErrorAndHelp("command was expected");
        return 2;
    }

    QSharedMemory shared(SHARED_MEMORY_KEY);
    if (shared.attach(QSharedMemory::ReadWrite)) {
        shared.lock();
        *(char *)shared.data() = commands[command];
        shared.unlock();
    }
    else {
        printError(APPLICATION_NAME " is not running");
        return 7;
    }
}
