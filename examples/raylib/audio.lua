raylib = raylib_init()
raylib.InitWindow(800, 450, "raylib")
raylib.SetTargetFPS(60)
raylib.LoadAudio("Amb-Coin","../Amb-Coin.ogg")

target = {
	x=10,
	y=40,
	width=10,
	height=10
}
raylib.PlayAudio("Amb-Coin")
while not raylib.WindowShouldClose() do
	-- simple game, that plays the coin sound when the user gets a point
	raylib.BeginDrawing()
	raylib.ClearBackground(raylib.BLACK)
	raylib.DrawRectangle(target.x, target.y, target.width, target.height, raylib.LIGHTGRAY)
	raylib.EndDrawing()
end

raylib.CloseWindow()