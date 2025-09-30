#include "tsuki/lua_engine.hpp"
#include <spdlog/spdlog.h>

namespace tsuki {

LuaEngine::LuaEngine() {
}

LuaEngine::~LuaEngine() {
    shutdown();
}

bool LuaEngine::init() {
    try {
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string,
                          sol::lib::math, sol::lib::table, sol::lib::debug,
                          sol::lib::os, sol::lib::io);

        spdlog::info("Lua engine initialized with sol3");
        return true;
    } catch (const sol::error& e) {
        setError(std::string("Failed to initialize Lua: ") + e.what());
        return false;
    }
}

void LuaEngine::shutdown() {
    // sol::state destructor handles cleanup
}

bool LuaEngine::loadFile(const std::string& filename) {
    try {
        auto result = lua.load_file(filename);
        return result.valid();
    } catch (const sol::error& e) {
        setError(std::string("Failed to load file '") + filename + "': " + e.what());
        return false;
    }
}

bool LuaEngine::loadString(const std::string& code) {
    try {
        auto result = lua.load(code);
        return result.valid();
    } catch (const sol::error& e) {
        setError(std::string("Failed to load string: ") + e.what());
        return false;
    }
}

bool LuaEngine::executeFile(const std::string& filename) {
    try {
        lua.script_file(filename);
        return true;
    } catch (const sol::error& e) {
        setError(std::string("Error executing file '") + filename + "': " + e.what());
        return false;
    }
}

bool LuaEngine::executeString(const std::string& code) {
    try {
        lua.script(code);
        return true;
    } catch (const sol::error& e) {
        setError(std::string("Error executing string: ") + e.what());
        return false;
    }
}

bool LuaEngine::callLoad() {
    try {
        sol::optional<sol::function> load_func = lua["tsuki"]["load"];
        if (load_func) {
            auto result = load_func.value()();
            if (!result.valid()) {
                sol::error err = result;
                setError(std::string("Error in tsuki.load: ") + err.what());
                return false;
            }
            return true;
        }
        return true; // No load function is not an error
    } catch (const sol::error& e) {
        setError(std::string("Error calling tsuki.load: ") + e.what());
        return false;
    }
}

bool LuaEngine::callUpdate(double dt) {
    try {
        sol::optional<sol::function> update_func = lua["tsuki"]["update"];
        if (update_func) {
            auto result = update_func.value()(dt);
            if (!result.valid()) {
                sol::error err = result;
                setError(std::string("Error in tsuki.update: ") + err.what());
                return false;
            }
            return true;
        }
        return true; // No update function is not an error
    } catch (const sol::error& e) {
        setError(std::string("Error calling tsuki.update: ") + e.what());
        return false;
    }
}

bool LuaEngine::callDraw() {
    try {
        sol::optional<sol::function> draw_func = lua["tsuki"]["draw"];
        if (draw_func) {
            auto result = draw_func.value()();
            if (!result.valid()) {
                sol::error err = result;
                setError(std::string("Error in tsuki.draw: ") + err.what());
                return false;
            }
            return true;
        }
        return true; // No draw function is not an error
    } catch (const sol::error& e) {
        setError(std::string("Error calling tsuki.draw: ") + e.what());
        return false;
    }
}

bool LuaEngine::callFunction(const std::string& function_name) {
    try {
        sol::optional<sol::function> func = lua[function_name];
        if (func) {
            auto result = func.value()();
            if (!result.valid()) {
                sol::error err = result;
                setError(std::string("Error in ") + function_name + ": " + err.what());
                return false;
            }
            return true;
        }
        setError(std::string("Function '") + function_name + "' not found");
        return false;
    } catch (const sol::error& e) {
        setError(std::string("Error calling function '") + function_name + "': " + e.what());
        return false;
    }
}

void LuaEngine::setError(const std::string& error) {
    last_error_ = error;
    spdlog::error("Lua error: {}", error);
}

} // namespace tsuki