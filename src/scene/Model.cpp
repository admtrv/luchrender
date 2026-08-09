/*
 * Model.cpp
 */

#include "Model.h"

#include "render/textures/TextureLoader.h"

#include <filesystem>

namespace BulletRender {
namespace scene {

// squared length below this carries no direction
constexpr float LENGTH2_EPSILON = 1e-20f;

// key for unique vertex - triplet of position, normal and uv indices
struct VNKey {
    int vertexIdx = -1;
    int normalIdx = -1;
    int texcoordIdx = -1;

    bool operator==(const VNKey& other) const noexcept
    {
        return vertexIdx == other.vertexIdx
            && normalIdx == other.normalIdx
            && texcoordIdx == other.texcoordIdx;
    }
};

struct VNKeyHash {
    size_t operator()(const VNKey& key) const noexcept
    {
        size_t h = static_cast<size_t>(static_cast<uint32_t>(key.vertexIdx));
        h = (h * 31) ^ static_cast<size_t>(static_cast<uint32_t>(key.normalIdx));
        h = (h * 31) ^ static_cast<size_t>(static_cast<uint32_t>(key.texcoordIdx));
        return h;
    }
};

static glm::vec3 safeNormalize(const glm::vec3& vector)
{
    float len2 = glm::dot(vector, vector);

    if (len2 <= LENGTH2_EPSILON)
    {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }

    return vector * glm::inversesqrt(len2);
}

// .mtl -> Material conversion: phong terms (kd, ks, ns, ke) + diffuse/specular maps
static std::vector<std::shared_ptr<render::Material>>
buildMaterials(const std::vector<tinyobj::material_t>& tinyMats, const std::string& baseDir)
{
    std::vector<std::shared_ptr<render::Material>> materials;
    materials.reserve(tinyMats.size());

    for (const auto& m : tinyMats)
    {
        auto mat = std::make_shared<render::Material>();

        // phong scalars/colors
        mat->setColor({m.diffuse[0], m.diffuse[1], m.diffuse[2]});
        mat->setSpecular({m.specular[0], m.specular[1], m.specular[2]});
        mat->setShininess(m.shininess);
        mat->setEmissive({m.emission[0], m.emission[1], m.emission[2]});

        // map_Kd, diffuse texture
        if (!m.diffuse_texname.empty())
        {
            std::string fullPath = (std::filesystem::path(baseDir) / m.diffuse_texname).string();
            auto tex = render::TextureLoader::instance().load(fullPath);
            if (tex)
            {
                mat->setTexture("uAlbedo", tex, 0);
            }
        }

        // map_Ks, specular mask
        if (!m.specular_texname.empty())
        {
            std::string fullPath = (std::filesystem::path(baseDir) / m.specular_texname).string();
            auto tex = render::TextureLoader::instance().load(fullPath);
            if (tex)
            {
                mat->setTexture("uSpecularMap", tex, 1);
            }
        }

        materials.push_back(std::move(mat));
    }

    return materials;
}

// generic Model

bool Model::loadObj(const std::string& path)
{
    m_meshes.clear();
    m_meshMaterialIndex.clear();
    m_materials.clear();

    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.vertex_color = false;
    config.mtl_search_path = std::filesystem::path(path).parent_path().string();

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "tinyobj error: " << reader.Error() << "\n";
        }
        return false;
    }

