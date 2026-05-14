/*
 * Texture2D.h
 */

#pragma once

#include "Texture.h"

#include <string>

namespace BulletRender {
namespace render {

struct Texture2DConfig {
    GLenum internalFormat = GL_RGBA8;
    bool sRGB = false;
    bool generateMipmaps = false;
    SamplerConfig sampler{};
};

// gl 2d texture, usable both as sampler and as image binding
class Texture2D final : public Texture {
public:
    Texture2D(int width, int height, const Texture2DConfig& cfg = {});

    void resize(int width, int height);
    void uploadPixels(const void* pixels, GLenum dataFormat, GLenum dataType);
    void generateMipmaps();

    void bindAsImage(unsigned unit, GLenum access = GL_WRITE_ONLY) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLenum getInternalFormat() const { return m_internalFormat; }
    bool hasMipmaps() const { return m_hasMipmaps; }

private:
    void allocStorage();

    int m_width = 0;
    int m_height = 0;
    GLenum m_internalFormat = GL_RGBA8;
    bool m_autoMipmaps = false;
    bool m_hasMipmaps = false;
};

} // namespace render
} // namespace BulletRender
