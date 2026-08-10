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

    unsigned getVertexCount() const;
    unsigned getTriangleCount() const;

    const std::vector<std::shared_ptr<render::Material>>& getMaterials() const { return m_materials; }
    int getMeshMaterialIndex(size_t meshIdx) const;

    // axis aligned bounds in model space, empty model reports zero sized box
    const glm::vec3& getBoundsMin() const { return m_boundsMin; }
    const glm::vec3& getBoundsMax() const { return m_boundsMax; }

protected:
    // single entry point for geometry, keeps bounds in sync
    void addMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, int materialIdx);
    void clearMeshes();

    std::vector<Mesh> m_meshes;
    std::vector<int> m_meshMaterialIndex;
    std::vector<std::shared_ptr<render::Material>> m_materials;

    glm::vec3 m_boundsMin{0.0f};
    glm::vec3 m_boundsMax{0.0f};
};

class Box : public Model {
public:
    Box();
    Box(float sizeX, float sizeY, float sizeZ);
};

class Sphere : public Model {
public:
    Sphere();
    Sphere(float radius, int segments, int rings);
};

} // namespace scene
} // namespace BulletRender
