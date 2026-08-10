/*
 * TreeView.h
 */

#pragma once

#include <string>
#include <vector>

namespace BulletRender {
namespace interface {

// ascii art tree rows, for example:
//
//   Objects
//   ├── Cube
//   │   └── Box
//   └── Box

class TreeView {
public:
    // row at current depth, last marks final entry among siblings
    // id separates rows with same label, returns true on click
    bool row(const void* id, const char* label, bool last, bool selected, bool foldable = false, bool folded = false);

    // enter and leave children of row just drawn
    void push(bool last) { m_levels.push_back(last); }
    void pop() { m_levels.pop_back(); }

    // last row had its arrow clicked
    bool toggled() const { return m_toggled; }

    void reset() { m_levels.clear(); }

private:
    std::string prefix(bool last) const;

    // box drawing glyphs, ui font loads range that covers them
    static constexpr const char* BLANK =  "    ";   // ancestor was last one
    static constexpr const char* TRUNK =  "│   ";   // ancestor continues below
    static constexpr const char* BRANCH = "├── ";   // sibling follows
    static constexpr const char* CORNER = "└── ";   // last child

    std::vector<bool> m_levels;
    bool m_toggled = false;
};

} // namespace interface
} // namespace BulletRender
