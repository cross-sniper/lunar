-- Ask luna to define and return the raylib functions(translation layer)
raylib = raylib_init()

-- Create a window
local width <const> = 800
local height <const> = 600

raylib.createWindow("Raylib Lunar Example", width, height)

raylib.SetTargetFPS(60)

-- Main game loop
while not raylib.windowShouldClose() do
    raylib.clearBackground()

    raylib.DrawFPS(30,30)

    mouse = raylib.GetMousePosition()
    raylib.DrawText(mouse.x .. "," .. mouse.y, mouse.x, mouse.y,20, 150,150,150)


    -- End drawing and swap buffers
    raylib.endDrawing()
end

-- Close the window when the loop ends
raylib.closeWindow()
