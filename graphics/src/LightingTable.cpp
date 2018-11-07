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

#include <algorithm>

#include "LightingTable.hpp"

namespace X3D
{
    void LightingTable::init(const Palette* palette)
    {
        Log::info("Calculating lighting table...");

        int indexOfFirstFullbright = TOTAL_COLORS - palette->totalFullbrights;

        for(int color = 0; color < indexOfFirstFullbright; ++color)
        {
            for(int shade = 0; shade < SHADES_PER_COLOR; ++shade)
            {
                const unsigned char* rgb = palette->getRGB(color);
                const int MAX_INTENSITY = 255;

                int r = std::min(MAX_INTENSITY, scaleIntensityByShade(rgb[0], shade));
                int g = std::min(MAX_INTENSITY, scaleIntensityByShade(rgb[1], shade));
                int b = std::min(MAX_INTENSITY, scaleIntensityByShade(rgb[2], shade));

                table[color][shade] = palette->nearestColor(r, g, b);
            }
        }

        for(int color = indexOfFirstFullbright; color < TOTAL_COLORS; ++color)
        {
            for(int shade = 0; shade < SHADES_PER_COLOR; ++shade)
            {
                // Fullbrights stay the same no matter the intensity
                table[color][shade] = color;
            }
        }
    }
}

