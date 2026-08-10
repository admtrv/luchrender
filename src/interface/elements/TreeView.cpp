/*
 * TreeView.cpp
 */

#include "interface/elements/TreeView.h"

#include "imgui.h"

namespace BulletRender {
namespace interface {

std::string TreeView::prefix(bool last) const
{
    std::string result;
    result.reserve(m_levels.size() * 4 + 4);

    // ancestor either keeps line running or leaves blank
    for (bool ancestorWasLast : m_levels)
    {
        result += ancestorWasLast ? BLANK : TRUNK;
    }

    result += last ? CORNER : BRANCH;
    return result;
}

bool TreeView::row(const void* id, const char* label, bool last, bool selected, bool foldable, bool folded)
{
    m_toggled = false;

    // names may repeat, addresses never collide
    ImGui::PushID(id);

    // top level rows have nothing above, branch would point at nothing
    if (!m_levels.empty())
    {
        ImGui::TextUnformatted(prefix(last).c_str());
        ImGui::SameLine(0.0f, 0.0f);
    }

    // only foldable row gets arrow, plain one would reserve empty space
    if (foldable)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (selected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // fold state belongs to caller, imgui only reports clicks
        ImGui::SetNextItemOpen(!folded);

        if (ImGui::TreeNodeEx("##row", flags, "%s", label))
        {
            ImGui::TreePop();
        }

        m_toggled = ImGui::IsItemToggledOpen();
    }
    else
    {
        // ## keeps label visible, pushed id decides identity
        ImGui::Selectable((std::string(label) + "##row").c_str(), selected);
    }

    const bool clicked = ImGui::IsItemClicked() && !m_toggled;

    ImGui::PopID();
    return clicked;
}

} // namespace interface
} // namespace BulletRender
