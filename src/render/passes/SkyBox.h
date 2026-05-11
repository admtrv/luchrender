/*
 * SkyBox.h
 */

#pragma once

#include "RenderPass.h"
#include "render/Shader.h"
#include "render/textures/CubeMap.h"

#include <glad/glad.h>

#include <memory>

namespace BulletRender {
namespace render {

#define SKYBOX_VERT_PATH "assets/shaders/skybox.vert.glsl"
#define SKYBOX_FRAG_PATH "assets/shaders/skybox.frag.glsl"

// classic inside-out cube skybox
class SkyBox final : public IRenderPass {
public:
    explicit SkyBox(std::shared_ptr<CubeMap> cubemap);
    ~SkyBox() override;

    void setCubeMap(std::shared_ptr<CubeMap> cm) { m_cubemap = std::move(cm); }
    std::shared_ptr<CubeMap> getCubeMap() const { return m_cubemap; }

    void setEnabled(bool e) { m_enabled = e; }
    bool isEnabled() const { return m_enabled; }

    void render(const scene::Scene& scene) override;

private:
    void buildCube();

    std::shared_ptr<CubeMap> m_cubemap;
    std::shared_ptr<GraphicsShader> m_shader;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    bool m_enabled = true;
};

} // namespace render
} // namespace BulletRender
