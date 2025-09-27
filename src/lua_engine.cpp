#include "tsuki/lua_engine.hpp"
#include <iostream>

namespace tsuki {

LuaEngine::LuaEngine() : L(nullptr) {
}

LuaEngine::~LuaEngine() {
    shutdown();
}

bool LuaEngine::init() {
    // Create new Lua state
    L = luaL_newstate();
    if (!L) {
        setError("Failed to create Lua state");
        return false;
    }

    // Load standard libraries
    luaL_openlibs(L);

    return true;
}

void LuaEngine::shutdown() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

bool LuaEngine::loadFile(const std::string& filename) {
    if (!L) {
        setError("Lua state not initialized");
        return false;
    }

    int result = luaL_loadfile(L, filename.c_str());
    if (result != LUA_OK) {
        setError(lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message from stack
        return false;
    }

    return true;
}

bool LuaEngine::loadString(const std::string& code) {
    if (!L) {
        setError("Lua state not initialized");
        return false;
    }

    int result = luaL_loadstring(L, code.c_str());
    if (result != LUA_OK) {
        setError(lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message from stack
        return false;
    }

    return true;
}

bool LuaEngine::executeFile(const std::string& filename) {
    if (!loadFile(filename)) {
        return false;
    }

    // Execute the loaded chunk
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        setError(lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message from stack
        return false;
    }

    return true;
}

bool LuaEngine::executeString(const std::string& code) {
    if (!loadString(code)) {
        return false;
    }

    // Execute the loaded chunk
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        setError(lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message from stack
        return false;
    }

    return true;
}

bool LuaEngine::callLoad() {
    return callLuaFunction("tsuki.load");
}

bool LuaEngine::callUpdate(double dt) {
    lua_getglobal(L, "tsuki");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return true; // No tsuki table, that's OK
    }

    lua_getfield(L, -1, "update");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2); // Remove nil and tsuki table
        return true; // No update function, that's OK
    }

    lua_pushnumber(L, dt);
    return callLuaFunction("", 1, 0); // Function is already on stack
}

bool LuaEngine::callDraw() {
    return callLuaFunction("tsuki.draw");
}

bool LuaEngine::callFunction(const std::string& function_name) {
    return callLuaFunction(function_name);
}

void LuaEngine::setError(const std::string& error) {
    last_error_ = error;
    std::cerr << "Lua Error: " << error << std::endl;
}

bool LuaEngine::callLuaFunction(const std::string& function_name, int args, int results) {
    if (!L) {
        setError("Lua state not initialized");
        return false;
    }

    // If function_name is empty, assume function is already on stack
    if (!function_name.empty()) {
        // Split function name by dots and navigate the tables
        std::string remaining = function_name;

        // Get the first part (should be the table name)
        size_t dot_pos = remaining.find('.');
        if (dot_pos != std::string::npos) {
            std::string table_name = remaining.substr(0, dot_pos);
            lua_getglobal(L, table_name.c_str());

            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                return true; // No table, that's OK
            }

            remaining = remaining.substr(dot_pos + 1);
            lua_getfield(L, -1, remaining.c_str());

            if (lua_isnil(L, -1)) {
                lua_pop(L, 2); // Remove nil and table
                return true; // No function, that's OK
            }

            lua_remove(L, -2); // Remove table, keep function
        } else {
            lua_getglobal(L, function_name.c_str());
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                return true; // No function, that's OK
            }
        }
    }

    // Call the function
    int result = lua_pcall(L, args, results, 0);
    if (result != LUA_OK) {
        setError(lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message from stack
        return false;
    }

    return true;
}

} // namespace tsuki