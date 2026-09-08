/*
 * DebugDraw.cpp
 */

#include "DebugDraw.h"

#include "Colors.h"
#include "Renderer.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace BulletRender {
namespace render {

constexpr float LENGTH_EPSILON = 1e-6f;     // shorter segments carry no direction

// transform
constexpr float AXIS_LENGTH = 1.0f;
constexpr float ARROW_HEAD_LENGTH = 0.15f;  // same head on every arrow
constexpr float ARROW_HEAD_ANGLE = 0.3f;    // how wide head opens, radians

// light
constexpr float DIR_LIGHT_RAY_LENGTH = 1.0f;
constexpr float DIR_LIGHT_GRID_STEP = 0.35f;
constexpr int DIR_LIGHT_RAYS = 3;               // rays per side of the grid

constexpr float POINT_LIGHT_RADIUS = 0.3f;
constexpr float SPOT_LIGHT_LENGTH = 1.5f;       // opening angle stays real, only length fixed

// camera
constexpr float CAMERA_FOV_DEG = 45.0f;
constexpr float CAMERA_NEAR = 0.2f;
constexpr float CAMERA_FAR = 1.5f;

// primitives
constexpr int SPHERE_MERIDIANS = 4;
constexpr int CIRCLE_SEGMENTS = 32;
constexpr int CONE_EDGES = 8;

// any vector not parallel to given one, used to build basis
static glm::vec3 anyPerpendicular(const glm::vec3& dir)
{
    const glm::vec3 reference = glm::abs(dir.y) < 0.9f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    return glm::normalize(glm::cross(dir, reference));
}

// scene

void DebugDraw::drawScene(const scene::Scene& scene)
{
    if (!m_lines)
    {
        return;
    }

    if (m_showTransforms)
    {
        for (const auto& object : scene.getObjects())
        {
            if (object && object->isVisible())
            {
                drawTransform(object->getTransform());
            }
        }
    }

    if (m_showBounds)
    {
        for (const auto& object : scene.getObjects())
        {
            if (object && object->isVisible() && object->getModel())
            {
                drawBounds(*object->getModel(), object->getTransform().getMatrix());
            }
        }
    }

    if (m_showLights)
    {
        for (const std::unique_ptr<scene::Light>& light : scene.getLights())
        {
            if (light && light->isVisible())
            {
                drawLight(*light);
            }
        }
    }

    if (m_showCameras)
    {
        // the active one is what we look through, its frustum surrounds the viewer
        for (const std::unique_ptr<scene::Camera>& camera : scene.getCameras())
        {
            if (camera && camera.get() != scene.getActiveCamera())
            {
                drawCamera(*camera, Renderer::getAspect());
            }
        }
    }
}

// gizmos

void DebugDraw::drawTransform(const scene::Transform& transform)
{
    const glm::mat4& mat = transform.getMatrix();
    const glm::vec3 origin = glm::vec3(mat[3]);

    // columns carry scale, normalizing keeps every gizmo the same length
    const glm::vec3 axisX = glm::normalize(glm::vec3(mat[0])) * AXIS_LENGTH;
    const glm::vec3 axisY = glm::normalize(glm::vec3(mat[1])) * AXIS_LENGTH;
    const glm::vec3 axisZ = glm::normalize(glm::vec3(mat[2])) * AXIS_LENGTH;

    drawArrow(origin, origin + axisX, colors::AxisX);
    drawArrow(origin, origin + axisY, colors::AxisY);
    drawArrow(origin, origin + axisZ, colors::AxisZ);
}

void DebugDraw::drawLight(const scene::Light& light)
{
    switch (light.getType())
    {
        case scene::LightType::Directional:
        {
            drawDirectionalLight(static_cast<const scene::DirectionalLight&>(light));
            break;
        }
        case scene::LightType::Point:
        {
            drawPointLight(static_cast<const scene::PointLight&>(light));
            break;
        }
        case scene::LightType::Spot:
        {
            drawSpotLight(static_cast<const scene::SpotLight&>(light));
            break;
        }
        case scene::LightType::Ambient:
        {
            break;
        }
    }
}

void DebugDraw::drawDirectionalLight(const scene::DirectionalLight& light)
{
    // direction points towards the light, rays travel opposite way
    const glm::vec3 dir = -glm::normalize(light.getDirection());

    // no position of its own, stand where shadow pass places its virtual camera
    const glm::vec3 center = light.getShadowTarget() + light.getDirection() * light.getShadowOrthoSize() * 2.0f;

    const glm::vec3 right = anyPerpendicular(dir);
    const glm::vec3 up = glm::normalize(glm::cross(dir, right));

    const glm::vec3 color = light.getColor();

    for (int i = -DIR_LIGHT_RAYS / 2; i <= DIR_LIGHT_RAYS / 2; i++)
    {
        for (int j = -DIR_LIGHT_RAYS / 2; j <= DIR_LIGHT_RAYS / 2; j++)
        {
            const glm::vec3 offset = right * (float(i) * DIR_LIGHT_GRID_STEP) + up * (float(j) * DIR_LIGHT_GRID_STEP);
            drawArrow(center + offset, center + offset + dir * DIR_LIGHT_RAY_LENGTH, color);
        }
    }
}

void DebugDraw::drawPointLight(const scene::PointLight& light)
{
    drawSphere(light.getPosition(), POINT_LIGHT_RADIUS, light.getColor());
}

void DebugDraw::drawSpotLight(const scene::SpotLight& light)
{
    // cone keeps real opening angle, only length fixed
    const float outerAngle = glm::acos(glm::clamp(light.getOuterCos(), -1.0f, 1.0f));
    drawCone(light.getPosition(), light.getDirection(), SPOT_LIGHT_LENGTH, outerAngle, light.getColor());
}

void DebugDraw::drawCamera(const scene::Camera& camera, float aspect)
{
    // real far plane stretch gizmo across scene, short frustum reads better
    const glm::mat4 proj = glm::perspective(glm::radians(CAMERA_FOV_DEG), aspect, CAMERA_NEAR, CAMERA_FAR);
    const glm::mat4 invViewProj = glm::inverse(proj * camera.getView());

    // unit cube in clip space back to world gives frustum corners
    glm::vec3 corners[8];
    int index = 0;
    for (int z = 0; z < 2; z++)
    {
        for (int y = 0; y < 2; y++)
        {
            for (int x = 0; x < 2; x++)
            {
                const glm::vec4 ndc(x ? 1.0f : -1.0f, y ? 1.0f : -1.0f, z ? 1.0f : -1.0f, 1.0f);
                const glm::vec4 world = invViewProj * ndc;
                corners[index++] = glm::vec3(world) / world.w;
            }
        }
    }

    drawBox(corners, colors::Yellow);
}

void DebugDraw::drawBounds(const scene::Model& model, const glm::mat4& matrix)
{
    const glm::vec3 min = model.getBoundsMin();
    const glm::vec3 max = model.getBoundsMax();

    // corners are built in model space and then follow object matrix
    glm::vec3 corners[8];
    int index = 0;
    for (int z = 0; z < 2; z++)
    {
        for (int y = 0; y < 2; y++)
        {
            for (int x = 0; x < 2; x++)
            {
                const glm::vec4 local(x ? max.x : min.x, y ? max.y : min.y, z ? max.z : min.z, 1.0f);
                corners[index++] = glm::vec3(matrix * local);
            }
        }
    }

    drawBox(corners, colors::White);
}

// primitives

void DebugDraw::drawBox(const glm::vec3 corners[8], const glm::vec3& color)
{
    // corners ordered by bits, x - bit 0, y - bit 1, z - bit 2
    static const int edges[12][2] = {
        {0,1}, {1,3}, {3,2}, {2,0},     // near face
        {4,5}, {5,7}, {7,6}, {6,4},     // far face
        {0,4}, {1,5}, {2,6}, {3,7}      // sides
    };

    for (const auto& edge : edges)
    {
        m_lines->addLine(corners[edge[0]], corners[edge[1]], color);
    }
}

void DebugDraw::drawCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const glm::vec3& color, int segments)
{
    const glm::vec3 axis = glm::normalize(normal);
    const glm::vec3 right = anyPerpendicular(axis) * radius;
    const glm::vec3 up = glm::normalize(glm::cross(axis, right)) * radius;

    glm::vec3 previous = center + right;
    for (int i = 1; i <= segments; i++)
    {
        const float angle = glm::two_pi<float>() * float(i) / float(segments);
        const glm::vec3 point = center + right * glm::cos(angle) + up * glm::sin(angle);
        m_lines->addLine(previous, point, color);
        previous = point;
    }
}

