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

// editor greys, dark to light
constexpr glm::vec3 Grey900 = {0.114f, 0.122f, 0.133f};
constexpr glm::vec3 Grey800 = {0.153f, 0.161f, 0.173f};
constexpr glm::vec3 Grey700 = {0.192f, 0.200f, 0.212f};
constexpr glm::vec3 Grey600 = {0.243f, 0.251f, 0.263f};
constexpr glm::vec3 Grey500 = {0.302f, 0.310f, 0.322f};
constexpr glm::vec3 Grey300 = {0.545f, 0.553f, 0.565f};
constexpr glm::vec3 Grey100 = {0.851f, 0.855f, 0.863f};

} // namespace colors
} // namespace BulletRender
