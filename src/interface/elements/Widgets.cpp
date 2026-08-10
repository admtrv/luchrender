/*
 * Widgets.cpp
 */

#include "interface/elements/Widgets.h"

#include "Colors.h"
#include "Config.h"

#include <algorithm>
#include <cstdarg>

namespace BulletRender {
namespace interface {

constexpr float TAG_BRIGHTNESS = 1.6f;          // palette tuned for 3d view, tags need more punch on dark ui
constexpr float COLOR_DRAG_SPEED = 0.005f;      // channels live in 0..1, steps must be small
constexpr float SPLITTER_THICKNESS = 6.0f;      // hit area, drawn line is thinner
constexpr float DRAG_RANGE_FRACTION = 0.002f;   // drag speed as share of range, keeps fields feeling alike

// wide enough for longest caption, scaled with ui font
float LABEL_COLUMN_WIDTH = LABEL_COLUMN_BASE * config::FontScale;

// three tagged fields, tag painted with color of its component
static bool dragComponents(glm::vec3& value, const char* const names[3], const glm::vec3 tints[3],
                           float speed, float min, float max, const char* format, float width)
{
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float cellWidth = (width - spacing * 2.0f) / 3.0f;

    // widest tag sets offset for all three, otherwise narrower glyph
    // lets its field start earlier and columns drift apart
    float tagWidth = 0.0f;
    for (int component = 0; component < 3; component++)
    {
        tagWidth = std::max(tagWidth, ImGui::CalcTextSize(names[component]).x);
    }
    tagWidth += spacing;

    const float startX = ImGui::GetCursorPosX();

    bool changed = false;

    for (int component = 0; component < 3; component++)
    {
        ImGui::PushID(component);

        // cells placed absolutely, rounding in one does not shift next
        const float cellX = startX + static_cast<float>(component) * (cellWidth + spacing);

        ImGui::SetCursorPosX(cellX);
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(tagColor(tints[component]), "%s", names[component]);

        ImGui::SameLine();
        ImGui::SetCursorPosX(cellX + tagWidth);

        ImGui::SetNextItemWidth(cellWidth - tagWidth);
        changed |= ImGui::DragFloat("##value", &value[component], speed, min, max, format);

        ImGui::PopID();

        if (component < 2)
        {
            ImGui::SameLine();
        }
    }

    return changed;
}


void fieldLabel(const char* label)
{
    // column starts where row does, reading cursor after text drifts per label
    const float startX = ImGui::GetCursorPosX();

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(startX + LABEL_COLUMN_WIDTH);
}

ImVec4 tagColor(const glm::vec3& color)
{
    const glm::vec3 lifted = glm::min(color * TAG_BRIGHTNESS, glm::vec3(1.0f));
    return {lifted.r, lifted.g, lifted.b, 1.0f};
}

void statRow(const char* label, const char* format, ...)
{
    fieldLabel(label);

    va_list args;
    va_start(args, format);
    ImGui::TextV(format, args);
    va_end(args);
}

bool dragVector3(const char* label, glm::vec3& value, float speed, float min, float max, const char* format)
{
    static const char* const AXIS_NAMES[] = {"x", "y", "z"};
    static const glm::vec3 AXIS_COLORS[] = {colors::AxisX, colors::AxisY, colors::AxisZ};

    ImGui::PushID(label);
    ImGui::TextUnformatted(label);

    const bool changed = dragComponents(value, AXIS_NAMES, AXIS_COLORS, speed, min, max, format,
                                        ImGui::GetContentRegionAvail().x);

    ImGui::PopID();
    return changed;
}

bool dragColor3(const char* label, glm::vec3& color)
{
    static const char* const CHANNEL_NAMES[] = {"r", "g", "b"};
    static const glm::vec3 CHANNEL_COLORS[] = {colors::Red, colors::Green, colors::Blue};

    ImGui::PushID(label);
    ImGui::TextUnformatted(label);

    // swatch sits at end of row, fields give up its width up front
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float swatchWidth = ImGui::GetFrameHeight();

    bool changed = dragComponents(color, CHANNEL_NAMES, CHANNEL_COLORS, COLOR_DRAG_SPEED, 0.0f, 1.0f, "%.2f",
                                  ImGui::GetContentRegionAvail().x - swatchWidth - spacing);

    ImGui::SameLine(0.0f, spacing);
    changed |= ImGui::ColorEdit3("##swatch", &color.x,
                                 ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    ImGui::PopID();
    return changed;
}

void errorText(const std::string& message)
{
    if (message.empty())
    {
        return;
    }

    ImGui::TextColored(tagColor(colors::Red), "%s", message.c_str());
}

bool dragScalarBare(float& value, float min, float max, const char* format)
{
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    const bool changed = ImGui::DragFloat("##value", &value, (max - min) * DRAG_RANGE_FRACTION, min, max, format);

    value = glm::clamp(value, min, max);
    return changed;
}

bool dragColor3Bare(glm::vec3& color)
{
    static const char* const CHANNEL_NAMES[] = {"r", "g", "b"};
    static const glm::vec3 CHANNEL_COLORS[] = {colors::Red, colors::Green, colors::Blue};

    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float swatchWidth = ImGui::GetFrameHeight();

    bool changed = dragComponents(color, CHANNEL_NAMES, CHANNEL_COLORS, COLOR_DRAG_SPEED, 0.0f, 1.0f, "%.2f",
                                  ImGui::GetContentRegionAvail().x - swatchWidth - spacing);

    ImGui::SameLine(0.0f, spacing);
    changed |= ImGui::ColorEdit3("##swatch", &color.x,
                                 ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    return changed;
}

void splitter(const char* id, float& fraction, float minFraction, float maxFraction)
{
    const float available = ImGui::GetContentRegionAvail().y;

    ImGui::InvisibleButton(id, {ImGui::GetContentRegionAvail().x, SPLITTER_THICKNESS});

    if (ImGui::IsItemActive() && available > 0.0f)
    {
        fraction = glm::clamp(fraction + ImGui::GetIO().MouseDelta.y / available, minFraction, maxFraction);
    }

    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    }

    // line down middle of hit area, brighter while used
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    const float middle = (min.y + max.y) * 0.5f;

    const glm::vec3 tint = ImGui::IsItemActive() || ImGui::IsItemHovered() ? colors::Grey300 : colors::Grey500;
    ImGui::GetWindowDrawList()->AddLine({min.x, middle}, {max.x, middle},
                                        ImGui::GetColorU32(ImVec4{tint.r, tint.g, tint.b, 0.8f}), 1.0f);
}

bool loadFromFileField(const char* id, char* path, size_t size, const char* hint)
{
    ImGui::PushID(id);
    ImGui::TextUnformatted("Load from file");

    // button keeps natural width, field takes rest of row
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    const float buttonWidth = ImGui::CalcTextSize("Load").x + ImGui::GetStyle().FramePadding.x * 2.0f;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - spacing);
    ImGui::InputTextWithHint("##path", hint, path, size);

    ImGui::SameLine(0.0f, spacing);
    const bool pressed = ImGui::Button("Load");

    ImGui::PopID();
    return pressed;
}

bool checkboxField(const char* label, bool& value)
{
    ImGui::PushID(label);
    fieldLabel(label);

    const bool changed = ImGui::Checkbox("##value", &value);

    ImGui::PopID();
    return changed;
}

bool dragScalarField(const char* label, float& value, float min, float max, const char* format)
{
    ImGui::PushID(label);
    fieldLabel(label);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    const bool changed = ImGui::DragFloat("##value", &value, (max - min) * DRAG_RANGE_FRACTION, min, max, format);

    ImGui::PopID();

    value = glm::clamp(value, min, max);
    return changed;
}

bool dragScalarField(const char* label, int& value, int min, int max, const char* format)
{
    ImGui::PushID(label);
    fieldLabel(label);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    const bool changed = ImGui::DragInt("##value", &value, static_cast<float>(max - min) * DRAG_RANGE_FRACTION,
                                        min, max, format);

    ImGui::PopID();

    value = glm::clamp(value, min, max);
    return changed;
}

bool inputTextField(const char* label, char* buffer, size_t size, const char* hint)
{
    ImGui::PushID(label);
    fieldLabel(label);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    const bool changed = hint != nullptr ? ImGui::InputTextWithHint("##value", hint, buffer, size)
                                         : ImGui::InputText("##value", buffer, size);

    ImGui::PopID();
    return changed;
}

} // namespace interface
} // namespace BulletRender
