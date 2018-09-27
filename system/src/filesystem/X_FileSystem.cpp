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

#include "X_FileSystem.hpp"
#include "X_DirectoryScanner.hpp"
#include "X_PakWriter.hpp"

namespace X3D
{
    FileSystem::FileSystem(MemoryManager& memoryManager)
        : pakManager(memoryManager, fileHandleCache)
    {
    #if 1
        FilePath path("b.txt");
        FileSearchRequest request(path, AllocationSource::zone);

        PakFile pakFile;

        printf("Found: %d\n", pakManager.readPakFile(request, pakFile));

        fwrite(pakFile.data.data, 1, pakFile.data.size, stdout);
    #endif

    #if 1
        PakWriter writer("test.pak", memoryManager, AllocationSource::zone);

        char data[] = "Hello world!\n";

        Array<char> arr(data, sizeof(data));

        writer.addInMemoryFile(arr, "test/a.txt");

        char data2[] = "Hello world take 2!\n";

        Array<char> arr2(data2, sizeof(data2));

        writer.addInMemoryFile(arr2, "test/b.txt");

    #endif
    }
}

