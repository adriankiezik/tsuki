-- Test for Problematic Keys
-- Specifically tests: enter, backslash, shift combinations, caps lock, tilda, minus, equals

local w, h = 800, 600
local problem_keys = {
    "enter", "backslash", "tilde", "minus", "equals", "caps", "capslock",
    "\\", "`", "~", "-", "=", "[", "]", ";", "'", ",", ".", "/",
    "shift", "ctrl", "alt"
}
local pressed = {}
local failed_keys = {}

function tsuki.load()
    w, h = tsuki.window.getWidth(), tsuki.window.getHeight()
    tsuki.window.setTitle("Problem Keys Test - Testing user's specific issues")
end

function tsuki.update(dt)
    pressed = {}
    failed_keys = {}

    for _, key in ipairs(problem_keys) do
        if tsuki.keyboard.isDown(key) then
            table.insert(pressed, key)
        end
    end
end

function tsuki.draw()
    tsuki.graphics.clear(0.1, 0.1, 0.2, 1)

    -- Title
    tsuki.graphics.setColor(1, 1, 1, 1)
    tsuki.graphics.print("Problem Keys Test", w/2 - 80, 20)
    tsuki.graphics.print("Testing: enter, backslash, shift, caps, tilda, minus, equals", w/2 - 200, 40)

    -- Show currently pressed problem keys
    local y = 80
    tsuki.graphics.setColor(0.8, 1, 0.8, 1)
    if #pressed > 0 then
        tsuki.graphics.print("SUCCESS - Problem keys detected:", 20, y)
        y = y + 25
        for _, key in ipairs(pressed) do
            tsuki.graphics.setColor(0.2, 1, 0.2, 1)
            tsuki.graphics.print("✓ " .. key, 40, y)
            y = y + 20
        end
    else
        tsuki.graphics.setColor(0.7, 0.7, 0.7, 1)
        tsuki.graphics.print("Press problem keys to test...", 20, y)
    end

    -- Instructions
    tsuki.graphics.setColor(1, 1, 0.8, 1)
    local inst_y = h - 200
    tsuki.graphics.print("Press these keys to test:", 20, inst_y)
    tsuki.graphics.setColor(0.9, 0.9, 1, 1)
    tsuki.graphics.print("• Enter key", 40, inst_y + 20)
    tsuki.graphics.print("• Backslash \\", 40, inst_y + 35)
    tsuki.graphics.print("• Tilde ~ (grave `)", 40, inst_y + 50)
    tsuki.graphics.print("• Minus - key", 40, inst_y + 65)
    tsuki.graphics.print("• Equals = key", 40, inst_y + 80)
    tsuki.graphics.print("• Caps Lock", 40, inst_y + 95)
    tsuki.graphics.print("• Shift, Ctrl, Alt", 40, inst_y + 110)
    tsuki.graphics.print("• Symbols: [ ] ; ' , . /", 40, inst_y + 125)
end