/*
 * DepthFrameBuffer.cpp
 */

#include "DepthFrameBuffer.h"

#include <iostream>

namespace BulletRender {
namespace render {

DepthFrameBuffer::DepthFrameBuffer(int width, int height) : m_width(width), m_height(height)
{
    create();
}

DepthFrameBuffer::~DepthFrameBuffer()
{
    destroy();
}

void DepthFrameBuffer::create()
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // depth texture
    glGenTextures(1, &m_depthTex);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
                 m_width, m_height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clamp to white border so samples outside the shadow map are not in shadow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

    // no color attachment for depth-only FBO
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "depth frame buffer is not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::destroy()
{
    if (m_depthTex)
    {
        glDeleteTextures(1, &m_depthTex);
        m_depthTex = 0;
    }
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

void DepthFrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void DepthFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace render
} // namespace BulletRender
