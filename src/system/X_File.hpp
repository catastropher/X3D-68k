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

#include <cstdio>

#include "geo/X_Vec3.h"
#include "system/X_FilePath.hpp"

static inline void swapIntBytes(unsigned int& val)
{
    
}

static inline void swapShortBytes(unsigned short& val)
{

}

class FileReader
{
public:
    FileReader() : file(nullptr) { }
    
    bool open(const char* fileName);

    bool open(FilePath& path)
    {
        return open(path.c_str());
    }

    int getSize() const
    {
        return size;
    }

    template<typename T>
    T read();

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
    void determineSize();

    FILE* file;
    int size;
    int flags;
};

template<>
inline unsigned int FileReader::read()
{
    unsigned int result;
    fread(&result, 4, 1, file);
    swapIntBytes(result);

    return result;
}

template<>
inline int FileReader::read()
{
    return read<unsigned int>();
}

template<>
inline unsigned char FileReader::read()
{
    return fgetc(file);
}

template<>
inline char FileReader::read()
{
    return read<unsigned char>();
}

template<>
inline unsigned short FileReader::read()
{
    unsigned short result;
    fread(&result, 2, 1, file);
    swapShortBytes(result);

    return result;
}

template<>
inline short FileReader::read()
{
    return read<short>();
}

template<>
inline Vec3 FileReader::read()
{
    unsigned int v[3];

    fread(v, 3 * sizeof(int), 1, file);

    swapIntBytes(v[0]);
    swapIntBytes(v[1]);
    swapIntBytes(v[2]);

    return Vec3(v[0], v[1], v[2]);
}



