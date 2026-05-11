/*
 * FullscreenTexturePass.h
 */

#pragma once

#include "FullscreenPass.h"
#include "render/Shader.h"
#include "render/textures/Texture2D.h"

#include <memory>

namespace BulletRender {
namespace render {

#define FULLSCREEN_VERT_PATH "assets/shaders/fullscreen.vert.glsl"
#define FULLSCREEN_FRAG_PATH "assets/shaders/fullscreen.frag.glsl"

// draws a single Texture2D over the whole screen
class FullscreenTexturePass final : public FullscreenPass {
public:
    explicit FullscreenTexturePass(std::shared_ptr<Texture2D> texture);
    FullscreenTexturePass(std::shared_ptr<Texture2D> texture, std::shared_ptr<GraphicsShader> shader);

    void setTexture(std::shared_ptr<Texture2D> texture) { m_texture = std::move(texture); }
    void setShader(std::shared_ptr<GraphicsShader> shader) { m_shader = std::move(shader); }

    std::shared_ptr<Texture2D> getTexture() const { return m_texture; }
    std::shared_ptr<GraphicsShader> getShader() const { return m_shader; }

    void render(const scene::Scene& scene) override;

private:
    std::shared_ptr<Texture2D> m_texture;
    std::shared_ptr<GraphicsShader> m_shader;
};

} // namespace render
} // namespace BulletRender
