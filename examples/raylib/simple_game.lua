-- Ask luna to define and return the raylib functions(translation layer)
raylib = raylib_init()

-- Create a window
local width <const> = 800
local height <const> = 600

raylib.createWindow("Raylib Lunar Example", width, height)

raylib.SetTargetFPS(60)
x = 40
y = 40


-- Main game loop
while not raylib.windowShouldClose() do

    raylib.DrawFPS(30,30)
    raylib.DrawText("this is a example, of what you can do, text :D", 30,60,20, 150,150,150)

    -- Begin drawing
    raylib.beginDrawing()

    -- Clear the background with a custom color (optional, default is black)
    raylib.clearBackground()

    if raylib.IsKeyDown(raylib.KEY_A) then
        x = x - 5
    end

    if raylib.IsKeyDown(raylib.KEY_D) then
        x = x + 5
    end
    if raylib.IsKeyDown(raylib.KEY_S) then
        y = y + 5
    end
    if raylib.IsKeyDown(raylib.KEY_W) then
        y = y - 5
    end

    -- Draw a rectangle with a custom color (optional, default is white)
    raylib.DrawRectangle(x, y, 20, 20, 255, 69, 0, 255)


    -- End drawing and swap buffers
    raylib.endDrawing()
end

-- Close the window when the loop ends
raylib.closeWindow()
