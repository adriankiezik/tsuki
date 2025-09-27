#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <functional>

namespace tsuki {

struct WindowSettings {
    std::string title = "Tsuki Game";
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    int display = 0;
};

class Window {
public:
    Window() = default;
    ~Window();

    bool init(const WindowSettings& settings = {});
    void shutdown();

    // Window properties
    void setTitle(const std::string& title);
    std::string getTitle() const;

    void setSize(int width, int height);
    int getWidth() const;
    int getHeight() const;

    void setFullscreen(bool fullscreen);
    bool isFullscreen() const;

    void setVSync(bool enabled);
    bool getVSync() const;

    // Window state
    void show();
    void hide();
    void minimize();
    void maximize();
    void restore();

    bool isVisible() const;
    bool isMinimized() const;
    bool isMaximized() const;

    // Event handling
    void pollEvents();
    bool shouldClose() const { return should_close_; }

    // Callbacks
    void setResizeCallback(std::function<void(int, int)> callback);
    void setCloseCallback(std::function<void()> callback);

    SDL_Window* getSDLWindow() const { return window_; }
    SDL_Renderer* getRenderer() const { return renderer_; }

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool should_close_ = false;

    std::function<void(int, int)> resize_callback_;
    std::function<void()> close_callback_;

    void handleEvent(const SDL_Event& event);
};

} // namespace tsuki