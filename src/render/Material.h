/*
 * Material.h
 */

#pragma once

#include "Shader.h"
#include "textures/Texture2D.h"

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace BulletRender {
namespace render {

struct TextureSlot {
    std::shared_ptr<Texture2D> texture;
    std::string uniformName;
    unsigned unit;
};

// material: shader + base color + named texture slots
class Material {
public:
    void setShader(std::shared_ptr<GraphicsShader> s) noexcept { m_shader = std::move(s); }
    std::shared_ptr<GraphicsShader> getShader() const noexcept { return m_shader; }

    void setColor(const glm::vec3& c) noexcept { m_color = c; }
    void clearColor() noexcept { m_color.reset(); }
    bool hasColor() const noexcept { return m_color.has_value(); }
    glm::vec3 getColor() const noexcept { return m_color.value_or(glm::vec3(1.0f)); }

    void setTexture(const std::string& uniformName, std::shared_ptr<Texture2D> texture, unsigned unit);
    void clearTextures() noexcept { m_textures.clear(); }
    const std::vector<TextureSlot>& getTextures() const noexcept { return m_textures; }

private:
    std::shared_ptr<GraphicsShader> m_shader;
    std::optional<glm::vec3> m_color;
    std::vector<TextureSlot> m_textures;
};

} // namespace render
} // namespace BulletRender
