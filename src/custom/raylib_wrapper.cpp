#include <raylib.h>

/* add stuff to be implemented, then when they are implemented, remove them from here.

 * TODO:
  - implement luna/lua support for the following functions:

    the image functions

    some of the 3d stuff
 text functions to add
RLAPI void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
RLAPI void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)
RLAPI void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint); // Draw one character (codepoint)
RLAPI void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint); // Draw multiple character (codepoint)



// Input-related functions: mouse
RLAPI bool IsMouseButtonPressed(int button);                  // Check if a mouse button has been pressed once
RLAPI bool IsMouseButtonDown(int button);                     // Check if a mouse button is being pressed
RLAPI bool IsMouseButtonReleased(int button);                 // Check if a mouse button has been released once
RLAPI bool IsMouseButtonUp(int button);                       // Check if a mouse button is NOT being pressed
RLAPI int GetMouseX(void);                                    // Get mouse position X
RLAPI int GetMouseY(void);                                    // Get mouse position Y
RLAPI Vector2 GetMousePosition(void);                         // Get mouse position XY
RLAPI Vector2 GetMouseDelta(void);                            // Get mouse delta between frames
RLAPI void SetMousePosition(int x, int y);                    // Set mouse position XY
RLAPI void SetMouseOffset(int offsetX, int offsetY);          // Set mouse offset
RLAPI void SetMouseScale(float scaleX, float scaleY);         // Set mouse scaling
RLAPI float GetMouseWheelMove(void);                          // Get mouse wheel movement for X or Y, whichever is larger
RLAPI Vector2 GetMouseWheelMoveV(void);                       // Get mouse wheel movement for both X and Y
RLAPI void SetMouseCursor(int cursor);                        // Set mouse cursor


// Input-related functions: keyboard
RLAPI bool IsKeyPressed(int key);                             // Check if a key has been pressed once
RLAPI bool IsKeyPressedRepeat(int key);                       // Check if a key has been pressed again (Only PLATFORM_DESKTOP)
RLAPI bool IsKeyDown(int key);                                // Check if a key is being pressed
RLAPI bool IsKeyReleased(int key);                            // Check if a key has been released once
RLAPI bool IsKeyUp(int key);                                  // Check if a key is NOT being pressed
RLAPI int GetKeyPressed(void);                                // Get key pressed (keycode), call it multiple times for keys queued, returns 0 when the queue is empty
RLAPI int GetCharPressed(void);                               // Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty
RLAPI void SetExitKey(int key);                               // Set a custom key to exit program (default is ESC)


*/
//every luna_ when refering to raylib, is just so we dont conflict with raylib

static int luna_is_key_pressed(luna_State *L){
    int key = lunaL_checkinteger(L,1);
    luna_pushboolean(L,IsKeyPressed(key));
    return 1;
}

static int luna_is_key_down(luna_State *L)
{
    int key = lunaL_checkinteger(L,1);
    luna_pushboolean(L,IsKeyDown(key));
    return 1;
}
static int luna_draw_fps(luna_State *L){
    int x,y;
    x = lunaL_checkinteger(L,1);
    y = lunaL_checkinteger(L,2);
    DrawFPS(x,y);
    return 0;
}

static int luna_get_time(luna_State *L){
    luna_pushinteger(L,GetTime());
    return 1;
}

static int luna_get_frame_time(luna_State* L)
{
    luna_pushinteger(L, GetFrameTime());
    return 1;
}
static int luna_get_fps(luna_State *L)
{
    luna_pushinteger(L,GetFPS());
    return 1;
}

static int luna_set_target_fps(luna_State *L)
{
    int _ = lunaL_optinteger(L,1, 60); // defaults to 60 fps, if no value given
    SetTargetFPS(_);
    return 0;
}

static int Luna_draw_rectangle(luna_State *L) {
    int posX   = lunaL_checkinteger(L, 1);
    int posY   = lunaL_checkinteger(L, 2);
    int width  = lunaL_checkinteger(L, 3);
    int height = lunaL_checkinteger(L, 4);

    Color color;
    color.r = lunaL_optinteger(L, 5, 255);  // Default to 255 if not provided
    color.g = lunaL_optinteger(L, 6, 255);
    color.b = lunaL_optinteger(L, 7, 255);
    color.a = lunaL_optinteger(L, 8, 255);

    DrawRectangle(posX, posY, width, height, color);
    return 0;
}


