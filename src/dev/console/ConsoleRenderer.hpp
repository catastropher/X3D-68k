#pragma once

struct Console;
struct X_Screen;
struct X_Font;

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
    ConsoleRenderer(Console& console_, X_Screen& screen_, X_Font& font_)
        : console(console_),
        screen(screen_),
        font(font_)
    {
        openState = X_CONSOLE_STATE_CLOSED;
        renderYOffset = 0;
        lastCursorBlink = getCurrentTime();
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
    
    X_Time getCurrentTime();

    Console& console;
    X_Screen& screen;

    bool showCursor;
    X_Time lastCursorBlink;
    
    X_ConsoleOpenState openState;
    int renderYOffset;
    X_Time consoleToggleTime;

    X_Font& font;
};
