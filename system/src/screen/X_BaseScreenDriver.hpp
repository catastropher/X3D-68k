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

#include "X_Palette.hpp"

namespace X3D
{
    struct ScreenSettings
    {
        int w;
        int h;
        bool fullscreen;
    };

    template<typename TScreenDriver>
    class BaseScreenDriver
    {
    public:
        void openWindow(ScreenSettings& settings);
        void updateWindow(ScreenSettings& settings);

        void closeWindow();

        void getScreenSettings(ScreenSettings& dest);

        void setTitle(const char* title);

        void setPalette(Palette* palette);
        Palette* getPalette();
        void updatePaletteColors(int colorStart, int total);

        void update(byte* buf, int w, int h);

    protected:
        Palette* currentPalette;
    };
}

