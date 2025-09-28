-- Font Size Test
-- Test different font sizes to ensure they render correctly

local fontsLoaded = false

function tsuki.load()
    tsuki.window.setTitle("Font Size Test - Press SPACE to cycle through sizes")

    -- Load fonts at different sizes
    local fontPath = "/usr/share/fonts/noto/NotoSans-Regular.ttf"
    local sizes = {8, 12, 16, 20, 24, 28, 32, 40, 48, 64}

    for i, size in ipairs(sizes) do
        local fontName = "size_" .. size
        local success = tsuki.graphics.loadFont(fontName, fontPath, size)
        if success then
            print("Loaded font at size " .. size .. "px")
        else
            print("Failed to load font at size " .. size .. "px")
        end
    end

    fontsLoaded = true
end

local currentTest = 1
local maxTests = 4
local spacePressed = false

function tsuki.update(dt)
    if tsuki.keyboard.isDown("space") then
        if not spacePressed then
            currentTest = currentTest + 1
            if currentTest > maxTests then
                currentTest = 1
            end
            spacePressed = true
        end
    else
        spacePressed = false
    end
end

function tsuki.draw()
    tsuki.graphics.clear(0.1, 0.1, 0.2, 1)
    tsuki.graphics.setColor(1, 1, 1, 1)

    if currentTest == 1 then
        -- Test 1: Multiple sizes using default font
        tsuki.graphics.print("Test 1: Default Font (24px) - Press SPACE to cycle tests", 20, 20)
        local sizes = {12, 16, 20, 24, 28, 32, 40, 48}
        local y = 60

        for i, size in ipairs(sizes) do
            tsuki.graphics.print(string.format("Size %dpx: The quick brown fox jumps over the lazy dog", size), 20, y)
            y = y + 30
        end

    elseif currentTest == 2 then
        -- Test 2: Small fonts
        tsuki.graphics.print("Test 2: Small Font Sizes - Press SPACE to cycle tests", 20, 20)
        local y = 60

        local sizes = {8, 10, 12, 14, 16, 18}
        for i, size in ipairs(sizes) do
            local fontName = "size_" .. size
            if tsuki.graphics.setFont(fontName) then
                tsuki.graphics.print(string.format("%dpx: Small text rendering test", size), 20, y)
            else
                tsuki.graphics.print(string.format("%dpx: Using default font", size), 20, y)
            end
            y = y + 25
        end

    elseif currentTest == 3 then
        -- Test 3: Medium fonts
        tsuki.graphics.print("Test 3: Medium Font Sizes - Press SPACE to cycle tests", 20, 20)
        local y = 60

        local sizes = {20, 24, 28, 32}
        for i, size in ipairs(sizes) do
            local fontName = "size_" .. size
            if tsuki.graphics.setFont(fontName) then
                tsuki.graphics.print(string.format("%dpx: Medium text test", size), 20, y)
            else
                tsuki.graphics.print(string.format("%dpx: Using default font", size), 20, y)
            end
            y = y + size + 10
        end

    elseif currentTest == 4 then
        -- Test 4: Large fonts
        tsuki.graphics.print("Test 4: Large Font Sizes - Press SPACE to cycle tests", 20, 20)
        local y = 80

        local sizes = {40, 48, 64}
        for i, size in ipairs(sizes) do
            local fontName = "size_" .. size
            if tsuki.graphics.setFont(fontName) then
                tsuki.graphics.print(string.format("%dpx: LARGE", size), 20, y)
            else
                tsuki.graphics.print(string.format("%dpx: DEFAULT", size), 20, y)
            end
            y = y + size + 15
        end
    end

    -- Reset to default font for UI text
    tsuki.graphics.setFont("default")
    tsuki.graphics.setColor(0.7, 0.9, 1, 1)
    tsuki.graphics.print(string.format("Current Test: %d/%d", currentTest, maxTests), 20, 550)
end