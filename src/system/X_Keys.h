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

#define X_TOTAL_KEYS 256

typedef enum X_Key
{
    X_KEY_ENTER = '\n',
    X_KEY_OPEN_CONSOLE = 128,
    X_KEY_SHIFT = 129
} X_Key;

#define X_KEY_QUEUE_SIZE 32

typedef struct X_KeyState
{
    _Bool keyDown[X_TOTAL_KEYS];
    
    _Bool textInputMode;
    unsigned char keyQueue[X_KEY_QUEUE_SIZE];
    int keyQueueHead;
    int keyQueueTail;
    unsigned char keyshift[X_TOTAL_KEYS];
} X_KeyState;

void x_keystate_init(X_KeyState* state);
void x_keystate_reset_keys(X_KeyState* state);
_Bool x_keystate_queue_empty(const X_KeyState* state);
_Bool x_keystate_dequeue(X_KeyState* state, X_Key* dest);
void x_keystate_send_key_press(X_KeyState* state, X_Key key);
void x_keystate_send_key_release(X_KeyState* state, X_Key key);

static inline _Bool x_keystate_key_down(const X_KeyState* keystate, X_Key key)
{
    return keystate->keyDown[key];
}

static inline void x_keystate_enable_text_input(X_KeyState* state)
{
    state->textInputMode = 1;
}

static inline void x_keystate_disable_text_input(X_KeyState* state)
{
    state->textInputMode = 0;
}

