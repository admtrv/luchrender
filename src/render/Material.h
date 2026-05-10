/*
 * Material.h
 */

#pragma once

#include "Shader.h"

#include <glm/glm.hpp>

#include <memory>

namespace BulletRender {
namespace render {

// simple material (color)
class Material {
public:
    void setShader(std::shared_ptr<GraphicsShader> s) noexcept { m_shader = std::move(s); }
    std::shared_ptr<GraphicsShader> getShader() const noexcept { return m_shader; }

    void setColor(const glm::vec3& c) noexcept { m_color = c; }
    const glm::vec3& getColor() const noexcept { return m_color; }

private:
    std::shared_ptr<GraphicsShader> m_shader;
    glm::vec3 m_color{1.0f, 1.0f, 1.0f};

};

} // namespace render
} // namespace BulletRender