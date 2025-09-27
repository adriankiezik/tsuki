# Tsuki Release Process

This document describes how to create releases for the Tsuki game engine.

## Overview

Tsuki uses GitHub Actions to automatically build cross-platform binaries and create releases. The process supports:

- **Automatic builds** on Linux, macOS, and Windows
- **Dependency bundling** for each platform
- **Cross-platform fusion** - create Windows executables from Linux
- **Automated releases** with GitHub Actions

## Release Workflow

### 1. Automatic Releases (Recommended)

Create a release by pushing a version tag:

```bash
# Create and push a version tag
git tag v1.0.0
git push origin v1.0.0
```

This automatically triggers the release workflow and creates:
- `tsuki-linux-x64.zip` - Linux binary with bundled libraries
- `tsuki-macos-x64.zip` - macOS app bundle + standalone binary
- `tsuki-windows-x64.zip` - Windows executable with DLLs

### 2. Manual Releases

Use the helper script for more control:

```bash
# Create a standard release
./scripts/create-release.sh v1.0.0

# Create a draft release
./scripts/create-release.sh --draft v1.0.0-beta

# Create a prerelease
./scripts/create-release.sh --prerelease v1.0.0-rc1

# Build binaries without creating release
./scripts/create-release.sh --no-release v1.0.0
```

### 3. Testing Releases Locally

Test the bundling process before creating a release:

```bash
# Test current platform
./scripts/test-release-bundle.sh

# Test specific platform
./scripts/test-release-bundle.sh --platform linux
./scripts/test-release-bundle.sh --platform windows --arch x86
```

## Version Naming

Follow semantic versioning (semver):

- **Major release**: `v1.0.0` (breaking changes)
- **Minor release**: `v1.1.0` (new features)
- **Patch release**: `v1.0.1` (bug fixes)
- **Prerelease**: `v1.0.0-beta`, `v1.0.0-rc1`

## Release Assets

Each release includes platform-specific packages:

### Linux (`tsuki-linux-x64.zip`)
- `tsuki` - Main executable
- `lib/` - Bundled shared libraries (if needed)
- `tsuki.sh` - Launcher script with library path

### macOS (`tsuki-macos-x64.zip`)
- `Tsuki.app/` - macOS application bundle
- `tsuki` - Standalone CLI binary

### Windows (`tsuki-windows-x64.zip`)
- `tsuki.exe` - Main executable
- `*.dll` - Required DLL files (SDL3, Lua, etc.)

## Cross-Platform Fusion

Once releases are published, developers can create cross-platform executables:

```bash
# Linux developer creating Windows executable
./tsuki --fuse mygame.tsuki mygame.exe --target windows

# This automatically downloads and uses the Windows release bundle
```

## Setting Up CI/CD

### Prerequisites

1. **Repository Secrets**: No additional secrets needed (uses `GITHUB_TOKEN`)
2. **GitHub Actions**: Enabled in repository settings
3. **Releases**: Enabled in repository settings

### Customizing Release URLs

The cross-platform fusion feature downloads binaries from GitHub releases. To use a custom repository:

1. Update the URL in `src/packaging.cpp`:
   ```cpp
   base_url = "https://github.com/YOUR-ORG/YOUR-REPO/releases/latest/download/";
   ```

2. Or set environment variable:
   ```bash
   export TSUKI_RELEASES_URL="https://your-custom-url.com/releases/"
   ```

## Troubleshooting

### Build Failures

1. **Dependency issues**: Check that SDL3, Lua, and libzip are properly installed
2. **Platform-specific problems**: Check the workflow logs in GitHub Actions
3. **Missing dependencies**: Some platforms may need additional setup

### Release Issues

1. **Permission errors**: Ensure `GITHUB_TOKEN` has write permissions
2. **Tag conflicts**: Delete and recreate tags if needed
3. **Asset upload failures**: Check file sizes and network connectivity

### Cross-Platform Fusion Issues

1. **Download failures**: Check internet connection and GitHub releases availability
2. **Bundle extraction errors**: Verify ZIP file integrity
3. **Binary compatibility**: Ensure target platform binaries are correct

## Workflow Files

- `.github/workflows/build.yml` - Continuous integration
- `.github/workflows/release.yml` - Release creation
- `scripts/create-release.sh` - Release helper script
- `scripts/test-release-bundle.sh` - Local testing script

## Contributing

When modifying the release process:

1. Test changes with `test-release-bundle.sh`
2. Create a draft release to verify CI/CD
3. Update this documentation as needed