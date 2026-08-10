/*
 * FogPass.h
 */

#pragma once

#include "RenderPass.h"
#include "render/Shader.h"
#include "render/Renderer.h"

#include <glm/gtc/matrix_inverse.hpp>

#include <memory>
#include <iostream>

#include <glm/gtc/matrix_inverse.hpp>
#include <glad/glad.h>

namespace BulletRender {
namespace render {

#define FOG_VERT_PATH "assets/shaders/fog.vert.glsl"
#define FOG_FRAG_PATH "assets/shaders/fog.frag.glsl"

// post-process fog pass using depth buffer
class Fog final : public IRenderPass {
public:
    explicit Fog(bool enabled = true, float start = 5.0f, float end = 50.0f);
    ~Fog();

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void setRange(float start, float end) { m_fogStart = start; m_fogEnd = end; }
    float getStart() const { return m_fogStart; }
    float getEnd() const { return m_fogEnd; }

    void render(const scene::Scene& scene) override;
    bool isActive() const override { return m_enabled; }

private:
    std::shared_ptr<GraphicsShader> m_prog;
    bool m_enabled = true;
    float m_fogStart = 5.0f;
    float m_fogEnd = 50.0f;
    GLuint m_Vao = 0;
};

} // namespace render
} // namespace BulletRender
