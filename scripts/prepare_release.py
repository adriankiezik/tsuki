#!/usr/bin/env python3
"""
Tsuki Engine - Release Preparation Script
Prepares release packages for GitHub releases including IntelliSense assets.
"""

import argparse
import shutil
import subprocess
import sys
import tarfile
import zipfile
from pathlib import Path

def run_command(cmd, check=True):
    """Run a shell command and return the result."""
    print(f"🔧 Running: {cmd}")
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if check and result.returncode != 0:
        print(f"❌ Command failed: {cmd}")
        print(f"Error: {result.stderr}")
        sys.exit(1)
    return result

def get_git_info():
    """Get current git information."""
    try:
        # Get current commit hash
        commit = run_command("git rev-parse --short HEAD").stdout.strip()

        # Get current tag if any
        tag_result = run_command("git describe --tags --exact-match HEAD", check=False)
        tag = tag_result.stdout.strip() if tag_result.returncode == 0 else None

        # Get branch
        branch = run_command("git rev-parse --abbrev-ref HEAD").stdout.strip()

        return {
            'commit': commit,
            'tag': tag,
            'branch': branch
        }
    except Exception as e:
        print(f"⚠️  Could not get git info: {e}")
        return {
            'commit': 'unknown',
            'tag': None,
            'branch': 'unknown'
        }

def create_intellisense_package(project_root: Path, output_dir: Path, version: str):
    """Create IntelliSense package for distribution."""
    print("📦 Creating IntelliSense package...")

    # Generate fresh definitions
    generate_script = project_root / "scripts" / "generate_lua_definitions.py"
    run_command(f"python3 {generate_script}")

    # Create package directory
    package_name = f"tsuki-intellisense-{version}"
    package_dir = output_dir / package_name
    package_dir.mkdir(parents=True, exist_ok=True)

    # Copy files from dist/
    dist_dir = project_root / "dist"
    if dist_dir.exists():
        for file in dist_dir.glob("*"):
            if file.is_file():
                shutil.copy2(file, package_dir)

    # Create setup script
    setup_script = package_dir / "setup.py"
    setup_content = f'''#!/usr/bin/env python3
"""
Tsuki Engine IntelliSense Setup Script
Automatically configures VSCode Lua IntelliSense for Tsuki projects.
"""

import json
import sys
from pathlib import Path

def setup_intellisense():
    """Set up IntelliSense in current directory."""
    current_dir = Path.cwd()

    # Copy definitions file
    definitions_src = Path(__file__).parent / "tsuki-definitions.lua"
    definitions_dst = current_dir / "tsuki-definitions.lua"

    if definitions_dst.exists():
        response = input("tsuki-definitions.lua already exists. Overwrite? (y/N): ")
        if response.lower() != 'y':
            print("Skipped definitions file.")
        else:
            definitions_dst.write_text(definitions_src.read_text())
            print("✅ Updated tsuki-definitions.lua")
    else:
        definitions_dst.write_text(definitions_src.read_text())
        print("✅ Created tsuki-definitions.lua")

    # Create .luarc.json
    luarc_file = current_dir / ".luarc.json"
    luarc_config = {{
        "Lua.runtime.version": "Lua 5.4",
        "Lua.diagnostics.globals": ["tsuki"],
        "Lua.workspace.checkThirdParty": False,
        "Lua.workspace.library": ["tsuki-definitions.lua"]
    }}

    if luarc_file.exists():
        response = input(".luarc.json already exists. Overwrite? (y/N): ")
        if response.lower() != 'y':
            print("Skipped .luarc.json")
        else:
            with open(luarc_file, 'w') as f:
                json.dump(luarc_config, f, indent=4)
            print("✅ Updated .luarc.json")
    else:
        with open(luarc_file, 'w') as f:
            json.dump(luarc_config, f, indent=4)
        print("✅ Created .luarc.json")

    print("\\n🎉 IntelliSense setup complete!")
    print("📝 Make sure you have the 'Lua' extension installed in VSCode")
    print("🔄 Restart VSCode to activate IntelliSense")

if __name__ == "__main__":
    setup_intellisense()
'''
    setup_script.write_text(setup_content)
    setup_script.chmod(0o755)

    return package_dir

def create_archives(package_dir: Path, output_dir: Path, package_name: str):
    """Create tar.gz and zip archives of the package."""
    print("🗜️  Creating archives...")

    # Create tar.gz
    tar_path = output_dir / f"{package_name}.tar.gz"
    with tarfile.open(tar_path, "w:gz") as tar:
        tar.add(package_dir, arcname=package_name)
    print(f"✅ Created {tar_path}")

    # Create zip
    zip_path = output_dir / f"{package_name}.zip"
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zip_file:
        for file_path in package_dir.rglob("*"):
            if file_path.is_file():
                arcname = package_name / file_path.relative_to(package_dir)
                zip_file.write(file_path, arcname)
    print(f"✅ Created {zip_path}")

    return tar_path, zip_path

def main():
    parser = argparse.ArgumentParser(description="Prepare Tsuki Engine release")
    parser.add_argument("--version", "-v", help="Version string (default: auto-detect from git)")
    parser.add_argument("--output", "-o", default="releases", help="Output directory (default: releases)")
    parser.add_argument("--clean", action="store_true", help="Clean output directory first")

    args = parser.parse_args()

    # Get project paths
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    output_dir = project_root / args.output

    # Clean output directory if requested
    if args.clean and output_dir.exists():
        print(f"🧹 Cleaning {output_dir}")
        shutil.rmtree(output_dir)

    output_dir.mkdir(parents=True, exist_ok=True)

    # Determine version
    git_info = get_git_info()
    if args.version:
        version = args.version
    elif git_info['tag']:
        version = git_info['tag']
    else:
        version = f"dev-{git_info['commit']}"

    print(f"🏷️  Preparing release: {version}")
    print(f"📂 Output directory: {output_dir}")

    # Create IntelliSense package
    package_dir = create_intellisense_package(project_root, output_dir, version)

    # Create archives
    package_name = package_dir.name
    tar_path, zip_path = create_archives(package_dir, output_dir, package_name)

    # Create release info
    release_info = output_dir / "RELEASE_INFO.md"
    import datetime
    timestamp = datetime.datetime.now().isoformat()
    release_content = f"""# Tsuki Engine Release {version}

## IntelliSense Package

This release includes Lua IntelliSense definitions for VSCode.

### Files

- `{tar_path.name}` - Linux/macOS archive
- `{zip_path.name}` - Windows archive

### Quick Setup

1. Download and extract the archive
2. Run `python3 setup.py` in your game project directory
3. Install "Lua" extension in VSCode
4. Restart VSCode

### Manual Setup

1. Copy `tsuki-definitions.lua` to your project
2. Copy `example-.luarc.json` to `.luarc.json` in your project
3. Restart VSCode

## Build Information

- Version: {version}
- Commit: {git_info['commit']}
- Branch: {git_info['branch']}
- Generated: {timestamp}

## Assets for GitHub Release

Upload these files to your GitHub release:

- `{tar_path.name}`
- `{zip_path.name}`

"""
    release_info.write_text(release_content)

    print(f"\\n🎉 Release preparation complete!")
    print(f"📋 Release info: {release_info}")
    print(f"📦 Packages created in: {output_dir}")
    print(f"🚀 Ready to upload to GitHub releases!")

if __name__ == "__main__":
    main()