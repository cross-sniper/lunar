-- Initialize raylib's functions, and set a variable to be the master for it
local raylib <const> = raylib_init()

-- Defines the size of the window
local width <const> = 800
local height <const> = 600

-- Creates the window
raylib.createWindow("Raylib Lunar Example", width, height)

-- set the fps cap to 60(Optional)
raylib.SetTargetFPS(60)


-- Main game loop
while not raylib.windowShouldClose() do
    raylib.clearBackground()
end

-- Close the window when the loop ends
raylib.closeWindow()
