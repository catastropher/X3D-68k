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

#include "geo/Vec2.hpp"
#include "math/FixedPoint.hpp"
#include "render/Screen.hpp"

typedef enum X_MouseButton
{
    X_MOUSE_LEFT = 0,
    X_MOUSE_RIGHT = 1
} X_MouseButton;

struct MouseState
{
    MouseState(struct Console* console, Screen* screen);

    Vec2 pos;
    Screen* screen;
    Vec2 offset;
    
    bool buttonDown[2];
    bool buttonPressed[2];
    
    bool mouseLook;
    x_fp16x16 xSpeed;
    x_fp16x16 ySpeed;
    
    bool invert;
};

struct Console;

void x_mousestate_set_pos(MouseState* state, Vec2 pos);
void x_mousestate_show_cursor(MouseState* state, bool showCursor);
void x_mousestate_update_pos(MouseState* state, Vec2 pos);
Vec2_fp16x16 x_mousestate_get_mouselook_angle_change(MouseState* state);

static inline void x_mousestate_send_button_press(MouseState* state, X_MouseButton button)
{
    state->buttonDown[button] = 1;
    state->buttonPressed[button] = 1;
}

static inline void x_mousestate_send_button_release(MouseState* state, X_MouseButton button)
{
    state->buttonDown[button] = 0;
}

static inline bool x_mousestate_button_down(MouseState* state, X_MouseButton button)
{
    return state->buttonDown[button];
}

static inline bool x_mousestate_button_pressed(MouseState* state, X_MouseButton button)
{
    bool wasPressed = state->buttonPressed[button];
    state->buttonPressed[button] = 0;
    return wasPressed;
}

static inline Vec2 x_mousestate_pos(MouseState* state)
{
    return state->pos;
}

static inline void x_mousestate_clear(MouseState* state)
{
    for(int i = 0; i < 2; ++i)
    {
        state->buttonDown[i] = 0;
        state->buttonPressed[i] = 0;
    }
}

