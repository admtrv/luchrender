/*
 * Renderer.cpp
 */

#include "Renderer.h"
#include "app/Window.h"

namespace BulletRender {
namespace render {

std::vector<std::shared_ptr<IRenderPass>> Renderer::s_pre;
std::vector<std::shared_ptr<IRenderPass>> Renderer::s_post;
std::unique_ptr<FrameBuffer> Renderer::s_sceneFbo;
std::unique_ptr<DepthFrameBuffer> Renderer::s_dirShadowFbo;
std::vector<std::unique_ptr<DepthFrameBuffer>> Renderer::s_spotShadowFbos;
std::shared_ptr<GraphicsShader> Renderer::s_shadowShader;
RenderConfig Renderer::s_config;
int Renderer::s_viewportWidth = 1;
int Renderer::s_viewportHeight = 1;

static constexpr int DIR_SHADOW_SIZE = 2048;
static constexpr int SPOT_SHADOW_SIZE = 1024;
static constexpr int MAX_SPOT_SHADOW_MAPS = 3;

// shader-side limits, must match counts in normal.frag.glsl
static constexpr int MAX_POINT_LIGHTS = 3;
static constexpr int MAX_SPOT_LIGHTS = 3;

// shadow map texture units, kept above any user-bound albedo
static constexpr unsigned DIR_SHADOW_UNIT = 8;
static constexpr unsigned SPOT_SHADOW_BASE_UNIT = 9; // 9, 10, 11 for 3 spots

// per-frame snapshot of scene lighting, bucketed by type
struct LightUniforms {
    glm::vec3 ambientColor{0.0f};

    bool hasDirectional = false;
    glm::vec3 dirDirection{0.0f, 1.0f, 0.0f};
    glm::vec3 dirColor{0.0f};
    bool dirCastsShadow = false;
    glm::mat4 dirLightVP{1.0f};
    const scene::DirectionalLight* dirLightPtr = nullptr;

    int pointCount = 0;
    glm::vec3 pointPos[MAX_POINT_LIGHTS]{};
    glm::vec3 pointColor[MAX_POINT_LIGHTS]{};
    float pointRange[MAX_POINT_LIGHTS]{};

