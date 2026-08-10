/*
 * Hierarchy.cpp
 */

#include "interface/Editor.h"

#include "interface/elements/Widgets.h"

#include "imgui.h"

namespace BulletRender {
namespace interface {

// payload id for reparenting drags
constexpr const char* OBJECT_DRAG_TYPE = "BR_OBJECT";

// marks camera frame is rendered through
constexpr const char* ACTIVE_MARKER = "*";

// label width drives where button sits
constexpr const char* ADD_BUTTON_LABEL = "Add";

void Editor::drawHierarchy()
{
    ImGui::TextUnformatted("Hierarchy");

    // button shares caption line, pinned right
    const float buttonWidth = ImGui::CalcTextSize(ADD_BUTTON_LABEL).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonWidth + ImGui::GetCursorPosX());

    drawCreateMenu();

    const float listHeight = ImGui::GetContentRegionAvail().y * m_hierarchyFraction;
    if (!ImGui::BeginChild("Hierarchy", {0.0f, listHeight}, ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        return;
    }

    // three independent trees, one per entity kind
    drawObjectSection();
    ImGui::Separator();

    drawLightSection();
    ImGui::Separator();

    drawCameraSection();

    ImGui::EndChild();
}

void Editor::drawObjectSection()
{
    const std::vector<std::unique_ptr<scene::SceneObject>>& objects = m_scene.getObjects();

    // own tree, guides start fresh instead of hanging off shared root
    m_tree.reset();

    if (m_tree.row("Objects", "Objects", true, false, true, m_objectsFolded))
    {
        m_selection.clear();
    }
    if (m_tree.toggled())
    {
        m_objectsFolded = !m_objectsFolded;
    }

    // dropping onto header detaches object back to scene root
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(OBJECT_DRAG_TYPE))
        {
            scene::SceneObject* dragged = *static_cast<scene::SceneObject**>(payload->Data);
            dragged->setParent(nullptr);
        }
        ImGui::EndDragDropTarget();
    }

    if (m_objectsFolded)
    {
        return;
    }

    // only roots start branch, children reached through parent
    size_t lastRoot = objects.size();
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i] && objects[i]->getParent() == nullptr)
        {
            lastRoot = i;
        }
    }

    m_tree.push(true);
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i] && objects[i]->getParent() == nullptr)
        {
            drawObjectNode(*objects[i], i, i == lastRoot);
        }
    }
    m_tree.pop();
}

void Editor::drawLightSection()
{
    const std::vector<std::unique_ptr<scene::Light>>& lights = m_scene.getLights();

    m_tree.reset();

    if (m_tree.row("Lights", "Lights", true, false, true, m_lightsFolded))
    {
        m_selection.clear();
    }
    if (m_tree.toggled())
    {
        m_lightsFolded = !m_lightsFolded;
    }

    if (m_lightsFolded)
    {
        return;
    }

    m_tree.push(true);
    for (size_t i = 0; i < lights.size(); ++i)
    {
        const bool last = i + 1 == lights.size();

        if (m_tree.row(lights[i].get(), lights[i]->getName().c_str(), last, m_selection.is(SelectionType::Light, i)))
        {
            m_selection = {SelectionType::Light, i};
        }
    }
    m_tree.pop();
}

void Editor::drawCameraSection()
{
    const std::vector<std::unique_ptr<scene::Camera>>& cameras = m_scene.getCameras();

    m_tree.reset();

    if (m_tree.row("Cameras", "Cameras", true, false, true, m_camerasFolded))
    {
        m_selection.clear();
    }
    if (m_tree.toggled())
    {
        m_camerasFolded = !m_camerasFolded;
    }

    if (m_camerasFolded)
    {
        return;
    }

    m_tree.push(true);
    for (size_t i = 0; i < cameras.size(); ++i)
    {
        const bool last = i + 1 == cameras.size();
        const bool active = cameras[i].get() == m_scene.getActiveCamera();

        // active one marked with symbol, name stays clean
        const std::string label = active ? cameras[i]->getName() + " " + ACTIVE_MARKER
                                         : cameras[i]->getName();

        if (m_tree.row(cameras[i].get(), label.c_str(), last, m_selection.is(SelectionType::Camera, i)))
        {
            m_selection = {SelectionType::Camera, i};
        }
    }
    m_tree.pop();
}

