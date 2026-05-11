/*
 * CubeMap.cpp
 */

#include "CubeMap.h"

#include <stb_image.h>

#include <cstring>
#include <iostream>
#include <vector>

namespace BulletRender {
namespace render {

static GLenum applySrgb(GLenum internalFormat, bool sRGB)
{
    if (!sRGB)
    {
        return internalFormat;
    }
    switch (internalFormat)
    {
        case GL_RGB8:  return GL_SRGB8;
        case GL_RGBA8: return GL_SRGB8_ALPHA8;
        default:       return internalFormat;
    }
}

CubeMap::CubeMap(const std::array<std::string, 6>& faces, const CubeMapConfig& cfg) : Texture(GL_TEXTURE_CUBE_MAP)
{
    m_internalFormat = applySrgb(cfg.internalFormat, cfg.sRGB);

    stbi_set_flip_vertically_on_load(cfg.flipVertically ? 1 : 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    for (int i = 0; i < 6; i++)
    {
        int w, h, c;
        stbi_uc* pixels = stbi_load(faces[i].c_str(), &w, &h, &c, 4);
        if (!pixels)
        {
            std::cerr << "cubemap load failed: " << faces[i] << " (" << stbi_failure_reason() << ")\n";
            continue;
        }

        if (i == 0)
        {
            m_faceSize = w;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     static_cast<GLint>(m_internalFormat),
                     w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        stbi_image_free(pixels);
    }

    applySampler(cfg.sampler);
}

// horizontal cross layout (4 wide, 3 tall):
//          [+Y]
//   [-X][+Z][+X][-Z]
//          [-Y]
CubeMap::CubeMap(const std::string& crossPath, const CubeMapConfig& cfg) : Texture(GL_TEXTURE_CUBE_MAP)
{
    m_internalFormat = applySrgb(cfg.internalFormat, cfg.sRGB);

    stbi_set_flip_vertically_on_load(cfg.flipVertically ? 1 : 0);

    int w, h, c;
    stbi_uc* pixels = stbi_load(crossPath.c_str(), &w, &h, &c, 4);
    if (!pixels)
    {
        std::cerr << "cubemap cross load failed: " << crossPath << " (" << stbi_failure_reason() << ")\n";
        return;
    }

    int face = w / 4;
    if (h != face * 3 || w != face * 4)
    {
        std::cerr << "cubemap cross expects 4x3 layout, got " << w << "x" << h << "\n";
        stbi_image_free(pixels);
        return;
    }

    m_faceSize = face;

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    // grid cell -> opengl face target
    struct Cell { int col; int row; GLenum target; };
    const Cell cells[6] = {
        {2, 1, GL_TEXTURE_CUBE_MAP_POSITIVE_X},   // +X
        {0, 1, GL_TEXTURE_CUBE_MAP_NEGATIVE_X},   // -X
        {1, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y},   // +Y
        {1, 2, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},   // -Y
        {1, 1, GL_TEXTURE_CUBE_MAP_POSITIVE_Z},   // +Z
        {3, 1, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},   // -Z
    };

    std::vector<stbi_uc> buffer(static_cast<size_t>(face) * face * 4);

    for (const Cell& cell : cells)
    {
        int px = cell.col * face;
        int py = cell.row * face;

        for (int row = 0; row < face; row++)
        {
            const stbi_uc* src = pixels + ((py + row) * w + px) * 4;
            stbi_uc* dst = buffer.data() + row * face * 4;
            std::memcpy(dst, src, static_cast<size_t>(face) * 4);
        }

        glTexImage2D(cell.target, 0,
                     static_cast<GLint>(m_internalFormat),
                     face, face, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    }

    stbi_image_free(pixels);

    applySampler(cfg.sampler);
}

void CubeMap::uploadFace(GLenum faceTarget, const void* pixels, int size)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glTexImage2D(faceTarget, 0,
                 static_cast<GLint>(m_internalFormat),
                 size, size, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

} // namespace render
} // namespace BulletRender
