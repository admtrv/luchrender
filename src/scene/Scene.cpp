/*
 * Scene.cpp
 */

#include "Scene.h"

#include <algorithm>

namespace BulletRender {
namespace scene {

SceneObject* Scene::addObject(Model* model)
{
    m_objects.emplace_back(std::make_unique<SceneObject>(model));
    return m_objects.back().get();
}

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


void Scene::removeObject(size_t index)
{
    if (index < m_objects.size())
    {
        m_objects.erase(m_objects.begin() + index);
    }
}

void Scene::addLight(const Light* light)
{
    m_lights.push_back(light);
}

void Scene::removeLight(size_t index)
{
    if (index < m_lights.size())
    {
        m_lights.erase(m_lights.begin() + index);
    }
}

void Scene::addCamera(const Camera* camera)
{
    if (camera == nullptr)
    {
        return;
    }

    m_cameras.push_back(camera);

    // scene without active camera renders nothing, so first one takes role
    if (m_activeCamera == nullptr)
    {
        m_activeCamera = camera;
    }
}

void Scene::removeCamera(size_t index)
{
    if (index >= m_cameras.size())
    {
        return;
    }

    // dropping active one leaves scene blind, fall back to any other
    if (m_cameras[index] == m_activeCamera)
    {
        m_activeCamera = nullptr;
    }

    m_cameras.erase(m_cameras.begin() + index);

    if (m_activeCamera == nullptr && !m_cameras.empty())
    {
        m_activeCamera = m_cameras.front();
    }
}

void Scene::clearCameras()
{
    m_cameras.clear();
    m_activeCamera = nullptr;
}

void Scene::setActiveCamera(const Camera* camera)
{
    // activating camera implies it belongs to scene
    if (camera != nullptr && std::find(m_cameras.begin(), m_cameras.end(), camera) == m_cameras.end())
    {
        m_cameras.push_back(camera);
    }

    m_activeCamera = camera;
}

} // namespace scene
} // namespace BulletRender
