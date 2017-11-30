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

#include "geo/X_Vec2.h"
#include "math/X_fix.h"
#include "render/X_Screen.h"

typedef enum X_MouseButton
{
    X_MOUSE_LEFT = 0,
    X_MOUSE_RIGHT = 1
} X_MouseButton;

typedef struct X_MouseState
{
    X_Vec2 pos;
    X_Screen* screen;
    X_Vec2 offset;
    
    _Bool buttonDown[2];
    _Bool buttonPressed[2];
    
    _Bool mouseLook;
    x_fp16x16 xSpeed;
    x_fp16x16 ySpeed;
    
    _Bool invert;
} X_MouseState;

struct X_Console;

void x_mousestate_init(X_MouseState* state, struct X_Console* console, X_Screen* screen);
void x_mousestate_set_pos(X_MouseState* state, X_Vec2 pos);
void x_mousestate_show_cursor(X_MouseState* state, _Bool showCursor);
void x_mousestate_update_pos(X_MouseState* state, X_Vec2 pos);
X_Vec2_fp16x16 x_mousestate_get_mouselook_angle_change(X_MouseState* state);

static inline void x_mousestate_send_button_press(X_MouseState* state, X_MouseButton button)
{
    state->buttonDown[button] = 1;
    state->buttonPressed[button] = 1;
}

static inline void x_mousestate_send_button_release(X_MouseState* state, X_MouseButton button)
{
    state->buttonDown[button] = 0;
}

static inline _Bool x_mousestate_button_down(X_MouseState* state, X_MouseButton button)
{
    return state->buttonDown[button];
}

static inline _Bool x_mousestate_button_pressed(X_MouseState* state, X_MouseButton button)
{
    _Bool wasPressed = state->buttonPressed[button];
    state->buttonPressed[button] = 0;
    return wasPressed;
}

static inline X_Vec2 x_mousestate_pos(X_MouseState* state)
{
    return state->pos;
}

static inline void x_mousestate_clear(X_MouseState* state)
{
    for(int i = 0; i < 2; ++i)
    {
        state->buttonDown[i] = 0;
        state->buttonPressed[i] = 0;
    }
}

