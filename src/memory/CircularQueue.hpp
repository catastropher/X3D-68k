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

template<typename T, int Size>
class CircularQueueIterator
{
public:
    CircularQueueIterator(const T* begin_, const T* current_)
        : begin(begin_),
        current(current_)
    {

    }

    bool operator==(const CircularQueueIterator& rhs) const
    {
       return current == rhs.current;
    }

    bool operator!=(const CircularQueueIterator& rhs) const
    {
        return !(*this == rhs);
    }

    const T& operator*() const
    {
        return *current;
    }

    CircularQueueIterator operator++()
    {
       current = current + 1 == begin + Size
           ? begin
           : current + 1;

       return CircularQueueIterator(begin, current);
    }

private:
    const T* const begin;
    const T* current;
};

template<typename T, int size>
class CircularQueue
{
public:
    CircularQueue()
        : head(items),
        tail(items)
    { }

    bool isFull()
    {
        return next(head) == tail;
    }

    bool isEmpty()
    {
        return head == tail;
    }

    void enqueue(T& item)
    {
        *allocate() = item;
    }

    T* allocate()
    {
        T* ptr = tail;
        tail = next(tail);

        return ptr;
    }

    T* dequeue()
    {
        T* ptr = head;
        head = next(head);

        return ptr;
    }

    CircularQueueIterator<T, size> begin() const
    {
        return CircularQueueIterator<T, size>(items, head);
    }

    CircularQueueIterator<T, size> end() const
    {
        return CircularQueueIterator<T, size>(items, tail);
    }

private:
    T* next(T* ptr)
    {
        return ptr + 1 == items + size
            ? items
            : ptr + 1;
    }

    T items[size];
    T* head;
    T* tail;
};

