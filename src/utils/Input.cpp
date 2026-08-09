/*
 * Input.cpp
 */

#include "Input.h"

namespace BulletRender {
namespace utils {

void Input::update(GLFWwindow* window)
{
    if (!window)
    {
        return;
    }

    for (const auto& pair : m_callbacks)
    {
        int glfwKey = pair.first;
        const InputCallback& callback = pair.second;

        bool isPressed = glfwGetKey(window, glfwKey) == GLFW_PRESS;
        bool wasPressed = m_keyState[glfwKey];

        // trigger callback on key press (transition from not pressed to pressed)
        if (isPressed && !wasPressed)
        {
            callback();
        }

        m_keyState[glfwKey] = isPressed;
    }
}

void Input::bindKey(InputKey key, const InputCallback& callback)
{
    int glfwKey = glfwKeyFromInputKey(key);
    m_callbacks[glfwKey] = callback;
    m_keyState[glfwKey] = false;
}

void Input::unbindKey(InputKey key)
{
    int glfwKey = glfwKeyFromInputKey(key);
    m_callbacks.erase(glfwKey);
}

int Input::glfwKeyFromInputKey(InputKey key) const
{
    switch (key)
    {
        case InputKey::SPACE:
            return GLFW_KEY_SPACE;
        case InputKey::W:
            return GLFW_KEY_W;
        case InputKey::A:
            return GLFW_KEY_A;
        case InputKey::S:
            return GLFW_KEY_S;
        case InputKey::D:
            return GLFW_KEY_D;
        case InputKey::C:
            return GLFW_KEY_C;
        case InputKey::G:
            return GLFW_KEY_G;
        case InputKey::UP:
            return GLFW_KEY_UP;
        case InputKey::DOWN:
            return GLFW_KEY_DOWN;
        case InputKey::LEFT:
            return GLFW_KEY_LEFT;
        case InputKey::RIGHT:
            return GLFW_KEY_RIGHT;
        case InputKey::ESCAPE:
            return GLFW_KEY_ESCAPE;
        default:
            return GLFW_KEY_UNKNOWN;
    }
}

} // namespace utils
} // namespace BulletRender
