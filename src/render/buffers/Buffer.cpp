/*
 * Buffer.cpp
 */

#include "Buffer.h"

namespace BulletRender {
namespace render {

Buffer::Buffer(GLenum target) : m_target(target) {}

Buffer::~Buffer()
{
    if (m_id)
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
}

void Buffer::allocate(std::size_t size, const void* data, GLenum usage)
{
    if (!m_id)
    {
        glGenBuffers(1, &m_id);
    }

    glBindBuffer(m_target, m_id);
    glBufferData(m_target, static_cast<GLsizeiptr>(size), data, usage);

    m_size = size;
}

void Buffer::upload(std::size_t offset, std::size_t size, const void* data)
{
    glBindBuffer(m_target, m_id);
    glBufferSubData(m_target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

} // namespace render
} // namespace BulletRender