size_t Editor::indexOfObject(const scene::SceneObject* object) const
{
    const std::vector<std::unique_ptr<scene::SceneObject>>& objects = m_scene.getObjects();
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i].get() == object)
        {
            return i;
        }
    }
    return objects.size();
}

void Editor::drawObjectNode(scene::SceneObject& object, size_t index, bool last)
{
    const std::vector<scene::SceneObject*> children = object.getChildren();

    const bool leaf = children.empty();
    const bool selected = m_selection.is(SelectionType::Object, index);
    const bool collapsed = m_collapsed.count(&object) != 0;

    ImGui::PushID(static_cast<int>(index));

    // hidden objects stay listed, dimmed
    const bool hidden = !object.isVisible();
    if (hidden)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    }

    const bool clicked = m_tree.row(&object, object.getName().c_str(), last, selected, !leaf, collapsed);

    if (hidden)
    {
        ImGui::PopStyleColor();
    }

    if (m_tree.toggled())
    {
        if (collapsed)
        {
            m_collapsed.erase(&object);
        }
        else
        {
            m_collapsed.insert(&object);
        }
    }

    if (clicked)
    {
        m_selection = {SelectionType::Object, index};
    }

    // drag object onto another to reparent
    if (ImGui::BeginDragDropSource())
    {
        scene::SceneObject* payload = &object;
        ImGui::SetDragDropPayload(OBJECT_DRAG_TYPE, &payload, sizeof(payload));
        ImGui::TextUnformatted(object.getName().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(OBJECT_DRAG_TYPE))
        {
            // setParent refuses cycles, bad drop is ignored
            scene::SceneObject* dragged = *static_cast<scene::SceneObject**>(payload->Data);
            if (dragged != &object)
            {
                dragged->setParent(&object);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (!leaf && !collapsed)
    {
        // row shape decides whether trunk keeps running under children
        m_tree.push(last);

        for (size_t i = 0; i < children.size(); ++i)
        {
            if (children[i])
            {
                drawObjectNode(*children[i], indexOfObject(children[i]), i + 1 == children.size());
            }
        }

        m_tree.pop();
    }

    ImGui::PopID();
}

// creation menu

void Editor::drawCreateMenu()
{
    if (ImGui::Button(ADD_BUTTON_LABEL))
    {
        ImGui::OpenPopup("Add");
    }

    if (!ImGui::BeginPopup("Add"))
    {
        return;
    }

    // only primitives, geometry from disk is picked in inspector
    if (ImGui::BeginMenu("Object"))
    {
        if (ImGui::MenuItem("Box"))
        {
            spawnObject(m_scene.addModel(std::make_unique<scene::Box>()), "Box");
        }
        if (ImGui::MenuItem("Sphere"))
        {
            spawnObject(m_scene.addModel(std::make_unique<scene::Sphere>()), "Sphere");
        }
        if (ImGui::MenuItem("Empty"))
        {
            spawnObject(nullptr, "Empty");
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Light"))
    {
        if (ImGui::MenuItem("Ambient"))
        {
            m_scene.createLight<scene::AmbientLight>();
        }
        if (ImGui::MenuItem("Directional"))
        {
            m_scene.createLight<scene::DirectionalLight>();
        }
        if (ImGui::MenuItem("Point"))
        {
            m_scene.createLight<scene::PointLight>();
        }
        if (ImGui::MenuItem("Spot"))
        {
            m_scene.createLight<scene::SpotLight>();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Camera"))
    {
        if (ImGui::MenuItem("Fly"))
        {
            m_scene.createCamera<scene::FlyCamera>();
        }
        if (ImGui::MenuItem("Orbit"))
        {
            m_scene.createCamera<scene::OrbitCamera>();
        }
        if (ImGui::MenuItem("Static"))
        {
            m_scene.createCamera<scene::StaticCamera>(glm::vec3{5.0f, 5.0f, 5.0f}, glm::vec3{0.0f});
        }
        ImGui::EndMenu();
    }

    ImGui::EndPopup();
}

scene::SceneObject* Editor::spawnObject(scene::Model* model, const std::string& name)
{
    scene::SceneObject* object = m_scene.addObject(model, name);
    object->getMaterial().setShader(m_shader);

    m_selection = {SelectionType::Object, m_scene.getObjects().size() - 1};
    return object;
}

} // namespace interface
} // namespace BulletRender
