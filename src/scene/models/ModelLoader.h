/*
 * ModelLoader.h
 */

#pragma once

#include "Model.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace BulletRender {
namespace scene {

// classic asset cache for Model, keyed by path
class ModelLoader {
public:
    static ModelLoader& instance();

    std::shared_ptr<Model> load(const std::string& path);

    void clear();

private:
    ModelLoader() = default;
    ~ModelLoader() = default;

    ModelLoader(const ModelLoader&) = delete;
    ModelLoader& operator=(const ModelLoader&) = delete;

    std::shared_ptr<Model> loadFromDisk(const std::string& path);

    std::unordered_map<std::string, std::weak_ptr<Model>> m_cache;
};

} // namespace scene
} // namespace BulletRender
