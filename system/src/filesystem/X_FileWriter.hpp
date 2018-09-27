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

#include <cstdio>

#include "memory/X_Array.hpp"
#include "memory/X_FixedLengthString.hpp"
#include "error/X_Exception.hpp"
#include "log/X_Log.hpp"
#include "X_FilePath.hpp"

namespace X3D
{
    class FileWriter
    {
    public:
        FileWriter()
            : file(nullptr)
        {

        }

        FileWriter(const FilePath& path)
        {
            file = fopen(path.c_str(), "wb");

            if(!file)
            {
                Log::error("Failed to open file %s for writing\n", path.c_str());

                throw Exception("Failed to open file for writing");
            }

            Log::debug("Opened file %s for writing", path.c_str());
        }

        template<typename T>
        FileWriter& write(const T& val);

        template<typename T>
        FileWriter& writeArray(const T* data, int count)
        {
            for(int i = 0; i < count; ++i)
            {
                write(data[i]);
            }

            return *this;
        }

        template<typename T>
        FileWriter& writeArray(const Array<T>& arr)
        {
            return writeArray(arr.data, arr.size);
        }

        template<int MaxLength>
        FileWriter& write(const FixedLengthString<MaxLength>& str)
        {
            return writeArray(str.c_str(), MaxLength);
        }

        FileWriter& saveOffset(int& dest)
        {
            dest = ftell(file);

            return *this;
        }

        FileWriter& seek(int offset)
        {
            fseek(file, offset, SEEK_SET);

            return *this;
        }

        ~FileWriter()
        {
            if(file)
            {
                fclose(file);
            }
        }

    private:
        FILE* file;
    };

    template<>
    inline FileWriter& FileWriter::write(const unsigned int& val)
    {
        fwrite(&val, 4, 1, file);

        return *this;
    }

    template<>
    inline FileWriter& FileWriter::write(const int& val)
    {
        return write((unsigned int)val);
    }

    template<>
    inline FileWriter& FileWriter::writeArray(const char* data, int count)
    {
        fwrite(data, 1, count, file);

        return *this;
    }
}

