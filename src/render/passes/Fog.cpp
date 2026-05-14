/*
 * FogPass.cpp
 */

#include "Fog.h"

namespace BulletRender {
namespace render {

Fog::Fog(bool enabled, float start, float end) : m_enabled(enabled), m_fogStart(start), m_fogEnd(end) {
    m_prog = std::make_shared<GraphicsShader>(FOG_VERT_PATH, FOG_FRAG_PATH);
    glGenVertexArrays(1, &m_Vao);
}

Fog::~Fog() {
    if (m_Vao)
    {
        glDeleteVertexArrays(1, &m_Vao);
    }
}

void Fog::render(const scene::Scene& scene)
{
    if (!m_enabled || !m_prog)
    {
        return;
    }

    const scene::Camera* cam = scene.getCamera();
    if (!cam)
    {
        std::cerr << "fog: no camera in scene\n";
        return;
    }

    FrameBuffer* sceneFbo = Renderer::getSceneFrameBuffer();
    if (!sceneFbo)
    {
        std::cerr << "fog: no scene framebuffer\n";
        return;
    }

    glm::mat4 view = cam->getView();
    glm::mat4 proj = cam->getProj(scene.getAspect());

    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    glDisable(GL_BLEND);

    glm::vec3 fogColor = glm::vec3(Renderer::getConfig().backgroundColor);

    m_prog->bind();
    m_prog->setMat4("uInvViewProj", glm::inverse(proj * view));
    m_prog->setVec3("uCameraPos", cam->getPosition());
    m_prog->setFloat("uFogStart", m_fogStart);
    m_prog->setFloat("uFogEnd", m_fogEnd);
    m_prog->setVec3("uFogColor", fogColor);

    // bind scene textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneFbo->getColorTexture());
    m_prog->setInt("uSceneColor", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sceneFbo->getDepthTexture());
    m_prog->setInt("uSceneDepth", 1);

    glBindVertexArray(m_Vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    if (blendEnabled)
    {
        glEnable(GL_BLEND);
    }

    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
}

} // namespace render
} // namespace BulletRender
