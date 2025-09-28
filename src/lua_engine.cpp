#include "tsuki/lua_engine.hpp"
#include "tsuki/debug_utils.hpp"
#include <iostream>
#include <fmt/format.h>

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
    return callLuaFunction("", "tsuki.update", 1, 0); // Function is already on stack, but show proper name in errors
}

bool LuaEngine::callDraw() {
    return callLuaFunction("tsuki.draw");
}

bool LuaEngine::callFunction(const std::string& function_name) {
    return callLuaFunction(function_name);
}

void LuaEngine::setError(const std::string& error) {
    try {
        last_error_ = error;

        // Check if error already contains a stack trace
        bool has_stack_trace = error.find("stack traceback:") != std::string::npos;

        if (has_stack_trace) {
            // Error already contains stack trace, print the entire message in light red
            fmt::print(fg(fmt::color::light_coral), "✗ {}\n", error);
        } else {
            // No stack trace in error, print error and generate one
            DebugPrinter::printError(error);

            if (L) {
                // Get stack trace using debug library
                lua_getglobal(L, "debug");
                if (!lua_isnil(L, -1)) {
                    lua_getfield(L, -1, "traceback");
                    if (lua_isfunction(L, -1)) {
                        lua_call(L, 0, 1);
                        if (lua_isstring(L, -1)) {
                            DebugPrinter::printStackTrace(lua_tostring(L, -1));
                        }
                        lua_pop(L, 1); // Remove traceback result
                    }
                    lua_pop(L, 1); // Remove debug table
                } else {
                    lua_pop(L, 1); // Remove nil
                }
            }
        }
    } catch (const std::exception& e) {
        // Fallback error handling to prevent "error in error handling"
        DebugPrinter::printHeader("💥 CRITICAL ERROR", fmt::color::dark_red);
        DebugPrinter::printError("Error in error handling system!");
        DebugPrinter::printKeyValue("Original error", error);
        DebugPrinter::printKeyValue("Handler exception", e.what());
        fmt::print("\n");
    } catch (...) {
        // Ultimate fallback
        DebugPrinter::printHeader("💥 CRITICAL ERROR", fmt::color::dark_red);
        DebugPrinter::printError("Unknown error in error handling!");
        DebugPrinter::printKeyValue("Original error", error);
        fmt::print("\n");
    }
}

bool LuaEngine::callLuaFunction(const std::string& function_name, int args, int results) {
    if (!L) {
        setError("Lua state not initialized");
        return false;
    }

    // Store original stack size for debugging
    int original_stack_size = lua_gettop(L);

    try {
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

        // Set up error handler for better error messages
        lua_getglobal(L, "debug");
        lua_getfield(L, -1, "traceback");
        lua_remove(L, -2); // Remove debug table
        int error_handler_index = lua_gettop(L) - args - 1;

        // Move error handler below function and args
        lua_insert(L, error_handler_index);

        // Call the function with error handler
        int result = lua_pcall(L, args, results, error_handler_index);

        // Remove error handler
        lua_remove(L, error_handler_index);

        if (result != LUA_OK) {
            std::string error_msg = "Unknown error";
            if (lua_isstring(L, -1)) {
                error_msg = lua_tostring(L, -1);
            }

            // Provide a meaningful function name even when empty
            std::string display_name = function_name.empty() ? "Lua function" : function_name;
            std::string full_error = "Function '" + display_name + "' failed: " + error_msg;
            setError(full_error);

            lua_pop(L, 1); // Remove error message from stack
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        // Restore stack and report error
        lua_settop(L, original_stack_size);
        setError("Exception in callLuaFunction('" + function_name + "'): " + e.what());
        return false;
    } catch (...) {
        // Restore stack and report error
        lua_settop(L, original_stack_size);
        setError("Unknown exception in callLuaFunction('" + function_name + "')");
        return false;
    }
}

bool LuaEngine::callLuaFunction(const std::string& function_name, const std::string& display_name, int args, int results) {
    if (!L) {
        setError("Lua state not initialized");
        return false;
    }

    // Store original stack size for debugging
    int original_stack_size = lua_gettop(L);

    try {
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

        // Set up error handler for better error messages
        lua_getglobal(L, "debug");
        lua_getfield(L, -1, "traceback");
        lua_remove(L, -2); // Remove debug table
        int error_handler_index = lua_gettop(L) - args - 1;

        // Move error handler below function and args
        lua_insert(L, error_handler_index);

        // Call the function with error handler
        int result = lua_pcall(L, args, results, error_handler_index);

        // Remove error handler
        lua_remove(L, error_handler_index);

        if (result != LUA_OK) {
            std::string error_msg = "Unknown error";
            if (lua_isstring(L, -1)) {
                error_msg = lua_tostring(L, -1);
            }

            // Use the provided display name for error reporting
            std::string final_display_name = display_name.empty() ?
                (function_name.empty() ? "Lua function" : function_name) : display_name;
            std::string full_error = "Function '" + final_display_name + "' failed: " + error_msg;
            setError(full_error);

            lua_pop(L, 1); // Remove error message from stack
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        // Restore stack and report error
        lua_settop(L, original_stack_size);
        std::string final_display_name = display_name.empty() ?
            (function_name.empty() ? "Lua function" : function_name) : display_name;
        setError("Exception in callLuaFunction('" + final_display_name + "'): " + e.what());
        return false;
    } catch (...) {
        // Restore stack and report error
        lua_settop(L, original_stack_size);
        std::string final_display_name = display_name.empty() ?
            (function_name.empty() ? "Lua function" : function_name) : display_name;
        setError("Unknown exception in callLuaFunction('" + final_display_name + "')");
        return false;
    }
}

} // namespace tsuki