/*
 * Shader.h
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include <string>
#include <initializer_list>

namespace BulletRender {
namespace render {

// base class for any GLSL program object
class Shader {
public:
    virtual ~Shader() = 0;

    void bind() const;
    unsigned getId() const { return m_id; }

    void setMat4(const char* name, const glm::mat4& mat) const;
    void setVec3(const char* name, const glm::vec3& vec) const;
    void setFloat(const char* name, float val) const;
    void setInt(const char* name, int val) const;

protected:
    Shader() = default;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // shared helpers for derived shader classes
    static std::string readFile(const std::string& path);
    static unsigned compileStage(GLenum type, const char* src, const std::string& tag);

    // attach the given stages, link them into m_id;
    bool linkProgram(std::initializer_list<unsigned> stages);

    unsigned m_id = 0;

private:
    int uniformLoc(const char* name) const;
};

// vertex + fragment stages, classic raster pipeline.
class GraphicsShader : public Shader {
public:
    GraphicsShader(const std::string& vertPath, const std::string& fragPath);

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
};

// gpu compute program, dispatched in work groups
class ComputeShader : public Shader {
public:
    explicit ComputeShader(const std::string& compPath);

    bool loadFromFile(const std::string& compPath);

    void dispatch(unsigned groupsX, unsigned groupsY = 1, unsigned groupsZ = 1) const;
    void memoryBarrier(GLbitfield bits = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) const;
};

} // namespace render
} // namespace BulletRender
