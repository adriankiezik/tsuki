#include "tsuki/lua_bindings.hpp"
#include "tsuki/tsuki.hpp"
#include "tsuki/debug_utils.hpp"
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <string>
#include <algorithm>

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

    // Also register modules globally for convenience
    registerModulesGlobally(L);
}

void LuaBindings::registerGraphics(lua_State* L) {
    lua_getglobal(L, "tsuki");

    // Create graphics table
    lua_newtable(L);

    // Add graphics functions
    setFunction(L, "clear", graphics_clear);
    setFunction(L, "setColor", graphics_setColor);
    setFunction(L, "rectangle", graphics_rectangle);
    setFunction(L, "circle", graphics_circle);
    setFunction(L, "line", graphics_line);
    setFunction(L, "print", graphics_print);
    setFunction(L, "printAligned", graphics_printAligned);
    setFunction(L, "getTextSize", graphics_getTextSize);
    setFunction(L, "loadFont", graphics_loadFont);
    setFunction(L, "setFont", graphics_setFont);

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

    int argc = lua_gettop(L);
    std::string text;
    float x = 0.0f, y = 0.0f;

    // Handle variable arguments
    if (argc >= 1) {
        // Convert any type to string
        if (lua_isstring(L, 1)) {
            text = lua_tostring(L, 1);
        } else if (lua_isnumber(L, 1)) {
            text = std::to_string(lua_tonumber(L, 1));
        } else if (lua_isboolean(L, 1)) {
            text = lua_toboolean(L, 1) ? "true" : "false";
        } else if (lua_isnil(L, 1)) {
            text = "nil";
        } else {
            text = "unknown";
        }
    }

    if (argc >= 2) {
        x = (float)luaL_checknumber(L, 2);
    }

    if (argc >= 3) {
        y = (float)luaL_checknumber(L, 3);
    }

    // Check for alignment parameter
    if (argc >= 4) {
        const char* align = luaL_checkstring(L, 4);
        engine_instance->getGraphics().print(text, x, y, align);
    } else {
        engine_instance->getGraphics().print(text, x, y);
    }
    return 0;
}

int LuaBindings::graphics_printAligned(lua_State* L) {
    if (!engine_instance) return 0;

    int argc = lua_gettop(L);
    if (argc < 5) {
        luaL_error(L, "printAligned requires at least 5 arguments: text, x, y, width, height");
        return 0;
    }

    // Get required parameters
    std::string text;
    if (lua_isstring(L, 1)) {
        text = lua_tostring(L, 1);
    } else if (lua_isnumber(L, 1)) {
        text = std::to_string(lua_tonumber(L, 1));
    } else if (lua_isboolean(L, 1)) {
        text = lua_toboolean(L, 1) ? "true" : "false";
    } else if (lua_isnil(L, 1)) {
        text = "nil";
    } else {
        text = "unknown";
    }

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float width = (float)luaL_checknumber(L, 4);
    float height = (float)luaL_checknumber(L, 5);

    // Check for alignment parameter
    if (argc >= 6) {
        const char* align = luaL_checkstring(L, 6);
        engine_instance->getGraphics().printAligned(text, x, y, width, height, align);
    } else {
        engine_instance->getGraphics().printAligned(text, x, y, width, height);
    }

    return 0;
}

int LuaBindings::graphics_getTextSize(lua_State* L) {
    if (!engine_instance) return 0;

    const char* text = luaL_checkstring(L, 1);
    auto [width, height] = engine_instance->getGraphics().getTextSize(text);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2; // Return two values: width, height
}

int LuaBindings::graphics_loadFont(lua_State* L) {
    if (!engine_instance) return 0;

    const char* name = luaL_checkstring(L, 1);
    const char* filename = luaL_checkstring(L, 2);
    float size = 20.0f;

    if (lua_gettop(L) >= 3) {
        size = (float)luaL_checknumber(L, 3);
    }

    bool success = engine_instance->getGraphics().loadFont(name, filename, size);
    lua_pushboolean(L, success);
    return 1;
}

int LuaBindings::graphics_setFont(lua_State* L) {
    if (!engine_instance) return 0;

    const char* name = luaL_checkstring(L, 1);
    bool success = engine_instance->getGraphics().setFont(name);
    lua_pushboolean(L, success);
    return 1;
}

