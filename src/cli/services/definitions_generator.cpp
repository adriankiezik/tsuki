#include "definitions_generator.hpp"
#include <tsuki/tsuki.hpp>
#include <tsuki/lua_bindings.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>

namespace tsuki::cli {

std::string DefinitionsGenerator::generate() {
    introspectLuaState();
    return formatDefinitions();
}

bool DefinitionsGenerator::saveToFile(const std::string& path) {
    try {
        std::string content = generate();
        std::ofstream file(path);
        if (!file) {
            spdlog::error("Failed to open file for writing: {}", path);
            return false;
        }
        file << content;
        file.close();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to save definitions: {}", e.what());
        return false;
    }
}

void DefinitionsGenerator::introspectLuaState() {
    try {
        // Create a lightweight Lua state just for introspection
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math);

        // Register bindings for introspection only (no engine initialization needed)
        tsuki::LuaBindings::registerForIntrospection(lua);

        lua_State* L = lua.lua_state();

        // Sol3 stores usertype metatables in the Lua registry with names like "sol.ClassName"
        // Iterate through the registry to find all sol3 usertypes automatically
        lua_pushvalue(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);

        while (lua_next(L, -2)) {
            if (lua_type(L, -2) == LUA_TSTRING) {
                const char* registry_key = lua_tostring(L, -2);
                std::string key_str(registry_key);

                // Check if this is a sol3 metatable (starts with "sol.")
                if (key_str.find("sol.") == 0 && lua_istable(L, -1)) {
                    // Extract class name (remove "sol." prefix)
                    std::string class_name = key_str.substr(4);

                    // Clean up the class name:
                    // - Remove namespace prefix (tsuki::)
                    // - Remove const/pointer qualifiers
                    // - Skip internal sol3 names
                    size_t namespace_pos = class_name.find("::");
                    if (namespace_pos != std::string::npos) {
                        class_name = class_name.substr(namespace_pos + 2);
                    }

                    // Remove "const " prefix
                    if (class_name.find("const ") == 0) {
                        class_name = class_name.substr(6);
                    }

                    // Remove pointer suffix
                    if (class_name.back() == '*') {
                        class_name.pop_back();
                    }

                    // Skip if it contains sol3 internal markers or template syntax
                    if (class_name.find("sol::") != std::string::npos ||
                        class_name.find('<') != std::string::npos ||
                        class_name.find('>') != std::string::npos) {
                        lua_pop(L, 1);
                        continue;
                    }

                    // Skip if we've already processed this class
                    if (classes_.find(class_name) != classes_.end()) {
                        lua_pop(L, 1);
                        continue;
                    }

                    ClassInfo class_info;
                    class_info.name = class_name;

                    // Sol3 stores methods in the __index table of the metatable
                    lua_getfield(L, -1, "__index");

                    if (lua_istable(L, -1)) {
                        // Iterate through the __index table to find all methods
                        lua_pushnil(L);
                        while (lua_next(L, -2)) {
                            if (lua_type(L, -2) == LUA_TSTRING) {
                                const char* method_key = lua_tostring(L, -2);
                                std::string method_name(method_key);

                                // Skip internal metamethods and constructors
                                if (method_name.find("__") != 0 && method_name != "new" &&
                                    lua_type(L, -1) == LUA_TFUNCTION) {
                                    MethodInfo method;
                                    method.name = method_name;
                                    getMethodSignature(class_name, method_name, method.params, method.return_type);
                                    class_info.methods.push_back(method);
                                }
                            }
                            lua_pop(L, 1);
                        }
                    }
                    lua_pop(L, 1);  // Pop __index table

                    if (!class_info.methods.empty()) {
                        classes_[class_name] = class_info;
                    }
                }
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);  // Pop registry

        // Also handle nested tables in the tsuki table (like debug)
        sol::optional<sol::table> tsuki_table = lua["tsuki"];
        if (!tsuki_table) {
            spdlog::error("tsuki table not found in Lua state");
            return;
        }

        // Automatically discover all modules in the tsuki table
        tsuki_table->for_each([&](sol::object key, sol::object value) {
            if (!key.is<std::string>()) return;

            std::string module_name = key.as<std::string>();

            // Skip lifecycle callbacks and helper functions
            if (module_name == "start" || module_name == "update" || module_name == "print") {
                return;
            }

            // Capitalize first letter for class name
            std::string class_name = module_name;
            if (!class_name.empty()) {
                class_name[0] = std::toupper(class_name[0]);
            }

            ClassInfo class_info;
            class_info.name = class_name;

            // The value can be:
            // 1. A sol3 type table (for introspection) - contains methods directly
            // 2. Userdata (for runtime) - need to get metatable
            // 3. A regular table (like debug) - contains methods directly
            value.push(L);

            if (lua_istable(L, -1)) {
                // It's a table - iterate directly to find methods
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    if (lua_type(L, -2) == LUA_TSTRING) {
                        const char* method_key = lua_tostring(L, -2);
                        std::string method_name(method_key);

                        // Skip internal metamethods and constructors
                        if (method_name.find("__") != 0 && method_name != "new" &&
                            lua_type(L, -1) == LUA_TFUNCTION) {
                            MethodInfo method;
                            method.name = method_name;
                            getMethodSignature(class_name, method_name, method.params, method.return_type);
                            class_info.methods.push_back(method);
                        }
                    }
                    lua_pop(L, 1);
                }
            } else if (lua_isuserdata(L, -1)) {
                // It's userdata - get its metatable and look in __index
                if (lua_getmetatable(L, -1)) {
                    lua_getfield(L, -1, "__index");
                    if (lua_istable(L, -1)) {
                        lua_pushnil(L);
                        while (lua_next(L, -2)) {
                            if (lua_type(L, -2) == LUA_TSTRING) {
                                const char* method_key = lua_tostring(L, -2);
                                std::string method_name(method_key);

                                if (method_name.find("__") != 0 && method_name != "new" &&
                                    lua_type(L, -1) == LUA_TFUNCTION) {
                                    MethodInfo method;
                                    method.name = method_name;
                                    getMethodSignature(class_name, method_name, method.params, method.return_type);
                                    class_info.methods.push_back(method);
                                }
                            }
                            lua_pop(L, 1);
                        }
                    }
                    lua_pop(L, 1);  // Pop __index
                    lua_pop(L, 1);  // Pop metatable
                }
            }
            lua_pop(L, 1);  // Pop value

            if (!class_info.methods.empty()) {
                classes_[class_name] = class_info;
            }
        });

    } catch (const std::exception& e) {
        spdlog::error("Error during introspection: {}", e.what());
    }
}

