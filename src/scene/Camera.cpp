/*
 * Camera.cpp
 */

#include "Camera.h"

#include "app/Window.h"


namespace BulletRender {
namespace scene {

// Camera

// view matrix is lookAt(pos, target, worldUp) -> upper-left 3x3 is the camera basis as rows
glm::vec3 Camera::getForward() const
{
    glm::mat4 v = getView();
    return -glm::vec3(v[0][2], v[1][2], v[2][2]);
}

glm::vec3 Camera::getRight() const
{
    glm::mat4 v = getView();
    return glm::vec3(v[0][0], v[1][0], v[2][0]);
}

glm::vec3 Camera::getUp() const
{
    glm::mat4 v = getView();
    return glm::vec3(v[0][1], v[1][1], v[2][1]);
}

// StaticCamera

glm::mat4 StaticCamera::getView() const
{
    return glm::lookAt(m_pos, m_target, m_up);
}

glm::mat4 StaticCamera::getProj(float aspect) const
{
    return glm::perspective(glm::radians(m_fovDeg), aspect, m_zNear, m_zFar);
}

// FlyCamera

FlyCamera::FlyCamera(glm::vec3 pos,
        float yaw,
        float pitch,
        float fovDeg,
        float speed,
        float zNear,
        float zFar,
        float mouseSensitivity,
        bool lockCursor)
    : Camera("Fly Camera")
    , m_pos(pos)
    , m_yaw(yaw)
    , m_pitch(pitch)
    , m_fovDeg(fovDeg)
    , m_speed(speed)
    , m_zNear(zNear)
    , m_zFar(zFar)
    , m_sensitivity(mouseSensitivity)
    , m_mode(lockCursor ? app::CursorMode::Captured : app::CursorMode::Normal)
    , m_mouseInit(false)
    , m_lastX(0.0)
    , m_lastY(0.0)
{}

glm::vec3 FlyCamera::forwardDir() const
{
    const float yawRad = glm::radians(m_yaw);
    const float pitchRad = glm::radians(m_pitch);

    glm::vec3 dir;
    dir.x = std::cos(pitchRad) * std::cos(yawRad);
    dir.y = std::sin(pitchRad);
    dir.z = std::cos(pitchRad) * std::sin(yawRad);

    return glm::normalize(dir);
}

glm::mat4 FlyCamera::getView() const
{
    const glm::vec3 f = forwardDir();
    return glm::lookAt(m_pos, m_pos + f, glm::vec3 WORLD_UP);
}

glm::mat4 FlyCamera::getProj(float aspect) const
{
    return glm::perspective(glm::radians(m_fovDeg), aspect > 0.0f ? aspect : 1.0f, m_zNear, m_zFar);
}

void FlyCamera::applyCursorMode()
{
    app::Window::setCursorMode(m_mode);

    // the cursor jumped, so the next frame starts a fresh delta
    m_mouseInit = false;
}

void FlyCamera::toggleCursorMode()
{
    m_mode = (m_mode == app::CursorMode::Captured) ? app::CursorMode::Normal : app::CursorMode::Captured;

    applyCursorMode();
}

void FlyCamera::update(float dt)
{
    // something else may have changed the cursor, put it back the way this camera wants it
    if (app::Window::getCursorMode() != m_mode)
    {
        applyCursorMode();
    }

    const bool togglePressed = app::Window::isKeyDown(utils::InputKey::F1);

    if (togglePressed && !m_prevTogglePressed)
    {
        toggleCursorMode();
    }

    m_prevTogglePressed = togglePressed;

    if (!app::Window::isHovered())
    {
        m_mouseInit = false;
        return;
    }

    // camera rotation
    if (m_mode == app::CursorMode::Captured)
    {
        double x;
        double y;
        app::Window::getCursorPos(x, y);

        if (!m_mouseInit)
        {
            m_lastX = x;
            m_lastY = y;
            m_mouseInit = true;
        }

        const double dx = x - m_lastX;
        const double dy = m_lastY - y;
        m_lastX = x;
        m_lastY = y;

        m_yaw += static_cast<float>(dx) * m_sensitivity;
        m_pitch += static_cast<float>(dy) * m_sensitivity;

        if (m_pitch >  YAW_LIMIT)
        {
            m_pitch =  YAW_LIMIT;
        }
        if (m_pitch < -YAW_LIMIT)
        {
            m_pitch = -YAW_LIMIT;
        }

        if (m_yaw > 180.f)
        {
            m_yaw -= 360.f;
        }
        if (m_yaw < -180.f)
        {
            m_yaw += 360.f;
        }
    }

    // moving
    const glm::vec3 fwd = forwardDir();
    const glm::vec3 right = glm::normalize(glm::cross(fwd, glm::vec3 WORLD_UP));

    const bool boost = app::Window::isKeyDown(utils::InputKey::LEFT_SHIFT) || app::Window::isKeyDown(utils::InputKey::RIGHT_SHIFT);
    const float step = m_speed * (boost ? 4.0f : 1.0f) * dt;

    if (app::Window::isKeyDown(utils::InputKey::W))
    {
        m_pos += fwd   * step;
    }
    if (app::Window::isKeyDown(utils::InputKey::S))
    {
        m_pos -= fwd   * step;
    }
    if (app::Window::isKeyDown(utils::InputKey::A))
    {
        m_pos -= right * step;
    }
    if (app::Window::isKeyDown(utils::InputKey::D))
    {
        m_pos += right * step;
    }
}

// OrbitCamera

OrbitCamera::OrbitCamera(glm::vec3 target, float radius, float fovDeg, float zNear, float zFar)
    : Camera("Orbit Camera"),
      m_target(target), m_radius(radius), m_fovDeg(fovDeg), m_zNear(zNear), m_zFar(zFar)
{}

glm::vec3 OrbitCamera::getPosition() const
{
    float e = glm::clamp(m_elevation, 0.01f, 3.13159f);
    return m_target + glm::vec3(
        m_radius * std::sin(e) * std::cos(m_azimuth),
        m_radius * std::cos(e),
        m_radius * std::sin(e) * std::sin(m_azimuth)
    );
}

void OrbitCamera::setPosition(const glm::vec3& pos)
{
    // orbit itself unchanged, whole rig slides so eye lands on pos
    m_target += pos - getPosition();
}

glm::mat4 OrbitCamera::getView() const
{
    return glm::lookAt(getPosition(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitCamera::getProj(float aspect) const
{
    return glm::perspective(glm::radians(m_fovDeg), aspect, m_zNear, m_zFar);
}

void OrbitCamera::update(float /*dt*/)
{
    double x = 0.0;
    double y = 0.0;
    app::Window::getCursorPos(x, y);

    const bool dragging = app::Window::isMouseDown(app::MouseButton::Left);

    if (dragging)
    {
        m_azimuth   += static_cast<float>(x - m_lastX) * 0.005f;
        m_elevation -= static_cast<float>(y - m_lastY) * 0.005f;
        m_elevation = glm::clamp(m_elevation, 0.01f, 3.13159f);
    }

    m_lastX = x;
    m_lastY = y;

    double scroll = app::Window::consumeScrollDelta();
    if (scroll != 0.0)
    {
        m_radius *= static_cast<float>(std::pow(0.9, scroll));
    }

    m_moving = dragging || scroll != 0.0;
}

} // namespace scene
} // namespace BulletRender
