/*
 * Loop.cpp
 */

#include "Loop.h"

#include <chrono>
#include <thread>

namespace BulletRender {
namespace app {

int Loop::s_frameRateLimit = 0;

void Loop::waitForFrameLimit(double frameStart)
{
    // vsync already paces swap, second limiter would add stutter
    if (s_frameRateLimit <= 0 || Window::getVSync())
    {
        return;
    }

    const double target = 1.0 / static_cast<double>(s_frameRateLimit);
    const double remaining = target - (utils::timeSeconds() - frameStart);

    if (remaining > 0.0)
    {
        std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1>>(remaining));
    }
}

Loop::Loop(scene::Scene& scene) : m_scene(scene)
{
    // initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // initialize backends
    ImGui_ImplGlfw_InitForOpenGL(Window::get(), true);
    ImGui_ImplOpenGL3_Init(config::GLSLVersion);
}

void Loop::run(const std::function<void(float)>& update)
{
    utils::FrameTimer timer;

    while (!Window::shouldClose())
    {
        const double frameStart = utils::timeSeconds();
        float dt = timer.tick();

        Window::pollEvents();

        if (m_beforeFrame)
        {
            m_beforeFrame();
        }

        // ImGui new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (update)
        {
            update(dt);
        }

        int fbw;
        int fbh;
        Window::getSize(fbw, fbh);
        render::Renderer::resizeViewport(fbw, fbh);

        const glm::vec4& clearColor = render::Renderer::getConfig().backgroundColor;
        render::Renderer::clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        render::Renderer::render(m_scene);

        // render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Window::swapBuffers();

        waitForFrameLimit(frameStart);
    }

    // cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace app
} // namespace BulletRender
