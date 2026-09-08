/*
 * Input.cpp
 */

#include "Input.h"

#include "app/Window.h"

namespace BulletRender {
namespace utils {

void Input::update()
{
    for (const auto& [key, callback] : m_callbacks)
    {
        const bool isPressed = app::Window::isKeyDown(static_cast<InputKey>(key));
        const bool wasPressed = m_keyState[key];

        // the callback fires on the press itself, not while the key is held
        if (isPressed && !wasPressed)
        {
            callback();
        }

        m_keyState[key] = isPressed;
    }
}

void Input::bindKey(InputKey key, const InputCallback& callback)
{
    const int glfwKey = static_cast<int>(key);

    m_callbacks[glfwKey] = callback;
    m_keyState[glfwKey] = false;
}

void Input::unbindKey(InputKey key)
{
    m_callbacks.erase(static_cast<int>(key));
}

} // namespace utils
} // namespace BulletRender
