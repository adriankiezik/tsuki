#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <unordered_set>

namespace tsuki {

enum class KeyCode {
    // Letters
    A = SDL_SCANCODE_A, B = SDL_SCANCODE_B, C = SDL_SCANCODE_C, D = SDL_SCANCODE_D,
    E = SDL_SCANCODE_E, F = SDL_SCANCODE_F, G = SDL_SCANCODE_G, H = SDL_SCANCODE_H,
    I = SDL_SCANCODE_I, J = SDL_SCANCODE_J, K = SDL_SCANCODE_K, L = SDL_SCANCODE_L,
    M = SDL_SCANCODE_M, N = SDL_SCANCODE_N, O = SDL_SCANCODE_O, P = SDL_SCANCODE_P,
    Q = SDL_SCANCODE_Q, R = SDL_SCANCODE_R, S = SDL_SCANCODE_S, T = SDL_SCANCODE_T,
    U = SDL_SCANCODE_U, V = SDL_SCANCODE_V, W = SDL_SCANCODE_W, X = SDL_SCANCODE_X,
    Y = SDL_SCANCODE_Y, Z = SDL_SCANCODE_Z,

    // Numbers
    Num1 = SDL_SCANCODE_1, Num2 = SDL_SCANCODE_2, Num3 = SDL_SCANCODE_3,
    Num4 = SDL_SCANCODE_4, Num5 = SDL_SCANCODE_5, Num6 = SDL_SCANCODE_6,
    Num7 = SDL_SCANCODE_7, Num8 = SDL_SCANCODE_8, Num9 = SDL_SCANCODE_9,
    Num0 = SDL_SCANCODE_0,

    // Function keys
    F1 = SDL_SCANCODE_F1, F2 = SDL_SCANCODE_F2, F3 = SDL_SCANCODE_F3,
    F4 = SDL_SCANCODE_F4, F5 = SDL_SCANCODE_F5, F6 = SDL_SCANCODE_F6,
    F7 = SDL_SCANCODE_F7, F8 = SDL_SCANCODE_F8, F9 = SDL_SCANCODE_F9,
    F10 = SDL_SCANCODE_F10, F11 = SDL_SCANCODE_F11, F12 = SDL_SCANCODE_F12,

    // Arrow keys
    Up = SDL_SCANCODE_UP, Down = SDL_SCANCODE_DOWN,
    Left = SDL_SCANCODE_LEFT, Right = SDL_SCANCODE_RIGHT,

    // Special keys
    Space = SDL_SCANCODE_SPACE, Enter = SDL_SCANCODE_RETURN,
    Escape = SDL_SCANCODE_ESCAPE, Tab = SDL_SCANCODE_TAB,
    Backspace = SDL_SCANCODE_BACKSPACE, Delete = SDL_SCANCODE_DELETE,
    Insert = SDL_SCANCODE_INSERT, Home = SDL_SCANCODE_HOME,
    End = SDL_SCANCODE_END, PageUp = SDL_SCANCODE_PAGEUP,
    PageDown = SDL_SCANCODE_PAGEDOWN,

    // Modifiers
    LeftShift = SDL_SCANCODE_LSHIFT, RightShift = SDL_SCANCODE_RSHIFT,
    LeftCtrl = SDL_SCANCODE_LCTRL, RightCtrl = SDL_SCANCODE_RCTRL,
    LeftAlt = SDL_SCANCODE_LALT, RightAlt = SDL_SCANCODE_RALT,
    LeftGui = SDL_SCANCODE_LGUI, RightGui = SDL_SCANCODE_RGUI
};

class Keyboard {
public:
    Keyboard() = default;
    ~Keyboard() = default;

    void update();

    // Key state queries
    bool isDown(KeyCode key) const;
    bool isUp(KeyCode key) const;

    // String conversion
    std::string getKeyName(KeyCode key) const;
    KeyCode getKeyFromName(const std::string& name) const;

    // Text input
    void setTextInput(bool enabled);
    bool hasTextInput() const;

    // Key repeat
    void setKeyRepeat(bool enabled);
    bool hasKeyRepeat() const;

    // Internal state management
    void handleKeyDown(KeyCode key);
    void handleKeyUp(KeyCode key);

private:
    std::unordered_set<KeyCode> pressed_keys_;
    bool text_input_enabled_ = false;
    bool key_repeat_enabled_ = true;
};

} // namespace tsuki