-- Tsuki Game Example
-- This demonstrates the Lua API for Tsuki

-- Game state
local player = {
    x = 400,
    y = 300,
    size = 50,
    speed = 200,
    color = {1, 0, 0, 1} -- Red
}

local circles = {}
local time = 0

-- tsuki.load() is called once at the start
function tsuki.load()
    print("Tsuki Lua Game Loaded!")
    tsuki.window.setTitle("Tsuki Lua Game Demo")

    -- Create some circles
    for i = 1, 5 do
        table.insert(circles, {
            x = math.random(100, 700),
            y = math.random(100, 500),
            radius = math.random(20, 50),
            color = {math.random(), math.random(), math.random(), 1}
        })
    end
end

-- tsuki.update(dt) is called every frame
function tsuki.update(dt)
    time = time + dt

    -- Handle input
    if tsuki.keyboard.isDown("left") then
        player.x = player.x - player.speed * dt
    end
    if tsuki.keyboard.isDown("right") then
        player.x = player.x + player.speed * dt
    end
    if tsuki.keyboard.isDown("up") then
        player.y = player.y - player.speed * dt
    end
    if tsuki.keyboard.isDown("down") then
        player.y = player.y + player.speed * dt
    end

    -- Keep player on screen
    local width = tsuki.window.getWidth()
    local height = tsuki.window.getHeight()

    if player.x < 0 then player.x = 0 end
    if player.x > width - player.size then player.x = width - player.size end
    if player.y < 0 then player.y = 0 end
    if player.y > height - player.size then player.y = height - player.size end

    -- Exit on escape
    if tsuki.keyboard.isDown("escape") then
        -- In a full implementation, we'd have tsuki.event.quit()
        print("Escape pressed - would quit game")
    end

    -- Animate circles
    for i, circle in ipairs(circles) do
        circle.y = circle.y + math.sin(time + i) * 20 * dt
    end
end

-- tsuki.draw() is called every frame for rendering
function tsuki.draw()
    -- Clear screen with dark blue
    tsuki.graphics.clear(0.1, 0.1, 0.3, 1.0)

    -- Draw background grid
    tsuki.graphics.setColor(0.2, 0.2, 0.4, 1.0)
    for x = 0, tsuki.window.getWidth(), 50 do
        tsuki.graphics.line(x, 0, x, tsuki.window.getHeight())
    end
    for y = 0, tsuki.window.getHeight(), 50 do
        tsuki.graphics.line(0, y, tsuki.window.getWidth(), y)
    end

    -- Draw circles
    for i, circle in ipairs(circles) do
        tsuki.graphics.setColor(circle.color[1], circle.color[2], circle.color[3], circle.color[4])
        tsuki.graphics.circle("fill", circle.x, circle.y, circle.radius)
    end

    -- Draw player
    tsuki.graphics.setColor(player.color[1], player.color[2], player.color[3], player.color[4])
    tsuki.graphics.rectangle("fill", player.x, player.y, player.size, player.size)

    -- Draw player border
    tsuki.graphics.setColor(1, 1, 1, 1) -- White
    tsuki.graphics.rectangle("line", player.x, player.y, player.size, player.size)

    -- Draw some text (placeholder - not implemented yet)
    -- tsuki.graphics.print("Use arrow keys to move!", 10, 10)
    -- tsuki.graphics.print("FPS: " .. tsuki.timer.getFPS(), 10, 30)
end