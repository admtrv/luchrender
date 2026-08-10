/*
 * Mesh.h
 */

#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <vector>

namespace BulletRender {
namespace scene {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv{0.0f, 0.0f};
};

class Mesh {
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;

    // geometry lives on gpu, only counts kept around
    unsigned getVertexCount() const { return m_vertexCount; }
    unsigned getIndexCount() const { return m_indexCount; }
    unsigned getTriangleCount() const { return m_indexCount / 3; }

private:
    unsigned m_vao=0;
    unsigned m_vbo=0;
    unsigned m_ebo=0;

    unsigned m_vertexCount=0;
    unsigned m_indexCount=0;
};

} // namespace scene
} // namespace BulletRender