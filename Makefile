# Tsuki Makefile

.PHONY: build clean lua-definitions dist release-prep help

# Build the engine
build:
	./build.sh

# Generate Lua type definitions from C++ bindings
lua-definitions:
	python3 scripts/generate_lua_definitions.py
	@echo "📋 Lua definitions updated in dist/"

# Create distribution package
dist: lua-definitions
	@echo "📦 Distribution package ready in dist/"

# Prepare release packages
release-prep:
	chmod +x scripts/prepare_release.py
	python3 scripts/prepare_release.py
	@echo "🚀 Release packages ready in releases/"

# Prepare release with specific version
release-version:
	@read -p "Enter version: " version; \
	python3 scripts/prepare_release.py --version "$$version"

# Clean all build artifacts
clean:
	rm -rf build/ releases/
	@echo "🧹 Cleaned build and release directories"

# Clean generated files (but keep source)
clean-generated:
	rm -rf dist/ releases/
	@echo "🧹 Cleaned generated distribution files"

# Build everything
all: build dist

# Show available commands
help:
	@echo "Tsuki Build Commands:"
	@echo ""
	@echo "🔨 Building:"
	@echo "  build              - Build the engine"
	@echo "  all                - Build engine + generate distributions"
	@echo ""
	@echo "📋 IntelliSense:"
	@echo "  lua-definitions    - Generate Lua type definitions"
	@echo "  dist               - Create distribution package"
	@echo ""
	@echo "🚀 Releases:"
	@echo "  release-prep       - Prepare release packages (auto-version)"
	@echo "  release-version    - Prepare release with specific version"
	@echo ""
	@echo "🧹 Cleaning:"
	@echo "  clean              - Clean all build/release artifacts"
	@echo "  clean-generated    - Clean only generated distribution files"
	@echo ""
	@echo "❓ help              - Show this help message"