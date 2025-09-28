-- Simple Image Test
-- Just load and display an image without complex interactions

function tsuki.load()
    local success = tsuki.graphics.loadImage("player", "player.png")
    if success then
        print("✓ Successfully loaded player.png")
    else
        print("✗ Failed to load player.png")
    end
end

function tsuki.update(dt)
    -- Empty update function to avoid errors
end

function tsuki.draw()
    -- Clear screen
    tsuki.graphics.clear(0.1, 0.1, 0.3, 1)

    -- Set white color for image
    tsuki.graphics.setColor(1, 1, 1, 1)

    -- Draw text
    tsuki.graphics.print("Simple Image Test", 10, 10)

    -- Try to draw the image at a fixed position
    tsuki.graphics.draw("player", 100, 100)

    -- Draw some reference text
    tsuki.graphics.print("Image should appear at (100, 100)", 10, 30)
end