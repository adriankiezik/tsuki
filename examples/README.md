# Tsuki Examples

This directory contains example projects demonstrating various Tsuki features.

## Setting up IntelliSense

To enable Lua IntelliSense in examples, run from each example directory:

```bash
cd examples/starter
tsuki generate-definitions
```

This will create:
- `tsuki-definitions.lua` - Lua type definitions
- `.luarc.json` - VSCode Lua LSP configuration

These files are gitignored and generated locally on your machine.

## Examples

- **starter/** - Basic game template with player movement
- **image_demo/** - Image loading and rendering demo
- **keytest/** - Keyboard input testing
- **problemkeys/** - Edge case keyboard handling
- **sidescroller/** - Side-scrolling game example
- **simple_image_test/** - Minimal image loading test

## Running Examples

From the project root:
```bash
./build/tsuki examples/starter/main.lua
```

Or from an example directory:
```bash
cd examples/starter
../../build/tsuki main.lua
```