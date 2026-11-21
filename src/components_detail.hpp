#pragma once

#include "detail.hpp"
#include <m3e/components.hpp>
#include <algorithm>

namespace m3e::detail {

inline float ContentWidth(const Style& style, float width) {
    return width > 0 ? style.Px(width) : std::max(style.Px(1), ImGui::GetContentRegionAvail().x);
}

inline Rect Slot(const Style& style, float width, float height) {
    return At(ImGui::GetCursorScreenPos(), ContentWidth(style, width), style.Px(height));
}

inline void Passive(Rect rect) { ImGui::Dummy({rect.W(), rect.H()}); }

struct RowStyle {
    Role background = Role::SurfaceContainerLow, ink = Role::OnSurface;
    float rounding = 12, padding = 16, elevation = 0;
    bool border = false;
};

Item Row(const Style&, const char* id, Rect, const ListEntry&, const RowStyle&, bool layout = true);
bool BeginSurface(const Style&, const char* id, ImVec2 pixelSize, Role, float rounding = 12,
                  bool border = false, ImGuiWindowFlags flags = 0, bool padding = true);
void EndSurface();

} // namespace m3e::detail