// Wrapper function to create a window
static int Luna_create_window(luna_State *L) {
    const char *title = lunaL_checkstring(L, 1);
    int width = lunaL_checkinteger(L, 2);
    int height = lunaL_checkinteger(L, 3);

    InitWindow(width, height, title);

    return 0;  // No return values
}



// Wrapper function to close the window
static int Luna_close_window(luna_State *L) {
    CloseWindow();
    return 0;  // No return values
}

// Wrapper function to clear the background
static int Luna_fill_bg(luna_State *L) {
    Color color;
    color.r = lunaL_optinteger(L, 1, 0);  // Default to 0 if not provided
    color.g = lunaL_optinteger(L, 2, 0);
    color.b = lunaL_optinteger(L, 3, 0);
    color.a = lunaL_optinteger(L, 4, 255);

    ClearBackground(color);
    return 0;
}

// Wrapper function to begin drawing
static int Luna_start_drawing(luna_State *L) {
    BeginDrawing();
    return 0;
}

// Wrapper function to end drawing
static int Luna_stop_drawing(luna_State *L) {
    EndDrawing();
    return 0;
}

static int Luna_should_close_window(luna_State *L)
{
    luna_pushboolean(L,WindowShouldClose());
    return 1;
}

//RLAPI void DrawText(const char *text, int posX, int posY, int fontSize, Color color);

// Wrapper function to draw text
static int Luna_draw_text(luna_State *L) {
    const char *text = lunaL_checkstring(L, 1);
    int posX = lunaL_checkinteger(L, 2);
    int posY = lunaL_checkinteger(L, 3);
    int fontSize = lunaL_checkinteger(L, 4);

    Color color;
    color.r = lunaL_optinteger(L, 5, 255);  // Default to 255 if not provided
    color.g = lunaL_optinteger(L, 6, 255);
    color.b = lunaL_optinteger(L, 7, 255);
    color.a = lunaL_optinteger(L, 8, 255);

    DrawText(text, posX, posY, fontSize, color);

    return 0;
}


// luna module registration function
static int init_raylib(luna_State *L) {
    // Register the createRaylibTable function as the module's entry point

    // Create a new table
    luna_newtable(L);

    // Add functions to the table
    luna_pushcfunction(L, Luna_create_window);
    luna_setfield(L, -2, "createWindow");

    luna_pushcfunction(L, Luna_draw_text);
    luna_setfield(L, -2, "DrawText");

    luna_pushcfunction(L, Luna_close_window);
    luna_setfield(L, -2, "closeWindow");

    luna_pushcfunction(L,Luna_draw_rectangle);
    luna_setfield(L, -2, "DrawRectangle");

    luna_pushcfunction(L, Luna_fill_bg);
    luna_setfield(L, -2, "clearBackground");

    luna_pushcfunction(L,luna_is_key_pressed);
    luna_setfield(L, -2, "IsKeyPressed");

    luna_pushcfunction(L, Luna_start_drawing);
    luna_setfield(L, -2, "beginDrawing");

    luna_pushcfunction(L, Luna_stop_drawing);
    luna_setfield(L, -2, "endDrawing");

    luna_pushcfunction(L,luna_is_key_down);
    luna_setfield(L,-2, "IsKeyDown");

    luna_pushcfunction(L, luna_set_target_fps);
    luna_setfield(L, -2, "SetTargetFPS");

    luna_pushcfunction(L, Luna_should_close_window);
    luna_setfield(L, -2, "windowShouldClose");

    luna_pushcfunction(L, luna_draw_fps);
    luna_setfield(L, -2, "DrawFPS");

    luna_pushcfunction(L, luna_get_fps);
    luna_setfield(L, -2, "GetFPS");

    luna_pushcfunction(L, luna_get_frame_time);
    luna_setfield(L, -2, "GetFrameTime");

    return 1;  // Return the table
}
