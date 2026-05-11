/*
 * TextureLoader.cpp
 */

#include "TextureLoader.h"

#include <stb_image.h>

#include <iostream>

namespace BulletRender {
namespace render {

TextureLoader& TextureLoader::instance()
{
    static TextureLoader inst;
    return inst;
}

std::shared_ptr<Texture2D> TextureLoader::load(const std::string& path, const TextureLoadOptions& opts)
{
    auto it = m_cache.find(path);
    if (it != m_cache.end())
    {
        if (auto cached = it->second.lock())
        {
            return cached;
        }
        m_cache.erase(it);
    }

    auto tex = loadFromDisk(path, opts);
    if (tex)
    {
        m_cache[path] = tex;
    }
    return tex;
}

void TextureLoader::clear()
{
    m_cache.clear();
}

std::shared_ptr<Texture2D> TextureLoader::loadFromDisk(const std::string& path, const TextureLoadOptions& opts)
{
    stbi_set_flip_vertically_on_load(opts.flipVertically ? 1 : 0);

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!pixels)
    {
        std::cerr << "texture load failed: " << path << " (" << stbi_failure_reason() << ")\n";
        return nullptr;
    }

    Texture2DConfig cfg;
    cfg.internalFormat = GL_RGBA8;
    cfg.sRGB = opts.sRGB;
    cfg.generateMipmaps = opts.generateMipmaps;
    cfg.sampler = opts.sampler;

    auto tex = std::make_shared<Texture2D>(width, height, cfg);
    tex->uploadPixels(pixels, GL_RGBA, GL_UNSIGNED_BYTE);

    stbi_image_free(pixels);
    return tex;
}

} // namespace render
} // namespace BulletRender
