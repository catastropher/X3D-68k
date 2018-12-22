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

#include "X_Mouse.h"
#include "platform/X_Platform.h"
#include "dev/X_Console.h"

static void cmd_mouselook(X_EngineContext* engineContext, int argc, char* argv[])
{
    X_MouseState* state = engineContext->getMouseState();
    
    if(argc != 2)
    {
        x_console_printf(engineContext->getConsole(), "Usage: mouse.look [0/1] -> enables/disables mouse look\n"
            "Currently %s\n", (state->mouseLook ? "enabled" : "disabled"));
        return;
    }
    
    state->mouseLook = atoi(argv[1]);
    x_mousestate_show_cursor(state, !state->mouseLook);
    x_mousestate_set_pos(state, x_screen_center(engineContext->getScreen()));
    
    state->offset.x = 0;
    state->offset.y = 0;
}

static void register_cmds(Console* console)
{
    x_console_register_cmd(console, "mouse.look", cmd_mouselook);
}

static void register_vars(X_MouseState* state, Console* console)
{
    x_console_register_var(console, &state->invert, "mouse.invert", X_CONSOLEVAR_BOOL, "0'", 1);
    x_console_register_var(console, &state->xSpeed, "mouse.xspeed", X_CONSOLEVAR_FP16X16, "1.0'", 1);
    x_console_register_var(console, &state->ySpeed, "mouse.yspeed", X_CONSOLEVAR_FP16X16, "1.0'", 1);
}

void x_mousestate_init(X_MouseState* state, Console* console, X_Screen* screen)
{
    state->screen = screen;
    state->mouseLook = 0;
    
    x_mousestate_clear(state);
    register_vars(state, console);
    register_cmds(console);
}

void x_mousestate_set_pos(X_MouseState* state, X_Vec2 pos)
{
    x_platform_mouse_set_position(pos);
    state->pos = pos;
}

void x_mousestate_show_cursor(X_MouseState* state, bool showCursor)
{
    x_platform_mouse_show_cursor(showCursor);
}

void x_mousestate_update_pos(X_MouseState* state, X_Vec2 pos)
{
    state->pos = pos;
    
    if(!state->mouseLook)
        return;
    
    X_Vec2 center = x_screen_center(state->screen);
    state->offset = x_vec2_sub(&pos, &center);
    
    x_mousestate_set_pos(state, center);
}

X_Vec2_fp16x16 x_mousestate_get_mouselook_angle_change(X_MouseState* state)
{
    x_fp16x16 baseSpeed = x_fp16x16_from_float(0.1);
    x_fp16x16 dx = x_fp16x16_mul(baseSpeed, state->xSpeed) * state->offset.x;
    x_fp16x16 dy = x_fp16x16_mul(baseSpeed, state->ySpeed) * state->offset.y;
    
    if(state->invert)
        dy = -dy;
    
    return x_vec2_make(dy, -dx);
}

