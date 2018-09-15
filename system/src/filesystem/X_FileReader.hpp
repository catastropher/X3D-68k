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
#include "memory/X_FixedLengthString.hpp"

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

        template<typename T>
        FileReader& read(T& dest);

        template<int MaxLength>
        FileReader& read(FixedLengthString<MaxLength>& dest);

        template<typename T>
        void readArray(T* dest, int count)
        {
            for(int i = 0; i < count; ++i)
            {
                dest[i] = read<T>();
            }
        }

        static char* readWholeFile(const char* fileName, int& size);

        ~FileReader();

    private:
        FileHandle* handle;
        void determineSize();
    };

    template<>
    inline FileReader& FileReader::read(unsigned int& dest)
    {
        fread(&dest, 4, 1, handle->file);
        swapIntBytes(dest);

        return *this;
    }

    template<>
    inline FileReader& FileReader::read(int& dest)
    {
        fread(&dest, 4, 1, handle->file);
        swapIntBytes(dest);

        return *this;
    }

    template<int MaxLength>
    inline FileReader& FileReader::read(FixedLengthString<MaxLength>& dest)
    {
        readArray(dest.getBuf(), MaxLength);

        return *this;
    }

    // template<>
    // inline unsigned char FileReader::read()
    // {
    //     return fgetc(handle->file);
    // }

    // template<>
    // inline char FileReader::read()
    // {
    //     return read<unsigned char>();
    // }

    // template<>
    // inline unsigned short FileReader::read()
    // {
    //     unsigned short result;
    //     fread(&result, 2, 1, handle->file);
    //     swapShortBytes(result);

    //     return result;
    // }

    // template<>
    // inline short FileReader::read()
    // {
    //     return read<unsigned short>();
    // }

    // template<int T>
    // inline 
}