void DebugDraw::drawSphere(const glm::vec3& center, float radius, const glm::vec3& color)
{
    drawSphere(center, radius, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), color);
}

void DebugDraw::drawSphere(const glm::vec3& center, float radius, const glm::quat& orientation, const glm::vec3& color)
{
    // meridians share poles, their normals lie in horizontal plane
    for (int i = 0; i < SPHERE_MERIDIANS; i++)
    {
        const float angle = glm::pi<float>() * float(i) / float(SPHERE_MERIDIANS);
        const glm::vec3 normal(glm::cos(angle), 0.0f, glm::sin(angle));

        drawCircle(center, orientation * normal, radius, color, CIRCLE_SEGMENTS);
    }
}

void DebugDraw::drawArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color)
{
    const glm::vec3 delta = to - from;
    const float length = glm::length(delta);
    if (length < LENGTH_EPSILON)
    {
        return;
    }

    const glm::vec3 dir = delta / length;
    const glm::vec3 base = to - dir * ARROW_HEAD_LENGTH;

    // shaft stops at head, so outline stays readable
    m_lines->addLine(from, base, color);

    // head is cone standing on its tip, apex at the far end of arrow
    drawCone(to, -dir, ARROW_HEAD_LENGTH, ARROW_HEAD_ANGLE, color);
}

void DebugDraw::drawCone(const glm::vec3& apex, const glm::vec3& direction, float length, float angleRad, const glm::vec3& color)
{
    const glm::vec3 dir = glm::normalize(direction);
    const glm::vec3 base = apex + dir * length;
    const float radius = length * glm::tan(angleRad);

    drawCircle(base, dir, radius, color, CONE_EDGES);

    // edges from apex to the base circle
    const glm::vec3 right = anyPerpendicular(dir) * radius;
    const glm::vec3 up = glm::normalize(glm::cross(dir, right)) * radius;

    for (int i = 0; i < CONE_EDGES; i++)
    {
        const float angle = glm::two_pi<float>() * float(i) / float(CONE_EDGES);
        m_lines->addLine(apex, base + right * glm::cos(angle) + up * glm::sin(angle), color);
    }
}

} // namespace render
} // namespace BulletRender
