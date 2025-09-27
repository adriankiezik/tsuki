#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <utility>

namespace tsuki {

enum class MouseButton {
    Left = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right = SDL_BUTTON_RIGHT,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2
};

class Mouse {
public:
    Mouse() = default;
    ~Mouse() = default;

    void update();

    // Position
    int getX() const { return static_cast<int>(x_); }
    int getY() const { return static_cast<int>(y_); }
    std::pair<int, int> getPosition() const { return {static_cast<int>(x_), static_cast<int>(y_)}; }

    void setPosition(int x, int y);
    void setX(int x);
    void setY(int y);

    // Button state
    bool isDown(MouseButton button) const;
    bool isUp(MouseButton button) const;

    // Visibility and capture
    void setVisible(bool visible);
    bool isVisible() const;

    void setRelativeMode(bool enabled);
    bool getRelativeMode() const;

    // Cursor
    void setCursor(); // Default cursor
    void setCursor(const std::string& cursor_type);

    // Internal state management
    void handleButtonDown(MouseButton button);
    void handleButtonUp(MouseButton button);
    void handleMotion(int x, int y);

private:
    float x_ = 0.0f, y_ = 0.0f;
    SDL_MouseButtonFlags button_state_ = 0;
    bool visible_ = true;
    bool relative_mode_ = false;
};

} // namespace tsuki