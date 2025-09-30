# Tsuki

A modern 2D game engine built with SDL3, featuring seamless cross-platform distribution and LuaJIT scripting.

## Quick Start

```bash
# Build the engine
./build.sh

# Create a new project (with IntelliSense auto-generated)
tsuki new mygame

# Run your game
cd mygame
tsuki .

# Package a game
tsuki --package . mygame.tsuki

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

## Manual Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libzip-dev curl \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev \
  libxss-dev libgl1-mesa-dev libasound2-dev libpulse-dev libudev-dev libdbus-1-dev \
  libwayland-dev libxkbcommon-dev wayland-protocols
# SDL3 and LuaJIT will be built automatically
./build.sh
```

### macOS
```bash
brew install cmake pkg-config libzip curl
./build.sh
```

### Windows
Use the pre-built releases or build from source (all dependencies are built automatically):
```bash
# Using Visual Studio 2022 or later
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

**Note:** No external dependencies needed - SDL3, LuaJIT, zlib, libzip, and curl are all built from source as static libraries.

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

**Project Management:**
- `tsuki new <project>` - Create new project with IntelliSense
- `tsuki generate-definitions` - Generate Lua type definitions

**Running Games:**
- `tsuki game_directory/` - Run from directory
- `tsuki game.tsuki` - Run packaged game
- `tsuki .` - Run current directory

**Packaging:**
- `tsuki --package dir/ output` - Create .tsuki package
- `tsuki --fuse game.tsuki output` - Create standalone executable
- `tsuki --fuse game.tsuki output.exe --target windows` - Cross-platform fusion
- `tsuki --fuse-all game.tsuki prefix` - Create executables for all platforms

**Other:**
- `tsuki --help` - Show help
- `tsuki --version` - Show version

## VSCode IntelliSense

Lua type definitions are automatically generated when you create a project:

```bash
tsuki new mygame  # Creates project with IntelliSense enabled
```

For existing projects or examples:

```bash
cd mygame
tsuki generate-definitions  # Generates tsuki-definitions.lua and .luarc.json
```

Requirements:
1. Install the "Lua" extension by sumneko in VSCode
2. Restart VSCode after generating definitions

## Documentation

- **Examples** - See `examples/` directory