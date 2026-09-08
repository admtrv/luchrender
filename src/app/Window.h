/*
 * Window.h
 */

#pragma once

#include "Config.h"
#include "utils/Input.h"

#include <string>

struct GLFWwindow;

namespace BulletRender {
namespace app {

enum class MouseButton {
    Left,
    Right,
    Middle
};

enum class CursorMode {
    Normal,     // visible, free to leave the window
    Captured    // hidden and locked, for looking around
};

struct WindowConfig {
    int width;
    int height;
    std::string title;
    bool resizable;
    bool vsync;             // 1 - vsync on, 0 - vsync off (unlimited fps)
    bool preferHighPerfGpu; // hint drivers to pick discrete GPU on hybrid systems

    explicit WindowConfig(int w = 800, int h = 600, std::string t = "BulletRender", bool r = true, bool v = false, bool hp = true)
        : width(w), height(h), title(std::move(t)), resizable(r), vsync(v), preferHighPerfGpu(hp) {}
};

class Window {
public:
    // lifetime
    static bool init(const WindowConfig& cfg);
    static void shutdown();

    static void pollEvents();
    static void swapBuffers();

    static bool shouldClose();
    static void setShouldClose(bool value);

    // properties
    static void getSize(int& width, int& height);
    static bool isHovered();

    static void setVSync(bool enabled);
    static bool getVSync() { return s_vsync; }

    // keyboard
    static bool isKeyDown(utils::InputKey key);

    // mouse
    static bool isMouseDown(MouseButton button);
    static void getCursorPos(double& x, double& y);

    static void setCursorMode(CursorMode mode);
    static CursorMode getCursorMode();

    // scroll delta accumulated since last call, returns dy and clears it
    static double consumeScrollDelta();

private:
    friend class Loop;   // imgui binds to the raw handle

    static GLFWwindow* get();
    static void scrollCallback(GLFWwindow* w, double xoffset, double yoffset);

    static GLFWwindow* s_Window;
    static double s_scrollAccum;
    static bool s_vsync;
};

} // namespace app
} // namespace BulletRender