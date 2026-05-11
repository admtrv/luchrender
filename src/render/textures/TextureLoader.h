/*
 * TextureLoader.h
 */

#pragma once

#include "Texture2D.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace BulletRender {
namespace render {

struct TextureLoadOptions {
    bool sRGB = true;
    bool generateMipmaps = true;
    bool flipVertically = true;
    SamplerConfig sampler{};
};

// classic asset cache for Texture2D, keyed by absolute path
class TextureLoader {
public:
    static TextureLoader& instance();

    std::shared_ptr<Texture2D> load(const std::string& path, const TextureLoadOptions& opts = {});

    void clear();

private:
    TextureLoader() = default;
    ~TextureLoader() = default;

    TextureLoader(const TextureLoader&) = delete;
    TextureLoader& operator=(const TextureLoader&) = delete;

    std::shared_ptr<Texture2D> loadFromDisk(const std::string& path, const TextureLoadOptions& opts);

    std::unordered_map<std::string, std::weak_ptr<Texture2D>> m_cache;
};

} // namespace render
} // namespace BulletRender
