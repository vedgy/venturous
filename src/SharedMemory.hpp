/*
 This file is part of Venturous.
 Copyright (C) 2014, 2017, 2019 Igor Kushnir <igorkuo AT Google mail>

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

# ifndef VENTUROUS_SHARED_MEMORY_HPP
# define VENTUROUS_SHARED_MEMORY_HPP

# include <QString>
# include <QDir>
# include <QSharedMemory>


inline QString getVenturousPreferencesDirName()
{
    QString result(PREFERENCES_DIR);
    if (! result.isEmpty() && result[0] == '~')
        result.replace(0, 1, QDir::homePath());
    return result;
}

namespace SharedMemory
{
inline QString key() {
    // Append the path to preferences to allow running independent Venturous
    // instances, e.g. by different users of the same system.
    return "venturousandventoolhJkNQqC{" + getVenturousPreferencesDirName();
}

namespace Symbol
{
constexpr char noCommand() noexcept { return 0; }
constexpr char play() noexcept { return 'P'; }
constexpr char pause() noexcept { return 'U'; }
constexpr char stop() noexcept { return 'S'; }
constexpr char previous() noexcept { return 'V'; }
constexpr char replayLast() noexcept { return 'L'; }
constexpr char nextFromHistory() noexcept { return 'T'; }
constexpr char nextRandom() noexcept { return 'R'; }
constexpr char next() noexcept { return 'N'; }
constexpr char playAll() noexcept { return 'A'; }
constexpr char showExternal() noexcept { return 'E'; }
constexpr char hideExternal() noexcept { return 'X'; }
constexpr char setExternalOptions() noexcept { return 'O'; }
constexpr char updateStatus() noexcept { return 'D'; }
constexpr char show() noexcept { return 'W'; }
constexpr char hide() noexcept { return 'H'; }
constexpr char quit() noexcept { return 'Q'; }

} // END namespace Symbol

inline void setSymbol(QSharedMemory & shared, char symbol)
{
    shared.lock();
    *static_cast<char *>(shared.data()) = symbol;
    shared.unlock();
}

} // END namespace SharedMemory

# endif // VENTUROUS_SHARED_MEMORY_HPP
