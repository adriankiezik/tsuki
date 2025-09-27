#include "tsuki/keyboard.hpp"

namespace tsuki {

void Keyboard::update() {
    // SDL handles keyboard state internally
}

bool Keyboard::isDown(KeyCode key) const {
    const bool* state = SDL_GetKeyboardState(nullptr);
    return state[static_cast<SDL_Scancode>(key)];
}

bool Keyboard::isUp(KeyCode key) const {
    return !isDown(key);
}

std::string Keyboard::getKeyName(KeyCode key) const {
    const char* name = SDL_GetScancodeName(static_cast<SDL_Scancode>(key));
    return name ? name : "";
}

KeyCode Keyboard::getKeyFromName(const std::string& name) const {
    SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
    return static_cast<KeyCode>(scancode);
}

void Keyboard::setTextInput(bool enabled) {
    text_input_enabled_ = enabled;
    // Note: SDL3 requires a window parameter - we'll leave this for later implementation
    // For now, just store the state
}

bool Keyboard::hasTextInput() const {
    return text_input_enabled_;
}

void Keyboard::setKeyRepeat(bool enabled) {
    key_repeat_enabled_ = enabled;
}

bool Keyboard::hasKeyRepeat() const {
    return key_repeat_enabled_;
}

void Keyboard::handleKeyDown(KeyCode key) {
    pressed_keys_.insert(key);
}

void Keyboard::handleKeyUp(KeyCode key) {
    pressed_keys_.erase(key);
}

} // namespace tsuki