raylib = raylib_init()
raylib.InitWindow(800, 450, "raylib")
raylib.SetTargetFPS(60)
img = raylib.LoadImage("allow","../allow.png")
texture = raylib.LoadTextureFromImage(img)
while not raylib.WindowShouldClose() do
	raylib.BeginDrawing()
	raylib.ClearBackground(raylib.RAYWHITE)
	raylib.DrawText("raylib is awesome!", 190, 200, 20, raylib.LIGHTGRAY)
	raylib.DrawTexture(texture, 10, 10)
	raylib.EndDrawing()
end