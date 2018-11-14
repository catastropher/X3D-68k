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

namespace X3D
{
    struct EntityDictionaryEntry
    {
        StringId hash;
        FixedLengthString<64> value;
    };

    class EntityDictionary
    {
    public:
        template<typename T>
        void set(StringId attributeName, const T& value)
        {
            auto entry = getOrCreateByHash(value);
            serialize(value, &entry->value[0]);
        }

        template<typename T>
        void get(StringId attributeName, T& dest)
        {
            auto entry = getByHash(attributeName);
            deserialize(&entry->value[0], dest);
        }

        template<typename T>
        static void serialize(const T& value, char* dest);

        template<typename T>
        static void deserialize(const char* str, T& dest);

        EntityDictionaryEntry* tryGetByHash(StringId hash);
        EntityDictionaryEntry* getOrCreateByHash(StringId hash);

        EntityDictionaryEntry* getByHash(StringId hash);

    private:
        static const int MAX_ENTRIES = 64;

        int totalEntries;
        EntityDictionaryEntry entries[MAX_ENTRIES];
    };
}