void DefinitionsGenerator::getMethodSignature(const std::string& class_name, const std::string& method_name,
                                              std::string& params, std::string& return_type) const {
    // Manual mapping of method signatures since Lua doesn't store parameter info

    // Default values
    params = "...";
    return_type = "any";

    if (class_name == "Graphics") {
        if (method_name == "clear") {
            params = "r: number?, g: number?, b: number?, a: number?";
            return_type = "nil";
        } else if (method_name == "setColor") {
            params = "r: number, g: number, b: number, a: number";
            return_type = "nil";
        } else if (method_name == "rectangle") {
            params = "mode: string, x: number, y: number, width: number, height: number";
            return_type = "nil";
        } else if (method_name == "circle") {
            params = "mode: string, x: number, y: number, radius: number";
            return_type = "nil";
        } else if (method_name == "line") {
            params = "x1: number, y1: number, x2: number, y2: number";
            return_type = "nil";
        } else if (method_name == "point") {
            params = "x: number, y: number";
            return_type = "nil";
        } else if (method_name == "print") {
            params = "text: string, x: number, y: number, align: string?";
            return_type = "nil";
        } else if (method_name == "getTextSize") {
            params = "text: string";
            return_type = "number, number";
        } else if (method_name == "loadFont") {
            params = "path: string, size: number";
            return_type = "string";
        } else if (method_name == "setFont") {
            params = "fontId: string";
            return_type = "nil";
        } else if (method_name == "loadImage") {
            params = "path: string";
            return_type = "string";
        } else if (method_name == "unloadImage") {
            params = "imageId: string";
            return_type = "nil";
        } else if (method_name == "draw") {
            params = "imageId: string, x: number, y: number";
            return_type = "nil";
        }
    } else if (class_name == "Keyboard") {
        if (method_name == "isDown") {
            params = "key: string";
            return_type = "boolean";
        } else if (method_name == "isUp") {
            params = "key: string";
            return_type = "boolean";
        }
    } else if (class_name == "Mouse") {
        if (method_name == "getPosition") {
            params = "";
            return_type = "number, number";
        } else if (method_name == "getX") {
            params = "";
            return_type = "number";
        } else if (method_name == "getY") {
            params = "";
            return_type = "number";
        } else if (method_name == "setPosition") {
            params = "x: number, y: number";
            return_type = "nil";
        } else if (method_name == "isDown") {
            params = "button: integer";
            return_type = "boolean";
        } else if (method_name == "isUp") {
            params = "button: integer";
            return_type = "boolean";
        } else if (method_name == "setVisible") {
            params = "visible: boolean";
            return_type = "nil";
        } else if (method_name == "isVisible") {
            params = "";
            return_type = "boolean";
        } else if (method_name == "setRelativeMode") {
            params = "enabled: boolean";
            return_type = "nil";
        } else if (method_name == "getRelativeMode") {
            params = "";
            return_type = "boolean";
        }
    } else if (class_name == "Window") {
        if (method_name == "getWidth") {
            params = "";
            return_type = "number";
        } else if (method_name == "getHeight") {
            params = "";
            return_type = "number";
        } else if (method_name == "setTitle") {
            params = "title: string";
            return_type = "nil";
        } else if (method_name == "getTitle") {
            params = "";
            return_type = "string";
        } else if (method_name == "setSize") {
            params = "width: number, height: number";
            return_type = "nil";
        }
    } else if (class_name == "Debug") {
        if (method_name == "stackTrace") {
            params = "";
            return_type = "string";
        }
    }
}


