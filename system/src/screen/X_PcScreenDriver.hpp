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

#include <SDL/SDL.h>

#include "X_BaseScreenDriver.hpp"
#include "log/X_Log.hpp"
#include "error/X_SystemException.hpp"

namespace X3D
{
    class PcScreenDriver : public BaseScreenDriver<PcScreenDriver>
    {
    public:
        void init()
        {
            Log::info("Init screen driver with SDL support");

            if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
            {
                Log::error("Failed to init SDL");
                throw SystemException(SystemErrorCode::screenDriverError);
            }

            screenSurface = nullptr;
        }
        
        void openWindow(ScreenSettings& settings)
        {
            const int BITS_PER_PIXEL = 8;
            int flags = SDL_SWSURFACE;

            if(settings.fullscreen)
            {
                flags |= SDL_FULLSCREEN;
            }

            screenSurface = SDL_SetVideoMode(
                settings.w,
                settings.h,
                BITS_PER_PIXEL,
                flags);

            if(!windowIsOpened())
            {
                Log::error("Failed to set video mode");
                throw SystemException(SystemErrorCode::screenDriverError);
            }
        }

        void closeWindow()
        {
            if(windowIsOpened())
            {
                SDL_FreeSurface(screenSurface);
                screenSurface = nullptr;
            }
        }

        void updateWindow(ScreenSettings& settings)
        {
            closeWindow();
            openWindow(settings);
        }

        void setPalette(const Palette* palette)
        {
            currentPalette = palette;
            updatePaletteColors(0, Palette::TOTAL_COLORS);
        }

        void updatePaletteColors(int colorStart, int total)
        {
            for(int i = colorStart; i < colorStart + total; ++i)
            {
                const unsigned char* color = currentPalette->getRGB(i);

                paletteColors[i].r = color[0];
                paletteColors[i].g = color[1];
                paletteColors[i].b = color[2];
            }

            if(SDL_SetColors(screenSurface, paletteColors, colorStart, total) != 1)
            {
                Log::error("Failed to update palette: %s\n", SDL_GetError());
                throw SystemException(SystemErrorCode::screenDriverError);
            }
        }

        void redraw()
        {
            SDL_UpdateRect(screenSurface, 0, 0, screenSurface->w, screenSurface->h);
        }

        void cleanup()
        {
            closeWindow();

            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }

        void getWindow(byte*& buf, int& w, int& h)
        {
            buf = (byte*)screenSurface->pixels;
            w = screenSurface->w;
            h = screenSurface->h;
        }

    private:
        bool windowIsOpened()
        {
            return screenSurface != nullptr;
        }

        void assertValidPalette()
        {
            if(currentPalette == nullptr)
            {
                Log::error("Screen has no palette");
                throw SystemException(SystemErrorCode::screenDriverError);
            }
        }

        SDL_Surface* screenSurface;
        SDL_Color paletteColors[Palette::TOTAL_COLORS];
    };
}

