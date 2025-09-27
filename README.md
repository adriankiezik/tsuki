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
sudo apt-get install build-essential cmake pkg-config liblua5.4-dev libzip-dev curl
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
# Linux developer creating Windows release
tsuki --fuse game.tsuki game.exe --target windows

# Automatically downloads and caches Windows binaries
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

**Other:**
- `tsuki --help` - Show help
- `tsuki --version` - Show version

## Documentation

- **Examples** - See `examples/` directory
- **API Reference** - Coming soon