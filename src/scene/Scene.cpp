/*
 * Scene.cpp
 */

#include "Scene.h"

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

} // namespace scene
} // namespace BulletRender
