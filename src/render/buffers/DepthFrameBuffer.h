/*
 * DepthFrameBuffer.h
 */

#pragma once

#include <glad/glad.h>

namespace BulletRender {
namespace render {

// depth-only FBO used as a shadow map target
class DepthFrameBuffer {
public:
    DepthFrameBuffer(int width, int height);
    ~DepthFrameBuffer();

    DepthFrameBuffer(const DepthFrameBuffer&) = delete;
    DepthFrameBuffer& operator=(const DepthFrameBuffer&) = delete;

    void bind();
    void unbind();

    GLuint getDepthTexture() const { return m_depthTex; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    void create();
    void destroy();

    GLuint m_fbo = 0;
    GLuint m_depthTex = 0;

    int m_width = 0;
    int m_height = 0;
};

} // namespace render
} // namespace BulletRender