// Industry-standard string-to-scancode mapping (LÖVE2D/Pygame approach)
// Avoids broken SDL_GetScancodeFromName() and uses stable SDL scancode constants
static SDL_Scancode getScancodeFromString(const std::string& input) {
    // Convert to lowercase for case-insensitive lookup
    std::string key = input;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    // Direct mapping to stable SDL_SCANCODE constants
    static const std::unordered_map<std::string, SDL_Scancode> scancode_map = {
        // Letters
        {"a", SDL_SCANCODE_A}, {"b", SDL_SCANCODE_B}, {"c", SDL_SCANCODE_C}, {"d", SDL_SCANCODE_D},
        {"e", SDL_SCANCODE_E}, {"f", SDL_SCANCODE_F}, {"g", SDL_SCANCODE_G}, {"h", SDL_SCANCODE_H},
        {"i", SDL_SCANCODE_I}, {"j", SDL_SCANCODE_J}, {"k", SDL_SCANCODE_K}, {"l", SDL_SCANCODE_L},
        {"m", SDL_SCANCODE_M}, {"n", SDL_SCANCODE_N}, {"o", SDL_SCANCODE_O}, {"p", SDL_SCANCODE_P},
        {"q", SDL_SCANCODE_Q}, {"r", SDL_SCANCODE_R}, {"s", SDL_SCANCODE_S}, {"t", SDL_SCANCODE_T},
        {"u", SDL_SCANCODE_U}, {"v", SDL_SCANCODE_V}, {"w", SDL_SCANCODE_W}, {"x", SDL_SCANCODE_X},
        {"y", SDL_SCANCODE_Y}, {"z", SDL_SCANCODE_Z},

        // Numbers
        {"0", SDL_SCANCODE_0}, {"1", SDL_SCANCODE_1}, {"2", SDL_SCANCODE_2}, {"3", SDL_SCANCODE_3},
        {"4", SDL_SCANCODE_4}, {"5", SDL_SCANCODE_5}, {"6", SDL_SCANCODE_6}, {"7", SDL_SCANCODE_7},
        {"8", SDL_SCANCODE_8}, {"9", SDL_SCANCODE_9},

        // Problem keys specifically mentioned by user
        {"enter", SDL_SCANCODE_RETURN}, {"return", SDL_SCANCODE_RETURN},
        {"backslash", SDL_SCANCODE_BACKSLASH}, {"\\", SDL_SCANCODE_BACKSLASH},
        {"tilde", SDL_SCANCODE_GRAVE}, {"~", SDL_SCANCODE_GRAVE}, {"grave", SDL_SCANCODE_GRAVE}, {"`", SDL_SCANCODE_GRAVE},
        {"minus", SDL_SCANCODE_MINUS}, {"-", SDL_SCANCODE_MINUS},
        {"equals", SDL_SCANCODE_EQUALS}, {"=", SDL_SCANCODE_EQUALS},
        {"caps", SDL_SCANCODE_CAPSLOCK}, {"capslock", SDL_SCANCODE_CAPSLOCK},

        // Other symbol keys
        {"semicolon", SDL_SCANCODE_SEMICOLON}, {";", SDL_SCANCODE_SEMICOLON},
        {"apostrophe", SDL_SCANCODE_APOSTROPHE}, {"'", SDL_SCANCODE_APOSTROPHE},
        {"comma", SDL_SCANCODE_COMMA}, {",", SDL_SCANCODE_COMMA},
        {"period", SDL_SCANCODE_PERIOD}, {".", SDL_SCANCODE_PERIOD},
        {"slash", SDL_SCANCODE_SLASH}, {"/", SDL_SCANCODE_SLASH},
        {"leftbracket", SDL_SCANCODE_LEFTBRACKET}, {"[", SDL_SCANCODE_LEFTBRACKET},
        {"rightbracket", SDL_SCANCODE_RIGHTBRACKET}, {"]", SDL_SCANCODE_RIGHTBRACKET},

        // Arrow keys
        {"up", SDL_SCANCODE_UP}, {"down", SDL_SCANCODE_DOWN},
        {"left", SDL_SCANCODE_LEFT}, {"right", SDL_SCANCODE_RIGHT},

        // Function keys
        {"f1", SDL_SCANCODE_F1}, {"f2", SDL_SCANCODE_F2}, {"f3", SDL_SCANCODE_F3}, {"f4", SDL_SCANCODE_F4},
        {"f5", SDL_SCANCODE_F5}, {"f6", SDL_SCANCODE_F6}, {"f7", SDL_SCANCODE_F7}, {"f8", SDL_SCANCODE_F8},
        {"f9", SDL_SCANCODE_F9}, {"f10", SDL_SCANCODE_F10}, {"f11", SDL_SCANCODE_F11}, {"f12", SDL_SCANCODE_F12},

        // Special keys
        {"space", SDL_SCANCODE_SPACE}, {"escape", SDL_SCANCODE_ESCAPE}, {"esc", SDL_SCANCODE_ESCAPE},
        {"tab", SDL_SCANCODE_TAB}, {"backspace", SDL_SCANCODE_BACKSPACE},
        {"delete", SDL_SCANCODE_DELETE}, {"del", SDL_SCANCODE_DELETE},
        {"insert", SDL_SCANCODE_INSERT}, {"ins", SDL_SCANCODE_INSERT},
        {"home", SDL_SCANCODE_HOME}, {"end", SDL_SCANCODE_END},
        {"pageup", SDL_SCANCODE_PAGEUP}, {"pagedown", SDL_SCANCODE_PAGEDOWN},

        // Modifier keys
        {"shift", SDL_SCANCODE_LSHIFT}, {"lshift", SDL_SCANCODE_LSHIFT}, {"rshift", SDL_SCANCODE_RSHIFT},
        {"ctrl", SDL_SCANCODE_LCTRL}, {"lctrl", SDL_SCANCODE_LCTRL}, {"rctrl", SDL_SCANCODE_RCTRL},
        {"alt", SDL_SCANCODE_LALT}, {"lalt", SDL_SCANCODE_LALT}, {"ralt", SDL_SCANCODE_RALT},
        {"gui", SDL_SCANCODE_LGUI}, {"lgui", SDL_SCANCODE_LGUI}, {"rgui", SDL_SCANCODE_RGUI}
    };

    auto it = scancode_map.find(key);
    return (it != scancode_map.end()) ? it->second : SDL_SCANCODE_UNKNOWN;
}

