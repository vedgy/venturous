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

# ifndef VENTUROUS_COMMON_TYPES_HPP
# define VENTUROUS_COMMON_TYPES_HPP

# include <functional>
# include <vector>
# include <string>


namespace CommonTypes
{
/// Collection of items to be played.
typedef std::vector<std::string> ItemCollection;
/// Function, which starts playing ItemCollection parameter.
typedef std::function<void(ItemCollection)> PlayItems;
}

# endif // VENTUROUS_COMMON_TYPES_HPP
