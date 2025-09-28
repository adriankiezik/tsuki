# Tsuki

A modern 2D game engine built with SDL3, featuring seamless cross-platform distribution and Lua scripting.

## Quick Start

```bash
# Build the engine
./build.sh

# Run a game directory
tsuki mygame/

# Package a game
tsuki --package mygame/ mygame.tsuki

# Create standalone executable
tsuki --fuse mygame.tsuki mygame_standalone

# Cross-platform: Create Windows .exe from Linux
tsuki --fuse mygame.tsuki mygame.exe --target windows

# Create executables for all platforms at once
tsuki --fuse-all mygame.tsuki mygame
```

## Game Structure


```lua
-- main.lua
tsuki = {}

function tsuki.load()
    -- Initialize your game
end

function tsuki.update(dt)
    -- Update game logic
end

function tsuki.draw()
    -- Render graphics
end
```

## Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config liblua5.4-dev libzip-dev curl \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev \
  libxss-dev libgl1-mesa-dev libasound2-dev libpulse-dev libudev-dev libdbus-1-dev \
  libwayland-dev libxkbcommon-dev wayland-protocols
# SDL3 will be built automatically
./build.sh
```

### macOS
```bash
brew install cmake pkg-config lua@5.4 libzip curl
./build.sh
```

### Windows
Use the pre-built releases or build with vcpkg:
```bash
vcpkg install lua:x64-windows libzip:x64-windows
./build.sh
```

## Distribution

### Cross-Platform Fusion
Create executables for any platform from any platform:
```bash
# Create a single platform executable
tsuki --fuse game.tsuki game.exe --target windows

# Create executables for ALL platforms at once
tsuki --fuse-all game.tsuki mygame
# Creates: mygame-linux, mygame-windows.exe, mygame-macos

# Automatically downloads and caches required binaries
```

## CLI Reference

**Running Games:**
- `tsuki game_directory/` - Run from directory
- `tsuki game.tsuki` - Run packaged game
- `tsuki game` - Auto-detect game.tsuki

**Packaging:**
- `tsuki --package dir/ output` - Create .tsuki package
- `tsuki --fuse game.tsuki output` - Create standalone executable
- `tsuki --fuse game.tsuki output.exe --target windows` - Cross-platform fusion
- `tsuki --fuse-all game.tsuki prefix` - Create executables for all platforms

**Other:**
- `tsuki --help` - Show help
- `tsuki --version` - Show version

## VSCode IntelliSense

For autocomplete and type hints in VSCode:

1. Install "Lua" extension by sumneko
2. Run `make dist` to generate definitions
3. IntelliSense is pre-configured in `examples/` directories

## Documentation

- **Examples** - See `examples/` directory