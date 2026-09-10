/*
 * Scene.cpp
 */

#include "Scene.h"

#include <algorithm>

namespace BulletRender {
namespace scene {

void SceneObject::setParent(SceneObject* parent, bool keepWorld)
{
    m_transform.setParent(parent != nullptr ? &parent->m_transform : nullptr, keepWorld);
}

void SceneObject::addChild(SceneObject* child, bool keepWorld)
{
    if (child != nullptr)
    {
        child->setParent(this, keepWorld);
    }
}

SceneObject* SceneObject::getParent() const
{
    Transform* parent = m_transform.getParent();
    return parent != nullptr ? parent->getOwner() : nullptr;
}

std::vector<SceneObject*> SceneObject::getChildren() const
{
    std::vector<SceneObject*> children;
    children.reserve(m_transform.getChildren().size());

    for (Transform* child : m_transform.getChildren())
    {
        if (SceneObject* owner = child->getOwner())
        {
            children.push_back(owner);
        }
    }

    return children;
}


SceneObject* Scene::addObject(std::shared_ptr<Model> model, const std::string& name)
{
    m_objects.emplace_back(std::make_unique<SceneObject>(std::move(model), name));
    SceneObject* object = m_objects.back().get();
    object->getTransform().setOwner(object);
    return object;
}

void Scene::removeObject(size_t index)
{
    if (index < m_objects.size())
    {
        m_objects.erase(m_objects.begin() + index);
    }
}

Light* Scene::pushLight(std::unique_ptr<Light> light)
{
    Light* raw = light.get();
    m_lights.push_back(std::move(light));
    return raw;
}

void Scene::removeLight(size_t index)
{
    if (index < m_lights.size())
    {
        m_lights.erase(m_lights.begin() + index);
    }
}

Camera* Scene::pushCamera(std::unique_ptr<Camera> camera)
{
    Camera* raw = camera.get();
    m_cameras.push_back(std::move(camera));

    // scene without active camera renders nothing, so first one takes role
    if (m_activeCamera == nullptr)
    {
        m_activeCamera = raw;
    }

    return raw;
}

void Scene::removeCamera(size_t index)
{
    if (index >= m_cameras.size())
    {
        return;
    }

    // dropping active one leaves scene blind, fall back to any other
    if (m_cameras[index].get() == m_activeCamera)
    {
        m_activeCamera = nullptr;
    }

    m_cameras.erase(m_cameras.begin() + index);

    if (m_activeCamera == nullptr && !m_cameras.empty())
    {
        m_activeCamera = m_cameras.front().get();
    }
}

void Scene::clearCameras()
{
    m_cameras.clear();
    m_activeCamera = nullptr;
}

void Scene::setActiveCamera(Camera* camera)
{
    m_activeCamera = camera;
}

} // namespace scene
} // namespace BulletRender
