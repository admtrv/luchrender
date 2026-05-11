/*
 * Model.h
 */

#pragma once

#include "tiny_obj_loader.h"

#include "Mesh.h"
#include "render/Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include <unordered_map>
#include <iostream>
#include <memory>

namespace BulletRender {
namespace scene {

class Model {
public:
    Model() = default;
    explicit Model(const std::string& path) { loadObj(path); }
    virtual ~Model() { m_meshes.clear(); }

    bool loadObj(const std::string& path);

    const std::vector<Mesh>& getMeshes() const { return m_meshes; }

    const std::vector<std::shared_ptr<render::Material>>& getMaterials() const { return m_materials; }
    int getMeshMaterialIndex(size_t meshIdx) const;

protected:
    std::vector<Mesh> m_meshes;
    std::vector<int> m_meshMaterialIndex;
    std::vector<std::shared_ptr<render::Material>> m_materials;
};

class Box : public Model {
public:
    Box();
    Box(float sizeX, float sizeY, float sizeZ);
};

} // namespace scene
} // namespace BulletRender
