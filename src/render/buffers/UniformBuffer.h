/*
 * UniformBuffer.h
 */

#pragma once

#include "Buffer.h"

#include <type_traits>

namespace BulletRender {
namespace render {

// gl uniform buffer object, bound to a shader binding slot
class UniformBuffer final : public Buffer {
public:
    explicit UniformBuffer(std::size_t size, GLenum usage = GL_DYNAMIC_DRAW);
    UniformBuffer(std::size_t size, const void* data, GLenum usage = GL_DYNAMIC_DRAW);

    void update(std::size_t offset, std::size_t size, const void* data);    // partial
    void update(const void* data);                                          // full
    template <typename T>
    void update(const T& value);                                            // typed full

    void bindToSlot(unsigned slot) const;                                               // whole buffer
    void bindRangeToSlot(unsigned slot, std::size_t offset, std::size_t size) const;    // sub-range
};

template <typename T>
void UniformBuffer::update(const T& value)
{
    static_assert(std::is_trivially_copyable_v<T>, "UBO upload requires trivially copyable type");
    update(0, sizeof(T), &value);
}

} // namespace render
} // namespace BulletRender
