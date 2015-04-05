/*
 This file is part of Venturous.
 Copyright (C) 2015 Igor Kushnir <igorkuo AT Google mail>

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

# ifndef VENTUROUS_FILE_PATTERN_HPP
# define VENTUROUS_FILE_PATTERN_HPP

# include <QtGlobal>
# include <QString>

# include <vector>

QT_FORWARD_DECLARE_CLASS(QStringList)


struct FilePattern {
    QString pattern;
    bool enabled;
};

inline bool operator == (const FilePattern & lhs, const FilePattern & rhs)
{
    return lhs.pattern == rhs.pattern && lhs.enabled == rhs.enabled;
}

inline bool operator != (const FilePattern & lhs, const FilePattern & rhs)
{
    return !(lhs == rhs);
}

using FilePatternList = std::vector<FilePattern>;

/// @return A list of enabled patterns from patternList.
QStringList getEnabledFilePatterns(const FilePatternList & patternList);

# endif // VENTUROUS_FILE_PATTERN_HPP
