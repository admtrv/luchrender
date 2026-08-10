/*
 * Inspectors.cpp
 */

#include "interface/Editor.h"

#include "interface/elements/Widgets.h"
#include "Colors.h"
#include "render/textures/TextureLoader.h"

#include "imgui.h"

#include <cstdio>

namespace BulletRender {
namespace interface {

// transform
constexpr float DRAG_SPEED_POSITION = 0.05f;
constexpr float DRAG_SPEED_ROTATION = 0.5f;
constexpr float DRAG_SPEED_SCALE = 0.01f;
constexpr float SCALE_MINIMUM = 0.001f;

// light
constexpr float INTENSITY_MAXIMUM = 50.0f;
constexpr float RANGE_MAXIMUM = 500.0f;
constexpr float SHADOW_SIZE_MAXIMUM = 200.0f;
constexpr float CONE_ANGLE_MAXIMUM = 89.0f;

// camera
constexpr float FOV_MINIMUM = 10.0f;
constexpr float FOV_MAXIMUM = 120.0f;
constexpr float CLIP_MINIMUM = 0.01f;           // near at zero collapses depth precision
constexpr float CLIP_MAXIMUM = 5000.0f;
constexpr float FLY_SPEED_MAXIMUM = 50.0f;
constexpr float ORBIT_RADIUS_MINIMUM = 0.1f;
constexpr float ORBIT_RADIUS_MAXIMUM = 200.0f;

// material
constexpr float SHININESS_MINIMUM = 1.0f;
constexpr float SHININESS_MAXIMUM = 256.0f;

// texture
constexpr float TEXTURE_PREVIEW_SIZE = 48.0f;
constexpr const char* ALBEDO_UNIFORM = "uAlbedo";   // only slot standard shader samples
constexpr unsigned ALBEDO_UNIT = 0;

// type names, indexed by matching enum
static const char* const LIGHT_TYPE_NAMES[] = {"Ambient", "Directional", "Point", "Spot"};
static const char* const CAMERA_TYPE_NAMES[] = {"Static", "Fly", "Orbit"};

static const char* lightTypeName(scene::LightType type)
{
    return LIGHT_TYPE_NAMES[static_cast<int>(type)];
}

static const char* cameraTypeName(scene::CameraType type)
{
    return CAMERA_TYPE_NAMES[static_cast<int>(type)];
}

// editable display name, every entity has one
static void nameField(scene::Named& named)
{
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%s", named.getName().c_str());

    if (inputTextField("Name", buffer, sizeof(buffer)))
    {
        named.setName(buffer);
    }
}

// inspector sections

void Editor::drawInspector()
{
    ImGui::TextUnformatted("Inspector");

    if (!ImGui::BeginChild("Inspector", {0.0f, 0.0f}, ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        return;
    }

    switch (m_selection.type)
    {
        case SelectionType::Object:
        {
            if (m_selection.index < m_scene.getObjects().size())
            {
                drawObjectInspector(*m_scene.getObjects()[m_selection.index]);
            }
            else
            {
                m_selection.clear();
            }
            break;
        }
        case SelectionType::Light:
        {
            if (m_selection.index < m_scene.getLights().size())
            {
                drawLightInspector(*m_scene.getLights()[m_selection.index]);
            }
            else
            {
                m_selection.clear();
            }
            break;
        }
        case SelectionType::Camera:
        {
            if (m_selection.index < m_scene.getCameras().size())
            {
                drawCameraInspector(*m_scene.getCameras()[m_selection.index], m_selection.index);
            }
            else
            {
                m_selection.clear();
            }
            break;
        }
        case SelectionType::None:
        {
            ImGui::TextDisabled("Nothing selected");
            break;
        }
    }

    ImGui::EndChild();
}

void Editor::drawObjectInspector(scene::SceneObject& object)
{
    nameField(object);

    bool visible = object.isVisible();
    if (checkboxField("Visible", visible))
    {
        object.setVisible(visible);
    }

    drawTransformInspector(object.getTransform());
    drawModelInspector(object);
    drawMaterialInspector(object.getMaterial());
    drawTextureInspector(object.getMaterial());

    ImGui::Separator();
    if (ImGui::Button("Delete"))
    {
        m_scene.removeObject(m_selection.index);
        m_selection.clear();
    }
}

void Editor::drawLightInspector(scene::Light& light)
{
    nameField(light);

    statRow("Type", "%s", lightTypeName(light.getType()));

    bool visible = light.isVisible();
    if (checkboxField("Visible", visible))
    {
        light.setVisible(visible);
    }

    glm::vec3 color = light.getColor();
    if (dragColor3("Color", color))
    {
        light.setColor(color);
    }

    float intensity = light.getIntensity();
    if (dragScalarField("Intensity", intensity, 0.0f, INTENSITY_MAXIMUM, "%.2f"))
    {
        light.setIntensity(intensity);
    }

    // only these two reach shadow pass, flag does nothing elsewhere
    const scene::LightType type = light.getType();
    if (type == scene::LightType::Directional || type == scene::LightType::Spot)
    {
        bool shadow = light.getCastsShadow();
        if (checkboxField("Casts shadow", shadow))
        {
            light.setCastsShadow(shadow);
        }
    }

    // type specific parameters
    switch (type)
    {
        case scene::LightType::Directional:
        {
            auto& directional = static_cast<scene::DirectionalLight&>(light);

            glm::vec3 direction = directional.getDirection();
            if (dragVector3("Direction", direction, 0.01f, -1.0f, 1.0f, "%.2f"))
            {
                directional.setDirection(direction);
            }

            glm::vec3 target = directional.getShadowTarget();
            if (dragVector3("Shadow target", target, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
            {
                directional.setShadowTarget(target);
            }

            float orthoSize = directional.getShadowOrthoSize();
            if (dragScalarField("Shadow size", orthoSize, 1.0f, SHADOW_SIZE_MAXIMUM, "%.1f"))
            {
                directional.setShadowOrthoSize(orthoSize);
            }
            break;
        }
        case scene::LightType::Point:
        {
            auto& point = static_cast<scene::PointLight&>(light);

            glm::vec3 position = point.getPosition();
            if (dragVector3("Position", position, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
            {
                point.setPosition(position);
            }

            float range = point.getRange();
            if (dragScalarField("Range", range, 0.0f, RANGE_MAXIMUM, "%.1f"))
            {
                point.setRange(range);
            }
            break;
        }
        case scene::LightType::Spot:
        {
            auto& spot = static_cast<scene::SpotLight&>(light);

            glm::vec3 position = spot.getPosition();
            if (dragVector3("Position", position, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
            {
                spot.setPosition(position);
            }

            glm::vec3 direction = spot.getDirection();
            if (dragVector3("Direction", direction, 0.01f, -1.0f, 1.0f, "%.2f"))
            {
                spot.setDirection(direction);
            }

            // cones stored as cosines, edited as degrees
            float inner = glm::degrees(std::acos(spot.getInnerCos()));
            float outer = glm::degrees(std::acos(spot.getOuterCos()));
            // both rows draw every frame, short circuit makes one flicker
            bool conesChanged = dragScalarField("Inner angle", inner, 0.0f, outer, "%.1f");
            conesChanged |= dragScalarField("Outer angle", outer, inner, CONE_ANGLE_MAXIMUM, "%.1f");

            if (conesChanged)
            {
                spot.setCones(inner, outer);
            }

            float range = spot.getRange();
            if (dragScalarField("Range", range, 0.0f, RANGE_MAXIMUM, "%.1f"))
            {
                spot.setRange(range);
            }
            break;
        }
        case scene::LightType::Ambient:
        {
            break;
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Delete"))
    {
        m_scene.removeLight(m_selection.index);
        m_selection.clear();
    }
}

void Editor::drawCameraInspector(scene::Camera& camera, size_t index)
{
    nameField(camera);

    statRow("Type", "%s", cameraTypeName(camera.getType()));

    const bool active = &camera == m_scene.getActiveCamera();
    statRow("Status", "%s", active ? "Active" : "Inactive");

    if (!active && ImGui::Button("Make active"))
    {
        m_scene.setActiveCamera(&camera);
    }

    ImGui::Separator();

    // controlled camera rewrites own pose every frame, editing fights input
    const bool driven = active && camera.getType() != scene::CameraType::Static;
    ImGui::BeginDisabled(driven);

    glm::vec3 position = camera.getPosition();
    if (dragVector3("Position", position, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
    {
        camera.setPosition(position);
    }

    ImGui::EndDisabled();

    float fov = camera.getFov();
    if (dragScalarField("Field of view", fov, FOV_MINIMUM, FOV_MAXIMUM, "%.0f"))
    {
        camera.setFov(fov);
    }

    // near stays in front of eye and behind far, else projection degenerates
    float zNear = camera.getNear();
    float zFar = camera.getFar();

    bool clipChanged = dragScalarField("Near", zNear, CLIP_MINIMUM, zFar - CLIP_MINIMUM, "%.2f");
    clipChanged |= dragScalarField("Far", zFar, zNear + CLIP_MINIMUM, CLIP_MAXIMUM, "%.1f");

    if (clipChanged)
    {
        camera.setClipPlanes(zNear, zFar);
    }

    // type specific parameters
    switch (camera.getType())
    {
        case scene::CameraType::Static:
        {
            auto& staticCamera = static_cast<scene::StaticCamera&>(camera);

            glm::vec3 target = staticCamera.getTarget();
            if (dragVector3("Target", target, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
            {
                staticCamera.setTarget(target);
            }
            break;
        }
        case scene::CameraType::Fly:
        {
            auto& flyCamera = static_cast<scene::FlyCamera&>(camera);

            float speed = flyCamera.getSpeed();
            if (dragScalarField("Speed", speed, 0.1f, FLY_SPEED_MAXIMUM, "%.1f"))
            {
                flyCamera.setSpeed(speed);
            }
            break;
        }
        case scene::CameraType::Orbit:
        {
            auto& orbitCamera = static_cast<scene::OrbitCamera&>(camera);

            glm::vec3 target = orbitCamera.getTarget();
            if (dragVector3("Target", target, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
            {
                orbitCamera.setTarget(target);
            }

            float radius = orbitCamera.getRadius();
            if (dragScalarField("Radius", radius, ORBIT_RADIUS_MINIMUM, ORBIT_RADIUS_MAXIMUM, "%.1f"))
            {
                orbitCamera.setRadius(radius);
            }
            break;
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Delete"))
    {
        m_scene.removeCamera(index);
        m_selection.clear();
    }
}

void Editor::drawTransformInspector(scene::Transform& transform)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    glm::vec3 position = transform.getLocalPosition();
    if (dragVector3("Position", position, DRAG_SPEED_POSITION, 0.0f, 0.0f, "%.2f"))
    {
        transform.setLocalPosition(position);
    }

    // quaternions cannot be edited by hand, euler angles are editable form
    glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.getLocalRotation()));
    if (dragVector3("Rotation", euler, DRAG_SPEED_ROTATION, 0.0f, 0.0f, "%.1f"))
    {
        transform.setLocalRotation(glm::quat(glm::radians(euler)));
    }

    glm::vec3 scale = transform.getLocalScale();
    if (dragVector3("Scale", scale, DRAG_SPEED_SCALE, SCALE_MINIMUM, 100.0f, "%.2f"))
    {
        transform.setLocalScale(glm::max(scale, glm::vec3(SCALE_MINIMUM)));
    }

    if (ImGui::Button("Reset"))
    {
        transform.reset();
    }
}

void Editor::drawModelInspector(scene::SceneObject& object)
{
    if (!ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (const scene::Model* model = object.getModel())
    {
        const glm::vec3 size = model->getBoundsMax() - model->getBoundsMin();


        statRow("Vertices", "%u", model->getVertexCount());
        statRow("Triangles", "%u", model->getTriangleCount());
        statRow("Submeshes", "%zu", model->getMeshes().size());     // one draw call each
        statRow("Size", "%.2f  %.2f  %.2f", size.x, size.y, size.z);

        ImGui::Separator();
    }
    else
    {
        ImGui::TextDisabled("No model");
    }

    if (loadFromFileField("model", m_modelPath, sizeof(m_modelPath), "path/to/model"))
    {
        // loaded geometry joins scene and goes to selected object
        if (scene::Model* model = m_scene.loadModel(m_modelPath))
        {
            object.setModel(model);
            m_modelError.clear();
        }
        else
        {
            m_modelError = "failed to load " + std::string(m_modelPath);
        }
    }

    errorText(m_modelError);
}

void Editor::drawMaterialInspector(render::Material& material)
{
    if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    // unset terms fall back to model mtl, checkbox takes term over
    bool hasColor = material.hasColor();
    glm::vec3 color = material.getColor();
    if (overrideField("Color", hasColor, [&] { return dragColor3Bare(color); }))
    {
        hasColor ? material.setColor(color) : material.clearColor();
    }

    bool hasSpecular = material.hasSpecular();
    glm::vec3 specular = material.getSpecular();
    if (overrideField("Specular", hasSpecular, [&] { return dragColor3Bare(specular); }))
    {
        hasSpecular ? material.setSpecular(specular) : material.clearSpecular();
    }

    bool hasEmissive = material.hasEmissive();
    glm::vec3 emissive = material.getEmissive();
    if (overrideField("Emissive", hasEmissive, [&] { return dragColor3Bare(emissive); }))
    {
        hasEmissive ? material.setEmissive(emissive) : material.clearEmissive();
    }

    bool hasShininess = material.hasShininess();
    float shininess = material.getShininess();
    if (overrideField("Shininess", hasShininess, [&] { return dragScalarBare(shininess, SHININESS_MINIMUM, SHININESS_MAXIMUM, "%.0f"); }))
    {
        hasShininess ? material.setShininess(shininess) : material.clearShininess();
    }

    // tight highlight only shows at right angle, low values make specular obvious
    if (hasShininess && ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("low values spread the highlight, high values tighten it");
    }
}

void Editor::drawTextureInspector(render::Material& material)
{
    if (!ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    bool anyDrawn = false;

    for (const render::TextureSlot& slot : material.getTextures())
    {
        if (!slot.texture)
        {
            continue;
        }

        anyDrawn = true;
        ImGui::PushID(slot.uniformName.c_str());

        // preview says what it is, uniform name means nothing to user
        ImGui::Image(static_cast<ImTextureID>(slot.texture->id()),
                     {TEXTURE_PREVIEW_SIZE, TEXTURE_PREVIEW_SIZE});
        ImGui::SameLine();

        // size and button share column beside thumbnail
        ImGui::BeginGroup();
        ImGui::Text("%d x %d", slot.texture->getWidth(), slot.texture->getHeight());

        const bool remove = ImGui::Button("Remove");
        ImGui::EndGroup();

        ImGui::PopID();

        if (remove)
        {
            material.clearTexture(slot.uniformName);
            break;                  // the list just changed under us
        }
    }

    // separator needs something above to separate from
    if (anyDrawn)
    {
        ImGui::Separator();
    }
    else
    {
        ImGui::TextDisabled("No textures");
    }

    if (loadFromFileField("texture", m_texturePath, sizeof(m_texturePath), "path/to/texture"))
    {
        if (auto texture = render::TextureLoader::instance().load(m_texturePath))
        {
            material.setTexture(ALBEDO_UNIFORM, texture, ALBEDO_UNIT);
            m_textureError.clear();
        }
        else
        {
            m_textureError = "failed to load " + std::string(m_texturePath);
        }
    }

    errorText(m_textureError);
}

} // namespace interface
} // namespace BulletRender
