#include "tsuki/lua_bindings.hpp"
#include "tsuki/tsuki.hpp"
#include <spdlog/spdlog.h>

namespace tsuki {

void LuaBindings::registerAll(sol::state& lua, Engine* engine) {

    // Bind enums
    lua.new_enum<DrawMode>("DrawMode",
        {{"Fill", DrawMode::Fill}, {"Line", DrawMode::Line}}
    );

    lua.new_enum<MouseButton>("MouseButton",
        {
            {"Left", MouseButton::Left},
            {"Middle", MouseButton::Middle},
            {"Right", MouseButton::Right},
            {"X1", MouseButton::X1},
            {"X2", MouseButton::X2}
        }
    );

    // Bind Color struct
    lua.new_usertype<Color>("Color",
        sol::constructors<Color(), Color(float, float, float, float)>(),
        "r", &Color::r,
        "g", &Color::g,
        "b", &Color::b,
        "a", &Color::a,
        "white", &Color::white,
        "black", &Color::black,
        "red", &Color::red,
        "green", &Color::green,
        "blue", &Color::blue
    );

    // Bind Graphics class
    lua.new_usertype<Graphics>("Graphics",
        sol::no_constructor,

        // Drawing functions
        "clear", sol::overload(
            sol::resolve<void()>(&Graphics::clear),
            [](Graphics& g, float r, float g_, float b, float a) {
                g.clear(Color(r, g_, b, a));
            }
        ),
        "setColor", [](Graphics& g, float r, float g_, float b, float a) {
            g.setColor(Color(r, g_, b, a));
        },
        "rectangle", [](Graphics& g, const std::string& mode, float x, float y, float w, float h) {
            DrawMode dm = (mode == "fill") ? DrawMode::Fill : DrawMode::Line;
            g.rectangle(dm, x, y, w, h);
        },
        "circle", [](Graphics& g, const std::string& mode, float x, float y, float radius) {
            DrawMode dm = (mode == "fill") ? DrawMode::Fill : DrawMode::Line;
            g.circle(dm, x, y, radius);
        },
        "line", &Graphics::line,
        "point", &Graphics::point,

        // Text functions
        "print", sol::resolve<void(const std::string&, float, float)>(&Graphics::print),
        "getTextSize", &Graphics::getTextSize,
        "loadFont", &Graphics::loadFont,
        "setFont", &Graphics::setFont,

        // Image functions
        "loadImage", &Graphics::loadImage,
        "unloadImage", &Graphics::unloadImage,
        "draw", sol::resolve<void(const std::string&, float, float)>(&Graphics::draw)
    );

    // Bind Keyboard class
    lua.new_usertype<Keyboard>("Keyboard",
        sol::no_constructor,
        "isDown", [](Keyboard& k, const std::string& key) {
            return k.isDown(k.getKeyFromName(key));
        },
        "isUp", [](Keyboard& k, const std::string& key) {
            return k.isUp(k.getKeyFromName(key));
        }
    );

    // Bind Mouse class
    lua.new_usertype<Mouse>("Mouse",
        sol::no_constructor,
        "getPosition", &Mouse::getPosition,
        "getX", &Mouse::getX,
        "getY", &Mouse::getY,
        "setPosition", &Mouse::setPosition,
        "isDown", &Mouse::isDown,
        "isUp", &Mouse::isUp,
        "setVisible", &Mouse::setVisible,
        "isVisible", &Mouse::isVisible,
        "setRelativeMode", &Mouse::setRelativeMode,
        "getRelativeMode", &Mouse::getRelativeMode
    );

    // Bind Window class
    lua.new_usertype<Window>("Window",
        sol::no_constructor,
        "getWidth", &Window::getWidth,
        "getHeight", &Window::getHeight,
        "setTitle", sol::resolve<void(const std::string&)>(&Window::setTitle),
        "getTitle", &Window::getTitle,
        "setSize", &Window::setSize
    );

    // Create tsuki table
    auto tsuki = lua.create_table();

    // Only set up actual engine instances if engine is provided
    if (engine) {
        // Set up in tsuki table
        tsuki["graphics"] = &engine->getGraphics();
        tsuki["keyboard"] = &engine->getKeyboard();
        tsuki["mouse"] = &engine->getMouse();
        tsuki["window"] = &engine->getWindow();

        // Also set as globals for convenience
        lua["graphics"] = &engine->getGraphics();
        lua["keyboard"] = &engine->getKeyboard();
        lua["mouse"] = &engine->getMouse();
        lua["window"] = &engine->getWindow();
    } else {
        // For introspection, store references to the global type tables that sol3 creates
        // Sol3's new_usertype creates a global table with the type name containing all methods
        lua_State* L = lua.lua_state();

        lua_getglobal(L, "Graphics");
        tsuki["graphics"] = sol::stack::pop<sol::object>(L);

        lua_getglobal(L, "Keyboard");
        tsuki["keyboard"] = sol::stack::pop<sol::object>(L);

        lua_getglobal(L, "Mouse");
        tsuki["mouse"] = sol::stack::pop<sol::object>(L);

        lua_getglobal(L, "Window");
        tsuki["window"] = sol::stack::pop<sol::object>(L);
    }

    // Helper functions
    tsuki["print"] = [engine](const sol::object& obj) {
        std::string text;
        if (obj.is<std::string>()) {
            text = obj.as<std::string>();
        } else if (obj.is<double>()) {
            text = std::to_string(obj.as<double>());
        } else if (obj.is<int>()) {
            text = std::to_string(obj.as<int>());
        } else if (obj.is<bool>()) {
            text = obj.as<bool>() ? "true" : "false";
        }
        if (engine) {
            spdlog::info("[Lua] {}", text);
        }
    };

    // Debug utilities
    auto debug = tsuki.create_named("debug");
    debug["stackTrace"] = [](sol::this_state s) -> std::string {
        sol::state_view lua_view(s);
        sol::optional<sol::function> traceback = lua_view["debug"]["traceback"];
        if (traceback) {
            auto result = traceback.value()();
            if (result.valid()) {
                return result;
            }
        }
        return "Stack trace unavailable";
    };

    // Set global tsuki table
    lua["tsuki"] = tsuki;
}

void LuaBindings::registerForIntrospection(sol::state& lua) {
    // Just call registerAll with nullptr to avoid code duplication
    registerAll(lua, nullptr);
}

// Empty implementations (kept for compatibility)
void LuaBindings::registerGraphics(sol::state&, Engine*) {}
void LuaBindings::registerKeyboard(sol::state&, Engine*) {}
void LuaBindings::registerMouse(sol::state&, Engine*) {}
void LuaBindings::registerWindow(sol::state&, Engine*) {}
void LuaBindings::registerDebug(sol::state&) {}

} // namespace tsuki