/*
 * Colors.h
 */

#pragma once

#include <glm/glm.hpp>

namespace BulletRender {
namespace colors {

// palette
constexpr glm::vec3 Red = {0.808f, 0.141f, 0.184f};     // #CE242F
constexpr glm::vec3 Green = {0.247f, 0.643f, 0.357f};   // #3FA45B
constexpr glm::vec3 Blue = {0.000f, 0.420f, 0.710f};    // #006BB5
constexpr glm::vec3 Yellow = {0.980f, 0.757f, 0.016f};  // #FAC104
constexpr glm::vec3 White = {1.000f, 1.000f, 1.000f};   // #FFFFFF

// coordinate axes
constexpr glm::vec3 AxisX = Red;
constexpr glm::vec3 AxisY = Green;
constexpr glm::vec3 AxisZ = Blue;

} // namespace colors
} // namespace BulletRender
