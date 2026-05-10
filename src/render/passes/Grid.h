/*
 * Grid.h
 */

#pragma once

#include "RenderPass.h"
#include "render/Shader.h"

#include <memory>
#include <glad/glad.h>

namespace BulletRender {
namespace render {

#define VERT_PATH "assets/shaders/grid.vert.glsl"
#define FRAG_PATH "assets/shaders/grid.frag.glsl"

class Grid final : public IRenderPass {
public:
    explicit Grid(bool enabled = true);
    ~Grid();

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void render(const scene::Scene& scene) override;

private:
    std::shared_ptr<GraphicsShader> m_prog;
    bool m_enabled = true;
    GLuint m_Vao = 0;
};

} // namespace render
} // namespace BulletRender
