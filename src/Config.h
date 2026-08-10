/*
 * Config.h
 */

#pragma once

namespace BulletRender {
namespace config {

// opengl version
constexpr int OpenGLMajor = 4;
constexpr int OpenGLMinor = 6;

// glsl version
constexpr const char* GLSLVersion = "#version 330";

// ui font
constexpr const char* FontPath = "assets/fonts/DejaVuSansMono.ttf";
constexpr float FontSize = 15.0f;
constexpr float FontScale = 1.3f;       // starting ui scale

} // namespace config
} // namespace BulletRender