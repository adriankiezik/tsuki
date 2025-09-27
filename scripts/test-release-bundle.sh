#!/bin/bash

# Script to test release bundling locally
set -e

PLATFORM=""
ARCH="x64"
VERSION="v1.0.0-test"

usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -p, --platform <platform>  Target platform (linux, macos, windows)"
    echo "  -a, --arch <arch>          Target architecture (x64, x86) [default: x64]"
    echo "  -v, --version <version>    Version string [default: v1.0.0-test]"
    echo "  -h, --help                Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --platform linux                # Test Linux bundling"
    echo "  $0 --platform windows --arch x86   # Test Windows x86 bundling"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--platform)
            PLATFORM="$2"
            shift 2
            ;;
        -a|--arch)
            ARCH="$2"
            shift 2
            ;;
        -v|--version)
            VERSION="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Auto-detect platform if not specified
if [[ -z "$PLATFORM" ]]; then
    case "$(uname -s)" in
        Linux*)     PLATFORM="linux";;
        Darwin*)    PLATFORM="macos";;
        CYGWIN*|MINGW*|MSYS*) PLATFORM="windows";;
        *)          echo "Unable to detect platform. Please specify with --platform"; exit 1;;
    esac
fi

echo "🔧 Testing release bundle for $PLATFORM ($ARCH)"
echo "Version: $VERSION"
echo ""

# Clean and build
echo "🏗️  Building Tsuki..."
./build.sh --clean
echo ""

# Create bundle directory
BUNDLE_DIR="test-release-bundle-$PLATFORM-$ARCH"
rm -rf "$BUNDLE_DIR"
mkdir -p "$BUNDLE_DIR"

echo "📦 Creating release bundle..."

case "$PLATFORM" in
    linux)
        echo "  📋 Bundling for Linux..."

        # Copy main executable
        cp build/tsuki "$BUNDLE_DIR/"

        # Copy required libraries
        mkdir -p "$BUNDLE_DIR/lib"

        # Find and copy shared libraries (if any)
        if command -v ldd &> /dev/null; then
            ldd build/tsuki | grep "=>" | awk '{print $3}' | grep -E "(lua|zip|SDL)" | while read lib; do
                if [[ -f "$lib" ]]; then
                    echo "    📚 Copying: $(basename "$lib")"
                    cp "$lib" "$BUNDLE_DIR/lib/" || true
                fi
            done
        fi

        # Create launcher script
        cat > "$BUNDLE_DIR/tsuki.sh" << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="$SCRIPT_DIR/lib:$LD_LIBRARY_PATH"
exec "$SCRIPT_DIR/tsuki" "$@"
EOF
        chmod +x "$BUNDLE_DIR/tsuki.sh"
        chmod +x "$BUNDLE_DIR/tsuki"

        echo "  ✅ Linux bundle created"
        ;;

    macos)
        echo "  🍎 Bundling for macOS..."

        # Create app bundle structure
        mkdir -p "$BUNDLE_DIR/Tsuki.app/Contents/MacOS"
        mkdir -p "$BUNDLE_DIR/Tsuki.app/Contents/Resources"

        # Copy executable
        cp build/tsuki "$BUNDLE_DIR/Tsuki.app/Contents/MacOS/"

        # Check for dynamic libraries
        if command -v otool &> /dev/null; then
            otool -L build/tsuki | grep -E "(SDL|lua|zip)" | awk '{print $1}' | grep -v "@executable_path" | while read lib; do
                if [[ -f "$lib" ]] && [[ "$lib" != "/usr/lib/"* ]] && [[ "$lib" != "/System/"* ]]; then
                    echo "    📚 Copying: $(basename "$lib")"
                    cp "$lib" "$BUNDLE_DIR/Tsuki.app/Contents/MacOS/" || true
                    libname=$(basename "$lib")
                    install_name_tool -change "$lib" "@executable_path/$libname" "$BUNDLE_DIR/Tsuki.app/Contents/MacOS/tsuki" || true
                fi
            done
        fi

        # Create Info.plist
        cat > "$BUNDLE_DIR/Tsuki.app/Contents/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>tsuki</string>
    <key>CFBundleIdentifier</key>
    <string>com.tsuki.engine</string>
    <key>CFBundleName</key>
    <string>Tsuki</string>
    <key>CFBundleVersion</key>
    <string>$VERSION</string>
    <key>CFBundleShortVersionString</key>
    <string>$VERSION</string>
</dict>
</plist>
EOF

        # Also create standalone binary
        cp build/tsuki "$BUNDLE_DIR/"
        chmod +x "$BUNDLE_DIR/tsuki"

        echo "  ✅ macOS bundle created"
        ;;

    windows)
        echo "  🪟 Bundling for Windows..."
        echo "  ⚠️  Note: This is a mock Windows bundle (running on $(uname -s))"

        # Copy executable (rename to .exe for consistency)
        cp build/tsuki "$BUNDLE_DIR/tsuki.exe"

        # Create mock DLLs for testing
        echo "Mock SDL3.dll" > "$BUNDLE_DIR/SDL3.dll"
        echo "Mock lua54.dll" > "$BUNDLE_DIR/lua54.dll"
        echo "Mock libzip.dll" > "$BUNDLE_DIR/libzip.dll"
        echo "Mock msvcr140.dll" > "$BUNDLE_DIR/msvcr140.dll"

        echo "  ✅ Windows bundle created (mock)"
        ;;

    *)
        echo "❌ Unsupported platform: $PLATFORM"
        exit 1
        ;;
esac

# Create archive
ARCHIVE_NAME="tsuki-$PLATFORM-$ARCH.zip"
echo ""
echo "🗜️  Creating archive: $ARCHIVE_NAME"
cd "$BUNDLE_DIR"
zip -r "../$ARCHIVE_NAME" .
cd ..

# Test the bundle
echo ""
echo "🧪 Testing bundle..."
cd "$BUNDLE_DIR"

case "$PLATFORM" in
    linux)
        echo "  Testing Linux bundle..."
        ./tsuki --version || echo "  ⚠️  Direct execution failed, try ./tsuki.sh --version"
        ./tsuki.sh --version || echo "  ❌ Bundle test failed"
        ;;
    macos)
        echo "  Testing macOS bundle..."
        ./tsuki --version || echo "  ❌ Standalone binary test failed"
        "./Tsuki.app/Contents/MacOS/tsuki" --version || echo "  ❌ App bundle test failed"
        ;;
    windows)
        echo "  Testing Windows bundle..."
        ./tsuki.exe --version || echo "  ❌ Windows executable test failed"
        ;;
esac

cd ..

echo ""
echo "📊 Bundle Summary:"
echo "  Platform: $PLATFORM"
echo "  Architecture: $ARCH"
echo "  Version: $VERSION"
echo "  Bundle directory: $BUNDLE_DIR"
echo "  Archive: $ARCHIVE_NAME"
echo "  Size: $(du -sh "$ARCHIVE_NAME" | cut -f1)"
echo ""
echo "📁 Bundle contents:"
ls -la "$BUNDLE_DIR"

echo ""
echo "✅ Release bundle test completed!"
echo ""
echo "🔍 To test cross-platform fusion with this bundle:"
echo "  1. Move $ARCHIVE_NAME to ~/.cache/tsuki/"
echo "  2. Create a test game and try: ./build/tsuki --fuse game.tsuki test.exe --target $PLATFORM"