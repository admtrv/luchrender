/*
 * Editor.h
 */

#pragma once

#include "interface/elements/TreeView.h"
#include "Config.h"
#include "render/DebugDraw.h"
#include "render/passes/Fog.h"
#include "scene/Scene.h"

#include <memory>
#include <set>
#include <string>

namespace BulletRender {
namespace interface {

// what inspector edits
enum class SelectionType {
    None,
    Object,
    Light,
    Camera
};

struct Selection {
    SelectionType type = SelectionType::None;
    size_t index = 0;

    void clear() { type = SelectionType::None; index = 0; }
    bool is(SelectionType t, size_t i) const { return type == t && index == i; }
};

// side panel driving scene, standalone tool only
class Editor {
public:
    Editor(scene::Scene& scene, render::DebugDraw& debug);

    // optional pieces panels expose when present
    void setFog(std::shared_ptr<render::Fog> fog) { m_fog = std::move(fog); }
    void setDefaultShader(std::shared_ptr<render::GraphicsShader> shader) { m_shader = std::move(shader); }

    void draw(float dt);

    // hand to Loop::setBeforeFrame, some ui work cannot happen inside frame
    void beforeFrame();

    // true while panel takes input, camera must stay still then
    bool wantsInput() const;

    // master switch for gizmos, shared with keyboard shortcut
    void setShowDebug(bool show) { m_showDebug = show; }
    bool getShowDebug() const { return m_showDebug; }
    void toggleShowDebug() { m_showDebug = !m_showDebug; }

    float getPanelWidth() const { return m_panelWidth; }

private:
    // Editor.cpp
    void updateFrameRate(float dt);
    void placePanel();

    // one method per tab, screen composes its own content
    // screens/Scene.cpp
    void drawScene();

    // screens/Hierarchy.cpp
    void drawHierarchy();
    void drawObjectSection();
    void drawLightSection();
    void drawCameraSection();
    void drawObjectNode(scene::SceneObject& object, size_t index, bool last);
    size_t indexOfObject(const scene::SceneObject* object) const;
    void drawCreateMenu();
    scene::SceneObject* spawnObject(scene::Model* model, const std::string& name);

    // screens/Settings.cpp
    void drawSettings(float dt);
    void drawFrameSection(float dt);
    void drawInterfaceSection();
    void drawBackgroundSection();
    void drawDebugSection();
    void drawFogSection();

    // screens/Inspectors.cpp
    void drawInspector();
    void drawObjectInspector(scene::SceneObject& object);
    void drawLightInspector(scene::Light& light);
    void drawCameraInspector(scene::Camera& camera, size_t index);
    void drawTransformInspector(scene::Transform& transform);
    void drawModelInspector(scene::SceneObject& object);
    void drawMaterialInspector(render::Material& material);
    void drawTextureInspector(render::Material& material);

// what editor drives
    scene::Scene& m_scene;
    render::DebugDraw& m_debug;
    std::shared_ptr<render::Fog> m_fog;
    std::shared_ptr<render::GraphicsShader> m_shader;

    // hierarchy
    TreeView m_tree;
    Selection m_selection;
    std::set<const scene::SceneObject*> m_collapsed;    // folded nodes, by identity, indices may shift
    float m_hierarchyFraction = 0.45f;                  // share of tab height, dragged by splitter
    bool m_objectsFolded = false;
    bool m_lightsFolded = false;
    bool m_camerasFolded = false;

    // file loading, path survives between frames
    char m_modelPath[256] = "";
    std::string m_modelError;
    char m_texturePath[256] = "";
    std::string m_textureError;

    // panel
    float m_panelWidth = 340.0f;
    float m_uiScale = config::FontScale;
    float m_fps = 0.0f;
    bool m_showDebug = true;
    bool m_themeApplied = false;
};

} // namespace interface
} // namespace BulletRender
