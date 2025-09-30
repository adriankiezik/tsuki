#pragma once

#include <sol/sol.hpp>

namespace tsuki {

// Forward declarations
class Engine;

class LuaBindings {
public:
    static void registerAll(sol::state& lua, Engine* engine);
    static void registerForIntrospection(sol::state& lua);

private:
    static void registerGraphics(sol::state& lua, Engine* engine);
    static void registerKeyboard(sol::state& lua, Engine* engine);
    static void registerMouse(sol::state& lua, Engine* engine);
    static void registerWindow(sol::state& lua, Engine* engine);
    static void registerDebug(sol::state& lua);
};

} // namespace tsuki