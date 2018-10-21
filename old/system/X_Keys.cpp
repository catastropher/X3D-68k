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

#include "X_Keys.h"

static void build_shift_keys(X_KeyState* state)
{
    for(int i = 0; i < X_TOTAL_KEYS; ++i)
        state->keyshift[i] = i;
}

void x_keystate_init(X_KeyState* state)
{
    build_shift_keys(state);
    x_keystate_reset_keys(state);
    
    state->textInputMode = 0;
    state->lastKeyPressed = X_KEY_INVALID;
}

void x_keystate_reset_keys(X_KeyState* state)
{
    for(int i = 0; i < X_TOTAL_KEYS; ++i)
        state->keyDown[i] = 0;
    
    state->keyQueueHead = 0;
    state->keyQueueTail = 0;
}

static int next_queue_position(int currentPos)
{
    ++currentPos;
    
    if(currentPos == X_KEY_QUEUE_SIZE)
        currentPos = 0;
    
    return currentPos;
}

static void x_keystate_enqueue(X_KeyState* state, X_Key key)
{
    state->keyQueue[state->keyQueueTail] = key;
    state->keyQueueTail = next_queue_position(state->keyQueueTail);
}

bool x_keystate_dequeue(X_KeyState* state, X_Key* dest)
{
    if(x_keystate_queue_empty(state))
        return 0;
    
    *dest = (X_Key)state->keyQueue[state->keyQueueHead];
    state->keyQueueHead = next_queue_position(state->keyQueueHead);
    
    return 1;
}

bool x_keystate_queue_empty(const X_KeyState* state)
{
    return !state->textInputMode || state->keyQueueHead == state->keyQueueTail;
}

void x_keystate_send_key_press(X_KeyState* state, X_Key key, X_Key unicodeKey)
{
    if(state->textInputMode)
    {
        x_keystate_enqueue(state, unicodeKey);
        state->lastKeyPressed = key;
        state->keyToRepeat = unicodeKey;
        state->nextKeyRepeat = state->currentTime + X_KEY_REPEAT_INITIAL_TIME;
    }
    
    state->keyDown[key] = 1;
}

void x_keystate_send_key_release(X_KeyState* state, X_Key key)
{
    state->keyDown[key] = 0;
    state->keyToRepeat = X_KEY_INVALID;
}

void x_keystate_handle_key_repeat(X_KeyState* state, X_Time currentTime)
{
    if(!state->textInputMode)
        return;
    
    state->currentTime = currentTime;
    bool validKeyIsStillPressed = state->keyToRepeat != X_KEY_INVALID && x_keystate_key_down(state, state->lastKeyPressed);
    
    if(!validKeyIsStillPressed || currentTime < state->nextKeyRepeat)
        return;
    
    x_keystate_enqueue(state, state->keyToRepeat);
    state->nextKeyRepeat = currentTime + X_KEY_REPEAT_TIME;
}

