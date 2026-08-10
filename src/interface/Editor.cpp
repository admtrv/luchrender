/*
 * Editor.cpp
 */

#include "interface/Editor.h"

#include "interface/elements/Fonts.h"
#include "interface/elements/Theme.h"
#include "interface/elements/Widgets.h"
#include "app/Window.h"

#include "imgui.h"

namespace BulletRender {
namespace interface {

constexpr float FPS_SMOOTHING = 0.1f;           // exponential average over measured frame rate
constexpr float PANEL_WIDTH_MINIMUM = 240.0f;

Editor::Editor(scene::Scene& scene, render::DebugDraw& debug)
    : m_scene(scene), m_debug(debug)
{}

// runs between frames, once imgui context exists
void Editor::beforeFrame()
{
    if (!m_themeApplied)
    {
        Theme::apply();
        m_themeApplied = true;
    }

    Fonts::apply();
}

void Editor::draw(float dt)
{
    updateFrameRate(dt);
    placePanel();

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Editor", nullptr, flags))
    {
        // drag applied next frame, placePanel reads this back
        m_panelWidth = ImGui::GetWindowWidth();

        if (ImGui::BeginTabBar("EditorTabs"))
        {
            if (ImGui::BeginTabItem("Scene"))
            {
                drawScene();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Settings"))
            {
                drawSettings(dt);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

bool Editor::wantsInput() const
{
    const ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

// smoothed, raw frame time too jumpy to read
void Editor::updateFrameRate(float dt)
{
    if (dt <= 0.0f)
    {
        return;
    }

    const float instant = 1.0f / dt;
    m_fps = m_fps > 0.0f ? m_fps + (instant - m_fps) * FPS_SMOOTHING : instant;
}

// pinned right at full height, only width is dragged
void Editor::placePanel()
{
    int width = 0;
    int height = 0;
    app::Window::getSize(width, height);

    ImGui::SetNextWindowSize({m_panelWidth, static_cast<float>(height)}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({PANEL_WIDTH_MINIMUM, static_cast<float>(height)}, {static_cast<float>(width), static_cast<float>(height)});
    ImGui::SetNextWindowPos({static_cast<float>(width) - m_panelWidth, 0.0f});
}

} // namespace interface
} // namespace BulletRender
