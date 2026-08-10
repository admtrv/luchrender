/*
 * Loop.h
 */

#pragma once

#include "Config.h"
#include "Window.h"
#include "render/Renderer.h"
#include "scene/Scene.h"
#include "utils/Time.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <functional>
#include <glm/glm.hpp>

namespace BulletRender {
namespace app {

class Loop {
public:
    explicit Loop(scene::Scene& scene);

    void run(const std::function<void(float)>& update);

    // hooks
    void setBeforeFrame(std::function<void()> callback) { m_beforeFrame = std::move(callback); }

    // zero means unlimited
    static void setFrameRateLimit(int fps) { s_frameRateLimit = fps; }
    static int getFrameRateLimit() { return s_frameRateLimit; }

private:
    static void waitForFrameLimit(double frameStart);

    scene::Scene& m_scene;
    std::function<void()> m_beforeFrame;
    static int s_frameRateLimit;
};

} // namespace app
} // namespace BulletRender
