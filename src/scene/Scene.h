/*
 * Scene.h
 */

#pragma once

#include "Model.h"
#include "Transform.h"
#include "Camera.h"
#include "Light.h"
#include "render/Material.h"

#include <vector>
#include <memory>

namespace BulletRender {
namespace scene {

// draw item
class SceneObject {
public:
    SceneObject(Model* model = nullptr) : m_model(model) {}

    void setModel(Model* model) { m_model = model; }
    Model* getModel() const { return m_model; }

    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

    render::Material& getMaterial() { return m_material; }
    const render::Material& getMaterial() const { return m_material; }

    // hierarchy shortcuts
    void setParent(SceneObject* parent, bool keepWorld = true);
    void addChild(SceneObject* child, bool keepWorld = true);

private:
    Model* m_model;
    Transform m_transform;
    render::Material m_material;
};


// everything single frame needs
class Scene {
public:
    // objects
    SceneObject* addObject(Model* model);
    void removeObject(size_t index);
    void clearObjects() { m_objects.clear(); }
    const std::vector<std::unique_ptr<SceneObject>>& getObjects() const { return m_objects; }

    // light
    void addLight(const Light* light);
    void removeLight(size_t index);
    void clearLights() { m_lights.clear(); }
    const std::vector<const Light*>& getLights() const { return m_lights; }

    // camera, first one added becomes active unless told otherwise
    void addCamera(const Camera* camera);
    void removeCamera(size_t index);
    void clearCameras();
    const std::vector<const Camera*>& getCameras() const { return m_cameras; }

    // the one the frame is rendered through
    void setActiveCamera(const Camera* camera);
    const Camera* getActiveCamera() const { return m_activeCamera; }

private:
    std::vector<std::unique_ptr<SceneObject>> m_objects;    // scene owns its objects
    std::vector<const Light*> m_lights;                     // scene does not own lights and cameras
    std::vector<const Camera*> m_cameras;
    const Camera* m_activeCamera = nullptr;
};

} // namespace scene
} // namespace BulletRender
