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

class BitSet
{
public:
    BitSet()
        : flags(0)
    {

    }

    BitSet(unsigned int flags_) : flags(flags_)
    {

    }

    bool isSet(int bit) const
    {
        return flags & (1 << bit);
    }

    bool isSetFromMask(unsigned int mask)
    {
        return flags & mask;
    }

    void setFromMask(unsigned int mask)
    {
        flags |= mask;
    }

    void resetFromMask(unsigned int mask)
    {
        flags &= (~mask);
    }

    void set(int bit, bool value)
    {
        flags = (flags & ~(1 << bit)) | ((int)value << bit);
    }

    void set(int bit)
    {
        flags |= (1 << bit);
    }

    void reset(int bit)
    {
        flags = flags & ~(1 << bit);
    }

    void clear()
    {
        flags = 0;
    }

    unsigned int getMask() const
    {
        return flags;
    }

private:
    unsigned int flags;
};

template<typename T>
class Flags
{
public:
    Flags()
    {
        
    }

    Flags(unsigned int flags)
        : bitSet(flags)
    {

    }

    void set(T flag)
    {
        bitSet.setFromMask((unsigned int)flag);
    }

    void reset(T flag)
    {
        return bitSet.resetFromMask((unsigned int)flag);
    }

    void set(unsigned int flags)
    {
        bitSet.setFromMask(flags);
    }

    void set(const Flags& set)
    {
        bitSet.setFromMask(set.getMask());
    }

    bool hasFlag(T flag)
    {
        return bitSet.isSetFromMask((int)flag);
    }

    void clear()
    {
        bitSet.clear();
    }

    unsigned int getMask() const
    {
        return bitSet.getMask();
    }

private:
    BitSet bitSet;
};

