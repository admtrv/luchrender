/*
 * FullscreenPass.cpp
 */

#include "FullscreenPass.h"

namespace BulletRender {
namespace render {

FullscreenPass::FullscreenPass()
{
    glGenVertexArrays(1, &m_vao);
}

FullscreenPass::~FullscreenPass()
{
    if (m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

void FullscreenPass::drawFullscreenTriangle()
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

} // namespace render
} // namespace BulletRender
