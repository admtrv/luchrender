/*
 * Settings.cpp
 */

#include "interface/Editor.h"

#include "interface/elements/Fonts.h"
#include "interface/elements/Widgets.h"
#include "Config.h"
#include "app/Loop.h"
#include "app/Window.h"
#include "render/Renderer.h"

#include "imgui.h"

namespace BulletRender {
namespace interface {

// frame
constexpr int FRAME_LIMIT_MINIMUM = 15;         // below this panel becomes hard to use
constexpr int FRAME_LIMIT_MAXIMUM = 240;
constexpr int FRAME_LIMIT_DEFAULT = 120;        // what turning limit back on falls to

// interface
constexpr float UI_SCALE_MINIMUM = 0.8f;
constexpr float UI_SCALE_MAXIMUM = 2.0f;

// fog
constexpr float FOG_DISTANCE_MAXIMUM = 500.0f;

void Editor::drawSettings(float dt)
{
    drawFrameSection(dt);
    drawInterfaceSection();
    drawBackgroundSection();
    drawDebugSection();
    drawFogSection();
}

void Editor::drawFrameSection(float dt)
{
    if (!ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    statRow("FPS", "%.1f", m_fps);
    statRow("Frame time", "%.2f ms", dt * 1000.0f);
    statRow("Aspect", "%.3f", render::Renderer::getAspect());

    bool vsync = app::Window::getVSync();
    if (checkboxField("VSync", vsync))
    {
        app::Window::setVSync(vsync);
    }

    // vsync already paces frames, limiter would fight it
    ImGui::BeginDisabled(vsync);

    // unlimited is own toggle, dragging through single digit rates
    // would freeze ui with no way back
    int limit = app::Loop::getFrameRateLimit();

    bool unlimited = limit <= 0;
    if (checkboxField("Unlimited", unlimited))
    {
        app::Loop::setFrameRateLimit(unlimited ? 0 : FRAME_LIMIT_DEFAULT);
        limit = app::Loop::getFrameRateLimit();
    }

    ImGui::BeginDisabled(unlimited);

    int shown = unlimited ? FRAME_LIMIT_DEFAULT : limit;
    if (dragScalarField("Max FPS", shown, FRAME_LIMIT_MINIMUM, FRAME_LIMIT_MAXIMUM, "%d") && !unlimited)
    {
        app::Loop::setFrameRateLimit(shown);
    }

    ImGui::EndDisabled();
    ImGui::EndDisabled();
}

void Editor::drawInterfaceSection()
{
    if (!ImGui::CollapsingHeader("Interface", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (dragScalarField("Font size", m_uiScale, UI_SCALE_MINIMUM, UI_SCALE_MAXIMUM, "%.2f"))
    {
        Fonts::setScale(m_uiScale);

        // captions grow with font, column follows or they overlap control
        LABEL_COLUMN_WIDTH = LABEL_COLUMN_BASE * m_uiScale;
    }
}

void Editor::drawBackgroundSection()
{
    if (!ImGui::CollapsingHeader("Background", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    const glm::vec4& current = render::Renderer::getConfig().backgroundColor;

    glm::vec3 background = glm::vec3(current);
    if (dragColor3("Color", background))
    {
        render::Renderer::setBackgroundColor(glm::vec4(background, current.a));
    }
}

void Editor::drawDebugSection()
{
    if (!ImGui::CollapsingHeader("Debug draw", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    checkboxField("Show gizmos", m_showDebug);

    // individual gizmos mean nothing while overlay is off
    ImGui::BeginDisabled(!m_showDebug);

    bool transforms = m_debug.getShowTransforms();
    if (checkboxField("Transforms", transforms))
    {
        m_debug.setShowTransforms(transforms);
    }

    bool lights = m_debug.getShowLights();
    if (checkboxField("Lights", lights))
    {
        m_debug.setShowLights(lights);
    }

    bool bounds = m_debug.getShowBounds();
    if (checkboxField("Bounds", bounds))
    {
        m_debug.setShowBounds(bounds);
    }

    bool cameras = m_debug.getShowCameras();
    if (checkboxField("Cameras", cameras))
    {
        m_debug.setShowCameras(cameras);
    }

    ImGui::EndDisabled();
}

void Editor::drawFogSection()
{
    if (!m_fog || !ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    bool enabled = m_fog->isEnabled();
    if (checkboxField("Enable fog", enabled))
    {
        m_fog->setEnabled(enabled);
    }

    ImGui::BeginDisabled(!enabled);

    float start = m_fog->getStart();
    float end = m_fog->getEnd();

    // both rows draw every frame, short circuit makes one flicker
    bool rangeChanged = dragScalarField("Start", start, 0.0f, end, "%.1f");
    rangeChanged |= dragScalarField("End", end, start, FOG_DISTANCE_MAXIMUM, "%.1f");

    if (rangeChanged)
    {
        m_fog->setRange(start, end);
    }

    ImGui::EndDisabled();
}

} // namespace interface
} // namespace BulletRender
