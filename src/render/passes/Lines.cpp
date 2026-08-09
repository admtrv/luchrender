/*
 * Lines.cpp
 */

#include "Lines.h"

#include "render/Renderer.h"

namespace BulletRender {
namespace render {

Lines::Lines(float thicknessPx) : m_thickness(thicknessPx) {
    m_prog = std::make_shared<GraphicsShader>(LINES_VERT_PATH, LINES_FRAG_PATH);
    ensureGpu_();
}

Lines::~Lines()
{
    if (m_Vbo)
    {
        glDeleteBuffers(1, &m_Vbo);
    }
    if (m_Vao)
    {
        glDeleteVertexArrays(1, &m_Vao);
    }
}

void Lines::ensureGpu_()
{
    if (m_Vao)
    {
        return;
    }

    glGenVertexArrays(1, &m_Vao);
    glGenBuffers(1, &m_Vbo);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);

    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)sizeof(glm::vec3));

    glBindVertexArray(0);
}

void Lines::setThickness(float px) { m_thickness = px; }

void Lines::addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
{
    m_vertices.push_back({a, color});
    m_vertices.push_back({b, color});
}

void Lines::addPolyline(const std::vector<glm::vec3>& pts, const glm::vec3& color)
{
    if (pts.size() < 2)
    {
        return;
    }

    for (size_t i = 1; i < pts.size(); i++)
    {
        addLine(pts[i - 1], pts[i], color);
    }
}

void Lines::render(const scene::Scene& scene)
{
    if (!m_prog || m_vertices.empty())
    {
        return;
    }

    const scene::Camera* cam = scene.getActiveCamera();
    if (!cam)
    {
        std::cerr << "lines: no camera in scene\n";
        clearCpu_();
        return;
    }

    // depth state, disabled lines show through geometry
    const GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (m_depthTest)
    {
        if (!depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
    else if (depthEnabled)
    {
        glDisable(GL_DEPTH_TEST);
    }

    // shader
    glm::mat4 view = cam->getView();
    glm::mat4 proj = cam->getProj(Renderer::getAspect());

    m_prog->bind();
    m_prog->setMat4("uView", view);
    m_prog->setMat4("uProj", proj);

    // geometry
    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(m_vertices.size() * sizeof(LineVertex)), m_vertices.data(), GL_DYNAMIC_DRAW);

    glLineWidth(m_thickness);
    glDrawArrays(GL_LINES, 0, (GLint)m_vertices.size());
    glBindVertexArray(0);

    // restore whatever caller had
    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    clearCpu_();
}

void Lines::clearCpu_()
{
    m_vertices.clear();
}

} // namespace render
} // namespace BulletRender
