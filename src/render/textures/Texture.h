/*
 * Texture.h
 */

#pragma once

#include <glad/glad.h>

namespace BulletRender {
namespace render {

struct SamplerConfig {
    GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum magFilter = GL_LINEAR;
    GLenum wrapS = GL_REPEAT;
    GLenum wrapT = GL_REPEAT;
    GLenum wrapR = GL_REPEAT;
    float maxAnisotropy = 1.0f;
};

// base class for any gl texture object
class Texture {
public:
    virtual ~Texture() = 0;

    unsigned id() const { return m_id; }
    GLenum target() const { return m_target; }

    void bind(unsigned unit) const;

    void setFilter(GLenum minFilter, GLenum magFilter);
    void setWrap(GLenum wrapS, GLenum wrapT);
    void setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR);
    void setAnisotropy(float value);

protected:
    explicit Texture(GLenum target);

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void applySampler(const SamplerConfig& cfg);

    unsigned m_id = 0;
    GLenum m_target;
};

} // namespace render
} // namespace BulletRender
