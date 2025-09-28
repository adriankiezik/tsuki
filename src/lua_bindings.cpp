#include "tsuki/lua_bindings.hpp"
#include "tsuki/tsuki.hpp"
#include "tsuki/debug_utils.hpp"
#include <iostream>
#include <cstring>

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
    registerDebug(L);
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

void LuaBindings::registerDebug(lua_State* L) {
    lua_getglobal(L, "tsuki");

    // Create debug table
    lua_newtable(L);

    setFunction(L, "stackTrace", debug_stackTrace);
    setFunction(L, "printStack", debug_printStack);
    setFunction(L, "getInfo", debug_getInfo);
    setFunction(L, "prettyInfo", debug_prettyInfo);

    lua_setfield(L, -2, "debug");
    lua_pop(L, 1);
}

// Debug functions
int LuaBindings::debug_stackTrace(lua_State* L) {
    // Get the message parameter if provided
    const char* msg = luaL_optstring(L, 1, "");

    lua_getglobal(L, "debug");
    if (!lua_isnil(L, -1)) {
        lua_getfield(L, -1, "traceback");
        if (lua_isfunction(L, -1)) {
            // Push the message if provided
            if (strlen(msg) > 0) {
                lua_pushstring(L, msg);
                lua_call(L, 1, 1);
            } else {
                lua_call(L, 0, 1);
            }

            if (lua_isstring(L, -1)) {
                const char* trace = lua_tostring(L, -1);
                // Copy the string to avoid memory issues
                std::string trace_copy(trace);
                lua_pop(L, 2); // Remove trace result and debug table
                lua_pushstring(L, trace_copy.c_str());
                return 1;
            }
            lua_pop(L, 1); // Remove non-string result
        }
        lua_pop(L, 1); // Remove debug table
    } else {
        lua_pop(L, 1); // Remove nil
    }

    lua_pushstring(L, "Stack trace not available");
    return 1;
}

int LuaBindings::debug_printStack(lua_State* L) {
    int top = lua_gettop(L);

    DebugPrinter::printHeader(fmt::format("📚 LUA STACK (size: {})", top), fmt::color::purple);

    for (int i = 1; i <= top; i++) {
        int type = lua_type(L, i);
        std::string typeName = lua_typename(L, type);
        std::string value;

        switch (type) {
            case LUA_TNUMBER:
                value = fmt::format("{}", lua_tonumber(L, i));
                break;
            case LUA_TSTRING:
                value = fmt::format("\"{}\"", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                value = lua_toboolean(L, i) ? "true" : "false";
                break;
            case LUA_TNIL:
                value = "nil";
                break;
            default:
                value = fmt::format("{:p}", lua_topointer(L, i));
                break;
        }

        DebugPrinter::printKeyValue(fmt::format("{} ({})", i, typeName), value, fmt::color::cyan, fmt::color::white);
    }

    fmt::print("\n");
    return 0;
}

int LuaBindings::debug_getInfo(lua_State* L) {
    const char* what = luaL_optstring(L, 1, "Slnf");
    int level = luaL_optinteger(L, 2, 1);

    lua_Debug ar;
    memset(&ar, 0, sizeof(ar)); // Initialize the structure

    if (lua_getstack(L, level, &ar)) {
        if (lua_getinfo(L, what, &ar)) {
            lua_newtable(L);

            // Source file
            if (ar.source && strlen(ar.source) > 0) {
                lua_pushstring(L, ar.source);
                lua_setfield(L, -2, "source");
            } else {
                lua_pushstring(L, "unknown");
                lua_setfield(L, -2, "source");
            }

            // Line number
            if (ar.currentline >= 0) {
                lua_pushinteger(L, ar.currentline);
                lua_setfield(L, -2, "line");
            } else {
                lua_pushinteger(L, -1);
                lua_setfield(L, -2, "line");
            }

            // Function name (be careful with memory)
            if (ar.name && strlen(ar.name) > 0) {
                lua_pushstring(L, ar.name);
                lua_setfield(L, -2, "name");
            } else {
                lua_pushstring(L, "anonymous");
                lua_setfield(L, -2, "name");
            }

            // Name type
            if (ar.namewhat && strlen(ar.namewhat) > 0) {
                lua_pushstring(L, ar.namewhat);
                lua_setfield(L, -2, "namewhat");
            } else {
                lua_pushstring(L, "");
                lua_setfield(L, -2, "namewhat");
            }

            // Function type
            if (ar.what && strlen(ar.what) > 0) {
                lua_pushstring(L, ar.what);
                lua_setfield(L, -2, "what");
            } else {
                lua_pushstring(L, "unknown");
                lua_setfield(L, -2, "what");
            }

            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

int LuaBindings::debug_prettyInfo(lua_State* L) {
    int level = luaL_optinteger(L, 1, 1);

    // Get function info
    lua_Debug ar;
    memset(&ar, 0, sizeof(ar));

    if (lua_getstack(L, level, &ar)) {
        if (lua_getinfo(L, "Slnf", &ar)) {
            // Compact one-line format: func@file:line
            std::string funcName = ar.name ? ar.name : "anonymous";
            std::string source = ar.source ? DebugPrinter::cleanSourcePath(ar.source) : "?";
            int line = ar.currentline >= 0 ? ar.currentline : 0;

            fmt::print(fg(fmt::color::cyan), "Debug: ");
            fmt::print(fg(fmt::color::white), "{}@{}:{}\n", funcName, source, line);

            // Get stack trace - only show Lua lines
            lua_getglobal(L, "debug");
            if (!lua_isnil(L, -1)) {
                lua_getfield(L, -1, "traceback");
                if (lua_isfunction(L, -1)) {
                    lua_call(L, 0, 1);
                    if (lua_isstring(L, -1)) {
                        DebugPrinter::printStackTrace(lua_tostring(L, -1));
                    }
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);
            } else {
                lua_pop(L, 1);
            }
        }
    }

    return 0;
}

// Utility functions
void LuaBindings::setFunction(lua_State* L, const char* name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
}

} // namespace tsuki