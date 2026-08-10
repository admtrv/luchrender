/*
 * Light.cpp
 */

#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>

namespace BulletRender {
namespace scene {

// dot product above this means direction is collinear with up
constexpr float COLLINEAR_DOT = 0.999f;

// Light

void Light::setColor(const glm::vec3& c)
{
    m_color = c;
}

glm::vec3 Light::getColor() const
{
    return m_color;
}

void Light::setIntensity(float i)
{
    m_intensity = i;
}

float Light::getIntensity() const
{
    return m_intensity;
}

void Light::setCastsShadow(bool v)
{
    m_castsShadow = v;
}

bool Light::getCastsShadow() const
{
    return m_castsShadow;
}

// DirectionalLight

DirectionalLight::DirectionalLight(glm::vec3 dir) : Light("Directional Light"), m_direction(glm::normalize(dir))
{
    m_castsShadow = true;
}

void DirectionalLight::setDirection(const glm::vec3& d)
{
    m_direction = glm::normalize(d);
}

glm::vec3 DirectionalLight::getDirection() const
{
    return m_direction;
}

void DirectionalLight::setShadowOrthoSize(float halfSize)
{
    m_orthoHalfSize = halfSize;
}

float DirectionalLight::getShadowOrthoSize() const
{
    return m_orthoHalfSize;
}

void DirectionalLight::setShadowTarget(const glm::vec3& t)
{
    m_shadowTarget = t;
}

glm::vec3 DirectionalLight::getShadowTarget() const
{
    return m_shadowTarget;
}

glm::mat4 DirectionalLight::getViewProj() const
{
    // place virtual camera back along the light direction, looking at the target
    float dist = m_orthoHalfSize * 2.0f;
    glm::vec3 eye = m_shadowTarget + m_direction * dist;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(m_direction, up)) > COLLINEAR_DOT)
    {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat4 view = glm::lookAt(eye, m_shadowTarget, up);
    glm::mat4 proj = glm::ortho(-m_orthoHalfSize, m_orthoHalfSize,
                                -m_orthoHalfSize, m_orthoHalfSize,
                                0.1f, dist * 2.0f);
    return proj * view;
}

// PointLight

PointLight::PointLight(glm::vec3 pos, float range) : Light("Point Light"), m_position(pos), m_range(range) {}

void PointLight::setPosition(const glm::vec3& p)
{
    m_position = p;
}

glm::vec3 PointLight::getPosition() const
{
    return m_position;
}

void PointLight::setRange(float r)
{
    m_range = r;
}

float PointLight::getRange() const
{
    return m_range;
}

// SpotLight

SpotLight::SpotLight(glm::vec3 pos, glm::vec3 dir, float innerDeg, float outerDeg, float range)
    : Light("Spot Light"),
      m_position(pos),
      m_direction(glm::normalize(dir)),
      m_innerCos(glm::cos(glm::radians(innerDeg))),
      m_outerCos(glm::cos(glm::radians(outerDeg))),
      m_range(range)
{
    m_castsShadow = true;
}

void SpotLight::setPosition(const glm::vec3& p)
{
    m_position = p;
}

glm::vec3 SpotLight::getPosition() const
{
    return m_position;
}

void SpotLight::setDirection(const glm::vec3& d)
{
    m_direction = glm::normalize(d);
}

glm::vec3 SpotLight::getDirection() const
{
    return m_direction;
}

void SpotLight::setCones(float innerDeg, float outerDeg)
{
    m_innerCos = glm::cos(glm::radians(innerDeg));
    m_outerCos = glm::cos(glm::radians(outerDeg));
}

float SpotLight::getInnerCos() const
{
    return m_innerCos;
}

float SpotLight::getOuterCos() const
{
    return m_outerCos;
}

void SpotLight::setRange(float r)
{
    m_range = r;
}

float SpotLight::getRange() const
{
    return m_range;
}

glm::mat4 SpotLight::getViewProj() const
{
    // fov from outer cone angle (cos -> angle, doubled because cone is half-angle)
    float outerAngle = glm::acos(glm::clamp(m_outerCos, -1.0f, 1.0f));
    float fov = outerAngle * 2.0f;

    glm::vec3 eye = m_position;
    glm::vec3 target = m_position + m_direction;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(m_direction, up)) > COLLINEAR_DOT)
    {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat4 view = glm::lookAt(eye, target, up);
    glm::mat4 proj = glm::perspective(fov, 1.0f, 0.1f, m_range);
    return proj * view;
}

} // namespace scene
} // namespace BulletRender
