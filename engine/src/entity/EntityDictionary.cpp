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

#include "X3D-System.hpp"

#include "entity/EntityDictionary.hpp"

namespace X3D
{
    EntityDictionaryEntry* EntityDictionary::tryGetByHash(StringId hash)
    {
        for(int i = 0; i < totalEntries; ++i)
        {
            if(entries[i].hash == hash)
            {
                return entries + i;
            }
        }

        return nullptr;
    }

    EntityDictionaryEntry* EntityDictionary::getOrCreateByHash(StringId hash)
    {
        auto entry = tryGetByHash(hash);

        if(entry != nullptr)
        {
            return entry;
        }

        auto newEntry = &entries[totalEntries++];

        if(totalEntries >= MAX_ENTRIES)
        {
            fatalError("Entity dictionary is full");
        }

        newEntry->hash = hash;
        newEntry->value.clear();

        return newEntry;
    }

    EntityDictionaryEntry* EntityDictionary::getByHash(StringId hash)
    {
        auto entry = tryGetByHash(hash);

        if(entry == nullptr)
        {
            fatalError("No such entity dictionary entry: %s\n", hash.toString());

            // FIXME: create engine class
            throw SystemException(SystemErrorCode::outOfMemory);
        }

        return entry;
    }

    template<>
    void EntityDictionary::serialize(const int& value, char* dest)
    {
        sprintf(dest, "%d", value);
    }
}