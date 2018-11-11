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

namespace X3D
{
    enum Keys
    {
        KEY_INVALID = 0,
        KEY_ENTER = '\n',
        KEY_TAB = '\t',
        KEY_OPEN_CONSOLE = 128,
        KEY_SHIFT,
        KEY_UP,
        KEY_DOWN,
        KEY_LEFT,
        KEY_RIGHT,
        KEY_ESCAPE,
        KEY_LCTRL,
        KEY_RCTRL,
        KEY_ALT
    };

    struct KeyState
    {
        static const int TOTAL_KEYS = 256;

        void setKey(int key)
        {
            keyBitSet[key / 32] |= 1 << (key & 31);
        }

        void resetKey(int key)
        {
            keyBitSet[key / 32] &= ~(1 << (key & 31));
        }

        bool keyIsSet(int key) const
        {
            return (keyBitSet[key / 32] & (1 << (key & 31))) != 0;
        }

        void clear()
        {
            for(int i = 0; i < TOTAL_KEYS / 32; ++i)
            {
                keyBitSet[i] = 0;
            }
        }

        unsigned int keyBitSet[TOTAL_KEYS / 32];
    };
}

