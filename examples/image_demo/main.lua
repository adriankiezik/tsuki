-- Image Demo: Simple demonstration of image loading, rotation, and scaling

local rotation = 0

function tsuki.load()
    -- Load the player image
    local success = graphics.loadImage("player", "assets/player.png")
    if success then
        print("✓ Successfully loaded player.png")
    else
        print("✗ Failed to load player.png")
    end

    window.setTitle("Image Demo - Loading, Rotation & Scale")
end


function tsuki.update(dt)
    -- Continuously rotate the image
    rotation = rotation + dt * 90  -- 90 degrees per second
end

function tsuki.draw()
    -- Clear with dark background
    graphics.clear(0.1, 0.1, 0.3, 1)

    -- Set white color for proper image display
    graphics.setColor(1, 1, 1, 1)

    graphics.circle("Fill", 50, 50, 30)

    -- Title
    graphics.print("Image Loading, Rotation & Scale Demo", 10, 10)

    -- Draw examples showing different image transformations
    local w, h = window.getWidth(), window.getHeight()

    -- 1. Normal image
    graphics.draw("player", 100, 100)
    graphics.print("Normal", 100, 170)

    -- 2. Scaled 2x
    graphics.draw("player", 250, 100, 0, 2, 2)
    graphics.print("2x Scale", 250, 230)

    -- 3. Scaled 0.5x
    graphics.draw("player", 450, 100, 0, 0.5, 0.5)
    graphics.print("0.5x Scale", 450, 140)

    -- 4. Rotating image (center rotation)
    local center_x, center_y = w/2, h - 150
    graphics.draw("player", center_x, center_y, math.rad(rotation), 1, 1, 32, 32)
    graphics.print("Rotating (auto)", center_x - 50, center_y + 50)

    -- 5. Different scales with rotation
    graphics.draw("player", 100, h - 150, math.rad(rotation), 1.5, 0.8, 32, 32)
    graphics.print("Scale + Rotation", 100, h - 80)

    -- Show current rotation angle
    graphics.print("Rotation: " .. math.floor(rotation % 360) .. "°", 10, h - 30)
end