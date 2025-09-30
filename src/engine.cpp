#include "tsuki/tsuki.hpp"
#include "tsuki/platform.hpp"
#include <SDL3/SDL.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef TSUKI_HAS_SDL_IMAGE
#include <SDL3_image/SDL_image.h>
#endif

#ifdef TSUKI_HAS_SDL_TTF
#include <SDL3_ttf/SDL_ttf.h>
#endif

namespace tsuki {

Engine& Engine::getInstance() {
    static Engine instance;
    return instance;
}

bool Engine::init() {
    // Initialize SDL (video and events only - audio is handled by miniaudio)
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

#ifdef TSUKI_HAS_SDL_IMAGE
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
#endif

#ifdef TSUKI_HAS_SDL_TTF
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
#ifdef TSUKI_HAS_SDL_IMAGE
        IMG_Quit();
#endif
        SDL_Quit();
        return false;
    }
#endif

    // Initialize window
    if (!window_.init()) {
#ifdef TSUKI_HAS_SDL_TTF
        TTF_Quit();
#endif
#ifdef TSUKI_HAS_SDL_IMAGE
        IMG_Quit();
#endif
        SDL_Quit();
        return false;
    }

    // Try adaptive VSync first (allows higher FPS while preventing tearing)
    // Fall back to regular VSync if adaptive isn't supported
    if (!window_.setVSync(-1)) { // SDL_RENDERER_VSYNC_ADAPTIVE
        window_.setVSync(true);
    }

    // Initialize subsystems
    if (!graphics_.init(window_.getRenderer())) {
        window_.shutdown();
#ifdef TSUKI_HAS_SDL_TTF
        TTF_Quit();
#endif
#ifdef TSUKI_HAS_SDL_IMAGE
        IMG_Quit();
#endif
        SDL_Quit();
        return false;
    }

    if (!audio_.init()) {
        graphics_.shutdown();
        window_.shutdown();
#ifdef TSUKI_HAS_SDL_TTF
        TTF_Quit();
#endif
#ifdef TSUKI_HAS_SDL_IMAGE
        IMG_Quit();
#endif
        SDL_Quit();
        return false;
    }

    // Initialize Lua engine
    if (!lua_engine_.init()) {
        audio_.shutdown();
        graphics_.shutdown();
        window_.shutdown();
#ifdef TSUKI_HAS_SDL_TTF
        TTF_Quit();
#endif
#ifdef TSUKI_HAS_SDL_IMAGE
        IMG_Quit();
#endif
        SDL_Quit();
        return false;
    }

    // Register Lua bindings
    LuaBindings::registerAll(lua_engine_.getLuaState(), this);

    running_ = true;
    return true;
}

void Engine::run() {
    if (!running_) {
        return;
    }

    if (load_callback_) {
        load_callback_();
    }

    timer_.update();

    while (running_ && !window_.shouldClose()) {
        timer_.update();
        double dt = timer_.getDelta();

        window_.pollEvents();
        keyboard_.update();
        mouse_.update();

        if (update_callback_) {
            update_callback_(dt);
        }

        graphics_.clear();

        if (draw_callback_) {
            draw_callback_();
        }

        graphics_.present();
    }

    quit();
}

void Engine::runLuaGame(const std::string& game_path) {
    if (!running_) {
        return;
    }

    // Save current working directory and change to game directory
    // This ensures that relative paths in Lua scripts work correctly
    std::filesystem::path original_cwd = std::filesystem::current_path();
    std::filesystem::path game_dir = std::filesystem::absolute(game_path);

    try {
        std::filesystem::current_path(game_dir);
    } catch (const std::exception& e) {
        std::cerr << "Failed to change to game directory: " << e.what() << std::endl;
        return;
    }

    // Load the main.lua file from the game (now relative to the game directory)
    std::string main_lua_path = "main.lua";

    // Try to load main.lua
    if (!lua_engine_.executeFile(main_lua_path)) {
        std::cerr << "Failed to load game: " << lua_engine_.getLastError() << std::endl;
        // Restore original directory before returning
        try {
            std::filesystem::current_path(original_cwd);
        } catch (...) {}
        return;
    }

    // Call start() if it exists
    lua_engine_.callStart();

    timer_.update();

    // Main game loop
    while (running_ && !window_.shouldClose()) {
        try {
            timer_.update();
            double dt = timer_.getDelta();

            window_.pollEvents();
            keyboard_.update();
            mouse_.update();

            graphics_.clear();

            // Call update(dt) for game logic and rendering
            lua_engine_.callUpdate(dt);

            graphics_.present();
        } catch (const std::exception& e) {
            std::cerr << "Critical engine error: " << e.what() << std::endl;
            std::cerr << "Attempting to continue..." << std::endl;
            // Try to recover by clearing graphics and presenting
            try {
                graphics_.clear();
                graphics_.present();
            } catch (...) {
                std::cerr << "Failed to recover, shutting down." << std::endl;
                running_ = false;
            }
        } catch (...) {
            std::cerr << "Unknown critical engine error, attempting to continue..." << std::endl;
            try {
                graphics_.clear();
                graphics_.present();
            } catch (...) {
                std::cerr << "Failed to recover, shutting down." << std::endl;
                running_ = false;
            }
        }
    }

    // Restore original working directory
    try {
        std::filesystem::current_path(original_cwd);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to restore original working directory: " << e.what() << std::endl;
    }

    quit();
}

void Engine::quit() {
    running_ = false;

    audio_.shutdown();
    graphics_.shutdown();
    window_.shutdown();

#ifdef TSUKI_HAS_SDL_TTF
    TTF_Quit();
#endif
#ifdef TSUKI_HAS_SDL_IMAGE
    IMG_Quit();
#endif
    SDL_Quit();
}

void Engine::setLoadCallback(std::function<void()> callback) {
    load_callback_ = std::move(callback);
}

void Engine::setUpdateCallback(std::function<void(double)> callback) {
    update_callback_ = std::move(callback);
}

void Engine::setDrawCallback(std::function<void()> callback) {
    draw_callback_ = std::move(callback);
}

} // namespace tsuki