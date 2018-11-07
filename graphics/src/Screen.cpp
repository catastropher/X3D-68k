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

#include "Screen.hpp"

namespace X3D
{
    void Screen::init(ScreenConfig& config)
    {
        screenDriver = ServiceLocator::get<ScreenDriver>();

        if(config.w.hasValue && config.h.hasValue)
        {
            ScreenSettings settings;
            settings.fullscreen = false;
            settings.w = config.w.value;
            settings.h = config.h.value;

            screenDriver->openWindow(settings);

            byte* texels;
            int w, h;

            screenDriver->getWindow(texels, w, h);

            screenTexture.setTexels(texels, w, h);

            setTexture(&screenTexture);
        }

        screenDriver->setPalette(config.palette);

        LightingTable* table = ServiceLocator::get<LightingTable>();
        setLightingTable(table);

        for(int i = 0; i < 256; ++i)
        {
            for(int j = 0; j < 64; ++j)
            {
                screenTexture.setTexel(i, j, table->table[i][j]);
            }
        }

        redraw();
    }

    void Screen::redraw()
    {
        screenDriver->redraw();
    }
}

