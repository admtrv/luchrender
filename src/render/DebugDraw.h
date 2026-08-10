/*
 * DebugDraw.h
 */

#pragma once

#include "passes/Lines.h"
#include "scene/Scene.h"
#include "scene/Transform.h"
#include "scene/Model.h"
#include "scene/Light.h"
#include "scene/Camera.h"

#include <glm/glm.hpp>

#include <memory>

namespace BulletRender {
namespace render {

class DebugDraw {
public:
    explicit DebugDraw(std::shared_ptr<Lines> lines) : m_lines(std::move(lines)) {}

    void drawScene(const scene::Scene& scene);

    void setShowTransforms(bool v) { m_showTransforms = v; }
    bool getShowTransforms() const { return m_showTransforms; }

    void setShowLights(bool v) { m_showLights = v; }
    bool getShowLights() const { return m_showLights; }

    void setShowBounds(bool v) { m_showBounds = v; }
    bool getShowBounds() const { return m_showBounds; }

    void setShowCameras(bool v) { m_showCameras = v; }
    bool getShowCameras() const { return m_showCameras; }

private:
    // gizmos

    void drawTransform(const scene::Transform& transform);                      // rgb axes
    void drawBounds(const scene::Model& model, const glm::mat4& matrix);        // model bounds
    void drawCamera(const scene::Camera& camera, float aspect);                 // view frustum

    void drawLight(const scene::Light& light);                          // depends on type
    void drawDirectionalLight(const scene::DirectionalLight& light);
    void drawPointLight(const scene::PointLight& light);
    void drawSpotLight(const scene::SpotLight& light);

    // primitives

    void drawBox(const glm::vec3 corners[8], const glm::vec3& color);
    void drawCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const glm::vec3& color, int segments);
    void drawSphere(const glm::vec3& center, float radius, const glm::vec3& color);
    void drawArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
    void drawCone(const glm::vec3& apex, const glm::vec3& direction, float length, float angleRad, const glm::vec3& color);

    std::shared_ptr<Lines> m_lines;

    bool m_showTransforms = true;
    bool m_showLights = true;
    bool m_showBounds = true;
    bool m_showCameras = true;
};

} // namespace render
} // namespace BulletRender
