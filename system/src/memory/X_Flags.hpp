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
    template<typename T>
    class Flags
    {
    public:
        Flags()
            : flags(0)
        {

        }

        bool hasFlag(T flag)
        {
            return (flags & (unsigned int)flag) != 0;
        }

        void setFlag(T flag)
        {
            flags |= (unsigned int)flag;
        }

        void toggleFlag(T flag)
        {
            flags ^= (unsigned int)flag;
        }

        void clearFlag(T flag)
        {
            flags &= (~(unsigned int)flag);
        }

        void operator=(unsigned int flags_)
        {
            flags = flags_;
        }

        void clear()
        {
            flags = 0;
        }

    private:
        unsigned int flags;
    };
}

