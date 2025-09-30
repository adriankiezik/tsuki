#pragma once

#include <string>
#include <memory>
#include <sol/sol.hpp>

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

    // Callback functions
    bool callStart();
    bool callUpdate(double dt);
    bool callFunction(const std::string& function_name);

    // Error handling
    std::string getLastError() const { return last_error_; }

    // Direct Lua state access for bindings
    sol::state& getLuaState() { return lua; }

private:
    sol::state lua;
    std::string last_error_;

    void setError(const std::string& error);
};

} // namespace tsuki