// Keyboard functions
int LuaBindings::keyboard_isDown(lua_State* L) {
    if (!engine_instance) return 0;

    const char* key_name = luaL_checkstring(L, 1);

    // Use direct scancode mapping instead of broken SDL name conversion
    SDL_Scancode scancode = getScancodeFromString(std::string(key_name));
    KeyCode key = static_cast<KeyCode>(scancode);

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

// Register modules globally for convenience (graphics.print vs tsuki.graphics.print)
void LuaBindings::registerModulesGlobally(lua_State* L) {
    // Get the tsuki table
    lua_getglobal(L, "tsuki");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    // Copy graphics table to global
    lua_getfield(L, -1, "graphics");
    if (!lua_isnil(L, -1)) {
        lua_setglobal(L, "graphics");
    } else {
        lua_pop(L, 1);
    }

    // Copy keyboard table to global
    lua_getfield(L, -1, "keyboard");
    if (!lua_isnil(L, -1)) {
        lua_setglobal(L, "keyboard");
    } else {
        lua_pop(L, 1);
    }

    // Copy mouse table to global
    lua_getfield(L, -1, "mouse");
    if (!lua_isnil(L, -1)) {
        lua_setglobal(L, "mouse");
    } else {
        lua_pop(L, 1);
    }

    // Copy window table to global
    lua_getfield(L, -1, "window");
    if (!lua_isnil(L, -1)) {
        lua_setglobal(L, "window");
    } else {
        lua_pop(L, 1);
    }

    // Copy debug table to global
    lua_getfield(L, -1, "debug");
    if (!lua_isnil(L, -1)) {
        lua_setglobal(L, "debug");
    } else {
        lua_pop(L, 1);
    }

    // Pop the tsuki table
    lua_pop(L, 1);
}

// Utility functions
void LuaBindings::setFunction(lua_State* L, const char* name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
}

} // namespace tsuki