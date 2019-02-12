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

#include <new>
#include <cstring>

#include "EngineQueue.hpp"
#include "memory/Memory.hpp"
#include "error/Log.hpp"

void EngineQueue::addEvent(EngineEvent& event)
{
    if(queue.isFull())
    {
        flush();
    }

    queue.enqueue(event);
}

void EngineQueue::flush()
{
    while(!queue.isEmpty())
    {
        EngineEvent& event = *queue.dequeue();

        EngineEventHandler* handler = handlerHead;
        EngineEventHandler* nextHandler;

        while(handler)
        {
            // It's possible to remove a handler during its execution, so grad the next just to be safe
            nextHandler = handler->next;

            bool done = !handler->invoke(event, engineContext);
            if(done)
            {
                break;
            }

            handler = nextHandler;
        }
    }
}

void EngineQueue::addHandler(const char* name, bool (*handler)(EngineEvent& event, EngineContext* engineContext), int priority)
{
    auto eventHandler = Zone::alloc<EngineEventHandler>();
    new (eventHandler) EngineEventHandler(name, handler, priority);

    if(handlerHead == nullptr || priority < handlerHead->priority)
    {
        eventHandler->next = handlerHead;
        handlerHead = eventHandler;
    }
    else
    {
        auto h = handlerHead;

        while(h->next && h->next->priority < priority)
        {
            h = h->next;
        }

        eventHandler->next = h->next;
        h->next = eventHandler;
    }

    Log::info("Added event handler %s, priority = %d", name, priority);
}

void EngineQueue::removeHandler(const char* name)
{
    EngineEventHandler* prev = nullptr;
    EngineEventHandler* handler = handlerHead;

    while(handler)
    {
        if(strcmp(name, handler->name) == 0)
        {
            auto next = handler->next;

            if(prev == nullptr)
            {
                handlerHead = handler->next;
            }
            else
            {
                prev->next = handler->next;
            }

            Zone::free(handler);
            handler = next;

            Log::info("Removed event handler %s", name);
        }
    }
}

#include <cstdio>

bool EngineQueue::systemHandler(EngineEvent& event, EngineContext* engineContext)
{
    switch(event.type)
    {
        case EVENT_LEVEL_LOAD_PROGRESS:
            printf("Receive progress event! %d/%d\n", event.levelLoadProgress.currentStep, event.levelLoadProgress.totalSteps);
            break;

        default:
            break;
    }

    return true;
}

