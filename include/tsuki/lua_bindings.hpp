#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace tsuki {

// Forward declarations
class Engine;

class LuaBindings {
public:
    static void registerAll(lua_State* L, Engine* engine);

private:
    static Engine* engine_instance;

    // Graphics bindings
    static void registerGraphics(lua_State* L);
    static int graphics_clear(lua_State* L);
    static int graphics_present(lua_State* L);
    static int graphics_setColor(lua_State* L);
    static int graphics_rectangle(lua_State* L);
    static int graphics_circle(lua_State* L);
    static int graphics_line(lua_State* L);
    static int graphics_print(lua_State* L);

    // Keyboard bindings
    static void registerKeyboard(lua_State* L);
    static int keyboard_isDown(lua_State* L);

    // Mouse bindings
    static void registerMouse(lua_State* L);
    static int mouse_getPosition(lua_State* L);
    static int mouse_isDown(lua_State* L);

    // Window bindings
    static void registerWindow(lua_State* L);
    static int window_getWidth(lua_State* L);
    static int window_getHeight(lua_State* L);
    static int window_setTitle(lua_State* L);

    // Debug bindings
    static void registerDebug(lua_State* L);
    static int debug_stackTrace(lua_State* L);
    static int debug_printStack(lua_State* L);
    static int debug_getInfo(lua_State* L);
    static int debug_prettyInfo(lua_State* L);

    // Utility functions
    static void createTable(lua_State* L, const char* name);
    static void setFunction(lua_State* L, const char* name, lua_CFunction func);
};

} // namespace tsuki