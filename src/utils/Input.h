/*
 * Input.h
 */

#pragma once

// glad has to come first wherever the gl headers appear
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <unordered_map>

namespace BulletRender {
namespace utils {

enum class InputKey : int {
    // letters
    A = GLFW_KEY_A, B = GLFW_KEY_B, C = GLFW_KEY_C, D = GLFW_KEY_D,
    E = GLFW_KEY_E, F = GLFW_KEY_F, G = GLFW_KEY_G, H = GLFW_KEY_H,
    I = GLFW_KEY_I, J = GLFW_KEY_J, K = GLFW_KEY_K, L = GLFW_KEY_L,
    M = GLFW_KEY_M, N = GLFW_KEY_N, O = GLFW_KEY_O, P = GLFW_KEY_P,
    Q = GLFW_KEY_Q, R = GLFW_KEY_R, S = GLFW_KEY_S, T = GLFW_KEY_T,
    U = GLFW_KEY_U, V = GLFW_KEY_V, W = GLFW_KEY_W, X = GLFW_KEY_X,
    Y = GLFW_KEY_Y, Z = GLFW_KEY_Z,

    // digits
    NUM_0 = GLFW_KEY_0, NUM_1 = GLFW_KEY_1, NUM_2 = GLFW_KEY_2, NUM_3 = GLFW_KEY_3,
    NUM_4 = GLFW_KEY_4, NUM_5 = GLFW_KEY_5, NUM_6 = GLFW_KEY_6, NUM_7 = GLFW_KEY_7,
    NUM_8 = GLFW_KEY_8, NUM_9 = GLFW_KEY_9,

    // function
    F1 = GLFW_KEY_F1, F2 = GLFW_KEY_F2, F3 = GLFW_KEY_F3, F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5, F6 = GLFW_KEY_F6, F7 = GLFW_KEY_F7, F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9, F10 = GLFW_KEY_F10, F11 = GLFW_KEY_F11, F12 = GLFW_KEY_F12,

    // arrows
    UP = GLFW_KEY_UP, DOWN = GLFW_KEY_DOWN, LEFT = GLFW_KEY_LEFT, RIGHT = GLFW_KEY_RIGHT,

    // modifiers
    LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT, RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
    LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL, RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
    LEFT_ALT = GLFW_KEY_LEFT_ALT, RIGHT_ALT = GLFW_KEY_RIGHT_ALT,

    // editing
    SPACE = GLFW_KEY_SPACE, ENTER = GLFW_KEY_ENTER, TAB = GLFW_KEY_TAB,
    BACKSPACE = GLFW_KEY_BACKSPACE, DELETE = GLFW_KEY_DELETE,
    INSERT = GLFW_KEY_INSERT, ESCAPE = GLFW_KEY_ESCAPE,

    // navigation
    HOME = GLFW_KEY_HOME, END = GLFW_KEY_END,
    PAGE_UP = GLFW_KEY_PAGE_UP, PAGE_DOWN = GLFW_KEY_PAGE_DOWN,

    // punctuation
    MINUS = GLFW_KEY_MINUS, EQUAL = GLFW_KEY_EQUAL,
    COMMA = GLFW_KEY_COMMA, PERIOD = GLFW_KEY_PERIOD, SLASH = GLFW_KEY_SLASH,
    SEMICOLON = GLFW_KEY_SEMICOLON, APOSTROPHE = GLFW_KEY_APOSTROPHE,
    LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET, RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
    BACKSLASH = GLFW_KEY_BACKSLASH, GRAVE = GLFW_KEY_GRAVE_ACCENT,

    // keypad
    KEYPAD_0 = GLFW_KEY_KP_0, KEYPAD_1 = GLFW_KEY_KP_1, KEYPAD_2 = GLFW_KEY_KP_2,
    KEYPAD_3 = GLFW_KEY_KP_3, KEYPAD_4 = GLFW_KEY_KP_4, KEYPAD_5 = GLFW_KEY_KP_5,
    KEYPAD_6 = GLFW_KEY_KP_6, KEYPAD_7 = GLFW_KEY_KP_7, KEYPAD_8 = GLFW_KEY_KP_8,
    KEYPAD_9 = GLFW_KEY_KP_9,
    KEYPAD_ADD = GLFW_KEY_KP_ADD, KEYPAD_SUBTRACT = GLFW_KEY_KP_SUBTRACT,
    KEYPAD_MULTIPLY = GLFW_KEY_KP_MULTIPLY, KEYPAD_DIVIDE = GLFW_KEY_KP_DIVIDE,
    KEYPAD_DECIMAL = GLFW_KEY_KP_DECIMAL, KEYPAD_ENTER = GLFW_KEY_KP_ENTER
};

using InputCallback = std::function<void()>;

class Input {
public:
    static Input& instance() {
        static Input m;
        return m;
    }

    void update();

    void bindKey(InputKey key, const InputCallback& callback);
    void unbindKey(InputKey key);

private:
    Input() = default;
    ~Input() = default;

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    std::unordered_map<int, InputCallback> m_callbacks;
    std::unordered_map<int, bool> m_keyState;
};

} // namespace utils
} // namespace BulletRender
