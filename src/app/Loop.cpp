/*
 * Loop.cpp
 */

#include "Loop.h"

namespace BulletRender {
namespace app {

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
        float dt = timer.tick();

        Window::pollEvents();

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
    }

    // cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace app
} // namespace BulletRender
