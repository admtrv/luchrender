/*
 * Lines.h
 */

#pragma once

#include "RenderPass.h"
#include "render/Shader.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <memory>

namespace BulletRender {
namespace render {

#define LINES_VERT_PATH "assets/shaders/lines.vert.glsl"
#define LINES_FRAG_PATH "assets/shaders/lines.frag.glsl"

struct LineVertex {
    glm::vec3 pos;
    glm::vec3 color;
};

class Lines final : public IRenderPass {
public:
    Lines(float thicknessPx = 2.0f);
    ~Lines();

    // API for frame
    void setThickness(float px);
    void setDepthTest(bool enabled) { m_depthTest = enabled; }                      // off makes lines show through geometry
    void addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
    void addPolyline(const std::vector<glm::vec3>& pts, const glm::vec3& color);

    void render(const scene::Scene& scene) override;

private:
    void ensureGpu_();
    void clearCpu_();

    std::shared_ptr<GraphicsShader> m_prog;
    GLuint m_Vao = 0;
    GLuint m_Vbo = 0;

    std::vector<LineVertex> m_vertices;
    float m_thickness = 2.0f;
    bool m_depthTest = true;
};

} // namespace render
} // namespace BulletRender
