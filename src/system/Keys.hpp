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

#include "system/Time.hpp"

#define X_TOTAL_KEYS 256

#define X_KEY_REPEAT_INITIAL_TIME 500
#define X_KEY_REPEAT_TIME 100

enum class KeyCode
{
    invalid = 0,
    enter = '\n',
    backtick = '`',
    tab = '\t',
    backspace = '\b',
    shift = 128,
    up,
    down,
    left,
    right,
    escape,
};

static inline char keyCodeToChar(KeyCode key)
{
    return (char)key;
}

#define X_KEY_QUEUE_SIZE 32

struct KeyBind
{

};

struct KeyState
{
    KeyState();

    bool keyDown[X_TOTAL_KEYS];
    
    bool textInputMode;
    unsigned char keyQueue[X_KEY_QUEUE_SIZE];
    int keyQueueHead;
    int keyQueueTail;
    unsigned char keyshift[X_TOTAL_KEYS];

    Time currentTime;
    Time nextKeyRepeat;
    KeyCode lastKeyPressed;
    KeyCode keyToRepeat;
};

void x_keystate_init(KeyState* state);
void x_keystate_reset_keys(KeyState* state);
bool x_keystate_queue_empty(const KeyState* state);
bool x_keystate_dequeue(KeyState* state, KeyCode* dest);
void x_keystate_send_key_press(KeyState* state, KeyCode key, KeyCode unicodeKey);
void x_keystate_send_key_release(KeyState* state, KeyCode key);
void x_keystate_handle_key_repeat(KeyState* state, Time currentTime);

static inline bool x_keystate_key_down(const KeyState* keystate, KeyCode key)
{
    return keystate->keyDown[(int)key];
}

static inline void x_keystate_enable_text_input(KeyState* state)
{
    state->textInputMode = 1;
}

static inline void x_keystate_disable_text_input(KeyState* state)
{
    state->textInputMode = 0;
}

