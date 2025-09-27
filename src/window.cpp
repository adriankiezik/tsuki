#include "tsuki/window.hpp"
#include <iostream>

namespace tsuki {

Window::~Window() {
    shutdown();
}

bool Window::init(const WindowSettings& settings) {
    Uint32 window_flags = SDL_WINDOW_OPENGL;
    if (settings.resizable) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    if (settings.fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }

    window_ = SDL_CreateWindow(
        settings.title.c_str(),
        settings.width, settings.height,
        window_flags
    );

    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    return true;
}

void Window::shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void Window::setTitle(const std::string& title) {
    if (window_) {
        SDL_SetWindowTitle(window_, title.c_str());
    }
}

std::string Window::getTitle() const {
    if (window_) {
        const char* title = SDL_GetWindowTitle(window_);
        return title ? title : "";
    }
    return "";
}

void Window::setSize(int width, int height) {
    if (window_) {
        SDL_SetWindowSize(window_, width, height);
    }
}

int Window::getWidth() const {
    if (window_) {
        int width;
        SDL_GetWindowSize(window_, &width, nullptr);
        return width;
    }
    return 0;
}

int Window::getHeight() const {
    if (window_) {
        int height;
        SDL_GetWindowSize(window_, nullptr, &height);
        return height;
    }
    return 0;
}

void Window::setFullscreen(bool fullscreen) {
    if (window_) {
        SDL_SetWindowFullscreen(window_, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    }
}

bool Window::isFullscreen() const {
    if (window_) {
        return (SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN) != 0;
    }
    return false;
}

void Window::setVSync(bool enabled) {
    if (renderer_) {
        SDL_SetRenderVSync(renderer_, enabled);
    }
}

bool Window::getVSync() const {
    if (renderer_) {
        int vsync;
        SDL_GetRenderVSync(renderer_, &vsync);
        return vsync != 0;
    }
    return false;
}

void Window::show() {
    if (window_) {
        SDL_ShowWindow(window_);
    }
}

void Window::hide() {
    if (window_) {
        SDL_HideWindow(window_);
    }
}

void Window::minimize() {
    if (window_) {
        SDL_MinimizeWindow(window_);
    }
}

void Window::maximize() {
    if (window_) {
        SDL_MaximizeWindow(window_);
    }
}

void Window::restore() {
    if (window_) {
        SDL_RestoreWindow(window_);
    }
}

bool Window::isVisible() const {
    if (window_) {
        return (SDL_GetWindowFlags(window_) & SDL_WINDOW_HIDDEN) == 0;
    }
    return false;
}

bool Window::isMinimized() const {
    if (window_) {
        return (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) != 0;
    }
    return false;
}

bool Window::isMaximized() const {
    if (window_) {
        return (SDL_GetWindowFlags(window_) & SDL_WINDOW_MAXIMIZED) != 0;
    }
    return false;
}

void Window::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

void Window::setResizeCallback(std::function<void(int, int)> callback) {
    resize_callback_ = std::move(callback);
}

void Window::setCloseCallback(std::function<void()> callback) {
    close_callback_ = std::move(callback);
}

void Window::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            should_close_ = true;
            if (close_callback_) {
                close_callback_();
            }
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            if (resize_callback_) {
                resize_callback_(event.window.data1, event.window.data2);
            }
            break;
        default:
            break;
    }
}

} // namespace tsuki