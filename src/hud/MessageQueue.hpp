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

#include <cstdarg>

#include "memory/CircularQueue.hpp"
#include "memory/FixedLengthString.hpp"
#include "system/Clock.hpp"
#include "system/Time.hpp"
#include "render/Screen.hpp"
#include "render/Font.hpp"
#include "render/Texture.hpp"

struct Message
{
    Message()
    {
        expirationTime = Clock::getTicks();
    }

    static const int maxMessageLength = 128;

    FixedLengthString<maxMessageLength> message;
    Time expirationTime;
};

class MessageQueue
{
public:
    MessageQueue(Duration messageLifetime_, Screen* screen_, Font* font_)
        : messageLifetime(messageLifetime_),
        screen(screen_),
        font(font_)
    {

    }

    void addMessage(const char* format, ...);
    void render();

private:
    static const int maxMessages = 8;

    void removeExpiredMessages();
    
    Duration messageLifetime;
    CircularQueue<Message, maxMessages> messages;

    Screen* screen;
    Font* font;
};

