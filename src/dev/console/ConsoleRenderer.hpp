#pragma once

#include "system/Clock.hpp"

struct Console;
struct Screen;
struct Font;

typedef enum X_ConsoleOpenState
{
    X_CONSOLE_STATE_OPEN,
    X_CONSOLE_STATE_CLOSED,
    X_CONSOLE_STATE_OPENING,
    X_CONSOLE_STATE_CLOSING
} X_ConsoleOpenState;

class ConsoleRenderer
{
public:
    ConsoleRenderer(Console& console_, Screen& screen_, Font& font_)
        : console(console_),
        screen(screen_),
        font(font_)
    {
        openState = X_CONSOLE_STATE_CLOSED;
        renderYOffset = 0;
        lastCursorBlink = Clock::getTicks();
        showCursor = true;
    }
    
    bool isVisible() const
    {
        return openState != X_CONSOLE_STATE_CLOSED;
    }
    
    void render();
    void show();
    void hide();

private:
    
    int getConsoleHeight();
    
    int getLineY(int lineNumber);
    char* getStartOfScrolledInput();
    int getNextEmptyLine();
    
    void renderBackground();
    void renderOutput();
    void renderInput();
    
    void handleCursorBlinking();
    void addCursorToInputBuf();
    void removeCursorFromInputBuf();
    
    void handleStateAnimations();
    void handleOpeningAnimation();
    void handleClosingAnimation();
    
    Console& console;
    Screen& screen;

    bool showCursor;
    Time lastCursorBlink;
    
    X_ConsoleOpenState openState;
    int renderYOffset;
    Time consoleToggleTime;

    Font& font;
};
