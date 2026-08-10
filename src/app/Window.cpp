/*
 * Window.cpp
 */

#include "Window.h"

#include <cstdlib>

// Windows: exported symbols pick the discrete GPU on hybrid systems
#ifdef _WIN32
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int           AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace BulletRender {
namespace app {

// Linux: env hints PRIME offload / Mesa to use the discrete GPU
static void preferHighPerfGpuHint()
{
#ifndef _WIN32
    setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 0);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 0);
    setenv("__VK_LAYER_NV_optimus",     "NVIDIA_only", 0);
    setenv("DRI_PRIME",                 "1", 0);
#endif
}

GLFWwindow *Window::s_Window = nullptr;
double Window::s_scrollAccum = 0.0;
bool Window::s_vsync = false;

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
    if (cfg.preferHighPerfGpu)
    {
        preferHighPerfGpuHint();
    }

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

    setVSync(cfg.vsync);

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

void Window::setVSync(bool enabled)
{
    s_vsync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

} // namespace app
} // namespace BulletRender