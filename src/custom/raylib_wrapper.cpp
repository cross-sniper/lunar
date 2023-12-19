#include <raylib.h>

/* add stuff to be implemented, then when they are implemented, remove them from here.

 * TODO:
  - implement luna/lua support for the following functions:

    the image functions

    some of the 3d stuff
RLAPI void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
RLAPI void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)
RLAPI void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint); // Draw one character (codepoint)
RLAPI void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint); // Draw multiple character (codepoint)


// Basic geometric 3D shapes drawing functions
RLAPI void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);                                    // Draw a line in 3D world space
RLAPI void DrawPoint3D(Vector3 position, Color color);                                                   // Draw a point in 3D space, actually a small line
RLAPI void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color); // Draw a circle in 3D world space
RLAPI void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color);                              // Draw a color-filled triangle (vertex in counter-clockwise order!)
RLAPI void DrawTriangleStrip3D(Vector3 *points, int pointCount, Color color);                            // Draw a triangle strip defined by points

RLAPI void DrawCubeV(Vector3 position, Vector3 size, Color color);                                       // Draw cube (Vector version)
RLAPI void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);        // Draw cube wires
RLAPI void DrawCubeWiresV(Vector3 position, Vector3 size, Color color);                                  // Draw cube wires (Vector version)
RLAPI void DrawSphere(Vector3 centerPos, float radius, Color color);                                     // Draw sphere
RLAPI void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);            // Draw sphere with extended parameters
RLAPI void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);         // Draw sphere wires
RLAPI void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone
RLAPI void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color); // Draw a cylinder with base at startPos and top at endPos
RLAPI void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone wires
RLAPI void DrawCylinderWiresEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color); // Draw a cylinder wires with base at startPos and top at endPos
RLAPI void DrawCapsule(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color); // Draw a capsule with the center of its sphere caps at startPos and endPos
RLAPI void DrawCapsuleWires(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color); // Draw capsule wireframe with the center of its sphere caps at startPos and endPos
RLAPI void DrawPlane(Vector3 centerPos, Vector2 size, Color color);                                      // Draw a plane XZ
RLAPI void DrawRay(Ray ray, Color color);                                                                // Draw a ray line
RLAPI void DrawGrid(int slices, float spacing);                                                          // Draw a grid (centered at (0, 0, 0))

*/
//every luna_ when refering to raylib, is just so we dont conflict with raylib

//DrawCube(Vector3 position, float width, float height, float length, Color color);
static int Luna_draw_cube_3d(luna_State *L){

    return 0;
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

    luna_pushcfunction(L, Luna_start_drawing);
    luna_setfield(L, -2, "beginDrawing");

    luna_pushcfunction(L, Luna_stop_drawing);
    luna_setfield(L, -2, "endDrawing");

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
