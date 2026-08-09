/*
 * Input.h
 */

#pragma once

#include <GLFW/glfw3.h>

#include <functional>
#include <unordered_map>

namespace BulletRender {
namespace utils {

enum class InputKey {
    SPACE,
    W, A, S, D,
    C, G,
    UP, DOWN, LEFT, RIGHT,
    ESCAPE
};

using InputCallback = std::function<void()>;

class Input {
public:
    static Input& instance() {
        static Input m;
        return m;
    }

    void update(GLFWwindow* window);

    void bindKey(InputKey key, const InputCallback& callback);
    void unbindKey(InputKey key);

private:
    Input() = default;
    ~Input() = default;

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    int glfwKeyFromInputKey(InputKey key) const;

    std::unordered_map<int, InputCallback> m_callbacks;
    std::unordered_map<int, bool> m_keyState;
};

} // namespace utils
} // namespace BulletRender
