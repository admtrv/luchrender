/*
 * Fonts.cpp
 */

#include "interface/elements/Fonts.h"

#include "Config.h"

#include <iostream>

namespace BulletRender {
namespace interface {

float Fonts::s_scale = config::FontScale;
bool Fonts::s_loaded = false;

// built in font is ascii only, hierarchy needs box drawing glyphs
static const ImWchar GLYPH_RANGES[] = {
    0x0020, 0x00FF,     // latin
    0x2500, 0x257F,     // box drawing
    0
};

ImFont* Fonts::build(float sizePixels)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font = io.Fonts->AddFontFromFileTTF(config::FontPath, sizePixels, nullptr, GLYPH_RANGES);
    if (font == nullptr)
    {
        std::cerr << "font load failed: " << config::FontPath << ", falling back to the built in one\n";
        font = io.Fonts->AddFontDefault();
    }

    return font;
}

void Fonts::setScale(float scale)
{
    s_scale = scale;

    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui::GetStyle().FontScaleMain = scale;
    }
}

void Fonts::apply()
{
    // context does not exist when editor is constructed, load waits for frame
    if (s_loaded)
    {
        return;
    }

    build(config::FontSize);
    ImGui::GetStyle().FontScaleMain = s_scale;
    s_loaded = true;
}

} // namespace interface
} // namespace BulletRender
