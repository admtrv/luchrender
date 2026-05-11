/*
* Camera.h
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

struct GLFWwindow;

namespace BulletRender {
namespace scene {

#define YAW_LIMIT   89.0f
#define WORLD_UP    {0.0f, 1.0f, 0.0f}

// basic camera
class Camera {
public:
    virtual ~Camera() = default;

    virtual glm::mat4 view() const = 0;
    virtual glm::mat4 proj(float aspect) const = 0;

    virtual float near() const = 0;
    virtual float far() const = 0;
    virtual glm::vec3 position() const = 0;

    // orientation basis derived from view() (transpose of upper 3x3)
    virtual glm::vec3 forward() const;
    virtual glm::vec3 right() const;
    virtual glm::vec3 up() const;

    virtual void update(GLFWwindow* win, float dt) {}
};

// static camera (position ->->-> target)
class StaticCamera : public Camera {
public:
    StaticCamera(const glm::vec3& pos,
            const glm::vec3& target,
            const glm::vec3& up = {0.0f, 1.0f, 0.0f},
            float fovDeg = 60.0f,
            float zNear = 0.1f,
            float zFar = 100.0f)
        : m_pos(pos), m_target(target), m_up(up), m_fovDeg(fovDeg), m_zNear(zNear), m_zFar(zFar)
    {}

    glm::mat4 view() const override;
    glm::mat4 proj(float aspect) const override;

    float near() const override { return m_zNear; }
    float far()  const override { return m_zFar; }
    glm::vec3 position() const override { return m_pos; }

private:
    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;
    float m_fovDeg;
    float m_zNear;
    float m_zFar;
};

// fly camera (Move: WASD, Camera: Mouse, Boost: Shift, Toggle Cursor: F1)
class FlyCamera : public Camera {
public:
    explicit FlyCamera(glm::vec3 pos = {0,1,5},
                       float yaw = -90.f,
                       float pitch = 0.f,
                       float fovDeg = 60.f,
                       float speed = 3.f,
                       float zNear = 0.1f,
                       float zFar = 100.f,
                       float mouseSensitivity = 0.1f,
                       bool lockCursor = true);

    glm::mat4 view() const override;
    glm::mat4 proj(float aspect) const override;

    float near() const override { return m_zNear; }
    float far()  const override { return m_zFar; }
    glm::vec3 position() const override { return m_pos; }

    void update(GLFWwindow* win, float dt) override;

private:
    enum class CursorMode {Locked, Free};

    glm::vec3 forwardDir() const;
    void applyCursorMode(GLFWwindow* win);
    void toggleCursorMode(GLFWwindow* win);

    glm::vec3 m_pos;
    float m_yaw;
    float m_pitch;
    float m_fovDeg;
    float m_speed;
    float m_zNear;
    float m_zFar;

    float m_sensitivity;
    CursorMode m_mode;
    bool m_mouseInit;
    double m_lastX;
    double m_lastY;

    bool m_prevTogglePressed = false;
};

// orbit camera (LMB drag = rotate, scroll = zoom)
class OrbitCamera : public Camera {
public:
    explicit OrbitCamera(glm::vec3 target = {0, 0, 0},
                         float radius = 10.0f,
                         float fovDeg = 60.0f,
                         float zNear = 0.1f,
                         float zFar = 1000.0f);

    glm::mat4 view() const override;
    glm::mat4 proj(float aspect) const override;
    glm::vec3 position() const override;

    float near() const override { return m_zNear; }
    float far()  const override { return m_zFar; }

    glm::vec3 target() const { return m_target; }
    float radius() const { return m_radius; }
    bool isMoving() const { return m_moving; }

    void update(GLFWwindow* win, float dt) override;

private:
    glm::vec3 m_target;
    float m_radius;
    float m_azimuth = 0.0f;
    float m_elevation = 1.5707963f;
    float m_fovDeg;
    float m_zNear;
    float m_zFar;

    double m_lastX = 0.0;
    double m_lastY = 0.0;
    bool m_moving = false;
};

} // namespace scene
} // namespace BulletRender
