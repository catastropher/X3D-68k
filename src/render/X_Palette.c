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

#include "X_Palette.h"

static int x_palette_calculate_color_distance(const X_Palette* palette, X_Color color, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char colorR, colorG, colorB;
    x_palette_get_rgb(palette, color, &colorR, &colorG, &colorB);
    
    int dR = (int)r - (int)colorR;
    int dG = (int)g - (int)colorG;
    int dB = (int)b - (int)colorB;
    
    return dR * dR + dG * dG + dB + dB;
}

X_Color x_palette_get_closest_color_from_rgb(const X_Palette* palette, unsigned char r, unsigned char g, unsigned char b)
{
    int minDist = 0x7FFFFFF;
    X_Color closestColor = 0;
    
    for(int i = 0; i < 256; ++i)
    {
        int dist = x_palette_calculate_color_distance(palette, i, r, g, b);
        
        if(dist < minDist)
        {
            minDist = dist;
            closestColor = i;
        }
    }
    
    return closestColor;
}

