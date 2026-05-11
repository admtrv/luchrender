/*
 * FullscreenTexturePass.cpp
 */

#include "FullscreenTexturePass.h"

namespace BulletRender {
namespace render {

FullscreenTexturePass::FullscreenTexturePass(std::shared_ptr<Texture2D> texture) : m_texture(std::move(texture))
{
    m_shader = std::make_shared<GraphicsShader>(FULLSCREEN_VERT_PATH, FULLSCREEN_FRAG_PATH);
}

FullscreenTexturePass::FullscreenTexturePass(std::shared_ptr<Texture2D> texture, std::shared_ptr<GraphicsShader> shader) : m_texture(std::move(texture)), m_shader(std::move(shader)) {}

void FullscreenTexturePass::render(const scene::Scene&)
{
    if (!m_enabled || !m_texture || !m_shader)
    {
        return;
    }

    GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
    if (depthWasOn) glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_texture->bind(0);
    m_shader->setInt("uTex", 0);

    drawFullscreenTriangle();

    if (depthWasOn) glEnable(GL_DEPTH_TEST);
}

} // namespace render
} // namespace BulletRender
