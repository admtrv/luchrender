/*
 * Buffer.h
 */

#pragma once

#include <glad/glad.h>

#include <cstddef>

namespace BulletRender {
namespace render {

// base class for any gl buffer object
class Buffer {
public:
    virtual ~Buffer() = 0;

    unsigned id() const { return m_id; }
    std::size_t size() const { return m_size; }
    GLenum target() const { return m_target; }

protected:
    explicit Buffer(GLenum target);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // glGenBuffers + glBufferData, stores size
    void allocate(std::size_t size, const void* data, GLenum usage);

    // glBufferSubData on the same target
    void upload(std::size_t offset, std::size_t size, const void* data);

    unsigned m_id = 0;
    std::size_t m_size = 0;
    GLenum m_target;
};

} // namespace render
} // namespace BulletRender
