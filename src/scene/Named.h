/*
 * Named.h
 */

#pragma once

#include <string>

namespace BulletRender {
namespace scene {

// display name
class Named {
public:
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }

protected:
    explicit Named(std::string name) : m_name(std::move(name)) {}
    ~Named() = default;

private:
    std::string m_name;
};

} // namespace scene
} // namespace BulletRender
