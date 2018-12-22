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
        lastCursorBlink = 0; //FIXME: x_enginecontext_get_time(engineContext);
        showCursor = true;
    }



    void render();

private:
    bool isVisible()
    {
        return openState != X_CONSOLE_STATE_CLOSED;
    }

    Console& console;
    X_Screen& screen;

    bool showCursor;
    X_Time lastCursorBlink;
    
    X_ConsoleOpenState openState;
    int renderYOffset;
    X_Time consoleToggleTime;

    X_Font& font;

    int getLineY(int lineNumber);
    char* getStartOfScrolledInput();
    int getNextEmptyLine();

    void renderBackground();
    void renderOutput();
    void renderInput();
};