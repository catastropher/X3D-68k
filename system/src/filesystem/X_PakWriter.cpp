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

#include "X_PakWriter.hpp"

namespace X3D
{
    void PakWriter::addInMemoryFile(const Array<char>& data, const FilePath& path)
    {
        PakEntry* entry = allocator.alloc();

        entry->fileName = path.c_str();
        entry->fileSize = data.size;

        writer
            .saveOffset(entry->fileOffset)
            .writeArray(data);
    }

    void PakWriter::writePlaceholderHeader()
    {
        int fakeFileTableOffset = 0;
        int fakeFileTableSize = 0;

        const int magicNumber = Pak::MAGIC_NUMBER;

        writer
            .write(magicNumber)
            .saveOffset(headerOffset)
            .write(fakeFileTableOffset)
            .write(fakeFileTableSize);
    }

    void PakWriter::write()
    {
        writeFileTable();
        writeRealHeader();
    }

    void PakWriter::writeFileTable()
    {
        writer.saveOffset(fileTableOffset);

        for(auto& entry : allocator)
        {
            writer
                .write(entry.fileName)
                .write(entry.fileOffset)
                .write(entry.fileSize);
        }

        fileTableSize = PakEntry::FILE_SIZE * allocator.getTotalAllocs();
    }

    void PakWriter::writeRealHeader()
    {
        writer
            .seek(headerOffset)
            .write(fileTableOffset)
            .write(fileTableSize);
    }
}

