/*
 * Scene.cpp
 */

#include "interface/Editor.h"

#include "interface/elements/Widgets.h"

#include "imgui.h"

namespace BulletRender {
namespace interface {

// how much of tab hierarchy may take
constexpr float HIERARCHY_FRACTION_MINIMUM = 0.15f;
constexpr float HIERARCHY_FRACTION_MAXIMUM = 0.85f;

// tree on top, selection below, draggable gap splits height
void Editor::drawScene()
{
    drawHierarchy();
    splitter("SceneSplit", m_hierarchyFraction, HIERARCHY_FRACTION_MINIMUM, HIERARCHY_FRACTION_MAXIMUM);
    drawInspector();
}

} // namespace interface
} // namespace BulletRender
