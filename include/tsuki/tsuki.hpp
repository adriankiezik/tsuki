#pragma once

#include "audio.hpp"
#include "event.hpp"
#include "graphics.hpp"
#include "keyboard.hpp"
#include "lua_engine.hpp"
#include "lua_bindings.hpp"
#include "math.hpp"
#include "mouse.hpp"
#include "packaging.hpp"
#include "system.hpp"
#include "timer.hpp"
#include "window.hpp"

namespace tsuki {

class Engine {
public:
    static Engine& getInstance();

    bool init();
    void run();
    void runLuaGame(const std::string& game_path);
    void quit();

    void setLoadCallback(std::function<void()> callback);
    void setUpdateCallback(std::function<void(double)> callback);
    void setDrawCallback(std::function<void()> callback);

    Audio& getAudio() { return audio_; }
    Event& getEvent() { return event_; }
    Graphics& getGraphics() { return graphics_; }
    Keyboard& getKeyboard() { return keyboard_; }
    Math& getMath() { return math_; }
    Mouse& getMouse() { return mouse_; }
    System& getSystem() { return system_; }
    Timer& getTimer() { return timer_; }
    Window& getWindow() { return window_; }
    LuaEngine& getLuaEngine() { return lua_engine_; }

private:
    Engine() = default;
    ~Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool running_ = false;

    std::function<void()> load_callback_;
    std::function<void(double)> update_callback_;
    std::function<void()> draw_callback_;

    Audio audio_;
    Event event_;
    Graphics graphics_;
    Keyboard keyboard_;
    LuaEngine lua_engine_;
    Math math_;
    Mouse mouse_;
    System system_;
    Timer timer_;
    Window window_;
};

// Global convenience functions (similar to LOVE's API)
inline Audio& audio() { return Engine::getInstance().getAudio(); }
inline Event& event() { return Engine::getInstance().getEvent(); }
inline Graphics& graphics() { return Engine::getInstance().getGraphics(); }
inline Keyboard& keyboard() { return Engine::getInstance().getKeyboard(); }
inline Math& math() { return Engine::getInstance().getMath(); }
inline Mouse& mouse() { return Engine::getInstance().getMouse(); }
inline System& system() { return Engine::getInstance().getSystem(); }
inline Timer& timer() { return Engine::getInstance().getTimer(); }
inline Window& window() { return Engine::getInstance().getWindow(); }

} // namespace tsuki