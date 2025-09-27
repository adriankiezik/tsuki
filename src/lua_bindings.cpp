#include "tsuki/lua_bindings.hpp"
#include "tsuki/tsuki.hpp"
#include <iostream>

namespace tsuki {

Engine* LuaBindings::engine_instance = nullptr;

void LuaBindings::registerAll(lua_State* L, Engine* engine) {
    engine_instance = engine;

    // Create main tsuki table
    lua_newtable(L);
    lua_setglobal(L, "tsuki");

    // Register all modules
    registerGraphics(L);
    registerKeyboard(L);
    registerMouse(L);
    registerWindow(L);
}

void LuaBindings::registerGraphics(lua_State* L) {
    lua_getglobal(L, "tsuki");

    // Create graphics table
    lua_newtable(L);

    // Add graphics functions
    setFunction(L, "clear", graphics_clear);
    setFunction(L, "present", graphics_present);
    setFunction(L, "setColor", graphics_setColor);
    setFunction(L, "rectangle", graphics_rectangle);
    setFunction(L, "circle", graphics_circle);
    setFunction(L, "line", graphics_line);
    setFunction(L, "print", graphics_print);

    // Set graphics table
    lua_setfield(L, -2, "graphics");
    lua_pop(L, 1); // Remove tsuki table
}

void LuaBindings::registerKeyboard(lua_State* L) {
    lua_getglobal(L, "tsuki");

    // Create keyboard table
    lua_newtable(L);

    setFunction(L, "isDown", keyboard_isDown);

    lua_setfield(L, -2, "keyboard");
    lua_pop(L, 1);
}

void LuaBindings::registerMouse(lua_State* L) {
    lua_getglobal(L, "tsuki");

    // Create mouse table
    lua_newtable(L);

    setFunction(L, "getPosition", mouse_getPosition);
    setFunction(L, "isDown", mouse_isDown);

    lua_setfield(L, -2, "mouse");
    lua_pop(L, 1);
}

void LuaBindings::registerWindow(lua_State* L) {
    lua_getglobal(L, "tsuki");

    // Create window table
    lua_newtable(L);

    setFunction(L, "getWidth", window_getWidth);
    setFunction(L, "getHeight", window_getHeight);
    setFunction(L, "setTitle", window_setTitle);

    lua_setfield(L, -2, "window");
    lua_pop(L, 1);
}

// Graphics functions
int LuaBindings::graphics_clear(lua_State* L) {
    if (!engine_instance) return 0;

    int args = lua_gettop(L);
    if (args >= 3) {
        // Clear with color
        float r = (float)luaL_checknumber(L, 1);
        float g = (float)luaL_checknumber(L, 2);
        float b = (float)luaL_checknumber(L, 3);
        float a = args >= 4 ? (float)luaL_checknumber(L, 4) : 1.0f;

        engine_instance->getGraphics().clear(Color(r, g, b, a));
    } else {
        engine_instance->getGraphics().clear();
    }

    return 0;
}

int LuaBindings::graphics_present(lua_State* L) {
    (void)L; // Suppress unused parameter warning
    if (!engine_instance) return 0;
    engine_instance->getGraphics().present();
    return 0;
}

int LuaBindings::graphics_setColor(lua_State* L) {
    if (!engine_instance) return 0;

    float r = (float)luaL_checknumber(L, 1);
    float g = (float)luaL_checknumber(L, 2);
    float b = (float)luaL_checknumber(L, 3);
    float a = lua_gettop(L) >= 4 ? (float)luaL_checknumber(L, 4) : 1.0f;

    engine_instance->getGraphics().setColor(Color(r, g, b, a));
    return 0;
}

int LuaBindings::graphics_rectangle(lua_State* L) {
    if (!engine_instance) return 0;

    const char* mode_str = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float width = (float)luaL_checknumber(L, 4);
    float height = (float)luaL_checknumber(L, 5);

    DrawMode mode = (strcmp(mode_str, "fill") == 0) ? DrawMode::Fill : DrawMode::Line;

    engine_instance->getGraphics().rectangle(mode, x, y, width, height);
    return 0;
}

int LuaBindings::graphics_circle(lua_State* L) {
    if (!engine_instance) return 0;

    const char* mode_str = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float radius = (float)luaL_checknumber(L, 4);

    DrawMode mode = (strcmp(mode_str, "fill") == 0) ? DrawMode::Fill : DrawMode::Line;

    engine_instance->getGraphics().circle(mode, x, y, radius);
    return 0;
}

int LuaBindings::graphics_line(lua_State* L) {
    if (!engine_instance) return 0;

    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float x2 = (float)luaL_checknumber(L, 3);
    float y2 = (float)luaL_checknumber(L, 4);

    engine_instance->getGraphics().line(x1, y1, x2, y2);
    return 0;
}

int LuaBindings::graphics_print(lua_State* L) {
    if (!engine_instance) return 0;

    const char* text = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    engine_instance->getGraphics().print(text, x, y);
    return 0;
}

// Keyboard functions
int LuaBindings::keyboard_isDown(lua_State* L) {
    if (!engine_instance) return 0;

    const char* key_name = luaL_checkstring(L, 1);

    // Simple key mapping (extend as needed)
    KeyCode key = KeyCode::Space;
    if (strcmp(key_name, "space") == 0) key = KeyCode::Space;
    else if (strcmp(key_name, "left") == 0) key = KeyCode::Left;
    else if (strcmp(key_name, "right") == 0) key = KeyCode::Right;
    else if (strcmp(key_name, "up") == 0) key = KeyCode::Up;
    else if (strcmp(key_name, "down") == 0) key = KeyCode::Down;
    else if (strcmp(key_name, "escape") == 0) key = KeyCode::Escape;

    bool is_down = engine_instance->getKeyboard().isDown(key);
    lua_pushboolean(L, is_down);
    return 1;
}

// Mouse functions
int LuaBindings::mouse_getPosition(lua_State* L) {
    if (!engine_instance) return 0;

    int x = engine_instance->getMouse().getX();
    int y = engine_instance->getMouse().getY();

    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

int LuaBindings::mouse_isDown(lua_State* L) {
    if (!engine_instance) return 0;

    int button = (int)luaL_checkinteger(L, 1);
    bool is_down = engine_instance->getMouse().isDown(static_cast<MouseButton>(button));

    lua_pushboolean(L, is_down);
    return 1;
}

// Window functions
int LuaBindings::window_getWidth(lua_State* L) {
    if (!engine_instance) return 0;

    int width = engine_instance->getWindow().getWidth();
    lua_pushinteger(L, width);
    return 1;
}

int LuaBindings::window_getHeight(lua_State* L) {
    if (!engine_instance) return 0;

    int height = engine_instance->getWindow().getHeight();
    lua_pushinteger(L, height);
    return 1;
}

int LuaBindings::window_setTitle(lua_State* L) {
    if (!engine_instance) return 0;

    const char* title = luaL_checkstring(L, 1);
    engine_instance->getWindow().setTitle(title);
    return 0;
}

// Utility functions
void LuaBindings::setFunction(lua_State* L, const char* name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
}

} // namespace tsuki