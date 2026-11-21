#include "detail.hpp"
#include <algorithm>
#include <limits>
#include <string>
#include <vector>

namespace m3e {

Theme Theme::TonalSpot(Scheme scheme) {
    // M3eThemeElement, seed #6750A4, tonal-spot, contrast 0, standard motion.
    static constexpr unsigned light[] = {
        0x655789, 0xfdf7ff, 0xd4c3fd, 0x493c6c, 0xd4c3fd, 0xc6b6ee, 0x352857, 0x524576,
        0x625c71, 0xfdf7ff, 0xe8def8, 0x554f63, 0xe8def8, 0xdad0ea, 0x423c50, 0x5f586e,
        0x7b5270, 0xfff7f9, 0xf4bfe3, 0x5f3956, 0xf4bfe3, 0xe5b2d5, 0x4a2642, 0x694260,
        0xa8364b, 0xfff7f7, 0xf97386, 0x6e0523, 0xfdf7fe, 0x34313a, 0x615d68, 0xffffff,
        0xf8f1fa, 0xf2ecf5, 0xece6f0, 0xe7e0ec, 0xded8e4, 0xfdf7fe, 0xe7e0ec, 0x0f0d12,
        0xa09ba1, 0xd4c3fd, 0x7d7983, 0xb5b0bb, 0, 0, 0x655789
    };
    static constexpr unsigned dark[] = {
        0xcdc0ec, 0x443a5f, 0x574d72, 0xe9deff, 0xded0fe, 0xd0c3ef, 0x3c3256, 0x594e74,
        0xcbc2db, 0x433d51, 0x3e384c, 0xc4bbd4, 0xe8def8, 0xdad0ea, 0x423c50, 0x5f586e,
        0xffcfef, 0x69415f, 0xf4bfe3, 0x5f3956, 0xf4bfe3, 0xe5b2d5, 0x4a2642, 0x694260,
        0xf97386, 0x490013, 0x871c34, 0xff97a3, 0x0f0d12, 0xeae3ef, 0xaea9b4, 0,
        0x141218, 0x1b181f, 0x211e26, 0x27242d, 0x0f0d12, 0x2e2b34, 0x27242d, 0xfdf7fe,
        0x575459, 0x645980, 0x78737e, 0x4a4650, 0, 0, 0xcdc0ec
    };
    static_assert(std::size(light) == static_cast<unsigned>(Role::Count));
    static_assert(std::size(dark) == std::size(light));
    Theme theme;
    const auto& palette = scheme == Scheme::Dark ? dark : light;
    for (std::size_t i = 0; i < std::size(palette); ++i) {
        const unsigned rgb = palette[i];
        theme.colors[i] = IM_COL32((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, 255);
    }
    return theme;
}

Fonts LoadFonts(const char* directory) {
    return LoadFonts(directory, Typeface::Roboto);
}

Fonts LoadFonts(const char* directory, Typeface typeface) {
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    const std::string prefix = std::string(directory) + "/";
    auto load = [&](const char* file, float size) -> ImFont* {
        const auto path = prefix + file;
        ImFontConfig config;
        config.Flags |= ImFontFlags_NoLoadError;
        return atlas->AddFontFromFileTTF(path.c_str(), size, &config);
    };
    const bool flex = typeface == Typeface::GoogleSansFlex;
    return {load(flex ? "GoogleSansFlex-Regular.ttf" : "Roboto-Regular.ttf", 16),
            load(flex ? "GoogleSansFlex-Medium.ttf" : "Roboto-Medium.ttf", 14),
            load("MaterialSymbolsOutlined.ttf", 24), load("MaterialSymbolsOutlined-Filled.ttf", 24),
            flex ? 2504.0f / 2000 : 2400.0f / 2048, 1152.0f / 960};
}

namespace detail {

Rect Rect::Inset(float amount) const {
    return {{min.x + amount, min.y + amount}, {max.x - amount, max.y - amount}};
}

ImU32 Opacity(ImU32 color, float alpha) {
    const auto a = static_cast<unsigned>(((color >> IM_COL32_A_SHIFT) & 255) * std::clamp(alpha, 0.0f, 1.0f) + .5f);
    return (color & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}

ImU32 Blend(ImU32 lhs, ImU32 rhs, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto a = ImGui::ColorConvertU32ToFloat4(lhs);
    auto b = ImGui::ColorConvertU32ToFloat4(rhs);
    const float alpha = Mix(a.w, b.w, t);
    if (alpha <= 0)
        return 0;
    return ImGui::ColorConvertFloat4ToU32({Mix(a.x * a.w, b.x * b.w, t) / alpha,
                                          Mix(a.y * a.w, b.y * b.w, t) / alpha,
                                          Mix(a.z * a.w, b.z * b.w, t) / alpha, alpha});
}

static float Cubic(float t, float p, float q) {
    const float u = 1 - t;
    return 3 * u * u * t * p + 3 * u * t * t * q + t * t * t;
}

float Ease(float x, Curve curve) {
    if (x <= 0 || x >= 1)
        return std::clamp(x, 0.0f, 1.0f);
    float low = 0, high = 1;
    for (int i = 0; i < 14; ++i) {
        const float mid = (low + high) / 2;
        if (Cubic(mid, curve.x1, curve.x2) < x)
            low = mid;
        else
            high = mid;
    }
    return Cubic((low + high) / 2, curve.y1, curve.y2);
}

float Animate(const Style& style, ImGuiID id, const char* channel, float target, float seconds, Curve curve) {
    auto* storage = ImGui::GetStateStorage();
    const ImGuiID key = StateKey(id, channel);
    const ImGuiID fromKey = StateKey(key, "from");
    const ImGuiID timeKey = StateKey(key, "time");
    const float now = static_cast<float>(ImGui::GetTime());
    const float old = storage->GetFloat(key, std::numeric_limits<float>::quiet_NaN());
    if (style.reducedMotion || seconds <= 0 || std::isnan(old)) {
        storage->SetFloat(key, target);
        storage->SetFloat(fromKey, target);
        storage->SetFloat(timeKey, now);
        return target;
    }
    const float from = storage->GetFloat(fromKey, old);
    const float start = storage->GetFloat(timeKey, now);
    const float current = Mix(from, old, Ease((now - start) / seconds, curve));
    if (old != target) {
        storage->SetFloat(key, target);
        storage->SetFloat(fromKey, current);
        storage->SetFloat(timeKey, now);
    }
    return current;
}

using Path = std::vector<ImVec2>;
constexpr float HalfPi = 1.57079632679f;

static Path RoundedPath(Rect rect, Corners c) {
    c.tl = std::max(c.tl, 0.0f); c.tr = std::max(c.tr, 0.0f);
    c.br = std::max(c.br, 0.0f); c.bl = std::max(c.bl, 0.0f);
    // CSS normalizes overlapping radii by a common factor.
    const float scale = 1.0f / std::max({1.0f, (c.tl+c.tr)/rect.W(), (c.bl+c.br)/rect.W(),
                                        (c.tr+c.br)/rect.H(), (c.tl+c.bl)/rect.H()});
    c.tl *= scale; c.tr *= scale; c.br *= scale; c.bl *= scale;
    Path path;
    path.reserve(48);
    auto arc = [&](float x, float y, float radius, float start) {
        if (radius < .01f) {
            path.emplace_back(x, y);
            return;
        }
        const int segments = std::clamp(static_cast<int>(std::ceil(std::sqrt(radius) * 1.4f)), 3, 18);
        for (int i = 0; i <= segments; ++i) {
            const float angle = start + HalfPi * i / segments;
            const ImVec2 point{x + radius * std::cos(angle), y + radius * std::sin(angle)};
            if (path.empty() || std::fabs(path.back().x - point.x) + std::fabs(path.back().y - point.y) > .001f)
                path.push_back(point);
        }
    };
    arc(rect.min.x+c.tl, rect.min.y+c.tl, c.tl, -2*HalfPi);
    arc(rect.max.x-c.tr, rect.min.y+c.tr, c.tr, -HalfPi);
    arc(rect.max.x-c.br, rect.max.y-c.br, c.br, 0);
    arc(rect.min.x+c.bl, rect.max.y-c.bl, c.bl, HalfPi);
    if (std::fabs(path.back().x - path.front().x) + std::fabs(path.back().y - path.front().y) < .001f)
        path.pop_back();
    return path;
}

static Corners Expand(Corners c, float amount) {
    return {std::max(0.0f, c.tl + amount), std::max(0.0f, c.tr + amount),
            std::max(0.0f, c.br + amount), std::max(0.0f, c.bl + amount)};
}

void Fill(ImDrawList* draw, Rect rect, Corners corners, ImU32 color) {
    if (!(color & IM_COL32_A_MASK) || rect.W() <= 0 || rect.H() <= 0)
        return;
    if (corners.tl == 0 && corners.tr == 0 && corners.br == 0 && corners.bl == 0) {
        draw->AddRectFilled(rect.min, rect.max, color);
        return;
    }
    const auto path = RoundedPath(rect, corners);
    draw->AddConvexPolyFilled(path.data(), static_cast<int>(path.size()), color);
}

void Stroke(ImDrawList* draw, Rect rect, Corners corners, ImU32 color, float thickness) {
    if (!(color & IM_COL32_A_MASK) || rect.W() <= thickness || rect.H() <= thickness || thickness <= 0)
        return;
    const auto path = RoundedPath(rect.Inset(thickness / 2), Expand(corners, -thickness / 2));
    draw->AddPolyline(path.data(), static_cast<int>(path.size()), color, ImDrawFlags_Closed, thickness);
}

void Shadow(const Style& style, ImDrawList* draw, Rect rect, Corners corners, int level, float opacity) {
    if (!level)
        return;
    const int layers = 3 + 2 * level;
    for (int i = layers; i >= 0; --i) {
        const float spread = style.Px(i * .75f);
        Rect shadow = rect.Inset(-spread);
        shadow.min.y += style.Px(static_cast<float>(level));
        shadow.max.y += style.Px(static_cast<float>(level));
        Fill(draw, shadow, Expand(corners, spread), Opacity(style.theme[Role::Shadow], opacity * (.03f + .035f * (i == 0))));
    }
}

static float Side(ImVec2 a, ImVec2 b, ImVec2 p) {
    return (b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x);
}

static Path Clip(Path circle, const Path& outline) {
    Path buffer;
    buffer.reserve(circle.size() + outline.size());
    for (std::size_t edge = 0; edge < outline.size() && !circle.empty(); ++edge) {
        const auto a = outline[edge];
        const auto b = outline[(edge + 1) % outline.size()];
        auto prev = circle.back();
        float prevSide = Side(a, b, prev);
        for (const auto point : circle) {
            const float side = Side(a, b, point);
            if ((side < 0) != (prevSide < 0)) {
                const float t = prevSide / (prevSide - side);
                buffer.emplace_back(Mix(prev.x, point.x, t), Mix(prev.y, point.y, t));
            }
            if (side >= 0)
                buffer.push_back(point);
            prev = point;
            prevSide = side;
        }
        circle.swap(buffer);
        buffer.clear();
    }
    return circle;
}

void State(const Style& style, ImDrawList* draw, const Item& item, Rect rect, Corners corners, ImU32 color,
           bool ripple, bool centered) {
    if (!item.id || item.disabled || rect.W() <= 0 || rect.H() <= 0)
        return;
    const float layer = item.held ? (style.reducedMotion || !ripple ? PressedOpacity : 0)
                                  : item.focused ? FocusOpacity : item.hovered ? HoverOpacity : 0;
    Fill(draw, rect, corners, Opacity(color, Animate(style, item.id, "layer", layer, .075f, Standard)));
    if (!ripple || style.reducedMotion) {
        ImGui::GetStateStorage()->SetFloat(StateKey(item.id, "rippleStart"), -10);
        return;
    }
    auto* storage = ImGui::GetStateStorage();
    const float now = static_cast<float>(ImGui::GetTime());
    const ImGuiID startKey = StateKey(item.id, "rippleStart");
    const ImGuiID endKey = StateKey(item.id, "rippleEnd");
    const ImGuiID xKey = StateKey(item.id, "rippleX");
    const ImGuiID yKey = StateKey(item.id, "rippleY");
    const ImGuiID wasHeldKey = StateKey(item.id, "rippleHeld");
    const bool wasHeld = storage->GetBool(wasHeldKey);
    storage->SetBool(wasHeldKey, item.held);
    if ((item.held && !wasHeld) || (item.pressed && !item.held && !wasHeld)) {
        auto origin = rect.Center();
        if (!centered && item.hovered && ImGui::IsMousePosValid()) {
            const auto mouse = ImGui::GetIO().MousePos;
            origin = {std::clamp(mouse.x, rect.min.x, rect.max.x), std::clamp(mouse.y, rect.min.y, rect.max.y)};
        }
        storage->SetFloat(xKey, origin.x - rect.min.x);
        storage->SetFloat(yKey, origin.y - rect.min.y);
        storage->SetFloat(startKey, now);
        storage->SetFloat(endKey, item.held ? -1.0f : now);
    } else if (wasHeld && !item.held) {
        storage->SetFloat(endKey, now);
    }
    const float start = storage->GetFloat(startKey, -10);
    const float end = storage->GetFloat(endKey, -1);
    const float fade = end < 0 ? 1.0f : 1 - Ease((now - end) / .35f, {0, 0, .2f, .1f});
    if (start < 0 || fade <= 0)
        return;
    const float enter = std::clamp((now - start) / .45f, 0.0f, 1.0f);
    const ImVec2 origin{rect.min.x + storage->GetFloat(xKey), rect.min.y + storage->GetFloat(yKey)};
    const float dx = std::max(origin.x - rect.min.x, rect.max.x - origin.x);
    const float dy = std::max(origin.y - rect.min.y, rect.max.y - origin.y);
    const float furthest = std::hypot(dx, dy);
    const float radius = 4 * furthest * enter;
    const float blur = std::min(style.Px(20), radius);
    const auto silhouette = RoundedPath(rect, corners);
    if (radius - blur >= furthest) {
        draw->AddConvexPolyFilled(silhouette.data(), static_cast<int>(silhouette.size()),
                                  Opacity(color, PressedOpacity * fade));
        return;
    }
    constexpr int layers = 5;
    const float alpha = 1 - std::pow(1 - PressedOpacity * fade, 1.0f / layers);
    for (int layer = 0; layer < layers; ++layer) {
        const float extent = radius + Mix(-blur, blur, (layer + .5f) / layers);
        if (extent < .5f)
            continue;
        Path circle;
        circle.reserve(32);
        for (int i = 0; i < 32; ++i) {
            const float angle = i * 4 * HalfPi / 32.0f;
            circle.emplace_back(origin.x + extent * std::cos(angle), origin.y + extent * std::sin(angle));
        }
        const auto clipped = Clip(std::move(circle), silhouette);
        if (clipped.size() >= 3)
            draw->AddConvexPolyFilled(clipped.data(), static_cast<int>(clipped.size()), Opacity(color, alpha));
    }
}

void Focus(const Style& style, ImDrawList* draw, const Item& item, Rect rect, Corners corners, bool inward) {
    auto* storage = ImGui::GetStateStorage();
    const auto key = StateKey(item.id, "focusStart");
    const float now = static_cast<float>(ImGui::GetTime());
    float start = storage->GetFloat(key, -1);
    if (!item.focused && start >= 0)
        storage->SetFloat(key, -1);
    if (!item.focused)
        return;
    if (start < 0) {
        start = now;
        storage->SetFloat(key, start);
    }
    const auto color = style.theme[Role::Secondary];
    const float progress = std::clamp((now - start) / .5f, 0.0f, 1.0f);
    const float pulse = style.reducedMotion ? 0 : std::sin(progress * 2 * HalfPi);
    const float thickness = style.Px(3) * (1 + pulse);
    const float offset = inward ? 0 : style.Px(2) + thickness;
    Stroke(draw, rect.Inset(-offset), Expand(corners, offset), color, thickness);
}

std::string_view Label(const char* label) {
    const std::string_view value = label ? label : "";
    return value.substr(0, value.find("##"));
}

ImFont* Font(const Style& style, Type type) {
    ImFont* font = type.medium && style.fonts.medium ? style.fonts.medium : style.fonts.regular;
    return font ? font : ImGui::GetFont();
}

float FontSize(const Style& style, Type type) {
    const bool material = style.fonts.regular || (type.medium && style.fonts.medium);
    return style.Px(type.size) * (material ? style.fonts.textEmScale : 1.0f);
}

float BaseFontSize(float screenSize) {
    const auto& s = ImGui::GetStyle();
    return screenSize / (s.FontScaleMain * s.FontScaleDpi);
}

static unsigned Encode(Icon cp, char (&utf8)[5]) {
    if (cp <= 0x7f) {
        utf8[0] = static_cast<char>(cp);
        return 1;
    }
    if (cp <= 0x7ff) {
        utf8[0] = static_cast<char>(0xc0 | (cp >> 6));
        utf8[1] = static_cast<char>(0x80 | (cp & 63));
        return 2;
    }
    if (cp <= 0xffff) {
        utf8[0] = static_cast<char>(0xe0 | (cp >> 12));
        utf8[1] = static_cast<char>(0x80 | ((cp >> 6) & 63));
        utf8[2] = static_cast<char>(0x80 | (cp & 63));
        return 3;
    }
    utf8[0] = static_cast<char>(0xf0 | (cp >> 18));
    utf8[1] = static_cast<char>(0x80 | ((cp >> 12) & 63));
    utf8[2] = static_cast<char>(0x80 | ((cp >> 6) & 63));
    utf8[3] = static_cast<char>(0x80 | (cp & 63));
    return 4;
}

static const char* NextCharacter(const char* current, const char* end) {
    ++current;
    while (current < end && (static_cast<unsigned char>(*current) & 0xc0) == 0x80)
        ++current;
    return current;
}

float TextWidth(const Style& style, Type type, std::string_view value) {
    if (value.empty())
        return 0;
    auto* font = Font(style, type);
    const auto* end = value.data() + value.size();
    float width = font->CalcTextSizeA(FontSize(style, type), 1e7f, 0, value.data(), end).x;
    for (auto* p = value.data(); type.tracking != 0 && p < end; p = NextCharacter(p, end))
        width += style.Px(type.tracking);
    return width;
}

void Text(const Style& style, ImDrawList* draw, Type type, ImVec2 top, ImU32 color, std::string_view value) {
    if (value.empty())
        return;
    auto* font = Font(style, type);
    const float size = FontSize(style, type);
    const auto* metrics = font->GetFontBaked(size);
    top.y += (style.Px(type.lineHeight) - metrics->Ascent + metrics->Descent) / 2;
    const char* end = value.data() + value.size();
    if (type.tracking == 0) {
        draw->AddText(font, size, top, color, value.data(), end);
        return;
    }
    for (auto* p = value.data(); p < end;) {
        const auto* next = NextCharacter(p, end);
        draw->AddText(font, size, top, color, p, next);
        top.x += font->CalcTextSizeA(size, 1e7f, 0, p, next).x + style.Px(type.tracking);
        p = next;
    }
}

void CenterText(const Style& style, ImDrawList* draw, Type type, ImVec2 center, ImU32 color, std::string_view value) {
    Text(style, draw, type, {center.x - TextWidth(style, type, value) / 2, center.y - style.Px(type.lineHeight) / 2},
         color, value);
}

void Symbol(const Style& style, ImDrawList* draw, Icon cp, float size, ImVec2 center, ImU32 color, bool filled) {
    auto* font = filled && style.fonts.symbolsFilled ? style.fonts.symbolsFilled : style.fonts.symbols;
    if (!cp || !font || cp > 0x10ffff)
        return;
    size *= style.fonts.symbolEmScale;
    char text[5]{};
    const char* end = text + Encode(cp, text);
    const auto width = font->CalcTextSizeA(size, 1e7f, 0, text, end).x;
    const auto* metrics = font->GetFontBaked(size);
    draw->AddText(font, size, {center.x - width / 2, center.y - (metrics->Ascent - metrics->Descent) / 2},
                  color, text, end);
}

} // namespace detail
} // namespace m3e
