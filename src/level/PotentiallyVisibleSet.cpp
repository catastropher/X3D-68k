#include "PotentiallyVisibleSet.hpp"
#include "X_BspNode.hpp"
#include "X_BspLevel.h"

void PotentiallyVisibleSet::decompressPvsForLeaf(X_BspLeaf& leaf, DecompressedLeafVisibleSet& dest)
{
    // TODO: int pvsSize = x_bspfile_node_pvs_size(this);
    auto& pvsFromLeaf = leaf.pvsFromLeaf;
    bool hasVisibilityInfoForCurrentLeaf = pvsFromLeaf.hasPvsData() && !leaf.isOutsideLevel();
    
    if(!hasVisibilityInfoForCurrentLeaf)
    {
        dest.markAllLeavesAsVisible(bytesPerEntry);
        return;
    }
    
    leaf.pvsFromLeaf.decompress(bytesPerEntry, dest);
}

PotentiallyVisibleSet::PotentiallyVisibleSet(BspLevel& level_)
    : level(level_)
{
    
}

void PotentiallyVisibleSet::setCompressedPvsData(unsigned char* compressedPvsData)
{
    pvs = compressedPvsData;
    bytesPerEntry = (level.totalNodes + 7) / 8;
}



void PotentiallyVisibleSet::markVisibleLeaves(DecompressedLeafVisibleSet& decompressedPvs, int currentFrame)
{
    int totalLeaves = x_bsplevel_get_level_model(&level)->totalBspLeaves;
    
    // We skip leaf 0 because it represents outside the level and should never be potentially visible.
    // Note that the PVS excludes leaf 0 for this reason.
    for(int i = 1; i < totalLeaves; ++i)
    {
        X_BspNode* leafNode = (X_BspNode*)x_bsplevel_get_leaf(&level, i);
        
        if(decompressedPvs.leafIsVisible(i))
        {
            leafNode->markAncestorsAsVisible(currentFrame);
        }
    }
}

PotentiallyVisibleSet::~PotentiallyVisibleSet()
{
    x_free(pvs);
}

void CompressedLeafVisibleSet::decompress(int pvsBytesPerEntry, DecompressedLeafVisibleSet& dest)
{
    unsigned char* decompressedPvsData = dest.getPvsBytes();
    unsigned char* decompressedPvsEnd = dest.getPvsBytes() + pvsBytesPerEntry;
    unsigned char* compressedData = compressedPvsData;
    
    while(decompressedPvsData < decompressedPvsEnd)
    {
        if(*compressedData == 0)
        {
            ++compressedData;
            int count = *compressedData++;
            
            for(int i = 0; i < count; ++i)
            {
                *decompressedPvsData++ = 0;
            }
        }
        else
        {
            *decompressedPvsData++ = *compressedData++;
        }
    }
}

void DecompressedLeafVisibleSet::markAllLeavesAsVisible(int pvsBytesPerEntry)
{
    memset(leafPvs, 0xFF, pvsBytesPerEntry);
}
