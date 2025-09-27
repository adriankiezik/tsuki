# Installing Tsuki Game Engine

## Quick Start

### Build and Install (Recommended)
```bash
# Build and install globally
./build.sh --clean --install

# This will:
# 1. Clean previous builds
# 2. Build the tsuki executable
# 3. Install to /usr/local/bin/tsuki (requires sudo)
```

### Build Only
```bash
# Just build (no installation)
./build.sh

# Use locally
./build/tsuki --help
```

## Installation Options

### Option 1: Global Installation (Recommended)
```bash
./build.sh --install
# or
sudo cp build/tsuki /usr/local/bin/tsuki
sudo chmod +x /usr/local/bin/tsuki
```

After installation, you can use `tsuki` from anywhere:
```bash
tsuki --version
tsuki mygame/
tsuki --package mygame/ mygame.tsuki
```

### Option 2: Local Usage
```bash
./build.sh
cd build
./tsuki --help
```

### Option 3: Manual Installation to Custom Path
```bash
./build.sh
# Copy to your preferred location
mkdir -p ~/.local/bin
cp build/tsuki ~/.local/bin/tsuki
# Make sure ~/.local/bin is in your PATH
```

## Build Script Options

```bash
./build.sh [options]

Options:
  --debug     Build in debug mode
  --make      Use Makefile instead of CMake
  --clean     Clean before building
  --run       Run after building
  --install   Install tsuki globally (requires sudo)
  --help      Show help message
```

## Verification

After installation, verify it works:
```bash
# Check version
tsuki --version

# Run the example
tsuki examples/working_example/

# Test packaging
tsuki --package examples/working_example/ test.tsuki
tsuki test.tsuki

# Test standalone distribution
tsuki --fuse test.tsuki standalone_game
./standalone_game
```

## Requirements

- C++23 compatible compiler (GCC 10+ or Clang 12+)
- CMake 3.20+
- SDL3 development libraries
- Lua 5.4 development libraries
- libzip development libraries
- pkg-config

### Ubuntu/Debian:
```bash
sudo apt install build-essential cmake libsdl3-dev liblua5.4-dev libzip-dev pkg-config
```

### Arch Linux:
```bash
sudo pacman -S base-devel cmake sdl3 lua libzip pkgconf
```

### Fedora:
```bash
sudo dnf install gcc-c++ cmake SDL3-devel lua-devel libzip-devel pkgconfig
```