/*
 * FullscreenPass.h
 */

#pragma once

#include "RenderPass.h"

#include <glad/glad.h>

namespace BulletRender {
namespace render {

// abstract base for any pass that covers the screen with one triangle
class FullscreenPass : public IRenderPass {
public:
    ~FullscreenPass() override;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

protected:
    FullscreenPass();

    // single oversized triangle, no vbo, uses gl_VertexID in the vertex shader
    void drawFullscreenTriangle();

    GLuint m_vao = 0;
    bool m_enabled = true;
};

} // namespace render
} // namespace BulletRender