    if (!reader.Warning().empty())
    {
        std::cerr << "tinyobj warn: " << reader.Warning() << "\n";
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t>& tinyMats = reader.GetMaterials();

    m_materials = buildMaterials(tinyMats, config.mtl_search_path);

    const size_t positionCount = attrib.vertices.size() / 3;
    const size_t normalCount = attrib.normals.size()  / 3;
    const size_t texcoordCount = attrib.texcoords.size() / 2;

    m_meshes.reserve(shapes.size());
    m_meshMaterialIndex.reserve(shapes.size());

    // per-bucket geometry, keyed by face material id
    struct Bucket {
        std::vector<Vertex> vertices;
        std::vector<unsigned> indices;
        std::unordered_map<VNKey, unsigned, VNKeyHash> uniqueMap;
        bool needRecomputeNormals = false;
    };

    for (const auto& shape : shapes)
    {
        const size_t faceCount = shape.mesh.indices.size() / 3;

        std::unordered_map<int, Bucket> buckets;

        for (size_t faceIdx = 0; faceIdx < faceCount; faceIdx++)
        {
            int matId = -1;
            if (faceIdx < shape.mesh.material_ids.size())
            {
                matId = shape.mesh.material_ids[faceIdx];
            }

            Bucket& bucket = buckets[matId];
            if (normalCount == 0)
            {
                bucket.needRecomputeNormals = true;
            }

            for (size_t corner = 0; corner < 3; corner++)
            {
                const tinyobj::index_t& cornerIdx = shape.mesh.indices[3 * faceIdx + corner];
                VNKey key{cornerIdx.vertex_index, cornerIdx.normal_index, cornerIdx.texcoord_index};

                auto it = bucket.uniqueMap.find(key);
                if (it != bucket.uniqueMap.end())
                {
                    bucket.indices.push_back(it->second);
                    continue;
                }

                if (cornerIdx.vertex_index < 0 || static_cast<size_t>(cornerIdx.vertex_index) >= positionCount)
                {
                    std::cerr << "obj vertex index out of range\n";
                    return false;
                }

                glm::vec3 position = {
                    attrib.vertices[3 * static_cast<size_t>(cornerIdx.vertex_index) + 0],
                    attrib.vertices[3 * static_cast<size_t>(cornerIdx.vertex_index) + 1],
                    attrib.vertices[3 * static_cast<size_t>(cornerIdx.vertex_index) + 2]
                };

                glm::vec3 normal(0.0f);
                if (cornerIdx.normal_index >= 0 && static_cast<size_t>(cornerIdx.normal_index) < normalCount)
                {
                    normal = {
                        attrib.normals[3 * static_cast<size_t>(cornerIdx.normal_index) + 0],
                        attrib.normals[3 * static_cast<size_t>(cornerIdx.normal_index) + 1],
                        attrib.normals[3 * static_cast<size_t>(cornerIdx.normal_index) + 2]
                    };
                }
                else
                {
                    bucket.needRecomputeNormals = true;
                }

                glm::vec2 uv(0.0f);
                if (cornerIdx.texcoord_index >= 0 && static_cast<size_t>(cornerIdx.texcoord_index) < texcoordCount)
                {
                    uv = {
                        attrib.texcoords[2 * static_cast<size_t>(cornerIdx.texcoord_index) + 0],
                        attrib.texcoords[2 * static_cast<size_t>(cornerIdx.texcoord_index) + 1]
                    };
                }

                unsigned newVertexIdx = static_cast<unsigned>(bucket.vertices.size());
                bucket.uniqueMap.emplace(key, newVertexIdx);
                bucket.vertices.push_back(Vertex{position, normal, uv});
                bucket.indices.push_back(newVertexIdx);
            }
        }

        for (auto& [matId, bucket] : buckets)
        {
            if (bucket.needRecomputeNormals)
            {
                for (auto& vertex : bucket.vertices)
                {
                    vertex.normal = glm::vec3(0.0f);
                }

                if (bucket.indices.size() % 3 != 0)
                {
                    std::cerr << "obj not triangulated as expected\n";
                    return false;
                }

                for (size_t i = 0; i < bucket.indices.size(); i += 3)
                {
                    Vertex& vA = bucket.vertices[bucket.indices[i + 0]];
                    Vertex& vB = bucket.vertices[bucket.indices[i + 1]];
                    Vertex& vC = bucket.vertices[bucket.indices[i + 2]];

                    glm::vec3 edge1 = vB.position - vA.position;
                    glm::vec3 edge2 = vC.position - vA.position;

                    glm::vec3 faceNormal = glm::cross(edge1, edge2);

                    vA.normal += faceNormal;
                    vB.normal += faceNormal;
                    vC.normal += faceNormal;
                }

                for (auto& vertex : bucket.vertices)
                {
                    vertex.normal = safeNormalize(vertex.normal);
                }
            }

            m_meshes.emplace_back(bucket.vertices, bucket.indices);
            m_meshMaterialIndex.push_back(matId);
        }
    }

    return true;
}

int Model::getMeshMaterialIndex(size_t meshIdx) const
{
    if (meshIdx >= m_meshMaterialIndex.size())
    {
        return -1;
    }
    return m_meshMaterialIndex[meshIdx];
}

// Box

Box::Box() : Box(1.0f, 1.0f, 1.0f) {}

Box::Box(float sizeX, float sizeY, float sizeZ)
{
    float hx = sizeX * 0.5f;
    float hy = sizeY * 0.5f;
    float hz = sizeZ * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;

    // front face (z+)
    vertices.push_back({{-hx, -hy,  hz}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ hx, -hy,  hz}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ hx,  hy,  hz}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-hx,  hy,  hz}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}});

    // back face (z-)
    vertices.push_back({{ hx, -hy, -hz}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{-hx, -hy, -hz}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-hx,  hy, -hz}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ hx,  hy, -hz}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}});

    // right face (x+)
    vertices.push_back({{ hx, -hy,  hz}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ hx, -hy, -hz}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ hx,  hy, -hz}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ hx,  hy,  hz}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}});

    // left face (x-)
    vertices.push_back({{-hx, -hy, -hz}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{-hx, -hy,  hz}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-hx,  hy,  hz}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-hx,  hy, -hz}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}});

    // top face (y+)
    vertices.push_back({{-hx,  hy,  hz}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ hx,  hy,  hz}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ hx,  hy, -hz}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-hx,  hy, -hz}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}});

    // bottom face (y-)
    vertices.push_back({{-hx, -hy, -hz}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ hx, -hy, -hz}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ hx, -hy,  hz}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-hx, -hy,  hz}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}});

    for (unsigned face = 0; face < 6; face++)
    {
        unsigned base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    m_meshes.emplace_back(vertices, indices);
    m_meshMaterialIndex.push_back(-1);
}

} // namespace scene
} // namespace BulletRender
