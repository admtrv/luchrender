/*
 * Texture.cpp
 */

#include "Texture.h"

namespace BulletRender {
namespace render {

Texture::Texture(GLenum target) : m_target(target)
{
    glGenTextures(1, &m_id);
}

Texture::~Texture()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void Texture::bind(unsigned unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(m_target, m_id);
}

void Texture::setFilter(GLenum minFilter, GLenum magFilter)
{
    glBindTexture(m_target, m_id);
    glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
}

void Texture::setWrap(GLenum wrapS, GLenum wrapT)
{
    glBindTexture(m_target, m_id);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapS));
    glTexParameteri(m_target, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapT));
}

void Texture::setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR)
{
    setWrap(wrapS, wrapT);
    glTexParameteri(m_target, GL_TEXTURE_WRAP_R, static_cast<GLint>(wrapR));
}

void Texture::setAnisotropy(float value)
{
#ifdef GL_TEXTURE_MAX_ANISOTROPY
    glBindTexture(m_target, m_id);
    glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY, value);
#else
    (void)value;
#endif
}

void Texture::applySampler(const SamplerConfig& cfg)
{
    setFilter(cfg.minFilter, cfg.magFilter);
    setWrap(cfg.wrapS, cfg.wrapT, cfg.wrapR);
    if (cfg.maxAnisotropy > 1.0f)
    {
        setAnisotropy(cfg.maxAnisotropy);
    }
}

} // namespace render
} // namespace BulletRender
