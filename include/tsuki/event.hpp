#pragma once

#include <SDL3/SDL.h>
#include <functional>
#include <vector>
#include <unordered_map>

namespace tsuki {

enum class EventType {
    Quit,
    KeyDown,
    KeyUp,
    MouseButtonDown,
    MouseButtonUp,
    MouseMove,
    WindowResize,
    WindowClose
};

struct TsukiEvent {
    EventType type;
    union {
        struct {
            int key;
            bool repeat;
        } keyboard;
        struct {
            int button;
            int x, y;
        } mouse_button;
        struct {
            int x, y;
            int dx, dy;
        } mouse_motion;
        struct {
            int width, height;
        } window_resize;
    } data;
};

class Event {
public:
    Event() = default;
    ~Event() = default;

    using EventCallback = std::function<void(const TsukiEvent&)>;

    void pollEvents();
    void processEvent(const SDL_Event& sdl_event);

    // Event callbacks
    void onKeyDown(EventCallback callback);
    void onKeyUp(EventCallback callback);
    void onMouseButtonDown(EventCallback callback);
    void onMouseButtonUp(EventCallback callback);
    void onMouseMove(EventCallback callback);
    void onWindowResize(EventCallback callback);
    void onQuit(EventCallback callback);

    // Direct event handling
    void handleEvent(const TsukiEvent& event);

private:
    std::unordered_map<EventType, std::vector<EventCallback>> callbacks_;

    TsukiEvent convertSDLEvent(const SDL_Event& sdl_event);
    void triggerCallbacks(EventType type, const TsukiEvent& event);
};

} // namespace tsuki