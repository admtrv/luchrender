/*
 * Theme.cpp
 */

#include "interface/elements/Theme.h"

#include "Colors.h"

#include "imgui.h"

namespace BulletRender {
namespace interface {

constexpr float WINDOW_ROUNDING = 0.0f;         // panel is flush with window edge
constexpr float FRAME_ROUNDING = 3.0f;
constexpr float SCROLLBAR_ROUNDING = 3.0f;
constexpr float BORDER_SIZE = 1.0f;

constexpr float FRAME_PADDING_X = 6.0f;
constexpr float FRAME_PADDING_Y = 4.0f;
constexpr float ITEM_SPACING_X = 8.0f;
constexpr float ITEM_SPACING_Y = 5.0f;
constexpr float WINDOW_PADDING = 8.0f;
constexpr float INDENT_SPACING = 16.0f;

static ImVec4 rgba(const glm::vec3& color, float alpha = 1.0f)
{
    return {color.r, color.g, color.b, alpha};
}

void Theme::apply()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = WINDOW_ROUNDING;
    style.ChildRounding = FRAME_ROUNDING;
    style.FrameRounding = FRAME_ROUNDING;
    style.PopupRounding = FRAME_ROUNDING;
    style.ScrollbarRounding = SCROLLBAR_ROUNDING;
    style.GrabRounding = FRAME_ROUNDING;
    style.TabRounding = FRAME_ROUNDING;

    style.WindowBorderSize = BORDER_SIZE;
    style.ChildBorderSize = BORDER_SIZE;
    style.FrameBorderSize = 0.0f;               // fields read as sunken by fill, not outline
    style.PopupBorderSize = BORDER_SIZE;

    style.WindowPadding = {WINDOW_PADDING, WINDOW_PADDING};
    style.FramePadding = {FRAME_PADDING_X, FRAME_PADDING_Y};
    style.ItemSpacing = {ITEM_SPACING_X, ITEM_SPACING_Y};
    style.ItemInnerSpacing = {ITEM_SPACING_X, ITEM_SPACING_Y};
    style.IndentSpacing = INDENT_SPACING;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    ImVec4* colors = style.Colors;

    // text
    colors[ImGuiCol_Text] = rgba(colors::Grey100);
    colors[ImGuiCol_TextDisabled] = rgba(colors::Grey300);
    colors[ImGuiCol_TextSelectedBg] = rgba(colors::Grey500, 0.6f);

    // surfaces
    colors[ImGuiCol_WindowBg] = rgba(colors::Grey900);
    colors[ImGuiCol_ChildBg] = rgba(colors::Grey900);
    colors[ImGuiCol_PopupBg] = rgba(colors::Grey800);
    colors[ImGuiCol_Border] = rgba(colors::Grey500, 0.5f);
    colors[ImGuiCol_BorderShadow] = rgba(colors::Grey900, 0.0f);

    // sunken fields
    colors[ImGuiCol_FrameBg] = rgba(colors::Grey800);
    colors[ImGuiCol_FrameBgHovered] = rgba(colors::Grey600);
    colors[ImGuiCol_FrameBgActive] = rgba(colors::Grey500);

    // title and headers
    colors[ImGuiCol_TitleBg] = rgba(colors::Grey900);
    colors[ImGuiCol_TitleBgActive] = rgba(colors::Grey800);
    colors[ImGuiCol_TitleBgCollapsed] = rgba(colors::Grey900);
    colors[ImGuiCol_MenuBarBg] = rgba(colors::Grey800);

    colors[ImGuiCol_Header] = rgba(colors::Grey700);
    colors[ImGuiCol_HeaderHovered] = rgba(colors::Grey600);
    colors[ImGuiCol_HeaderActive] = rgba(colors::Grey500);

    // controls
    colors[ImGuiCol_Button] = rgba(colors::Grey700);
    colors[ImGuiCol_ButtonHovered] = rgba(colors::Grey600);
    colors[ImGuiCol_ButtonActive] = rgba(colors::Grey500);

    colors[ImGuiCol_CheckMark] = rgba(colors::Grey100);
    colors[ImGuiCol_SliderGrab] = rgba(colors::Grey500);
    colors[ImGuiCol_SliderGrabActive] = rgba(colors::Grey300);

    colors[ImGuiCol_ScrollbarBg] = rgba(colors::Grey900);
    colors[ImGuiCol_ScrollbarGrab] = rgba(colors::Grey700);
    colors[ImGuiCol_ScrollbarGrabHovered] = rgba(colors::Grey600);
    colors[ImGuiCol_ScrollbarGrabActive] = rgba(colors::Grey500);

    colors[ImGuiCol_ResizeGrip] = rgba(colors::Grey700, 0.0f);      // panel edge is dragged, no corner grip
    colors[ImGuiCol_ResizeGripHovered] = rgba(colors::Grey600, 0.5f);
    colors[ImGuiCol_ResizeGripActive] = rgba(colors::Grey500, 0.7f);

    // tabs
    colors[ImGuiCol_Tab] = rgba(colors::Grey800);
    colors[ImGuiCol_TabHovered] = rgba(colors::Grey600);
    colors[ImGuiCol_TabSelected] = rgba(colors::Grey700);
    colors[ImGuiCol_TabDimmed] = rgba(colors::Grey900);
    colors[ImGuiCol_TabDimmedSelected] = rgba(colors::Grey800);

    // separators
    colors[ImGuiCol_Separator] = rgba(colors::Grey500, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = rgba(colors::Grey500);
    colors[ImGuiCol_SeparatorActive] = rgba(colors::Grey300);
    colors[ImGuiCol_DragDropTarget] = rgba(colors::Yellow, 0.9f);

    // navigation
    colors[ImGuiCol_NavCursor] = rgba(colors::Grey500);
    colors[ImGuiCol_NavWindowingHighlight] = rgba(colors::Grey300, 0.7f);
    colors[ImGuiCol_NavWindowingDimBg] = rgba(colors::Grey900, 0.6f);
    colors[ImGuiCol_ModalWindowDimBg] = rgba(colors::Grey900, 0.6f);
}

} // namespace interface
} // namespace BulletRender