    int spotCount = 0;
    glm::vec3 spotPos[MAX_SPOT_LIGHTS]{};
    glm::vec3 spotDir[MAX_SPOT_LIGHTS]{};
    glm::vec3 spotColor[MAX_SPOT_LIGHTS]{};
    float spotRange[MAX_SPOT_LIGHTS]{};
    float spotInnerCos[MAX_SPOT_LIGHTS]{};
    float spotOuterCos[MAX_SPOT_LIGHTS]{};
    int spotCastsShadow[MAX_SPOT_LIGHTS]{};
    glm::mat4 spotLightVP[MAX_SPOT_LIGHTS]{};
    const scene::SpotLight* spotLightPtrs[MAX_SPOT_LIGHTS]{};
};

static LightUniforms collectLights(const scene::Scene& scene)
{
    LightUniforms out;

    for (const scene::Light* light : scene.getLights())
    {
        if (!light) continue;

        glm::vec3 weighted = light->getColor() * light->getIntensity();

        switch (light->getType())
        {
            case scene::LightType::Ambient:
            {
                out.ambientColor += weighted;
                break;
            }
            case scene::LightType::Directional:
            {
                // only first directional wins, rest ignored
                if (!out.hasDirectional)
                {
                    auto* d = static_cast<const scene::DirectionalLight*>(light);
                    out.hasDirectional = true;
                    out.dirDirection = d->getDirection();
                    out.dirColor = weighted;
                    out.dirCastsShadow = d->getCastsShadow();
                    if (out.dirCastsShadow)
                    {
                        out.dirLightVP = d->getViewProj();
                        out.dirLightPtr = d;
                    }
                }
                break;
            }
            case scene::LightType::Point:
            {
                if (out.pointCount < MAX_POINT_LIGHTS)
                {
                    auto* p = static_cast<const scene::PointLight*>(light);
                    int i = out.pointCount++;
                    out.pointPos[i] = p->getPosition();
                    out.pointColor[i] = weighted;
                    out.pointRange[i] = p->getRange();
                }
                break;
            }
            case scene::LightType::Spot:
            {
                if (out.spotCount < MAX_SPOT_LIGHTS)
                {
                    auto* s = static_cast<const scene::SpotLight*>(light);
                    int i = out.spotCount++;
                    out.spotPos[i] = s->getPosition();
                    out.spotDir[i] = s->getDirection();
                    out.spotColor[i] = weighted;
                    out.spotRange[i] = s->getRange();
                    out.spotInnerCos[i] = s->getInnerCos();
                    out.spotOuterCos[i] = s->getOuterCos();
                    bool cast = s->getCastsShadow();
                    out.spotCastsShadow[i] = cast ? 1 : 0;
                    if (cast)
                    {
                        out.spotLightVP[i] = s->getViewProj();
                        out.spotLightPtrs[i] = s;
                    }
                }
                break;
            }
        }
    }

    return out;
}

void Renderer::init(const RenderConfig& cfg)
{
    s_config = cfg;
    glEnable(GL_DEPTH_TEST);

    // create framebuffer with current window size
    int width, height;
    app::Window::getSize(width, height);
    s_sceneFbo = std::make_unique<FrameBuffer>(width, height);

    // shadow resources
    s_dirShadowFbo = std::make_unique<DepthFrameBuffer>(DIR_SHADOW_SIZE, DIR_SHADOW_SIZE);
    s_spotShadowFbos.clear();
    for (int i = 0; i < MAX_SPOT_SHADOW_MAPS; i++)
    {
        s_spotShadowFbos.emplace_back(std::make_unique<DepthFrameBuffer>(SPOT_SHADOW_SIZE, SPOT_SHADOW_SIZE));
    }
    s_shadowShader = std::make_shared<GraphicsShader>(SHADOW_VERT_PATH, SHADOW_FRAG_PATH);

    std::cout << "GL vendor:   " << glGetString(GL_VENDOR)   << "\n";
    std::cout << "GL renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL version:  " << glGetString(GL_VERSION)  << "\n";
}

void Renderer::clear(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::resizeViewport(int width, int height)
{
    s_viewportWidth = width;
    s_viewportHeight = height;

    glViewport(0, 0, width, height);
    if (s_sceneFbo)
    {
        s_sceneFbo->resize(width, height);
    }
}

float Renderer::getAspect()
{
    // minimized window reports zero height, square keeps the projection finite
    return s_viewportHeight > 0 ? float(s_viewportWidth) / float(s_viewportHeight) : 1.0f;
}

void Renderer::registerPrePass(std::shared_ptr<IRenderPass> pass)
{
    s_pre.push_back(std::move(pass));
}

void Renderer::registerPostPass(std::shared_ptr<IRenderPass> pass)
{
    s_post.push_back(std::move(pass));
}

void Renderer::render(const scene::Scene& scene)
{
    // ShadowPass first, fills depth maps for shadow-casting lights
    renderShadowPass(scene);

    // restore main viewport after shadow pass changed it
    int viewportW, viewportH;
    app::Window::getSize(viewportW, viewportH);
    glViewport(0, 0, viewportW, viewportH);

    // render scene to framebuffer only if we have post-passes
    bool useFramebuffer = !s_post.empty() && s_sceneFbo;

    if (useFramebuffer)
    {
        s_sceneFbo->bind();
        // clear framebuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // PrePass
    for (auto& p : s_pre)
    {
        p->render(scene);
    }

    // BasePass
    renderBasePass(scene);

    // unbind framebuffer
    if (useFramebuffer)
    {
        s_sceneFbo->unbind();

        // PostPass
        for (auto& p : s_post)
        {
            p->render(scene);
        }
    }
}

static void renderSceneDepthOnly(const scene::Scene& scene, GraphicsShader& shadowShader, const glm::mat4& lightVP)
{
    shadowShader.bind();
    shadowShader.setMat4("uLightVP", lightVP);

    for (const auto& object : scene.getObjects())
    {
        if (!object) continue;
        const scene::Model* model = object->getModel();
        if (!model) continue;

        shadowShader.setMat4("uModel", object->getTransform().getMatrix());

        for (const auto& mesh : model->getMeshes())
        {
            mesh.draw();
        }
    }
}

void Renderer::renderShadowPass(const scene::Scene& scene)
{
    if (!s_shadowShader) return;

    LightUniforms snapshot = collectLights(scene);

    // directional
    if (snapshot.hasDirectional && snapshot.dirCastsShadow && s_dirShadowFbo)
    {
        s_dirShadowFbo->bind();
        glViewport(0, 0, s_dirShadowFbo->getWidth(), s_dirShadowFbo->getHeight());
        glClear(GL_DEPTH_BUFFER_BIT);

        // front-face culling reduces self-shadowing acne on flat surfaces
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        renderSceneDepthOnly(scene, *s_shadowShader, snapshot.dirLightVP);

        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);

        s_dirShadowFbo->unbind();
    }

    // spots
    for (int i = 0; i < snapshot.spotCount && i < (int)s_spotShadowFbos.size(); i++)
    {
        if (!snapshot.spotCastsShadow[i]) continue;

        auto& fbo = s_spotShadowFbos[i];
        fbo->bind();
        glViewport(0, 0, fbo->getWidth(), fbo->getHeight());
        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        renderSceneDepthOnly(scene, *s_shadowShader, snapshot.spotLightVP[i]);

        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);

        fbo->unbind();
    }
}

static void applyLights(GraphicsShader& shader, const LightUniforms& lights)
{
    shader.setVec3("uAmbientColor", lights.ambientColor);

    shader.setInt("uHasDirLight", lights.hasDirectional ? 1 : 0);
    shader.setVec3("uDirLightDir", lights.dirDirection);
    shader.setVec3("uDirLightColor", lights.dirColor);
    shader.setInt("uDirCastsShadow", lights.dirCastsShadow ? 1 : 0);
    shader.setMat4("uDirLightVP", lights.dirLightVP);

    shader.setInt("uPointCount", lights.pointCount);
    for (int i = 0; i < lights.pointCount; i++)
    {
        std::string base = "uPointLights[" + std::to_string(i) + "].";
        shader.setVec3((base + "position").c_str(), lights.pointPos[i]);
        shader.setVec3((base + "color").c_str(), lights.pointColor[i]);
        shader.setFloat((base + "range").c_str(), lights.pointRange[i]);
    }

    shader.setInt("uSpotCount", lights.spotCount);
    for (int i = 0; i < lights.spotCount; i++)
    {
        std::string base = "uSpotLights[" + std::to_string(i) + "].";
        shader.setVec3((base + "position").c_str(), lights.spotPos[i]);
        shader.setVec3((base + "direction").c_str(), lights.spotDir[i]);
        shader.setVec3((base + "color").c_str(), lights.spotColor[i]);
        shader.setFloat((base + "range").c_str(), lights.spotRange[i]);
        shader.setFloat((base + "innerCos").c_str(), lights.spotInnerCos[i]);
        shader.setFloat((base + "outerCos").c_str(), lights.spotOuterCos[i]);

        std::string vpName = "uSpotLightVP[" + std::to_string(i) + "]";
        shader.setMat4(vpName.c_str(), lights.spotLightVP[i]);

        std::string csName = "uSpotCastsShadow[" + std::to_string(i) + "]";
        shader.setInt(csName.c_str(), lights.spotCastsShadow[i]);
    }
}

void Renderer::bindShadowMaps(GraphicsShader& shader)
{
    // directional shadow map on fixed texture unit
    if (s_dirShadowFbo)
    {
        glActiveTexture(GL_TEXTURE0 + DIR_SHADOW_UNIT);
        glBindTexture(GL_TEXTURE_2D, s_dirShadowFbo->getDepthTexture());
        shader.setInt("uDirShadowMap", (int)DIR_SHADOW_UNIT);
    }

    // spot shadow maps on consecutive units
    for (int i = 0; i < (int)s_spotShadowFbos.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + SPOT_SHADOW_BASE_UNIT + i);
        glBindTexture(GL_TEXTURE_2D, s_spotShadowFbos[i]->getDepthTexture());

        std::string name = "uSpotShadowMap[" + std::to_string(i) + "]";
        shader.setInt(name.c_str(), (int)(SPOT_SHADOW_BASE_UNIT + i));
    }
}

void Renderer::renderBasePass(const scene::Scene& scene)
{
    const scene::Camera* cam = scene.getCamera();
    if (!cam)
    {
        std::cerr << "renderer: no camera set in scene\n";
        return;
    }

    LightUniforms lights = collectLights(scene);

    for (const auto& object : scene.getObjects())
    {
        if (!object)
        {
            continue;
        }

        const scene::Model* model = object->getModel();
        if (!model)
        {
            continue;
        }

        const Material& objectMaterial = object->getMaterial();

        const auto& meshes = model->getMeshes();
        for (size_t meshIdx = 0; meshIdx < meshes.size(); meshIdx++)
        {
            int matId = model->getMeshMaterialIndex(meshIdx);
            const Material* meshMaterial = nullptr;
            if (matId >= 0 && static_cast<size_t>(matId) < model->getMaterials().size())
            {
                meshMaterial = model->getMaterials()[matId].get();
            }

            // shader: object override has priority
            auto shader = objectMaterial.getShader();
            if (!shader && meshMaterial)
            {
                shader = meshMaterial->getShader();
            }
            if (!shader)
            {
                continue;
            }

            shader->bind();
            shader->setMat4("uView", cam->getView());
            shader->setMat4("uProj", cam->getProj(Renderer::getAspect()));
            shader->setMat4("uModel", object->getTransform().getMatrix());
            shader->setMat3("uNormalMatrix", object->getTransform().getNormalMatrix());
            shader->setVec3("uCameraPos", cam->getPosition());
            applyLights(*shader, lights);
            bindShadowMaps(*shader);

            // color (kd): object override > model material > white
            glm::vec3 color(1.0f);
            if (objectMaterial.hasColor())
            {
                color = objectMaterial.getColor();
            }
            else if (meshMaterial && meshMaterial->hasColor())
            {
                color = meshMaterial->getColor();
            }
            shader->setVec3("uColor", color);

            // specular (ks): object override > model material > default 0.5
            glm::vec3 specColor(0.5f);
            if (objectMaterial.hasSpecular())
            {
                specColor = objectMaterial.getSpecular();
            }
            else if (meshMaterial && meshMaterial->hasSpecular())
            {
                specColor = meshMaterial->getSpecular();
            }
            shader->setVec3("uMatSpecular", specColor);

            // shininess (ns): object override > model material > default 32
            float shininess = 32.0f;
            if (objectMaterial.hasShininess())
            {
                shininess = objectMaterial.getShininess();
            }
            else if (meshMaterial && meshMaterial->hasShininess())
            {
                shininess = meshMaterial->getShininess();
            }
            shader->setFloat("uMatShininess", shininess);

            // emissive (ke): object override > model material > black
            glm::vec3 emissive(0.0f);
            if (objectMaterial.hasEmissive())
            {
                emissive = objectMaterial.getEmissive();
            }
            else if (meshMaterial && meshMaterial->hasEmissive())
            {
                emissive = meshMaterial->getEmissive();
            }
            shader->setVec3("uMatEmissive", emissive);

            // textures: object override first, then mesh material as fallback
            bool hasAlbedo = false;
            bool hasSpecMap = false;
            for (const auto& slot : objectMaterial.getTextures())
            {
                if (!slot.texture)
                {
                    continue;
                }
                slot.texture->bind(slot.unit);
                shader->setInt(slot.uniformName.c_str(), static_cast<int>(slot.unit));
                if (slot.uniformName == "uAlbedo") hasAlbedo = true;
                if (slot.uniformName == "uSpecularMap") hasSpecMap = true;
            }
            if (meshMaterial)
            {
                for (const auto& slot : meshMaterial->getTextures())
                {
                    if (!slot.texture) continue;
                    if (hasAlbedo && slot.uniformName == "uAlbedo") continue;
                    if (hasSpecMap && slot.uniformName == "uSpecularMap") continue;

                    slot.texture->bind(slot.unit);
                    shader->setInt(slot.uniformName.c_str(), static_cast<int>(slot.unit));
                    if (slot.uniformName == "uAlbedo") hasAlbedo = true;
                    if (slot.uniformName == "uSpecularMap") hasSpecMap = true;
                }
            }
            shader->setInt("uHasAlbedo", hasAlbedo ? 1 : 0);
            shader->setInt("uHasSpecMap", hasSpecMap ? 1 : 0);

            meshes[meshIdx].draw();
        }
    }
}

} // namespace render
} // namespace BulletRender