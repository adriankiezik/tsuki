#include "tsuki/mouse.hpp"

namespace tsuki {

void Mouse::update() {
    button_state_ = SDL_GetMouseState(&x_, &y_);
}

bool Mouse::isDown(MouseButton button) const {
    return (button_state_ & static_cast<SDL_MouseButtonFlags>(1 << (static_cast<int>(button) - 1))) != 0;
}

bool Mouse::isUp(MouseButton button) const {
    return !isDown(button);
}

void Mouse::setPosition(int x, int y) {
    x_ = static_cast<float>(x);
    y_ = static_cast<float>(y);
    SDL_WarpMouseGlobal(static_cast<float>(x), static_cast<float>(y));
}

void Mouse::setX(int x) {
    setPosition(x, static_cast<int>(y_));
}

void Mouse::setY(int y) {
    setPosition(static_cast<int>(x_), y);
}

void Mouse::setVisible(bool visible) {
    visible_ = visible;
    if (visible) {
        SDL_ShowCursor();
    } else {
        SDL_HideCursor();
    }
}

bool Mouse::isVisible() const {
    return visible_;
}

void Mouse::setRelativeMode(bool enabled) {
    relative_mode_ = enabled;
    // Note: SDL3 API has changed - we'll leave this for later implementation
}

bool Mouse::getRelativeMode() const {
    return relative_mode_;
}

void Mouse::setCursor() {
    SDL_SetCursor(SDL_GetDefaultCursor());
}

void Mouse::setCursor(const std::string& cursor_type) {
    // Note: SDL3 system cursor constants have changed - simplified implementation for now
    (void)cursor_type; // Suppress unused parameter warning

    // Default cursor for now
    SDL_Cursor* cursor = SDL_GetDefaultCursor();
    if (cursor) {
        SDL_SetCursor(cursor);
    }
}

void Mouse::handleButtonDown(MouseButton button) {
    button_state_ |= static_cast<SDL_MouseButtonFlags>(1 << (static_cast<int>(button) - 1));
}

void Mouse::handleButtonUp(MouseButton button) {
    button_state_ &= ~static_cast<SDL_MouseButtonFlags>(1 << (static_cast<int>(button) - 1));
}

void Mouse::handleMotion(int x, int y) {
    x_ = static_cast<float>(x);
    y_ = static_cast<float>(y);
}

} // namespace tsuki