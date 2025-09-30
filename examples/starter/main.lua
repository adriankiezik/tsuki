-- Tsuki Starter Example
-- This example demonstrates basic usage of the Tsuki 2D game engine

-- Initialize the game state
local player = {
    x = 400,
    y = 300,
    width = 50,
    height = 50,
    speed = 200
}

local circle = {
    x = 200,
    y = 150,
    radius = 30,
    color = {r = 1.0, g = 0.5, b = 0.0} -- Orange
}

local time = 0
local message = "Welcome to Tsuki Engine!"

function tsuki.load()
    -- This function is called once when the game starts
    -- Set window title
    tsuki.window:setTitle("Tsuki Starter Example")
end


function tsuki.update(dt)
    -- This function is called every frame
    -- dt is the time in seconds since the last frame

    time = time + dt

    -- Move player with arrow keys
    if tsuki.keyboard:isDown("left") then
        player.x = player.x - player.speed * dt
    end
    if tsuki.keyboard:isDown("right") then
        player.x = player.x + player.speed * dt
    end
    if tsuki.keyboard:isDown("up") then
        player.y = player.y - player.speed * dt
    end
    if tsuki.keyboard:isDown("down") then
        player.y = player.y + player.speed * dt
    end

    -- Keep player within window bounds
    local windowWidth = tsuki.window:getWidth()
    local windowHeight = tsuki.window:getHeight()

    if player.x < 0 then player.x = 0 end
    if player.x + player.width > windowWidth then
        player.x = windowWidth - player.width
    end
    if player.y < 0 then player.y = 0 end
    if player.y + player.height > windowHeight then
        player.y = windowHeight - player.height
    end

    -- Animate the circle
    circle.x = 200 + math.sin(time * 2) * 100
    circle.y = 150 + math.cos(time * 1.5) * 50


end

function tsuki.draw()
    -- This function is called every frame to render graphics

    -- Clear the screen with a dark blue background
    tsuki.graphics:clear(0.1, 0.1, 0.3, 1.0)

    -- Draw a filled rectangle for the player (white)
    tsuki.graphics:setColor(1.0, 1.0, 1.0, 1.0)
    tsuki.graphics:rectangle("fill", player.x, player.y, player.width, player.height)

    -- Draw a border around the player (yellow)
    tsuki.graphics:setColor(1.0, 1.0, 0.0, 1.0)
    tsuki.graphics:rectangle("line", player.x, player.y, player.width, player.height)

    -- Draw the animated circle
    tsuki.graphics:setColor(circle.color.r, circle.color.g, circle.color.b, 1.0)
    tsuki.graphics:circle("fill", circle.x, circle.y, circle.radius)

    -- Draw a line from player to circle
    tsuki.graphics:setColor(0.5, 0.8, 1.0, 1.0)
    tsuki.graphics:line(
        player.x + player.width/2,
        player.y + player.height/2,
        circle.x,
        circle.y
    )

    -- Draw some text
    tsuki.graphics:setColor(1.0, 1.0, 1.0, 1.0)
    tsuki.graphics:print(message, 10, 10)
    tsuki.graphics:print("Use arrow keys to move the player", 10, 30)
    tsuki.graphics:print("Press ESC to exit", 10, 50)

    -- Get and display mouse position
    local mouseX, mouseY = tsuki.mouse:getPosition()
    tsuki.graphics:print("Mouse: " .. mouseX .. ", " .. mouseY, 10, 70)

    -- Draw a small circle at mouse position
    tsuki.graphics:setColor(1.0, 0.0, 1.0, 0.7)
    tsuki.graphics:circle("fill", mouseX, mouseY, 5)
end