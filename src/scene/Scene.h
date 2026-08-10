/*
 * Scene.h
 */

#pragma once

#include "Model.h"
#include "Named.h"
#include "Transform.h"
#include "Camera.h"
#include "Light.h"
#include "render/Material.h"

#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <utility>

namespace BulletRender {
namespace scene {

// draw item
class SceneObject : public Named {
public:
    SceneObject(Model* model = nullptr, std::string name = "Object")
        : Named(std::move(name)), m_model(model) {}

    void setModel(Model* model) { m_model = model; }
    Model* getModel() const { return m_model; }

    // hidden objects skipped while drawing
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

    render::Material& getMaterial() { return m_material; }
    const render::Material& getMaterial() const { return m_material; }

    // hierarchy shortcuts
    void setParent(SceneObject* parent, bool keepWorld = true);
    void addChild(SceneObject* child, bool keepWorld = true);
    SceneObject* getParent() const;
    std::vector<SceneObject*> getChildren() const;

private:
    Model* m_model;
    Transform m_transform;
    render::Material m_material;
    bool m_visible = true;
};


// everything single frame needs, owns whole content
class Scene {
public:
    // models, shared between objects
    Model* addModel(std::unique_ptr<Model> model);
    Model* loadModel(const std::string& path);
    void removeModel(size_t index);
    void clearModels();
    const std::vector<std::unique_ptr<Model>>& getModels() const { return m_models; }

    // objects
    SceneObject* addObject(Model* model, const std::string& name = "Object");
    void removeObject(size_t index);
    void clearObjects() { m_objects.clear(); }
    const std::vector<std::unique_ptr<SceneObject>>& getObjects() const { return m_objects; }

    // light
    template <typename T, typename... Args>
    T* createLight(Args&&... args);
    void removeLight(size_t index);
    void clearLights() { m_lights.clear(); }
    const std::vector<std::unique_ptr<Light>>& getLights() const { return m_lights; }

    // camera, first one added becomes active unless told otherwise
    template <typename T, typename... Args>
    T* createCamera(Args&&... args);
    void removeCamera(size_t index);
    void clearCameras();
    const std::vector<std::unique_ptr<Camera>>& getCameras() const { return m_cameras; }

    // the one the frame is rendered through
    void setActiveCamera(Camera* camera);
    Camera* getActiveCamera() const { return m_activeCamera; }

private:
    Light* pushLight(std::unique_ptr<Light> light);
    Camera* pushCamera(std::unique_ptr<Camera> camera);

    // scene owns everything it holds, editor creates and drops it at runtime
    std::vector<std::unique_ptr<Model>> m_models;
    std::vector<std::unique_ptr<SceneObject>> m_objects;
    std::vector<std::unique_ptr<Light>> m_lights;
    std::vector<std::unique_ptr<Camera>> m_cameras;
    Camera* m_activeCamera = nullptr;
};

template <typename T, typename... Args>
T* Scene::createLight(Args&&... args)
{
    static_assert(std::is_base_of_v<Light, T>, "T must derive from Light");

    auto light = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw = light.get();
    pushLight(std::move(light));
    return raw;
}

template <typename T, typename... Args>
T* Scene::createCamera(Args&&... args)
{
    static_assert(std::is_base_of_v<Camera, T>, "T must derive from Camera");

    auto camera = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw = camera.get();
    pushCamera(std::move(camera));
    return raw;
}

} // namespace scene
} // namespace BulletRender