std::string DefinitionsGenerator::formatDefinitions() const {
    std::ostringstream out;

    out << "-- Tsuki Lua API Definitions\n";
    out << "-- Auto-generated by introspecting Lua state\n";
    out << "-- Place this file in your project root and add it to .luarc.json workspace.library\n\n";
    out << "---@meta tsuki\n\n";

    // Generate class definitions (sorted alphabetically for consistency)
    std::vector<std::string> sorted_classes;
    for (const auto& [class_name, _] : classes_) {
        sorted_classes.push_back(class_name);
    }
    std::sort(sorted_classes.begin(), sorted_classes.end());

    for (const auto& class_name : sorted_classes) {
        const auto& class_info = classes_.at(class_name);

        out << "---@class " << class_name << "\n";

        for (const auto& method : class_info.methods) {
            if (method.params.empty()) {
                out << "---@field " << method.name << " fun(self: " << class_name << "): " << method.return_type << "\n";
            } else {
                out << "---@field " << method.name << " fun(self: " << class_name << ", " << method.params << "): " << method.return_type << "\n";
            }
        }

        out << "local " << class_name << " = {}\n\n";
    }

    // Generate main tsuki table dynamically
    out << "---@class tsuki\n";

    // Add discovered modules
    for (const auto& class_name : sorted_classes) {
        std::string module_name = class_name;
        if (!module_name.empty()) {
            module_name[0] = std::tolower(module_name[0]);
        }
        out << "---@field " << module_name << " " << class_name << "\n";
    }

    // Add lifecycle callbacks
    out << "---@field print fun(value: any)\n";
    out << "---@field start fun()?\n";
    out << "---@field update fun(dt: number)?\n";
    out << "tsuki = {}\n\n";

    // Add global aliases for convenience (so you can use graphics instead of tsuki.graphics)
    out << "-- Global aliases for convenience\n";
    for (const auto& class_name : sorted_classes) {
        std::string module_name = class_name;
        if (!module_name.empty()) {
            module_name[0] = std::tolower(module_name[0]);
        }
        out << "---@type " << class_name << "\n";
        out << module_name << " = nil\n";
    }

    return out.str();
}

} // namespace tsuki::cli