// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "memory/FixedLengthString.hpp"
#include "memory/List.hpp"

struct X_Font;
struct X_Texture;

struct StatusBarItem
{
    const char* name;
    FixedLengthString<256> value;

    StatusBarItem* next;
};

class StatusBar
{
public:
    static void setItem(const char* name, const char* format, ...);
    static void removeItem(const char* name);

    static void render(X_Texture& texture, const X_Font& font);

private:
    static List<StatusBarItem> items;
};

