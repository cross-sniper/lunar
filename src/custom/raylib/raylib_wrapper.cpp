#include <raylib.h>
#include <vector>
struct IMG
{
    const char* name;
    Image imageSource;
};
struct TECX
{
    const char* name;
    Texture2D textureSource;
};
static void luna_pushColor(luna_State *L, Color color, auto *name) {
    luna_newtable(L);

    luna_pushstring(L, "r");
    luna_pushinteger(L, color.r);
    luna_settable(L, -3);

    luna_pushstring(L, "g");
    luna_pushinteger(L, color.g);
    luna_settable(L, -3);

    luna_pushstring(L, "b");
    luna_pushinteger(L, color.b);
    luna_settable(L, -3);

    luna_pushstring(L, "a");
    luna_pushinteger(L, color.a);
    luna_settable(L, -3);

    luna_setfield(L, -2, name);
}


std::vector<IMG> LoadedImages;
std::vector<TECX> LoadedTextures;

static int luna_is_key_pressed(luna_State *L){
    int key = lunaL_checkinteger(L,1);
    luna_pushboolean(L,IsKeyPressed(key));
    return 1;
}

static int luna_load_image(luna_State *L){
    const char *name = lunaL_checkstring(L,1);
    const char *path = lunaL_checkstring(L,2);
    Image img = LoadImage(path);
    LoadedImages.push_back({name,img});
    luna_pushstring(L,name);
    return 1;
}
static int luna_load_texture_from_image(luna_State *L){
    const char *name = lunaL_checkstring(L,1);
    //find the image in the vector
    for (int i = 0; i < LoadedImages.size(); i++)
    {
        if (LoadedImages[i].name == name)
        {
            Texture2D texture = LoadTextureFromImage(LoadedImages[i].imageSource);
            LoadedTextures.push_back((TECX){name,texture});
            break;
        }
    }
    luna_pushstring(L,name);
    return 1;
}

static int Luna_draw_texure(luna_State *L) {
    Texture2D texture;
    const char *name = lunaL_checkstring(L, 1);
    const int x = lunaL_checkinteger(L, 2);
    const int y = lunaL_checkinteger(L, 3);
    //find the texture, and draw it
    for (int i = 0; i < LoadedTextures.size(); i++)
    {
        if (LoadedTextures[i].name == name)
        {
            texture = LoadedTextures[i].textureSource;
            break;
        }
    }

    DrawTexture(texture, x, y, WHITE);
    return 0;
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
    int TargetFpsToMatch = lunaL_optinteger(L,1, 60); // defaults to 60 fps, if no value given
    SetTargetFPS(TargetFpsToMatch);
    return 0;
}

static Color luna_getColor(luna_State *L, int StartOfTable) {
    Color color = {0};  // Default to black in case of failure

    // Check if the provided argument is a table
    if (!luna_istable(L, StartOfTable)) {
        lunaL_error(L, "Argument is not a table");
        return color;
    }

    // Iterate through the table
    luna_pushnil(L);  // Push a nil key to start the iteration

    while (luna_next(L, StartOfTable) != 0) {
        // Key is at index -2 and value is at index -1

        // Assuming you have 'r', 'g', 'b', and 'a' keys in the table
        const char *key = luna_tostring(L, -2);

        if (key) {
            if (strcmp(key, "r") == 0) {
                color.r = luna_tointeger(L, -1);
            } else if (strcmp(key, "g") == 0) {
                color.g = luna_tointeger(L, -1);
            } else if (strcmp(key, "b") == 0) {
                color.b = luna_tointeger(L, -1);
            } else if (strcmp(key, "a") == 0) {
                color.a = luna_tointeger(L, -1);
            }
        }

        // Pop the value, leaving the key for the next iteration
        luna_pop(L, 1);
    }

    return color;
}

static int Luna_draw_rectangle(luna_State *L) {
    int posX   = lunaL_checkinteger(L, 1);
    int posY   = lunaL_checkinteger(L, 2);
    int width  = lunaL_checkinteger(L, 3);
    int height = lunaL_checkinteger(L, 4);

    Color color;
    color = luna_getColor(L,5);


    DrawRectangle(posX, posY, width, height, color);
    return 0;
}


// Wrapper function to create a window
static int Luna_create_window(luna_State *L) {
    int width = lunaL_checkinteger(L, 1);
    int height = lunaL_checkinteger(L, 2);
    const char *title = lunaL_checkstring(L, 3);

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
    color = luna_getColor(L, 1);

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
    color = luna_getColor(L, 5);


    DrawText(text, posX, posY, fontSize, color);

    return 0;
}
// Helper function to add Raylib key constants to the luna table
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
#include "raylib_init_audio.cpp"



// Function to create a new color from a luna/Luna table
static int luna_create_color(luna_State *L) {
    // Check if the argument is a table
    if (!luna_istable(L, 1)) {
        return lunaL_error(L, "Argument is not a table");
    }
    const char *name = lunaL_checkstring(L, 2);

    // Default color
    Color color = luna_getColor(L,1);
    printf("%i\n",color.r);
    printf("%i\n",color.g);
    printf("%i\n",color.b);
    printf("%i\n",color.a);
    printf("%s\n", name);

    // Push the color onto the luna stack with a specified name
    luna_pushColor(L, color, name);

    return 0;  // Number of return values
}

static void luna_init_colors(luna_State *L){
    luna_pushColor(L, RED, "RED");
    luna_pushColor(L, GREEN, "GREEN");
    luna_pushColor(L, BLUE, "BLUE");
    luna_pushColor(L, WHITE, "WHITE");
    luna_pushColor(L, LIGHTGRAY, "LIGHTGRAY");
    luna_pushColor(L, BLACK, "BLACK");
}

static int init_raylib(luna_State *L) {
    // Create a new table
    luna_newtable(L);

    // Add functions to the table using addRaylibFunction
    addRaylibFunction(L, Luna_create_window, "InitWindow");
    addRaylibFunction(L, Luna_draw_text, "DrawText");
    addRaylibFunction(L, luna_create_color, "NewColor");
    addRaylibFunction(L, Luna_close_window, "CloseWindow");
    addRaylibFunction(L, Luna_draw_rectangle, "DrawRectangle");
    addRaylibFunction(L, Luna_fill_bg, "ClearBackground");
    addRaylibFunction(L, luna_is_key_pressed, "IsKeyPressed");
    addRaylibFunction(L, luna_load_image, "LoadImage");
    addRaylibFunction(L, Luna_draw_texure, "DrawTexture");
    addRaylibFunction(L, luna_load_texture_from_image, "LoadTextureFromImage");
    addRaylibFunction(L, Luna_start_drawing, "BeginDrawing");
    addRaylibFunction(L, Luna_stop_drawing, "EndDrawing");
    addRaylibFunction(L, luna_is_key_down, "IsKeyDown");
    addRaylibFunction(L, luna_set_target_fps, "SetTargetFPS");
    addRaylibFunction(L, Luna_should_close_window, "WindowShouldClose");
    addRaylibFunction(L, luna_draw_fps, "DrawFPS");
    addRaylibFunction(L, luna_get_fps, "GetFPS");
    addRaylibFunction(L, luna_get_frame_time, "GetFrameTime");
    addRaylibFunction(L, Luna_get_mouse_position, "GetMousePosition");

    init_raylib_keys(L);
    luna_init_colors(L);
    raylib_init_audio(L);

    return 1;  // Return the table
}
