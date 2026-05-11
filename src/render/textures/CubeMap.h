/*
 * CubeMap.h
 */

#pragma once

#include "Texture.h"

#include <array>
#include <string>

namespace BulletRender {
namespace render {

struct CubeMapConfig {
    GLenum internalFormat = GL_RGBA8;
    bool sRGB = true;
    bool flipVertically = false;
    SamplerConfig sampler{
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .wrapS = GL_CLAMP_TO_EDGE,
        .wrapT = GL_CLAMP_TO_EDGE,
        .wrapR = GL_CLAMP_TO_EDGE,
        .maxAnisotropy = 1.0f,
    };
};

// gl cubemap texture, 6 faces in order: +X, -X, +Y, -Y, +Z, -Z
class CubeMap final : public Texture {
public:
    // 6 separate files
    explicit CubeMap(const std::array<std::string, 6>& faces, const CubeMapConfig& cfg = {});

    // single image, horizontal cross layout (4 wide, 3 tall):
    //       [+Y]
    //   [-X][+Z][+X][-Z]
    //       [-Y]
    explicit CubeMap(const std::string& crossPath, const CubeMapConfig& cfg = {});

    int faceSize() const { return m_faceSize; }
    GLenum internalFormat() const { return m_internalFormat; }

private:
    void uploadFace(GLenum faceTarget, const void* pixels, int size);

    int m_faceSize = 0;
    GLenum m_internalFormat = GL_RGBA8;
};

} // namespace render
} // namespace BulletRender
