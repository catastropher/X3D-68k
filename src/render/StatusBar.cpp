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

#include "StatusBar.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "memory/Memory.hpp"

List<StatusBarItem> StatusBar::items;

void StatusBar::setItem(const char* name, const char* format, ...)
{
    auto itemToSet = items.findFirst([name](auto& item) { return item.name == name; });

    if(itemToSet == nullptr)
    {
        itemToSet = items.createNode();
        itemToSet->name = name;

        items.append(itemToSet);
    }

    va_list list;
    va_start(list, format);

    itemToSet->value.format(format, list);

    va_end(list);
}

void StatusBar::removeItem(const char* name)
{
    items.deleteWhere([name](auto item) { return item.name == name; });
}

void StatusBar::render(X_Texture& texture, const Font& font)
{
    Vec2i pos = { 0, 0 };

    for(auto item = items.head; item != nullptr; item = item->next)
    {
        char text[512];
        sprintf(text, "%s: %s", item->name, item->value.c_str());

        const int SPACING = 10;
        int w = font.calcWidthOfStr(text) + SPACING;

        if(pos.x + w >= texture.getW())
        {
            pos.y += font.getH();
            pos.x = 0;
        }

        texture.drawStr(text, font, pos);

        pos.x += w;
    }
}

