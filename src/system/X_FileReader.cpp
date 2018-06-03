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

#include "X_FileReader.hpp"
#include "X_FileSystem.hpp"
#include "error/X_log.h"
#include "memory/X_Memory.hpp"

bool FileReader::open(const char* fileName)
{
    FileLocation location;
    if(!FileSystem::locateFile(fileName, location))
    {
        Log::error("Failed to open file %s for reading", fileName);

        return false;
    }

    file = location.file;
    determineSize();

    Log::info("Opened file %s for reading", fileName);

    return true;
}

char* FileReader::readWholeFile(const char* fileName, int& size)
{
    FileReader reader;
    if(!reader.open(fileName))
    {
        return nullptr;
    }

    char* buf = Zone::alloc<char>(reader.getSize() + 1);
    reader.readArray(buf, reader.getSize());

    size = reader.getSize();
    buf[size] = '\0';

    return buf;
}

void FileReader::determineSize()
{
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
}

FileReader::~FileReader()
{
    if(file != nullptr)
    {
        fclose(file);
    }
}

