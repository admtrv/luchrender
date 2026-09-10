/*
 * ModelLoader.cpp
 */

#include "ModelLoader.h"

#include <iostream>

namespace BulletRender {
namespace scene {

ModelLoader& ModelLoader::instance()
{
    static ModelLoader inst;
    return inst;
}

std::shared_ptr<Model> ModelLoader::load(const std::string& path)
{
    auto it = m_cache.find(path);
    if (it != m_cache.end())
    {
        if (auto cached = it->second.lock())
        {
            return cached;
        }
        m_cache.erase(it);
    }

    auto model = loadFromDisk(path);
    if (model)
    {
        m_cache[path] = model;
    }
    return model;
}

void ModelLoader::clear()
{
    m_cache.clear();
}

std::shared_ptr<Model> ModelLoader::loadFromDisk(const std::string& path)
{
    auto model = std::make_shared<Model>();

    if (!model->loadObj(path))
    {
        std::cerr << "model load failed: " << path << '\n';
        return nullptr;
    }

    return model;
}

} // namespace scene
} // namespace BulletRender
