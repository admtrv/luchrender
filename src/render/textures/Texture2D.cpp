/*
 * Texture2D.cpp
 */

#include "Texture2D.h"

namespace BulletRender {
namespace render {

// returns sRGB equivalent of a base format if sRGB flag is set
static GLenum applySrgb(GLenum internalFormat, bool sRGB)
{
    if (!sRGB)
    {
        return internalFormat;
    }

    switch (internalFormat)
    {
        case GL_RGB8:
            return GL_SRGB8;
        case GL_RGBA8:
            return GL_SRGB8_ALPHA8;
        default:
            return internalFormat;
    }
}

Texture2D::Texture2D(int width, int height, const Texture2DConfig& cfg) : Texture(GL_TEXTURE_2D),
    m_width(width),
    m_height(height),
    m_internalFormat(applySrgb(cfg.internalFormat, cfg.sRGB)),
    m_autoMipmaps(cfg.generateMipmaps)
{
    allocStorage();
    applySampler(cfg.sampler);
}

void Texture2D::allocStorage()
{
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(m_internalFormat),
                 m_width, m_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_hasMipmaps = false;
}

void Texture2D::resize(int width, int height)
{
    if (width == m_width && height == m_height)
    {
        return;
    }
    m_width = width;
    m_height = height;
    allocStorage();
}

void Texture2D::uploadPixels(const void* pixels, GLenum dataFormat, GLenum dataType)
{
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(m_internalFormat),
                 m_width, m_height, 0,
                 dataFormat, dataType, pixels);
    m_hasMipmaps = false;

    if (m_autoMipmaps)
    {
        generateMipmaps();
    }
}

void Texture2D::generateMipmaps()
{
    glBindTexture(GL_TEXTURE_2D, m_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    m_hasMipmaps = true;
}

void Texture2D::bindAsImage(unsigned unit, GLenum access) const
{
    glBindImageTexture(unit, m_id, 0, GL_FALSE, 0, access, m_internalFormat);
}

} // namespace render
} // namespace BulletRender
