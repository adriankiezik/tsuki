#pragma once

#include <string>
#include <memory>

extern "C" {
#ifdef __APPLE__
    #include <lua5.4/lua.h>
    #include <lua5.4/lualib.h>
    #include <lua5.4/lauxlib.h>
#else
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
#endif
}

namespace tsuki {

class LuaEngine {
public:
    LuaEngine();
    ~LuaEngine();

    // Disable copy operations
    LuaEngine(const LuaEngine&) = delete;
    LuaEngine& operator=(const LuaEngine&) = delete;

    bool init();
    void shutdown();

    // Script loading and execution
    bool loadFile(const std::string& filename);
    bool loadString(const std::string& code);
    bool executeFile(const std::string& filename);
    bool executeString(const std::string& code);

    // Callback functions (like LOVE's love.load, love.update, love.draw)
    bool callLoad();
    bool callUpdate(double dt);
    bool callDraw();
    bool callFunction(const std::string& function_name);

    // Error handling
    std::string getLastError() const { return last_error_; }

    // Direct Lua state access for bindings
    lua_State* getLuaState() const { return L; }

private:
    lua_State* L;
    std::string last_error_;

    void setError(const std::string& error);
    bool callLuaFunction(const std::string& function_name, int args = 0, int results = 0);
};

} // namespace tsuki