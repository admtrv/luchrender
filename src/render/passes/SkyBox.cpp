/*
 * SkyBox.cpp
 */

#include "SkyBox.h"

#include "render/Renderer.h"

#include "scene/Camera.h"

namespace BulletRender {
namespace render {

SkyBox::SkyBox(std::shared_ptr<CubeMap> cubemap) : m_cubemap(std::move(cubemap))
{
    m_shader = std::make_shared<GraphicsShader>(SKYBOX_VERT_PATH, SKYBOX_FRAG_PATH);
    buildCube();
}

SkyBox::~SkyBox()
{
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void SkyBox::buildCube()
{
    static const float kCube[] = {
        // +X
         1, -1, -1,   1,  1, -1,   1,  1,  1,
         1,  1,  1,   1, -1,  1,   1, -1, -1,
        // -X
        -1, -1,  1,  -1,  1,  1,  -1,  1, -1,
        -1,  1, -1,  -1, -1, -1,  -1, -1,  1,
        // +Y
        -1,  1, -1,   1,  1, -1,   1,  1,  1,
         1,  1,  1,  -1,  1,  1,  -1,  1, -1,
        // -Y
        -1, -1,  1,   1, -1,  1,   1, -1, -1,
         1, -1, -1,  -1, -1, -1,  -1, -1,  1,
        // +Z
        -1, -1,  1,   1, -1,  1,   1,  1,  1,
         1,  1,  1,  -1,  1,  1,  -1, -1,  1,
        // -Z
         1, -1, -1,  -1, -1, -1,  -1,  1, -1,
        -1,  1, -1,   1,  1, -1,   1, -1, -1,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCube), kCube, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void SkyBox::render(const scene::Scene& scene)
{
    if (!m_enabled || !m_cubemap || !m_shader)
    {
        return;
    }

    const scene::Camera* cam = scene.getCamera();
    if (!cam)
    {
        return;
    }

    GLint prevDepthFunc = GL_LESS;
    glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);
    glDepthFunc(GL_LEQUAL);

    m_shader->bind();
    m_shader->setMat4("uView", cam->getView());
    m_shader->setMat4("uProj", cam->getProj(Renderer::getAspect()));
    m_cubemap->bind(0);
    m_shader->setInt("uSkybox", 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(prevDepthFunc);
}

} // namespace render
} // namespace BulletRender
