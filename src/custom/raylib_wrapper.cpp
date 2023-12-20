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
//RLAPI Vector2 GetMousePosition(void);
static int Luna_get_mouse_position(luna_State *L)
{
    luna_newtable(L);
    Vector2 mouse_pos = GetMousePosition();
    luna_pushinteger(L, mouse_pos.x);
    luna_setfield(L, -2, "x");
    luna_pushinteger(L, mouse_pos.y);
    luna_setfield(L, -2, "y");
    return 1;
}

//this is just so this file dosent become unredable
#include "raylib_init_helper.cpp"

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
    addRaylibFunction(L, Luna_get_mouse_position, "GetMousePosition");

    init_raylib_keys(L);

    return 1;  // Return the table
}
