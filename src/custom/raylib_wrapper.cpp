#include <raylib.h>
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
// Helper function to add Raylib key constants to the Lua table
static int addRaylibKeyConstant(luna_State *L, const char *keyName, int keyValue) {
    luna_pushinteger(L, keyValue);
    luna_setfield(L, -2, keyName);
    return 1;
}
static int addRaylibFunction(luna_State *L, int (*function)(luna_State *), const char *name) {
    luna_pushcfunction(L, function);
    luna_setfield(L, -2, name);
    return 1;
}

static int init_raylib_keys(luna_State *L)
{
    // Alphanumeric keys
    addRaylibKeyConstant(L, "KEY_APOSTROPHE", KEY_APOSTROPHE);
    addRaylibKeyConstant(L, "KEY_COMMA", KEY_COMMA);
    addRaylibKeyConstant(L, "KEY_MINUS", KEY_MINUS);
    addRaylibKeyConstant(L, "KEY_PERIOD", KEY_PERIOD);
    addRaylibKeyConstant(L, "KEY_SLASH", KEY_SLASH);
    addRaylibKeyConstant(L, "KEY_ZERO", KEY_ZERO);
    addRaylibKeyConstant(L, "KEY_ONE", KEY_ONE);
    addRaylibKeyConstant(L, "KEY_TWO", KEY_TWO);
    addRaylibKeyConstant(L, "KEY_THREE", KEY_THREE);
    addRaylibKeyConstant(L, "KEY_FOUR", KEY_FOUR);
    addRaylibKeyConstant(L, "KEY_FIVE", KEY_FIVE);
    addRaylibKeyConstant(L, "KEY_SIX", KEY_SIX);
    addRaylibKeyConstant(L, "KEY_SEVEN", KEY_SEVEN);
    addRaylibKeyConstant(L, "KEY_EIGHT", KEY_EIGHT);
    addRaylibKeyConstant(L, "KEY_NINE", KEY_NINE);
    addRaylibKeyConstant(L, "KEY_SEMICOLON", KEY_SEMICOLON);
    addRaylibKeyConstant(L, "KEY_EQUAL", KEY_EQUAL);
    addRaylibKeyConstant(L, "KEY_A", KEY_A);
    addRaylibKeyConstant(L, "KEY_B", KEY_B);
    addRaylibKeyConstant(L, "KEY_C", KEY_C);
    addRaylibKeyConstant(L, "KEY_D", KEY_D);
    addRaylibKeyConstant(L, "KEY_E", KEY_E);
    addRaylibKeyConstant(L, "KEY_F", KEY_F);
    addRaylibKeyConstant(L, "KEY_G", KEY_G);
    addRaylibKeyConstant(L, "KEY_H", KEY_H);
    addRaylibKeyConstant(L, "KEY_I", KEY_I);
    addRaylibKeyConstant(L, "KEY_J", KEY_J);
    addRaylibKeyConstant(L, "KEY_K", KEY_K);
    addRaylibKeyConstant(L, "KEY_L", KEY_L);
    addRaylibKeyConstant(L, "KEY_M", KEY_M);
    addRaylibKeyConstant(L, "KEY_N", KEY_N);
    addRaylibKeyConstant(L, "KEY_O", KEY_O);
    addRaylibKeyConstant(L, "KEY_P", KEY_P);
    addRaylibKeyConstant(L, "KEY_Q", KEY_Q);
    addRaylibKeyConstant(L, "KEY_R", KEY_R);
    addRaylibKeyConstant(L, "KEY_S", KEY_S);
    addRaylibKeyConstant(L, "KEY_T", KEY_T);
    addRaylibKeyConstant(L, "KEY_U", KEY_U);
    addRaylibKeyConstant(L, "KEY_V", KEY_V);
    addRaylibKeyConstant(L, "KEY_W", KEY_W);
    addRaylibKeyConstant(L, "KEY_X", KEY_X);
    addRaylibKeyConstant(L, "KEY_Y", KEY_Y);
    addRaylibKeyConstant(L, "KEY_Z", KEY_Z);
    addRaylibKeyConstant(L, "KEY_LEFT_BRACKET", KEY_LEFT_BRACKET);
    addRaylibKeyConstant(L, "KEY_BACKSLASH", KEY_BACKSLASH);
    addRaylibKeyConstant(L, "KEY_RIGHT_BRACKET", KEY_RIGHT_BRACKET);
    addRaylibKeyConstant(L, "KEY_GRAVE", KEY_GRAVE);

    // Function keys
    addRaylibKeyConstant(L, "KEY_SPACE", KEY_SPACE);
    addRaylibKeyConstant(L, "KEY_ESCAPE", KEY_ESCAPE);
    addRaylibKeyConstant(L, "KEY_ENTER", KEY_ENTER);
    addRaylibKeyConstant(L, "KEY_TAB", KEY_TAB);
    addRaylibKeyConstant(L, "KEY_BACKSPACE", KEY_BACKSPACE);
    addRaylibKeyConstant(L, "KEY_INSERT", KEY_INSERT);
    addRaylibKeyConstant(L, "KEY_DELETE", KEY_DELETE);
    addRaylibKeyConstant(L, "KEY_RIGHT", KEY_RIGHT);
    addRaylibKeyConstant(L, "KEY_LEFT", KEY_LEFT);
    addRaylibKeyConstant(L, "KEY_DOWN", KEY_DOWN);
    addRaylibKeyConstant(L, "KEY_UP", KEY_UP);
    addRaylibKeyConstant(L, "KEY_PAGE_UP", KEY_PAGE_UP);
    addRaylibKeyConstant(L, "KEY_PAGE_DOWN", KEY_PAGE_DOWN);
    addRaylibKeyConstant(L, "KEY_HOME", KEY_HOME);
    addRaylibKeyConstant(L, "KEY_END", KEY_END);
    addRaylibKeyConstant(L, "KEY_CAPS_LOCK", KEY_CAPS_LOCK);
    addRaylibKeyConstant(L, "KEY_SCROLL_LOCK", KEY_SCROLL_LOCK);
    addRaylibKeyConstant(L, "KEY_NUM_LOCK", KEY_NUM_LOCK);
    addRaylibKeyConstant(L, "KEY_PRINT_SCREEN", KEY_PRINT_SCREEN);
    addRaylibKeyConstant(L, "KEY_PAUSE", KEY_PAUSE);
    addRaylibKeyConstant(L, "KEY_F1", KEY_F1);
    addRaylibKeyConstant(L, "KEY_F2", KEY_F2);
    addRaylibKeyConstant(L, "KEY_F3", KEY_F3);
    addRaylibKeyConstant(L, "KEY_F4", KEY_F4);
    addRaylibKeyConstant(L, "KEY_F5", KEY_F5);
    addRaylibKeyConstant(L, "KEY_F6", KEY_F6);
    addRaylibKeyConstant(L, "KEY_F7", KEY_F7);
    addRaylibKeyConstant(L, "KEY_F8", KEY_F8);
    addRaylibKeyConstant(L, "KEY_F9", KEY_F9);
    addRaylibKeyConstant(L, "KEY_F10", KEY_F10);
    addRaylibKeyConstant(L, "KEY_F11", KEY_F11);
    addRaylibKeyConstant(L, "KEY_F12", KEY_F12);
    addRaylibKeyConstant(L, "KEY_LEFT_SHIFT", KEY_LEFT_SHIFT);
    addRaylibKeyConstant(L, "KEY_LEFT_CONTROL", KEY_LEFT_CONTROL);
    addRaylibKeyConstant(L, "KEY_LEFT_ALT", KEY_LEFT_ALT);
    addRaylibKeyConstant(L, "KEY_LEFT_SUPER", KEY_LEFT_SUPER);
    addRaylibKeyConstant(L, "KEY_RIGHT_SHIFT", KEY_RIGHT_SHIFT);
    addRaylibKeyConstant(L, "KEY_RIGHT_CONTROL", KEY_RIGHT_CONTROL);
    addRaylibKeyConstant(L, "KEY_RIGHT_ALT", KEY_RIGHT_ALT);
    addRaylibKeyConstant(L, "KEY_RIGHT_SUPER", KEY_RIGHT_SUPER);
    addRaylibKeyConstant(L, "KEY_KB_MENU", KEY_KB_MENU);

    // Keypad keys
    addRaylibKeyConstant(L, "KEY_KP_0", KEY_KP_0);
    addRaylibKeyConstant(L, "KEY_KP_1", KEY_KP_1);
    addRaylibKeyConstant(L, "KEY_KP_2", KEY_KP_2);
    addRaylibKeyConstant(L, "KEY_KP_3", KEY_KP_3);
    addRaylibKeyConstant(L, "KEY_KP_4", KEY_KP_4);
    addRaylibKeyConstant(L, "KEY_KP_5", KEY_KP_5);
    addRaylibKeyConstant(L, "KEY_KP_6", KEY_KP_6);
    addRaylibKeyConstant(L, "KEY_KP_7", KEY_KP_7);
    addRaylibKeyConstant(L, "KEY_KP_8", KEY_KP_8);
    addRaylibKeyConstant(L, "KEY_KP_9", KEY_KP_9);
    addRaylibKeyConstant(L, "KEY_KP_DECIMAL", KEY_KP_DECIMAL);
    addRaylibKeyConstant(L, "KEY_KP_DIVIDE", KEY_KP_DIVIDE);
    addRaylibKeyConstant(L, "KEY_KP_MULTIPLY", KEY_KP_MULTIPLY);
    addRaylibKeyConstant(L, "KEY_KP_SUBTRACT", KEY_KP_SUBTRACT);
    addRaylibKeyConstant(L, "KEY_KP_ADD", KEY_KP_ADD);
    addRaylibKeyConstant(L, "KEY_KP_ENTER", KEY_KP_ENTER);
    addRaylibKeyConstant(L, "KEY_KP_EQUAL", KEY_KP_EQUAL);

    // Android key buttons
    addRaylibKeyConstant(L, "KEY_BACK", KEY_BACK);
    addRaylibKeyConstant(L, "KEY_MENU", KEY_MENU);
    addRaylibKeyConstant(L, "KEY_VOLUME_UP", KEY_VOLUME_UP);
    addRaylibKeyConstant(L, "KEY_VOLUME_DOWN", KEY_VOLUME_DOWN);

    return 1;
}

static int init_raylib(luna_State *L) {
    // Create a new table
    luna_newtable(L);

    // Add functions to the table using addRaylibFunction
    addRaylibFunction(L, Luna_create_window, "createWindow");
    addRaylibFunction(L, Luna_draw_text, "DrawText");
    addRaylibFunction(L, Luna_close_window, "closeWindow");
    addRaylibFunction(L, Luna_draw_rectangle, "DrawRectangle");
    addRaylibFunction(L, Luna_fill_bg, "clearBackground");
    addRaylibFunction(L, luna_is_key_pressed, "IsKeyPressed");
    addRaylibFunction(L, Luna_start_drawing, "beginDrawing");
    addRaylibFunction(L, Luna_stop_drawing, "endDrawing");
    addRaylibFunction(L, luna_is_key_down, "IsKeyDown");
    addRaylibFunction(L, luna_set_target_fps, "SetTargetFPS");
    addRaylibFunction(L, Luna_should_close_window, "windowShouldClose");
    addRaylibFunction(L, luna_draw_fps, "DrawFPS");
    addRaylibFunction(L, luna_get_fps, "GetFPS");
    addRaylibFunction(L, luna_get_frame_time, "GetFrameTime");

    init_raylib_keys(L);

    return 1;  // Return the table
}
