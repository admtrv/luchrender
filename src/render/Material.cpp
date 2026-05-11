/*
 * Material.cpp
 */

#include "Material.h"

namespace BulletRender {
namespace render {

void Material::setTexture(const std::string& uniformName, std::shared_ptr<Texture2D> texture, unsigned unit)
{
    for (auto& slot : m_textures)
    {
        if (slot.uniformName == uniformName)
        {
            slot.texture = std::move(texture);
            slot.unit = unit;
            return;
        }
    }
    m_textures.push_back({std::move(texture), uniformName, unit});
}

} // namespace render
} // namespace BulletRender
