/*
 * Grid.cpp
 */

#include "Grid.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glad/glad.h>

namespace BulletRender {
namespace render {

Grid::Grid(bool enabled) : m_enabled(enabled) {
    m_prog = std::make_shared<GraphicsShader>(VERT_PATH, FRAG_PATH);
    glGenVertexArrays(1, &m_Vao);
}

Grid::~Grid() {
    if (m_Vao)
    {
        glDeleteVertexArrays(1, &m_Vao);
    }
}

void Grid::render(const scene::Scene& scene)
{
    if (!m_enabled || !m_prog)
    {
        return;
    }

    const scene::Camera* cam = scene.getCamera();

    if (!cam )
    {
        std::cerr << "grid: no camera in scene\n";
        return;
    }

    glm::mat4 view = cam->view();
    glm::mat4 proj = cam->proj(scene.getAspect());

    // depth
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
