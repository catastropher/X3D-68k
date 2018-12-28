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

struct X_BspLeaf;
class DecompressedLeafVisibleSet;
struct BspLevel;

class PotentiallyVisibleSet
{
public:
    PotentiallyVisibleSet(BspLevel& level_);
    
    void setCompressedPvsData(unsigned char* compressedPvsData);
    
    unsigned char* getCompressedPvsData()
    {
        return pvs;
    }
    
    int getBytesPerEntry()
    {
        return bytesPerEntry;
    }
    
    void decompressPvsForLeaf(X_BspLeaf& leaf, DecompressedLeafVisibleSet& dest);
    void markVisibleLeaves(DecompressedLeafVisibleSet& decompressedPvs, int currentFrame);
    
    void updatePvsData();
    
    ~PotentiallyVisibleSet();
    
private:
    BspLevel& level;
    unsigned char* pvs;
    int bytesPerEntry;
};

class DecompressedLeafVisibleSet
{
public:
    bool leafIsVisible(int leafId)
    {
        int bitIndex = leafId - 1;
        
        return leafPvs[bitIndex / 32] & (1 << (bitIndex & 31));
    }
    
    unsigned char* getPvsBytes()
    {
        return (unsigned char*)leafPvs;
    }
    
    void markAllLeavesAsVisible(int pvsBytesPerEntry);
    
private:
    unsigned int leafPvs[1024 / sizeof(unsigned int)];
};

class CompressedLeafVisibleSet
{
public:
    bool hasPvsData()
    {
        return compressedPvsData != nullptr;
    }
    
    void setCompressedBytes(unsigned char* compressedPvsBytes)
    {
        compressedPvsData = compressedPvsBytes;
    }
    
    void decompress(int pvsBytesPerEntry, DecompressedLeafVisibleSet& dest);
    
private:
    unsigned char* compressedPvsData;
};

