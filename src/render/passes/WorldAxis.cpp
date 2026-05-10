/*
 * WorldAxis.cpp
 */

#include "WorldAxis.h"

#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace BulletRender {
namespace render {

WorldAxis::WorldAxis(bool enabled) : m_enabled(enabled)
{
    m_prog = std::make_shared<GraphicsShader>(AXIS_VERT_PATH, AXIS_FRAG_PATH);
    glGenVertexArrays(1, &m_Vao);
}

WorldAxis::~WorldAxis()
{
    if (m_Vao)
    {
        glDeleteVertexArrays(1, &m_Vao);
    }
}

void WorldAxis::render(const scene::Scene& scene)
{
    if (!m_enabled || !m_prog)
    {
        return;
    }

    const scene::Camera* cam = scene.getCamera();
    if (!cam)
    {
        std::cerr << "axis: no camera in scene\n";
        return;
    }

    glm::mat4 view = cam->view();
    glm::mat4 proj = cam->proj(scene.getAspect());

    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (!depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    m_prog->bind();
    m_prog->setMat4("uInvViewProj", glm::inverse(proj * view));
    m_prog->setMat4("uViewProj", proj * view);
    m_prog->setFloat("uNear", cam->near());
    m_prog->setFloat("uFar", cam->far());

    glBindVertexArray(m_Vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    if (!depthEnabled)
    {
        glDisable(GL_DEPTH_TEST);
    }
}

} // namespace render
} // namespace BulletRender
