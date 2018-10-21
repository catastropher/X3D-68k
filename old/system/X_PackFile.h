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

#include "X_File.h"

#define X_PACKFILE_HEADER_MAGIC_NUMBER (('P') + ('A' << 8) + ('C' << 16) + ('K' << 24))

typedef struct X_PackFileHeader
{
    int fileTableOffset;
    int fileTableSize;
} X_PackFileHeader;

#define X_PACKFILEENTRY_NAME_LENGTH 56
#define X_PACKFILEENTRY_SIZE 64

typedef struct X_PackFileEntry
{
    char name[X_PACKFILEENTRY_NAME_LENGTH];
    int fileOffset;
    int size;
} X_PackFileEntry;

typedef struct X_PackFile
{
    X_File file;
    X_PackFileHeader header;
    X_PackFileEntry* entries;
    int totalEntries;
} X_PackFile;

bool x_packfile_read_from_file(X_PackFile* file, const char* fileName);
void x_packfile_print_files(X_PackFile* file);
void x_packfile_cleanup(X_PackFile* file);
char* x_packfile_load_file(X_PackFile* file, const char* fileName);
bool x_packfile_extract(X_PackFile* file, const char* dirToExtractTo);
X_PackFileEntry* x_packfile_find_file(X_PackFile* file, const char* fileToFind);

