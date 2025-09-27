#include "tsuki/event.hpp"

namespace tsuki {

void Event::pollEvents() {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        processEvent(sdl_event);
    }
}

void Event::processEvent(const SDL_Event& sdl_event) {
    TsukiEvent event = convertSDLEvent(sdl_event);
    handleEvent(event);
}

void Event::onKeyDown(EventCallback callback) {
    callbacks_[EventType::KeyDown].push_back(std::move(callback));
}

void Event::onKeyUp(EventCallback callback) {
    callbacks_[EventType::KeyUp].push_back(std::move(callback));
}

void Event::onMouseButtonDown(EventCallback callback) {
    callbacks_[EventType::MouseButtonDown].push_back(std::move(callback));
}

void Event::onMouseButtonUp(EventCallback callback) {
    callbacks_[EventType::MouseButtonUp].push_back(std::move(callback));
}

void Event::onMouseMove(EventCallback callback) {
    callbacks_[EventType::MouseMove].push_back(std::move(callback));
}

void Event::onWindowResize(EventCallback callback) {
    callbacks_[EventType::WindowResize].push_back(std::move(callback));
}

void Event::onQuit(EventCallback callback) {
    callbacks_[EventType::Quit].push_back(std::move(callback));
}

void Event::handleEvent(const TsukiEvent& event) {
    triggerCallbacks(event.type, event);
}

TsukiEvent Event::convertSDLEvent(const SDL_Event& sdl_event) {
    TsukiEvent event;

    switch (sdl_event.type) {
        case SDL_EVENT_QUIT:
            event.type = EventType::Quit;
            break;
        case SDL_EVENT_KEY_DOWN:
            event.type = EventType::KeyDown;
            event.data.keyboard.key = sdl_event.key.scancode;
            event.data.keyboard.repeat = sdl_event.key.repeat != 0;
            break;
        case SDL_EVENT_KEY_UP:
            event.type = EventType::KeyUp;
            event.data.keyboard.key = sdl_event.key.scancode;
            event.data.keyboard.repeat = false;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            event.type = EventType::MouseButtonDown;
            event.data.mouse_button.button = sdl_event.button.button;
            event.data.mouse_button.x = static_cast<int>(sdl_event.button.x);
            event.data.mouse_button.y = static_cast<int>(sdl_event.button.y);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            event.type = EventType::MouseButtonUp;
            event.data.mouse_button.button = sdl_event.button.button;
            event.data.mouse_button.x = static_cast<int>(sdl_event.button.x);
            event.data.mouse_button.y = static_cast<int>(sdl_event.button.y);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            event.type = EventType::MouseMove;
            event.data.mouse_motion.x = static_cast<int>(sdl_event.motion.x);
            event.data.mouse_motion.y = static_cast<int>(sdl_event.motion.y);
            event.data.mouse_motion.dx = static_cast<int>(sdl_event.motion.xrel);
            event.data.mouse_motion.dy = static_cast<int>(sdl_event.motion.yrel);
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            event.type = EventType::WindowResize;
            event.data.window_resize.width = sdl_event.window.data1;
            event.data.window_resize.height = sdl_event.window.data2;
            break;
        default:
            event.type = EventType::Quit; // Default fallback
            break;
    }

    return event;
}

void Event::triggerCallbacks(EventType type, const TsukiEvent& event) {
    auto it = callbacks_.find(type);
    if (it != callbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

} // namespace tsuki