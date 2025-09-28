-- Simple Side-Scroller: Up/Down to dodge, R to restart

local w, h = 800, 600
local player = {x = 50, y = 300, size = 20}
local obstacles = {}
local game = {time = 0, score = 0, speed = 200, spawn_rate = 1, spawn_timer = 0, over = false}

function tsuki.load()
    w, h = tsuki.window.getWidth(), tsuki.window.getHeight()
    player.y = h / 2
end

function tsuki.update(dt)
    if not game.over then
        game.time = game.time + dt
        game.score = math.floor(game.time * 10)
        game.speed = 200 + game.time * 30
        game.spawn_rate = math.max(0.3, 1 - game.time * 0.05)

        -- Player movement
        if tsuki.keyboard.isDown("up") then player.y = player.y - 300 * dt end
        if tsuki.keyboard.isDown("down") then player.y = player.y + 300 * dt end
        player.y = math.max(0, math.min(h - player.size, player.y))

        -- Spawn obstacles
        game.spawn_timer = game.spawn_timer + dt
        if game.spawn_timer >= game.spawn_rate then
            table.insert(obstacles, {x = w, y = math.random(0, h - 40), w = 30, h = 40})
            game.spawn_timer = 0
        end

        -- Update obstacles
        for i = #obstacles, 1, -1 do
            obstacles[i].x = obstacles[i].x - game.speed * dt
            if obstacles[i].x < -50 then
                table.remove(obstacles, i)
                game.score = game.score + 5
            end
        end

        -- Check collisions
        for _, obs in ipairs(obstacles) do
            if player.x < obs.x + obs.w and player.x + player.size > obs.x and
               player.y < obs.y + obs.h and player.y + player.size > obs.y then
                game.over = true
            end
        end
    end

    -- Restart
    if tsuki.keyboard.isDown("r") then
        player.y = h / 2
        obstacles = {}
        game = {time = 0, score = 0, speed = 200, spawn_rate = 1, spawn_timer = 0, over = false}
    end
end

function tsuki.draw()
    tsuki.graphics.clear(0.1, 0.1, 0.2, 1)

    -- Player
    tsuki.graphics.setColor(0.2, 0.8, 0.3, 1)
    tsuki.graphics.rectangle("fill", player.x, player.y, player.size, player.size)

    -- Obstacles
    tsuki.graphics.setColor(0.8, 0.3, 0.3, 1)
    for _, obs in ipairs(obstacles) do
        tsuki.graphics.rectangle("fill", obs.x, obs.y, obs.w, obs.h)
    end

    -- UI
    tsuki.graphics.setColor(1, 1, 1, 1)
    tsuki.graphics.print("Score: " .. game.score, 10, 10)
    if game.over then
        tsuki.graphics.print("GAME OVER - Press R to restart", w/2 - 120, h/2)
    end
end