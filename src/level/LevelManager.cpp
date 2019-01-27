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

#include "LevelManager.hpp"
#include "BspLevelLoader.hpp"
#include "entity/EntityManager.hpp"

void LevelManager::switchLevel(const char *fileName)
{
    if(currentLevel != nullptr)
    {
        unloadLevel(currentLevel);
    }

    currentLevel = loadLevel(fileName);
}

void LevelManager::unloadLevel(BspLevel *level)
{
    entityManager.destroyAllEntities();
}

BspLevel *LevelManager::loadLevel(const char *fileName)
{
    BspLevel* newLevel = new BspLevel;  // FIXME: better memory management
    x_bsplevel_load_from_bsp_file(newLevel, fileName, &engineQueue);

    entityManager.createEntitesInLevel(*newLevel);

    return newLevel;
}
