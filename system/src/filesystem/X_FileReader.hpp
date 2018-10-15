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

#include "X_FileHandle.hpp"
#include "memory/X_Array.hpp"

namespace X3D
{
    static inline void swapIntBytes(unsigned int& val)
    {
        
    }

    static inline void swapIntBytes(int& val)
    {
        
    }

    static inline void swapShortBytes(unsigned short& val)
    {

    }

    class FileReader
    {
    public:
        FileReader()
            : handle(nullptr)
        {

        }

        FileReader(FileHandle* handle_)
            : handle(handle_)
        {

        }

        bool read(Array<char>& dest, int count)
        {
            dest.size = fread(dest.data, 1, count, handle->file);

            return dest.size == count;
        }

        void seek(int offset)
        {
            fseek(handle->file, offset, SEEK_SET);
        }

    private:
        FileHandle* handle;
    };
}


