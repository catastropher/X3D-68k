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

#include "memory/X_UnrolledListAllocator.hpp"
#include "X_FileWriter.hpp"
#include "memory/X_MemoryManager.hpp"
#include "X_FilePath.hpp"
#include "X_Pak.hpp"


namespace X3D
{
    class PakWriter
    {
    public:
        PakWriter(const FilePath& fileName, MemoryManager& memoryManager, AllocationSource source = AllocationSource::zone)
            : writer(fileName),
            allocator(16, memoryManager, source)
        {
            writePlaceholderHeader();
        }

        void addInMemoryFile(const Array<char>& data, const FilePath& path);

        ~PakWriter()
        {
            write();   
        }

    private:
        void write();

        void writePlaceholderHeader();
        void writeRealHeader();
        void writeFileTable();

        FileWriter writer;
        UnrolledListAllocator<PakEntry> allocator;

        int headerOffset;
        int fileTableOffset;
        int fileTableSize;
    };
}

