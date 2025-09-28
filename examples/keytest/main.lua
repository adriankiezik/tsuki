-- Comprehensive Keyboard Test
-- Press any key to see if it's detected. ESC to quit.

local w, h = 800, 600
local pressed_keys = {}
local test_keys = {
    -- Letters
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",

    -- Numbers
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",

    -- Function keys
    "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",

    -- Arrow keys
    "up", "down", "left", "right",

    -- Special keys mentioned by user
    "enter", "backslash", "tilde", "minus", "equals", "caps", "capslock",

    -- Symbol keys (all variations)
    "`", "~", "-", "=", "\\", "[", "]", ";", "'", ",", ".", "/",

    -- Other special keys
    "space", "escape", "tab", "backspace", "delete", "insert",
    "home", "end", "pageup", "pagedown",

    -- Modifiers
    "shift", "ctrl", "alt", "gui",

    -- Test both names for common keys
    "return", "esc", "del", "ins"
}

function tsuki.load()
    w, h = tsuki.window.getWidth(), tsuki.window.getHeight()
    tsuki.window.setTitle("Comprehensive Keyboard Test - Press keys to test")
end

function tsuki.update(dt)
    -- Check all test keys
    pressed_keys = {}
    for _, key in ipairs(test_keys) do
        if tsuki.keyboard.isDown(key) then
            table.insert(pressed_keys, key)
        end
    end

    -- Exit on escape
    if tsuki.keyboard.isDown("escape") then
        -- Note: In a real application you'd call a quit function
        -- For this test, we'll just clear the pressed keys
        if #pressed_keys == 1 and pressed_keys[1] == "escape" then
            -- ESC is the only key pressed, so we're trying to quit
        end
    end
end

function tsuki.draw()
    tsuki.graphics.clear(0.1, 0.1, 0.2, 1)

    -- Title
    tsuki.graphics.setColor(1, 1, 1, 1)
    tsuki.graphics.print("Comprehensive Keyboard Test", w/2 - 150, 20)
    tsuki.graphics.print("Press any key to test. ESC to quit.", w/2 - 120, 40)

    -- Show currently pressed keys
    tsuki.graphics.setColor(0.8, 1, 0.8, 1)
    local y = 80
    if #pressed_keys > 0 then
        tsuki.graphics.print("Currently pressed keys:", 20, y)
        y = y + 25
        for i, key in ipairs(pressed_keys) do
            tsuki.graphics.setColor(0.2, 1, 0.2, 1)
            tsuki.graphics.print("• " .. key, 40, y)
            y = y + 20
            if y > h - 40 then break end -- Don't overflow screen
        end
    else
        tsuki.graphics.setColor(0.7, 0.7, 0.7, 1)
        tsuki.graphics.print("No keys currently pressed", 20, y)
    end

    -- Show supported key categories
    tsuki.graphics.setColor(0.6, 0.8, 1, 1)
    local info_y = h - 120
    tsuki.graphics.print("Supported key types:", w/2 + 20, info_y)
    tsuki.graphics.setColor(0.8, 0.8, 1, 1)
    tsuki.graphics.print("• Letters: a-z, A-Z", w/2 + 40, info_y + 20)
    tsuki.graphics.print("• Numbers: 0-9", w/2 + 40, info_y + 35)
    tsuki.graphics.print("• Function: F1-F12", w/2 + 40, info_y + 50)
    tsuki.graphics.print("• Arrows: up, down, left, right", w/2 + 40, info_y + 65)
    tsuki.graphics.print("• Special: space, enter, tab, etc.", w/2 + 40, info_y + 80)
    tsuki.graphics.print("• Modifiers: shift, ctrl, alt, gui", w/2 + 40, info_y + 95)
end