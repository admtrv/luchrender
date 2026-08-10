/*
* Camera.h
 */

#pragma once

#include "Named.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

struct GLFWwindow;

namespace BulletRender {
namespace scene {

#define YAW_LIMIT   89.0f
#define WORLD_UP    {0.0f, 1.0f, 0.0f}

enum class CameraType {
    Static,
    Fly,
    Orbit
};

// basic camera
class Camera : public Named {
public:
    explicit Camera(std::string name = "Camera") : Named(std::move(name)) {}
    virtual ~Camera() = default;

    virtual CameraType getType() const = 0;

    virtual glm::mat4 getView() const = 0;
    virtual glm::mat4 getProj(float aspect) const = 0;

    virtual float getNear() const = 0;
    virtual float getFar() const = 0;
    virtual glm::vec3 getPosition() const = 0;

    virtual void setPosition(const glm::vec3& pos) = 0;
    virtual void setClipPlanes(float zNear, float zFar) = 0;

    virtual float getFov() const = 0;
    virtual void setFov(float fovDeg) = 0;

    // orientation basis derived from getView() (transpose of upper 3x3)
    virtual glm::vec3 getForward() const;
    virtual glm::vec3 getRight() const;
    virtual glm::vec3 getUp() const;

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
        : Camera("Static Camera"),
          m_pos(pos), m_target(target), m_up(up), m_fovDeg(fovDeg), m_zNear(zNear), m_zFar(zFar)
    {}

    CameraType getType() const override { return CameraType::Static; }

    glm::mat4 getView() const override;
    glm::mat4 getProj(float aspect) const override;

    float getNear() const override { return m_zNear; }
    float getFar()  const override { return m_zFar; }
    glm::vec3 getPosition() const override { return m_pos; }

    void setPosition(const glm::vec3& pos) override { m_pos = pos; }
    void setClipPlanes(float zNear, float zFar) override { m_zNear = zNear; m_zFar = zFar; }

    float getFov() const override { return m_fovDeg; }
    void setFov(float fovDeg) override { m_fovDeg = fovDeg; }

    void setTarget(const glm::vec3& target) { m_target = target; }
    glm::vec3 getTarget() const { return m_target; }

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

    CameraType getType() const override { return CameraType::Fly; }

    glm::mat4 getView() const override;
    glm::mat4 getProj(float aspect) const override;

    float getNear() const override { return m_zNear; }
    float getFar()  const override { return m_zFar; }
    glm::vec3 getPosition() const override { return m_pos; }

    void setPosition(const glm::vec3& pos) override { m_pos = pos; }
    void setClipPlanes(float zNear, float zFar) override { m_zNear = zNear; m_zFar = zFar; }

    float getFov() const override { return m_fovDeg; }
    void setFov(float fovDeg) override { m_fovDeg = fovDeg; }

    float getSpeed() const { return m_speed; }
    void setSpeed(float speed) { m_speed = speed; }

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

    CameraType getType() const override { return CameraType::Orbit; }

    glm::mat4 getView() const override;
    glm::mat4 getProj(float aspect) const override;
    glm::vec3 getPosition() const override;

    float getNear() const override { return m_zNear; }
    float getFar()  const override { return m_zFar; }

    void setPosition(const glm::vec3& pos) override;
    void setClipPlanes(float zNear, float zFar) override { m_zNear = zNear; m_zFar = zFar; }

    float getFov() const override { return m_fovDeg; }
    void setFov(float fovDeg) override { m_fovDeg = fovDeg; }

    glm::vec3 getTarget() const { return m_target; }
    void setTarget(const glm::vec3& target) { m_target = target; }

    float getRadius() const { return m_radius; }
    void setRadius(float radius) { m_radius = radius; }
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
