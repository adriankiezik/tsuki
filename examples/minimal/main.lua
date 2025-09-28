-- Working Tsuki Example Game
-- This demonstrates the Lua API for Tsuki

-- Game state
local player = {
    x = 400,
    y = 300,
    size = 50,
    speed = 200,
    color = {1, 0, 0, 1} -- Red
}

local time = 0
local frame_count = 0

-- tsuki.load() is called once at the start
function tsuki.load()
    print("Tsuki Working Example Loaded!")
    tsuki.window.setTitle("Tsuki Working Example")
end

-- tsuki.update(dt) is called every frame
function tsuki.update(dt)
    time = time + dt
    frame_count = frame_count + 1

    -- Simple input handling (without the complex isDown calls for now)
    -- Keep player centered for this demo
    player.x = 400 + math.sin(time) * 100
    player.y = 300 + math.cos(time * 0.7) * 50

    -- Limit frame rate by only doing expensive operations every 60 frames
    if frame_count % 60 == 0 then
        print("Frame:", frame_count, "Time:", string.format("%.2f", time))
    end
end

-- tsuki.draw() is called every frame for rendering
function tsuki.draw()
    -- Clear screen with dark blue
    tsuki.graphics.clear(0.1, 0.1, 0.3, 1.0)

    -- Draw a simple animated background
    tsuki.graphics.setColor(0.2, 0.2, 0.4, 0.5)

    -- Draw the animated player
    tsuki.graphics.setColor(player.color[1], player.color[2], player.color[3], player.color[4])
    tsuki.graphics.rectangle("fill", player.x, player.y, player.size, player.size)

    -- Draw player border
    tsuki.graphics.setColor(1, 1, 1, 1) -- White
    tsuki.graphics.rectangle("line", player.x, player.y, player.size, player.size)
end