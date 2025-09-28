#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default build type
BUILD_TYPE="Release"
BUILD_SYSTEM="cmake"
CLEAN=0
RUN=0
INSTALL=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --make)
            BUILD_SYSTEM="make"
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --run)
            RUN=1
            shift
            ;;
        --install)
            INSTALL=1
            shift
            ;;
        --help)
            echo "Usage: ./build.sh [options]"
            echo "Options:"
            echo "  --debug     Build in debug mode"
            echo "  --make      Use Makefile instead of CMake"
            echo "  --clean     Clean before building"
            echo "  --run       Run after building"
            echo "  --install   Install tsuki globally (requires sudo)"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${GREEN}Building Tsuki...${NC}"

# Make sure we're in the project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check for essential dependencies on Linux
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    missing_deps=()

    if ! pkg-config --exists lua5.4 2>/dev/null && ! pkg-config --exists lua 2>/dev/null; then
        missing_deps+=("liblua5.4-dev")
    fi

    if ! pkg-config --exists libzip 2>/dev/null; then
        missing_deps+=("libzip-dev")
    fi

    if ! pkg-config --exists x11 2>/dev/null; then
        missing_deps+=("libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev")
    fi

    if ! pkg-config --exists gl 2>/dev/null; then
        missing_deps+=("libgl1-mesa-dev")
    fi

    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing dependencies detected. Install with:${NC}"
        echo -e "${YELLOW}sudo apt-get install -y ${missing_deps[*]} libxss-dev libasound2-dev libpulse-dev libudev-dev libdbus-1-dev libwayland-dev libxkbcommon-dev wayland-protocols${NC}"
        echo -e "${YELLOW}Continuing build anyway...${NC}"
        echo
    fi
fi

if [ "$BUILD_SYSTEM" = "cmake" ]; then
    # CMake build
    if [ $CLEAN -eq 1 ]; then
        echo -e "${YELLOW}Cleaning build directory...${NC}"
        rm -rf build
    fi

    mkdir -p build
    cd build

    echo -e "${YELLOW}Configuring with CMake (${BUILD_TYPE})...${NC}"
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    cmake $CMAKE_ARGS .. || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

    echo -e "${YELLOW}Building...${NC}"
    cmake --build . -j$(nproc) || { echo -e "${RED}Build failed${NC}"; exit 1; }

    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Binary location: build/tsuki${NC}"

    if [ $INSTALL -eq 1 ]; then
        echo -e "${YELLOW}Installing tsuki globally...${NC}"
        if [ "$EUID" -eq 0 ]; then
            # Running as root
            cp tsuki /usr/local/bin/tsuki
            chmod +x /usr/local/bin/tsuki
            echo -e "${GREEN}Tsuki installed to /usr/local/bin/tsuki${NC}"
            echo -e "${GREEN}You can now run 'tsuki' from anywhere!${NC}"
        else
            # Not running as root, try sudo
            echo -e "${YELLOW}Installing to /usr/local/bin requires sudo...${NC}"
            echo -e "${YELLOW}Running: sudo cp tsuki /usr/local/bin/tsuki${NC}"

            if sudo -n true 2>/dev/null; then
                # sudo works without password
                sudo cp tsuki /usr/local/bin/tsuki
                sudo chmod +x /usr/local/bin/tsuki
                echo -e "${GREEN}Tsuki installed to /usr/local/bin/tsuki${NC}"
                echo -e "${GREEN}You can now run 'tsuki' from anywhere!${NC}"
            else
                # sudo requires password
                sudo cp tsuki /usr/local/bin/tsuki && sudo chmod +x /usr/local/bin/tsuki
                if [ $? -eq 0 ]; then
                    echo -e "${GREEN}Tsuki installed to /usr/local/bin/tsuki${NC}"
                    echo -e "${GREEN}You can now run 'tsuki' from anywhere!${NC}"
                else
                    echo -e "${RED}Failed to install tsuki globally${NC}"
                    echo -e "${YELLOW}Manual installation:${NC}"
                    echo -e "${YELLOW}  sudo cp $(pwd)/tsuki /usr/local/bin/tsuki${NC}"
                    echo -e "${YELLOW}  sudo chmod +x /usr/local/bin/tsuki${NC}"
                    echo -e "${YELLOW}Alternative: You can use $(pwd)/tsuki locally${NC}"
                fi
            fi
        fi
    fi

    if [ $RUN -eq 1 ]; then
        echo -e "${YELLOW}Running tsuki help...${NC}"
        ./tsuki --help
    fi
else
    # Make build
    if [ $CLEAN -eq 1 ]; then
        echo -e "${YELLOW}Cleaning...${NC}"
        make clean
    fi

    if [ "$BUILD_TYPE" = "Debug" ]; then
        echo -e "${YELLOW}Building in debug mode...${NC}"
        make debug || { echo -e "${RED}Build failed${NC}"; exit 1; }
    else
        echo -e "${YELLOW}Building...${NC}"
        make || { echo -e "${RED}Build failed${NC}"; exit 1; }
    fi

    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Binary location: ./tsuki${NC}"

    if [ $INSTALL -eq 1 ]; then
        echo -e "${YELLOW}Installing tsuki globally...${NC}"
        if [ "$EUID" -eq 0 ]; then
            # Running as root
            cp tsuki /usr/local/bin/tsuki
            chmod +x /usr/local/bin/tsuki
            echo -e "${GREEN}Tsuki installed to /usr/local/bin/tsuki${NC}"
            echo -e "${GREEN}You can now run 'tsuki' from anywhere!${NC}"
        else
            # Not running as root, try sudo
            echo -e "${YELLOW}Installing to /usr/local/bin requires sudo...${NC}"
            echo -e "${YELLOW}Running: sudo cp tsuki /usr/local/bin/tsuki${NC}"

            if sudo -n true 2>/dev/null; then
                # sudo works without password
                sudo cp tsuki /usr/local/bin/tsuki
                sudo chmod +x /usr/local/bin/tsuki
                echo -e "${GREEN}Tsuki installed to /usr/local/bin/tsuki${NC}"
                echo -e "${GREEN}You can now run 'tsuki' from anywhere!${NC}"
            else
                # sudo requires password
                sudo cp tsuki /usr/local/bin/tsuki && sudo chmod +x /usr/local/bin/tsuki
                if [ $? -eq 0 ]; then
                    echo -e "${GREEN}Tsuki installed to /usr/local/bin/tsuki${NC}"
                    echo -e "${GREEN}You can now run 'tsuki' from anywhere!${NC}"
                else
                    echo -e "${RED}Failed to install tsuki globally${NC}"
                    echo -e "${YELLOW}Manual installation:${NC}"
                    echo -e "${YELLOW}  sudo cp $(pwd)/tsuki /usr/local/bin/tsuki${NC}"
                    echo -e "${YELLOW}  sudo chmod +x /usr/local/bin/tsuki${NC}"
                    echo -e "${YELLOW}Alternative: You can use $(pwd)/tsuki locally${NC}"
                fi
            fi
        fi
    fi

    if [ $RUN -eq 1 ]; then
        echo -e "${YELLOW}Running tsuki help...${NC}"
        ./tsuki --help
    fi
fi