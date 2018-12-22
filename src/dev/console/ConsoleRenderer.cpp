#include "render/X_Screen.h"
#include "../X_Console.h"
#include "ConsoleRenderer.hpp"
#include "render/X_Font.h"

// static X_Screen* x_console_get_screen(X_Console* console)
// {
//     return console->engineContext->getScreen();
// }

// static X_Texture* x_console_get_canvas(X_Console* console)
// {
//     return &x_console_get_screen(console)->canvas;
// }

// static const X_Palette* x_console_get_palette(X_Console* console)
// {
//     return x_console_get_screen(console)->palette;
// }

// static void x_console_handle_cursor_blinking(X_Console* console)
// {
//     X_Time currentTime = x_enginecontext_get_time(console->engineContext);
    
//     if(currentTime - console->lastCursorBlink >= X_CONSOLE_CURSOR_BLINK_TIME)
//     {
//         console->showCursor = !console->showCursor;
//         console->lastCursorBlink = currentTime;
//     }
// }

// static void x_console_handle_opening_animation(X_Console* console)
// {
//     int timePassed = x_enginecontext_get_time(console->engineContext) - console->consoleToggleTime;
//     int consoleHeight = x_console_h(console);
    
//     console->renderYOffset = -consoleHeight * (X_CONSOLE_TOGGLE_TIME - timePassed) / X_CONSOLE_TOGGLE_TIME;
    
//     if(console->renderYOffset >= 0)
//     {
//         console->renderYOffset = 0;
//         console->openState = X_CONSOLE_STATE_OPEN;
//     }
// }

// static void x_console_handle_closing_animation(X_Console* console)
// {
//     int timePassed = x_enginecontext_get_time(console->engineContext) - console->consoleToggleTime;
//     int consoleHeight = x_console_h(console);
    
//     console->renderYOffset = -consoleHeight * timePassed / X_CONSOLE_TOGGLE_TIME;
    
//     if(console->renderYOffset <= -consoleHeight)
//         console->openState = X_CONSOLE_STATE_CLOSED;
// }

// static void x_console_handle_open_state_animation(X_Console* console)
// {
//     if(console->openState == X_CONSOLE_STATE_OPENING)
//         x_console_handle_opening_animation(console);
//     else if(console->openState == X_CONSOLE_STATE_CLOSING)
//         x_console_handle_closing_animation(console);
// }

void ConsoleRenderer::renderOutput()
{
    for(int i = 0; i < console.size.y; ++i)
    {
        screen.canvas.drawStr(
            console.getLine(i),
            font,
            { 0, getLineY(i) });
    }
}

int ConsoleRenderer::getLineY(int lineNumber)
{
    return lineNumber * console.font->getH() + renderYOffset;
}

void ConsoleRenderer::renderBackground()
{
    const X_Palette* palette = screen.palette;

    X_Color backgroundColor = palette->black;
    X_Color lineColor = palette->darkRed;
    
    X_Vec2 topLeft =
    {
        0,
        getLineY(0)
    };

    X_Vec2 bottomRight =
    {
        x_screen_w(&screen) - 1,
        getLineY(console.size.y)
    };
    
    screen.canvas.fillRect(topLeft, bottomRight, backgroundColor);
    
    X_Vec2 bottomLeft = x_vec2_make(0, bottomRight.y);
    screen.canvas.drawLine(bottomLeft, bottomRight, lineColor);
}

static void x_console_add_cursor_to_input_buf(Console* console)
{
    const unsigned char CURSOR_CHAR = 11;
    console->input[console->inputPos] = CURSOR_CHAR;
    console->input[console->inputPos + 1] = '\0';
}

char* ConsoleRenderer::getStartOfScrolledInput()
{
    char* input = console.input;
    int inputLenghtIncludingCursor = console.inputPos + 3; 
    
    if(inputLenghtIncludingCursor >= console.size.x)
    {
        int charsToScrollHorizontallyBy = inputLenghtIncludingCursor - console.size.x;
        input += charsToScrollHorizontallyBy;
    }
    
    return input;
}

int ConsoleRenderer::getNextEmptyLine()
{
    return console.cursor.y + (console.cursor.x == 0 ? 0 : 1);
}

void ConsoleRenderer::renderInput()
{
    //x_console_handle_cursor_blinking(console);
    
    if(showCursor)
    {
        //x_console_add_cursor_to_input_buf(console);
    }
        
    char* scrolledInput = getStartOfScrolledInput();
    int nextEmptyLine = getNextEmptyLine();
    int inputLineY = getLineY(nextEmptyLine);
    const int CHARS_IN_CURSOR = 2;

    //canvas->drawChar(']', *console->font, { 0, inputLineY });
    //canvas->drawStr(scrolledInput, *console->font, { console->font->getW() * CHARS_IN_CURSOR, inputLineY });
    
    //x_console_remove_cursor_from_input_buf(console);
}

void ConsoleRenderer::render()
{
    // x_console_handle_open_state_animation(console);
    
    // if(!isVisible())
    // {
    //     return;
    // }
    
    renderBackground();
    renderOutput();
    // x_console_render_input(console);
}

