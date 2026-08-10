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

// material: shader + phong terms (kd, ks, ns, ke) + named texture slots
class Material {
public:
    void setShader(std::shared_ptr<GraphicsShader> s) noexcept { m_shader = std::move(s); }
    std::shared_ptr<GraphicsShader> getShader() const noexcept { return m_shader; }

    // kd, diffuse color
    void setColor(const glm::vec3& c) noexcept { m_color = c; }
    void clearColor() noexcept { m_color.reset(); }
    bool hasColor() const noexcept { return m_color.has_value(); }
    glm::vec3 getColor() const noexcept { return m_color.value_or(glm::vec3(1.0f)); }

    // ks, specular color
    void setSpecular(const glm::vec3& c) noexcept { m_specular = c; }
    void clearSpecular() noexcept { m_specular.reset(); }
    bool hasSpecular() const noexcept { return m_specular.has_value(); }
    glm::vec3 getSpecular() const noexcept { return m_specular.value_or(glm::vec3(0.5f)); }

    // ns, shininess exponent
    void setShininess(float n) noexcept { m_shininess = n; }
    void clearShininess() noexcept { m_shininess.reset(); }
    bool hasShininess() const noexcept { return m_shininess.has_value(); }
    float getShininess() const noexcept { return m_shininess.value_or(32.0f); }

    // ke, emissive color
    void setEmissive(const glm::vec3& c) noexcept { m_emissive = c; }
    void clearEmissive() noexcept { m_emissive.reset(); }
    bool hasEmissive() const noexcept { return m_emissive.has_value(); }
    glm::vec3 getEmissive() const noexcept { return m_emissive.value_or(glm::vec3(0.0f)); }

    void setTexture(const std::string& uniformName, std::shared_ptr<Texture2D> texture, unsigned unit);
    void clearTexture(const std::string& uniformName) noexcept;
    void clearTextures() noexcept { m_textures.clear(); }
    const std::vector<TextureSlot>& getTextures() const noexcept { return m_textures; }

private:
    std::shared_ptr<GraphicsShader> m_shader;
    std::optional<glm::vec3> m_color;
    std::optional<glm::vec3> m_specular;
    std::optional<float> m_shininess;
    std::optional<glm::vec3> m_emissive;
    std::vector<TextureSlot> m_textures;
};

} // namespace render
} // namespace BulletRender
