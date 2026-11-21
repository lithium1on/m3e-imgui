#pragma once

#include <m3e/imgui.hpp>
#include <array>
#include <cmath>
#include <string_view>

namespace m3e::detail {

struct Rect {
    ImVec2 min, max;
    float W() const { return max.x - min.x; }
    float H() const { return max.y - min.y; }
    ImVec2 Center() const { return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f}; }
    Rect Inset(float amount) const;
};
inline Rect At(ImVec2 p, float w, float h) { return {p, {p.x + w, p.y + h}}; }
inline Rect Around(ImVec2 center, float w, float h) { return At({center.x - w / 2, center.y - h / 2}, w, h); }
inline float Mix(float a, float b, float t) { return a + (b - a) * t; }
inline std::size_t Index(Size size) { return static_cast<std::size_t>(size); }
inline float Density(const Style& style) { return 4.0f * std::fmax(-3.0f, style.density); }

struct Corners {
    float tl, tr, br, bl;
    Corners(float all = 0) : tl(all), tr(all), br(all), bl(all) {}
    Corners(float a, float b, float c, float d) : tl(a), tr(b), br(c), bl(d) {}
};

struct Curve { float x1, y1, x2, y2; };
constexpr Curve Standard{.2f, 0, 0, 1};
constexpr Curve Decelerate{0, 0, 0, 1};
constexpr Curve Accelerate{.3f, 0, 1, 1};
constexpr Curve Spatial{.27f, 1.06f, .18f, 1};
constexpr Curve FastEffects{.31f, .94f, .34f, 1};
constexpr Curve Effects{.34f, .8f, .34f, 1};
inline Curve FastSpatial(const Style& style) {
    return style.motion == MotionPreset::Expressive ? Curve{.42f, 1.67f, .21f, .90f} : Spatial;
}
inline Curve DefaultSpatial(const Style& style) {
    return style.motion == MotionPreset::Expressive ? Curve{.38f, 1.21f, .22f, 1} : Spatial;
}
constexpr float HoverOpacity = .08f;
constexpr float FocusOpacity = .1f;
constexpr float PressedOpacity = .1f;

struct Item {
    ImGuiID id = 0;
    bool visible = false, pressed = false, hovered = false, held = false, focused = false, disabled = false;
};

Item Interact(const char* label, Rect hit, bool disabled, bool layout = true, bool pressOnClick = false);
bool SkipItems();
bool NavVisible();
bool NavStep(bool vertical, float& direction, bool& boundary);
void Edited(ImGuiID id);
void FocusCurrentItem();
bool NavIntoSubmenu();
bool SubmenuBack();
void CloseMenuChain();
void CloseChildMenus();
bool EscapePressed();
bool ClickedOutsideCurrentPopup();
void ReserveLayout(Rect rect);
ImGuiID StateKey(ImGuiID id, const char* channel);

float Ease(float x, Curve curve);
float Animate(const Style& style, ImGuiID id, const char* channel, float target, float seconds, Curve curve);
ImU32 Opacity(ImU32 color, float alpha);
ImU32 Blend(ImU32 lhs, ImU32 rhs, float t);
void Fill(ImDrawList* draw, Rect rect, Corners corners, ImU32 color);
void Stroke(ImDrawList* draw, Rect rect, Corners corners, ImU32 color, float thickness);
void Shadow(const Style& style, ImDrawList* draw, Rect rect, Corners corners, int level, float opacity = 1);
void State(const Style& style, ImDrawList* draw, const Item& item, Rect rect, Corners corners, ImU32 color,
           bool ripple = true, bool centered = false);
void Focus(const Style& style, ImDrawList* draw, const Item& item, Rect rect, Corners corners, bool inward = false);

struct Type {
    float size;
    float lineHeight;
    float tracking;
    bool medium = false;
};
constexpr Type LabelLarge{14, 20, .1f, true};
constexpr Type LabelMedium{12, 16, .5f, true};
constexpr Type BodySmall{12, 16, .4f};
constexpr Type BodyMedium{14, 20, .25f};
constexpr Type BodyLarge{16, 24, .5f};
constexpr Type TitleSmall{14, 20, .1f, true};
constexpr Type HeadlineSmall{24, 32, 0};
constexpr Type HeadlineLarge{32, 40, 0};
std::string_view Label(const char* label);
float TextWidth(const Style& style, Type type, std::string_view value);
void Text(const Style& style, ImDrawList* draw, Type type, ImVec2 top, ImU32 color, std::string_view value);
void CenterText(const Style& style, ImDrawList* draw, Type type, ImVec2 center, ImU32 color, std::string_view value);
void Symbol(const Style& style, ImDrawList* draw, Icon codepoint, float size, ImVec2 center, ImU32 color,
            bool filled = false);
ImFont* Font(const Style& style, Type type);
float FontSize(const Style& style, Type type);
float BaseFontSize(float screenSize);

} // namespace m3e::detail
