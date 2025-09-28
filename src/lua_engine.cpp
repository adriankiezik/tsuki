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

    // Add our own debug.traceback function using Lua's C API
    lua_getglobal(L, "debug");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "debug");
        lua_getglobal(L, "debug");
    }

    // Add a working traceback function
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* msg = luaL_optstring(L, 1, "");
        int level = luaL_optinteger(L, 2, 1);

        std::string result = msg;
        result += "\nstack traceback:";

        lua_Debug ar;
        for (int i = level; i < level + 10; i++) {
            if (!lua_getstack(L, i, &ar)) break;
            if (!lua_getinfo(L, "Sln", &ar)) break;

            if (ar.source && ar.currentline > 0) {
                std::string source = ar.source;
                if (source[0] == '@') source = source.substr(1);

                // Extract filename
                size_t lastSlash = source.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    source = source.substr(lastSlash + 1);
                }

                result += "\n\t" + source + ":" + std::to_string(ar.currentline) + ":";
                if (ar.name && strlen(ar.name) > 0) {
                    result += " in function '" + std::string(ar.name) + "'";
                } else if (ar.what && strcmp(ar.what, "main") == 0) {
                    result += " in main chunk";
                } else {
                    result += " in function";
                }
            }
        }

        lua_pushstring(L, result.c_str());
        return 1;
    });
    lua_setfield(L, -2, "traceback");
    lua_pop(L, 1); // Remove debug table

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
    setError(error, "", "");
}

void LuaEngine::setError(const std::string& error, const std::string& function_name, const std::string& file_context) {
    try {
        last_error_ = error;

        // Check if error already contains a stack trace
        bool has_stack_trace = error.find("stack traceback:") != std::string::npos;

        if (has_stack_trace) {
            // Error already contains stack trace, print the function name and then the full trace
            if (!function_name.empty()) {
                DebugPrinter::printError(fmt::format("Error in '{}':", function_name));
            }

            // Parse and display the error with stack trace properly
            DebugPrinter::printStackTrace(error);
        } else {
            // Show the original Lua error message prominently
            if (!function_name.empty()) {
                DebugPrinter::printError(fmt::format("Error in '{}':", function_name));
                fmt::print(fg(fmt::color::light_coral), "{}\n", error);
            } else {
                DebugPrinter::printError(error);
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

        // Set up error handler to capture complete stack traces using C API
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* msg = lua_tostring(L, 1);
            std::string original_error = msg ? msg : "Unknown error";

            std::string result = original_error + "\nstack traceback:";

            lua_Debug ar;
            bool first_frame = true;

            // Start from level 1 to capture the actual error location
            for (int level = 1; level <= 15; level++) {
                if (!lua_getstack(L, level, &ar)) break;
                if (!lua_getinfo(L, "Sln", &ar)) break;

                // Skip C functions and internal frames
                if (!ar.source || ar.currentline <= 0) continue;

                std::string source = ar.source;
                if (source[0] == '@') source = source.substr(1);

                // Skip [C] functions
                if (source.find("[C]") != std::string::npos) continue;

                result += "\n\t";

                // Extract filename from path
                size_t lastSlash = source.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    source = source.substr(lastSlash + 1);
                }

                result += source + ":" + std::to_string(ar.currentline) + ":";

                // For the first frame (error location), include the original error message
                if (first_frame && original_error.find(source + ":" + std::to_string(ar.currentline)) != std::string::npos) {
                    // The original error already contains file:line info, just add function info
                    if (ar.name && strlen(ar.name) > 0) {
                        result += " in function '" + std::string(ar.name) + "'";
                    } else if (ar.what && strcmp(ar.what, "main") == 0) {
                        result += " in main chunk";
                    } else {
                        result += " in function";
                    }
                    first_frame = false;
                } else {
                    // For other frames or if no match, show full info
                    if (ar.name && strlen(ar.name) > 0) {
                        result += " in function '" + std::string(ar.name) + "'";
                    } else if (ar.what && strcmp(ar.what, "main") == 0) {
                        result += " in main chunk";
                    } else {
                        result += " in function";
                    }
                    first_frame = false;
                }
            }

            lua_pushstring(L, result.c_str());
            return 1;
        });

        // Move error handler to the correct position
        int error_handler_index = lua_gettop(L) - args - 1;
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

            // Extract file context from error message if available
            std::string file_context;
            size_t file_pos = error_msg.find(".lua:");
            if (file_pos != std::string::npos) {
                size_t start = error_msg.find_last_of("/\\", file_pos);
                if (start == std::string::npos) start = 0; else start++;
                size_t end = error_msg.find(":", file_pos + 5);
                if (end != std::string::npos) {
                    file_context = error_msg.substr(start, end - start);
                }
            }

            setError(error_msg, display_name, file_context);

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

        // Set up error handler to capture complete stack traces using C API
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* msg = lua_tostring(L, 1);
            std::string original_error = msg ? msg : "Unknown error";

            std::string result = original_error + "\nstack traceback:";

            lua_Debug ar;
            bool first_frame = true;

            // Start from level 1 to capture the actual error location
            for (int level = 1; level <= 15; level++) {
                if (!lua_getstack(L, level, &ar)) break;
                if (!lua_getinfo(L, "Sln", &ar)) break;

                // Skip C functions and internal frames
                if (!ar.source || ar.currentline <= 0) continue;

                std::string source = ar.source;
                if (source[0] == '@') source = source.substr(1);

                // Skip [C] functions
                if (source.find("[C]") != std::string::npos) continue;

                result += "\n\t";

                // Extract filename from path
                size_t lastSlash = source.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    source = source.substr(lastSlash + 1);
                }

                result += source + ":" + std::to_string(ar.currentline) + ":";

                // For the first frame (error location), include the original error message
                if (first_frame && original_error.find(source + ":" + std::to_string(ar.currentline)) != std::string::npos) {
                    // The original error already contains file:line info, just add function info
                    if (ar.name && strlen(ar.name) > 0) {
                        result += " in function '" + std::string(ar.name) + "'";
                    } else if (ar.what && strcmp(ar.what, "main") == 0) {
                        result += " in main chunk";
                    } else {
                        result += " in function";
                    }
                    first_frame = false;
                } else {
                    // For other frames or if no match, show full info
                    if (ar.name && strlen(ar.name) > 0) {
                        result += " in function '" + std::string(ar.name) + "'";
                    } else if (ar.what && strcmp(ar.what, "main") == 0) {
                        result += " in main chunk";
                    } else {
                        result += " in function";
                    }
                    first_frame = false;
                }
            }

            lua_pushstring(L, result.c_str());
            return 1;
        });

        // Move error handler to the correct position
        int error_handler_index = lua_gettop(L) - args - 1;
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

            // Extract file context from error message if available
            std::string file_context;
            size_t file_pos = error_msg.find(".lua:");
            if (file_pos != std::string::npos) {
                size_t start = error_msg.find_last_of("/\\", file_pos);
                if (start == std::string::npos) start = 0; else start++;
                size_t end = error_msg.find(":", file_pos + 5);
                if (end != std::string::npos) {
                    file_context = error_msg.substr(start, end - start);
                }
            }

            setError(error_msg, final_display_name, file_context);

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