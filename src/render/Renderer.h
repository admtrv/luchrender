/*
 * Renderer.h
 */

#pragma once

#include "Shader.h"
#include "buffers/FrameBuffer.h"
#include "buffers/DepthFrameBuffer.h"
#include "scene/Model.h"
#include "scene/Mesh.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "passes/RenderPass.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include <algorithm>
#include <string>
#include <memory>
#include <iostream>

#define SHADOW_VERT_PATH "assets/shaders/shadow.vert.glsl"
#define SHADOW_FRAG_PATH "assets/shaders/shadow.frag.glsl"

namespace BulletRender {
namespace render {

// fwd
class Model;
class Shader;
class IRenderPass;
class FrameBuffer;

struct RenderConfig {
    glm::vec4 backgroundColor;

    explicit RenderConfig(const glm::vec4& color = {0.05f, 0.05f, 0.08f, 1.0f}) : backgroundColor(color) {}
};

class Renderer {
public:
    static void init(const RenderConfig& cfg = RenderConfig());
    static void clear(float r, float g, float b, float a);
    static void render(const scene::Scene& scene);
    static void resizeViewport(int width, int height);

    static void registerPrePass(std::shared_ptr<IRenderPass> pass);
    static void registerOverlayPass(std::shared_ptr<IRenderPass> pass);     // on the final image, untouched by post-passes
    static void registerPostPass(std::shared_ptr<IRenderPass> pass);

    static FrameBuffer* getSceneFrameBuffer() { return s_sceneFbo.get(); }
    static const RenderConfig& getConfig() { return s_config; }

    static float getAspect();

private:
    static void renderBasePass(const scene::Scene& scene);
    static void renderShadowPass(const scene::Scene& scene);
    static void bindShadowMaps(GraphicsShader& shader);

    static std::vector<std::shared_ptr<IRenderPass>> s_pre;
    static std::vector<std::shared_ptr<IRenderPass>> s_overlay;
    static std::vector<std::shared_ptr<IRenderPass>> s_post;

    static std::unique_ptr<FrameBuffer> s_sceneFbo;
    static std::unique_ptr<DepthFrameBuffer> s_dirShadowFbo;
    static std::vector<std::unique_ptr<DepthFrameBuffer>> s_spotShadowFbos;
    static std::shared_ptr<GraphicsShader> s_shadowShader;

    static RenderConfig s_config;

    static int s_viewportWidth;
    static int s_viewportHeight;
};


} // namespace render
} // namespace BulletRender