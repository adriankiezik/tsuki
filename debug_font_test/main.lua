-- Debug Font Size Test
-- Simple test to see exactly what's happening with font sizes

function tsuki.load()
    tsuki.window.setTitle("Debug Font Size Test")

    -- Load just two fonts at very different sizes
    local fontPath = "/usr/share/fonts/noto/NotoSans-Regular.ttf"

    print("Loading small font...")
    local success1 = tsuki.graphics.loadFont("small", fontPath, 12)
    print("Small font loaded:", success1)

    print("Loading large font...")
    local success2 = tsuki.graphics.loadFont("large", fontPath, 48)
    print("Large font loaded:", success2)
end

function tsuki.draw()
    tsuki.graphics.clear(0.1, 0.1, 0.2, 1)
    tsuki.graphics.setColor(1, 1, 1, 1)

    -- Test 1: Default font
    tsuki.graphics.print("Default font (should be 24px)", 20, 50)

    -- Test 2: Small font
    local setSuccess1 = tsuki.graphics.setFont("small")
    print("Set small font success:", setSuccess1)
    tsuki.graphics.print("Small font (should be 12px) - ABCDEFG", 20, 100)

    -- Test 3: Large font
    local setSuccess2 = tsuki.graphics.setFont("large")
    print("Set large font success:", setSuccess2)
    tsuki.graphics.print("Large font (should be 48px) - ABCDEFG", 20, 150)

    -- Test 4: Back to default
    tsuki.graphics.setFont("default")
    tsuki.graphics.print("Back to default font", 20, 250)

    tsuki.graphics.setColor(0.7, 0.7, 0.7, 1)
    tsuki.graphics.print("Check console for debug messages", 20, 300)
end