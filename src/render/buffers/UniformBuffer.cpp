/*
 * UniformBuffer.cpp
 */

#include "UniformBuffer.h"

namespace BulletRender {
namespace render {

UniformBuffer::UniformBuffer(std::size_t size, GLenum usage) : Buffer(GL_UNIFORM_BUFFER)
{
    allocate(size, nullptr, usage);
}

UniformBuffer::UniformBuffer(std::size_t size, const void* data, GLenum usage) : Buffer(GL_UNIFORM_BUFFER)
{
    allocate(size, data, usage);
}

void UniformBuffer::update(std::size_t offset, std::size_t size, const void* data)
{
    upload(offset, size, data);
}

void UniformBuffer::update(const void* data)
{
    upload(0, m_size, data);
}

void UniformBuffer::bindToSlot(unsigned slot) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_id);
}

void UniformBuffer::bindRangeToSlot(unsigned slot, std::size_t offset, std::size_t size) const
{
    glBindBufferRange(GL_UNIFORM_BUFFER, slot, m_id, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
}

} // namespace render
} // namespace BulletRender
