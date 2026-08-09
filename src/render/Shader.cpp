/*
 * Shader.cpp
 */

#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace BulletRender {
namespace render {

// base shader

Shader::~Shader()
{
    if (m_id)
    {
        glDeleteProgram(m_id);
        m_id = 0;
    }
}

std::string Shader::readFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return {};
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// compile shader of type
unsigned Shader::compileStage(GLenum type, const char* src, const std::string& tag)
{
    unsigned shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

    if (!ok)
    {
        int size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);

        std::string log(size, '\0');
        glGetShaderInfoLog(shader, size, nullptr, log.data());

        std::cerr << "shader compile error (" << tag << "): " << log << "\n";

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::linkProgram(std::initializer_list<unsigned> stages)
{
    // any stage failed to compile
    for (unsigned s : stages)
    {
        if (s == 0)
        {
            for (unsigned cleanup : stages)
            {
                if (cleanup)
                {
                    glDeleteShader(cleanup);
                }
            }
            return false;
        }
    }

    m_id = glCreateProgram();
    for (unsigned s : stages)
    {
        glAttachShader(m_id, s);
    }
    glLinkProgram(m_id);

    int ok = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &ok);

    if (!ok)
    {
        int size = 0;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &size);

        std::string log(size, '\0');
        glGetProgramInfoLog(m_id, size, nullptr, log.data());

        std::cerr << "program link error: " << log << "\n";

        glDeleteProgram(m_id);
        m_id = 0;
    }

    for (unsigned s : stages)
    {
        glDeleteShader(s);
    }

    return m_id != 0;
}

void Shader::bind() const
{
    glUseProgram(m_id);
}

int Shader::uniformLoc(const char* name) const
{
    return glGetUniformLocation(m_id, name);
}

void Shader::setMat4(const char* name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(uniformLoc(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const char* name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(uniformLoc(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const char* name, const glm::vec3& vec) const
{
    glUniform3fv(uniformLoc(name), 1, &vec.x);
}

void Shader::setFloat(const char* name, float val) const
{
    glUniform1f(uniformLoc(name), val);
}

void Shader::setInt(const char* name, int val) const
{
    glUniform1i(uniformLoc(name), val);
}

// graphics shader

GraphicsShader::GraphicsShader(const std::string& vertPath, const std::string& fragPath)
{
    loadFromFiles(vertPath, fragPath);
}

bool GraphicsShader::loadFromFiles(const std::string& vertPath, const std::string& fragPath)
{
    std::string vertString = readFile(vertPath);
    std::string fragString = readFile(fragPath);

    if (vertString.empty() || fragString.empty())
    {
        std::cerr << "cannot read " << vertPath << " or " << fragPath << "\n";
        return false;
    }

    unsigned vertexShader = compileStage(GL_VERTEX_SHADER, vertString.c_str(), vertPath);
    unsigned fragmentShader = compileStage(GL_FRAGMENT_SHADER, fragString.c_str(), fragPath);

    return linkProgram({vertexShader, fragmentShader});
}

// compute shader

ComputeShader::ComputeShader(const std::string& compPath)
{
    loadFromFile(compPath);
}

bool ComputeShader::loadFromFile(const std::string& compPath)
{
    std::string compString = readFile(compPath);

    if (compString.empty())
    {
        std::cerr << "cannot read " << compPath << "\n";
        return false;
    }

    unsigned computeStage = compileStage(GL_COMPUTE_SHADER, compString.c_str(), compPath);

    return linkProgram({computeStage});
}

void ComputeShader::dispatch(unsigned groupsX, unsigned groupsY, unsigned groupsZ) const
{
    glDispatchCompute(groupsX, groupsY, groupsZ);
}

void ComputeShader::memoryBarrier(GLbitfield bits) const
{
    glMemoryBarrier(bits);
}

} // namespace render
} // namespace BulletRender
