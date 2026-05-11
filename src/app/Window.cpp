/*
 * Window.cpp
 */

#include "Window.h"

namespace BulletRender {
namespace app {

GLFWwindow *Window::s_Window = nullptr;
double Window::s_scrollAccum = 0.0;

void Window::scrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset)
{
    s_scrollAccum += yoffset;
}

double Window::consumeScrollDelta()
{
    double v = s_scrollAccum;
    s_scrollAccum = 0.0;
    return v;
}

bool Window::init(const WindowConfig& cfg)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config::OpenGLMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config::OpenGLMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // resizable
    glfwWindowHint(GLFW_RESIZABLE, cfg.resizable ? GLFW_TRUE : GLFW_FALSE);

    s_Window = glfwCreateWindow(cfg.width, cfg.height, cfg.title.c_str(), nullptr, nullptr);

    if (!s_Window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(s_Window);

    glfwSetScrollCallback(s_Window, scrollCallback);

    glfwSwapInterval(cfg.vsync ? 1 : 0);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    int w, h; glfwGetFramebufferSize(s_Window, &w, &h);
    glViewport(0,0,w,h);


    return true;
}

void Window::shutdown()
{
    if (s_Window)
    {
        glfwDestroyWindow(s_Window);
        s_Window = nullptr;
    }

    glfwTerminate();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(s_Window);
}

void Window::setShouldClose(bool value)
{
    glfwSetWindowShouldClose(s_Window, value ? GLFW_TRUE : GLFW_FALSE);
}

void Window::pollEvents()
{
    glfwPollEvents();
}
void Window::swapBuffers()
{
    glfwSwapBuffers(s_Window);
}

GLFWwindow *Window::get()
{
    return s_Window;
}

void Window::getSize(int& width, int& height)
{
    glfwGetFramebufferSize(s_Window, &width, &height);
}

} // namespace app
} // namespace BulletRender