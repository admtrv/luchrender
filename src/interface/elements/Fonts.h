/*
 * Fonts.h
 */

#pragma once

#include "imgui.h"

#include <string>

namespace BulletRender {
namespace interface {

// ui typeface, glyph ranges and scale in one place
class Fonts {
public:
    // scales whole ui, glyphs rasterised on demand so text stays sharp
    static void setScale(float scale);

    // loads face on first call, needs live imgui context, call between frames
    static void apply();

    static float getScale() { return s_scale; }

private:
    static ImFont* build(float sizePixels);

    static float s_scale;
    static bool s_loaded;
};

} // namespace interface
} // namespace BulletRender
