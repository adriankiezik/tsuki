#!/usr/bin/env python3
"""
Tsuki - Lua Definitions Generator
Automatically generates Lua type definitions from C++ binding code.
"""

import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Optional

class LuaFunction:
    def __init__(self, name: str, module: str = ""):
        self.name = name
        self.module = module
        self.params: List[Tuple[str, str]] = []  # (name, type)
        self.returns: List[str] = []
        self.description = ""

    def add_param(self, name: str, param_type: str):
        self.params.append((name, param_type))

    def add_return(self, return_type: str):
        self.returns.append(return_type)

class LuaDefinitionGenerator:
    def __init__(self, cpp_file_path: str):
        self.cpp_file_path = cpp_file_path
        self.functions: Dict[str, LuaFunction] = {}
        self.modules: Dict[str, List[str]] = {}  # module -> functions

        # Type mappings from C++ to Lua
        self.type_mapping = {
            'float': 'number',
            'double': 'number',
            'int': 'integer',
            'bool': 'boolean',
            'const char*': 'string',
            'char*': 'string',
            'string': 'string',
            'void': 'nil'
        }

    def parse_cpp_file(self):
        """Parse the C++ lua_bindings.cpp file to extract function information."""
        with open(self.cpp_file_path, 'r') as f:
            content = f.read()

        # Find all setFunction calls to identify Lua functions
        set_function_pattern = r'setFunction\(L,\s*"([^"]+)",\s*([^)]+)\);'
        matches = re.findall(set_function_pattern, content)

        current_module = ""

        # Parse registerXXX functions to determine modules
        register_pattern = r'void\s+LuaBindings::register(\w+)\(lua_State\*\s*L\)'
        register_matches = re.findall(register_pattern, content)

        for match in matches:
            func_name, cpp_func_name = match

            # Determine module from context
            module = self._determine_module(content, cpp_func_name)
            full_name = f"{module}.{func_name}" if module else func_name

            func = LuaFunction(func_name, module)

            # Parse the C++ function to extract parameter and return types
            self._parse_cpp_function(content, cpp_func_name, func)

            self.functions[full_name] = func

            if module not in self.modules:
                self.modules[module] = []
            self.modules[module].append(func_name)

    def _determine_module(self, content: str, cpp_func_name: str) -> str:
        """Determine which module a function belongs to based on its name."""
        if 'graphics_' in cpp_func_name:
            return 'graphics'
        elif 'keyboard_' in cpp_func_name:
            return 'keyboard'
        elif 'mouse_' in cpp_func_name:
            return 'mouse'
        elif 'window_' in cpp_func_name:
            return 'window'
        elif 'debug_' in cpp_func_name:
            return 'debug'
        return ""

    def _parse_cpp_function(self, content: str, cpp_func_name: str, func: LuaFunction):
        """Parse a C++ function to extract parameter and return information."""
        # Find the function definition
        func_pattern = rf'int\s+LuaBindings::{cpp_func_name}\(lua_State\*\s*L\)\s*\{{([^}}]+)}}'
        match = re.search(func_pattern, content, re.DOTALL)

        if not match:
            return

        func_body = match.group(1)

        # Parse luaL_checknumber, luaL_checkstring, etc. to determine parameters
        check_patterns = [
            (r'luaL_checknumber\(L,\s*(\d+)\)', 'number'),
            (r'luaL_checkstring\(L,\s*(\d+)\)', 'string'),
            (r'luaL_checkinteger\(L,\s*(\d+)\)', 'integer'),
            (r'luaL_optstring\(L,\s*(\d+)', 'string?'),
            (r'luaL_optnumber\(L,\s*(\d+)', 'number?'),
            (r'luaL_optinteger\(L,\s*(\d+)', 'integer?'),
        ]

        # First collect all parameters with their indices
        param_indices = {}
        for pattern, param_type in check_patterns:
            matches = re.findall(pattern, func_body)
            for match in matches:
                param_index = int(match)
                param_indices[param_index] = param_type

        # Add parameters in correct order using predefined names
        for i in sorted(param_indices.keys()):
            param_name = self._generate_param_name(cpp_func_name, i)
            param_type = param_indices[i]
            func.add_param(param_name, param_type)

        # Parse return statements to determine return types
        if 'lua_pushboolean' in func_body:
            func.add_return('boolean')
        elif 'lua_pushinteger' in func_body:
            func.add_return('integer')
        elif 'lua_pushnumber' in func_body:
            func.add_return('number')
        elif 'lua_pushstring' in func_body:
            func.add_return('string')
        elif 'return 2' in func_body:
            # Special case for functions returning multiple values
            func.returns = ['number', 'number']
        elif 'return 0' in func_body:
            pass  # void return

        # Add descriptions based on function names
        func.description = self._generate_description(cpp_func_name)

        # Special handling for flexible functions
        if cpp_func_name == 'graphics_print':
            func.params = [('text', 'any'), ('x', 'number?'), ('y', 'number?'), ('align', 'string?')]
        elif cpp_func_name == 'graphics_printAligned':
            func.params = [('text', 'any'), ('x', 'number'), ('y', 'number'), ('width', 'number'), ('height', 'number'), ('align', 'string?')]
        elif cpp_func_name == 'graphics_getTextSize':
            func.returns = ['number', 'number']  # width, height

    def _generate_param_name(self, cpp_func_name: str, index: int) -> str:
        """Generate meaningful parameter names based on function name and index."""
        param_names = {
            'graphics_clear': ['r', 'g', 'b', 'a'],
            'graphics_setColor': ['r', 'g', 'b', 'a'],
            'graphics_rectangle': ['mode', 'x', 'y', 'width', 'height'],
            'graphics_circle': ['mode', 'x', 'y', 'radius'],
            'graphics_line': ['x1', 'y1', 'x2', 'y2'],
            'graphics_print': ['text', 'x', 'y', 'align'],
            'graphics_printAligned': ['text', 'x', 'y', 'width', 'height', 'align'],
            'graphics_getTextSize': ['text'],
            'graphics_loadFont': ['name', 'filename', 'size'],
            'graphics_setFont': ['name'],
            'keyboard_isDown': ['key'],
            'mouse_isDown': ['button'],
            'window_setTitle': ['title'],
            'debug_stackTrace': ['msg'],
            'debug_getInfo': ['what', 'level'],
            'debug_prettyInfo': ['level'],
        }

        if cpp_func_name in param_names and index <= len(param_names[cpp_func_name]):
            return param_names[cpp_func_name][index - 1]

        return f"param{index}"

    def _generate_description(self, cpp_func_name: str) -> str:
        """Generate descriptions for functions."""
        descriptions = {
            'graphics_clear': 'Clear the screen with optional color',
            'graphics_present': 'Present the rendered frame to the screen',
            'graphics_setColor': 'Set the current drawing color',
            'graphics_rectangle': 'Draw a rectangle',
            'graphics_circle': 'Draw a circle',
            'graphics_line': 'Draw a line',
            'graphics_print': 'Print text to screen with optional alignment',
            'graphics_printAligned': 'Print text within a bounded rectangle with alignment',
            'graphics_getTextSize': 'Get the width and height of text in pixels',
            'graphics_loadFont': 'Load a font from file with specified size',
            'graphics_setFont': 'Set the current font for text rendering',
            'keyboard_isDown': 'Check if a key is currently pressed',
            'mouse_getPosition': 'Get current mouse position',
            'mouse_isDown': 'Check if a mouse button is currently pressed',
            'window_getWidth': 'Get window width',
            'window_getHeight': 'Get window height',
            'window_setTitle': 'Set window title',
            'debug_stackTrace': 'Get stack trace with optional message',
            'debug_printStack': 'Print current Lua stack to console',
            'debug_getInfo': 'Get debug information about a function',
            'debug_prettyInfo': 'Print formatted debug information',
        }

        return descriptions.get(cpp_func_name, "")

    def generate_definitions(self) -> str:
        """Generate the complete Lua definitions file."""
        output = []
        output.append("-- Tsuki Lua API Definitions")
        output.append("-- Auto-generated from C++ binding code")
        output.append("-- Place this file in your project root and add it to .luarc.json workspace.library")
        output.append("")
        output.append("---@meta tsuki")
        output.append("")

        # Generate main tsuki class
        output.append("---@class tsuki")
        for module in sorted(self.modules.keys()):
            if module:
                module_descriptions = {
                    'debug': 'Debug utilities and stack inspection',
                    'graphics': 'Drawing and rendering functions',
                    'keyboard': 'Keyboard input handling',
                    'mouse': 'Mouse input handling',
                    'window': 'Window management functions'
                }
                desc = module_descriptions.get(module, f"{module.title()} module")
                output.append(f"---{desc}")
                output.append(f"---@field {module} tsuki_{module}")
        output.append("---Called once when the game starts")
        output.append("---@field load fun()")
        output.append("---Called every frame for game logic updates")
        output.append("---@field update fun(dt: number)")
        output.append("---Called every frame for rendering")
        output.append("---@field draw fun()")
        output.append("tsuki = {}")
        output.append("")

        # Generate module classes
        for module in sorted(self.modules.keys()):
            if not module:
                continue

            output.append(f"---@class tsuki_{module}")

            for func_name in self.modules[module]:
                full_name = f"{module}.{func_name}"
                if full_name in self.functions:
                    func = self.functions[full_name]

                    # Generate parameter string
                    params = []
                    for param_name, param_type in func.params:
                        params.append(f"{param_name}: {param_type}")

                    # Generate return type
                    if func.returns:
                        return_type = ", ".join(func.returns)
                    else:
                        return_type = "nil"

                    param_str = ", ".join(params)

                    # Add description if available
                    if func.description:
                        output.append(f"---{func.description}")

                    output.append(f"---@field {func_name} fun({param_str}): {return_type}")

            output.append("")

        # Generate global module aliases for convenience API
        output.append("-- Global module aliases for convenience")
        for module in sorted(self.modules.keys()):
            if module:
                output.append(f"---@type tsuki_{module}")
                output.append(f"{module} = {{}}")
        output.append("")

        return "\n".join(output)

    def save_definitions(self, output_path: str):
        """Save the generated definitions to a file."""
        definitions = self.generate_definitions()
        with open(output_path, 'w') as f:
            f.write(definitions)
        print(f"Generated Lua definitions: {output_path}")

    def generate_readme(self) -> str:
        """Generate README for the distribution package."""
        return """# Tsuki - Lua IntelliSense Package

This package provides VSCode IntelliSense support for Tsuki Lua scripting.

## Quick Setup

1. **Install VSCode Extension**: "Lua" by sumneko
2. **Copy files**: Place `tsuki-definitions.lua` in your project root
3. **Create config**: Add `.luarc.json` to your project root:

```json
{
    "Lua.runtime.version": "Lua 5.4",
    "Lua.diagnostics.globals": ["tsuki"],
    "Lua.workspace.checkThirdParty": false,
    "Lua.workspace.library": ["tsuki-definitions.lua"]
}
```

4. **Restart VSCode**: Reload window for changes to take effect

## Features

- ✅ Full autocomplete for all Tsuki functions
- ✅ Parameter hints with type information
- ✅ Documentation on hover
- ✅ Type checking and error detection

## API Overview

The Tsuki provides these main modules:

- `tsuki.graphics.*` - Drawing and rendering functions
- `tsuki.keyboard.*` - Keyboard input handling
- `tsuki.mouse.*` - Mouse input handling
- `tsuki.window.*` - Window management
- `tsuki.debug.*` - Debug utilities
- Lifecycle callbacks: `tsuki.load()`, `tsuki.update(dt)`, `tsuki.draw()`

## Example Project Structure

```
my-game/
├── main.lua              # Your game code
├── tsuki-definitions.lua # This file
└── .luarc.json          # VSCode configuration
```

## Troubleshooting

- **No autocomplete**: Check Lua extension is installed and enabled
- **Duplicate functions**: Ensure only one `.luarc.json` exists
- **Missing types**: Restart VSCode after configuration changes

For more help, visit: https://github.com/your-username/tsuki
"""

    def create_dist_package(self, dist_dir: Path):
        """Create complete distribution package."""
        dist_dir.mkdir(exist_ok=True)

        # Generate main definitions file
        definitions_file = dist_dir / "tsuki-definitions.lua"
        self.save_definitions(str(definitions_file))

        # Generate README
        readme_file = dist_dir / "README.md"
        with open(readme_file, 'w') as f:
            f.write(self.generate_readme())
        print(f"Generated README: {readme_file}")

        # Generate example .luarc.json
        luarc_file = dist_dir / "example-.luarc.json"
        luarc_content = """{
    "Lua.runtime.version": "Lua 5.4",
    "Lua.diagnostics.globals": ["tsuki"],
    "Lua.workspace.checkThirdParty": false,
    "Lua.workspace.library": ["tsuki-definitions.lua"]
}"""
        with open(luarc_file, 'w') as f:
            f.write(luarc_content)
        print(f"Generated example config: {luarc_file}")

        # Generate version info
        version_file = dist_dir / "VERSION"
        with open(version_file, 'w') as f:
            import datetime
            f.write(f"Generated: {datetime.datetime.now().isoformat()}\n")
            f.write("Package: Tsuki Lua IntelliSense\n")
            f.write("Source: Auto-generated from C++ bindings\n")
        print(f"Generated version info: {version_file}")

def main():
    # Get paths
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    cpp_file = project_root / "src" / "lua_bindings.cpp"
    dist_dir = project_root / "dist"

    if not cpp_file.exists():
        print(f"Error: Could not find {cpp_file}")
        sys.exit(1)

    # Generate definitions
    generator = LuaDefinitionGenerator(str(cpp_file))
    generator.parse_cpp_file()
    generator.create_dist_package(dist_dir)

    print("✅ Lua IntelliSense package generated successfully!")
    print(f"📦 Distribution: {dist_dir}")
    print("🚀 Ready for GitHub releases!")

if __name__ == "__main__":
    main()