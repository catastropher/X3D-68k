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

#include "Keys.hpp"

static void build_shift_keys(KeyState* state)
{
    for(int i = 0; i < X_TOTAL_KEYS; ++i)
        state->keyshift[i] = i;
}

void x_keystate_init(KeyState* state)
{
    build_shift_keys(state);
    x_keystate_reset_keys(state);
    
    state->textInputMode = 0;
    state->lastKeyPressed = KeyCode::invalid;
}

void x_keystate_reset_keys(KeyState* state)
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

static void x_keystate_enqueue(KeyState* state, KeyCode key)
{
    state->keyQueue[state->keyQueueTail] = keyCodeToChar(key);
    state->keyQueueTail = next_queue_position(state->keyQueueTail);
}

bool x_keystate_dequeue(KeyState* state, KeyCode* dest)
{
    if(x_keystate_queue_empty(state))
        return 0;
    
    *dest = (KeyCode)state->keyQueue[state->keyQueueHead];
    state->keyQueueHead = next_queue_position(state->keyQueueHead);
    
    return 1;
}

bool x_keystate_queue_empty(const KeyState* state)
{
    return !state->textInputMode || state->keyQueueHead == state->keyQueueTail;
}

void x_keystate_send_key_press(KeyState* state, KeyCode key, KeyCode unicodeKey)
{
    if(state->textInputMode)
    {
        x_keystate_enqueue(state, unicodeKey);
        state->lastKeyPressed = key;
        state->keyToRepeat = unicodeKey;
        state->nextKeyRepeat = state->currentTime + Duration::fromMilliseconds(500);
    }
    
    state->keyDown[(int)key] = 1;
}

void x_keystate_send_key_release(KeyState* state, KeyCode key)
{
    state->keyDown[(int)key] = 0;
    state->keyToRepeat = KeyCode::invalid;
}

void x_keystate_handle_key_repeat(KeyState* state, Time currentTime)
{
    if(!state->textInputMode)
        return;
    
    state->currentTime = currentTime;
    bool validKeyIsStillPressed = state->keyToRepeat != KeyCode::invalid && x_keystate_key_down(state, state->lastKeyPressed);
    
    if(!validKeyIsStillPressed || currentTime < state->nextKeyRepeat)
        return;
    
    x_keystate_enqueue(state, state->keyToRepeat);
    state->nextKeyRepeat = currentTime + Duration::fromMilliseconds(100);
}

