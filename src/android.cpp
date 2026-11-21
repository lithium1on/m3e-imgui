// Token data is adapted from AndroidX Compose Material 3.
// Copyright 2021-2026 The Android Open Source Project
// Licensed under the Apache License, Version 2.0. See licenses/AndroidX-Apache-2.0.txt.
#include <m3e/android.hpp>
#include "detail.hpp"
#include <algorithm>
#include <cmath>

namespace m3e::android {
using namespace m3e::detail;

Theme DefaultTheme(Scheme scheme) {
    constexpr unsigned light[] = {
        0x6750a4, 0xffffff, 0xeaddff, 0x4f378b, 0xeaddff, 0xd0bcff, 0x21005d, 0x4f378b,
        0x625b71, 0xffffff, 0xe8def8, 0x4a4458, 0xe8def8, 0xccc2dc, 0x1d192b, 0x4a4458,
        0x7d5260, 0xffffff, 0xffd8e4, 0x633b48, 0xffd8e4, 0xefb8c8, 0x31111d, 0x633b48,
        0xb3261e, 0xffffff, 0xf9dedc, 0x8c1d18, 0xfef7ff, 0x1d1b20, 0x49454f, 0xffffff,
        0xf7f2fa, 0xf3edf7, 0xece6f0, 0xe6e0e9, 0xded8e1, 0xfef7ff, 0xe7e0ec, 0x322f35,
        0xf5eff7, 0xd0bcff, 0x79747e, 0xcac4d0, 0, 0, 0x6750a4
    };
    constexpr unsigned dark[] = {
        0xd0bcff, 0x381e72, 0x4f378b, 0xeaddff, 0xeaddff, 0xd0bcff, 0x21005d, 0x4f378b,
        0xccc2dc, 0x332d41, 0x4a4458, 0xe8def8, 0xe8def8, 0xccc2dc, 0x1d192b, 0x4a4458,
        0xefb8c8, 0x492532, 0x633b48, 0xffd8e4, 0xffd8e4, 0xefb8c8, 0x31111d, 0x633b48,
        0xf2b8b5, 0x601410, 0x8c1d18, 0xf9dedc, 0x141218, 0xe6e0e9, 0xcac4d0, 0x0f0d13,
        0x1d1b20, 0x211f26, 0x2b2930, 0x36343b, 0x141218, 0x3b383e, 0x49454f, 0xe6e0e9,
        0x322f35, 0x6750a4, 0x938f99, 0x49454f, 0, 0, 0xd0bcff
    };
    static_assert(std::size(light) == static_cast<std::size_t>(Role::Count));
    static_assert(std::size(dark) == std::size(light));
    const auto& palette = scheme == Scheme::Dark ? dark : light;
    Theme theme;
    for (std::size_t i = 0; i < std::size(palette); ++i) {
        const unsigned rgb = palette[i];
        theme.colors[i] = IM_COL32((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, 255);
    }
    return theme;
}

namespace {

constexpr float Pi = 3.14159265359f;
constexpr Type TitleMedium{16, 24, .15f, true};
constexpr Type TitleLarge{22, 28, 0};

struct SpringSpec { float damping, stiffness; };

SpringSpec FastSpatialSpring(const Style& s) {
    return s.motion == MotionPreset::Expressive ? SpringSpec{.6f, 800} : SpringSpec{.9f, 1400};
}

// Solve the damped spring with the previous sample's position and velocity as initial conditions.
float Spring(const Style& s, ImGuiID id, const char* channel, float target, SpringSpec spec) {
    auto* storage = ImGui::GetStateStorage();
    const auto key = StateKey(id, channel);
    const auto positionKey = StateKey(key, "position");
    const auto velocityKey = StateKey(key, "velocity");
    const auto timeKey = StateKey(key, "time");
    const float now = static_cast<float>(ImGui::GetTime());
    const float last = storage->GetFloat(timeKey, now);
    const float oldTarget = storage->GetFloat(key, target);
    float position = storage->GetFloat(positionKey, target);
    float velocity = storage->GetFloat(velocityKey);
    const float dt = now - last;

    if (s.reducedMotion) {
        position = target;
        velocity = 0;
    } else if (dt > .7f) {
        position = oldTarget;
        velocity = 0;
    } else if (position != oldTarget || velocity != 0) {
        const float omega = std::sqrt(spec.stiffness);
        const float decay = omega * spec.damping;
        const float frequency = omega * std::sqrt(1 - spec.damping * spec.damping);
        const float offset = position - oldTarget;
        const float sinComponent = (velocity + decay * offset) / frequency;
        const float sinT = std::sin(frequency * dt), cosT = std::cos(frequency * dt);
        const float envelope = std::exp(-decay * dt);
        const float displacement = offset * cosT + sinComponent * sinT;
        position = oldTarget + envelope * displacement;
        velocity = envelope * (frequency * (-offset * sinT + sinComponent * cosT) - decay * displacement);
        if (std::fabs(position - oldTarget) < .0001f && std::fabs(velocity) < .001f) {
            position = oldTarget;
            velocity = 0;
        }
    }
    storage->SetFloat(key, target);
    storage->SetFloat(positionKey, position);
    storage->SetFloat(velocityKey, velocity);
    storage->SetFloat(timeKey, now);
    return position;
}

struct FabMetrics {
    float size, radius, icon;
};

struct ExtendedMetrics {
    FabMetrics fab;
    float padding, gap;
    Type type;
};

constexpr FabMetrics FabSizes[]{{40, 12, 24}, {56, 16, 24}, {80, 20, 28}, {96, 28, 36}};
constexpr ExtendedMetrics ExtendedSizes[]{
    {{56, 16, 24}, 16, 8, TitleMedium},
    {{80, 20, 28}, 26, 12, TitleLarge},
    {{96, 28, 36}, 28, 16, HeadlineSmall}
};

struct FabColors { ImU32 background, ink; };

FabColors FabPalette(const Theme& theme, FabColor color, bool disabled) {
    if (disabled)
        return {Opacity(theme[Role::OnSurface], .12f), Opacity(theme[Role::OnSurface], .38f)};
    switch (color) {
        case FabColor::Secondary: return {theme[Role::SecondaryContainer], theme[Role::OnSecondaryContainer]};
        case FabColor::Tertiary: return {theme[Role::TertiaryContainer], theme[Role::OnTertiaryContainer]};
        case FabColor::Surface: return {theme[Role::SurfaceContainerHigh], theme[Role::Primary]};
        case FabColor::Primary: return {theme[Role::PrimaryContainer], theme[Role::OnPrimaryContainer]};
    }
    return {theme[Role::PrimaryContainer], theme[Role::OnPrimaryContainer]};
}

void FabShadow(const Style& s, ImDrawList* draw, Rect box, float radius, bool raised) {
    // Diffused ambient shadow plus the downward cast shadow. The 6 dp profile
    // is sampled from a physical Android device using Compose's default FAB elevation.
    const float elevation = raised ? 8.0f / 6.0f : 1.0f;
    const auto ink = s.theme[Role::Shadow];
    auto lobe = [&](float opacity, float sigma, float offset) {
        sigma *= elevation;
        offset = s.Px(offset * elevation);
        float coverage = 0;
        const int layers = static_cast<int>(std::ceil(sigma * 6));
        for (int layer = layers; layer >= -layers; --layer) {
            const float distance = .5f * layer;
            const float desired = opacity * .5f * std::erfc(distance / (sigma * 1.41421356f));
            const auto color = Opacity(ink, (desired - coverage) / (1 - coverage));
            const float alpha = static_cast<float>((color & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) / 255;
            if (alpha == 0)
                continue;
            const float spread = s.Px(distance);
            auto bounds = box.Inset(-spread);
            bounds.min.y += offset;
            bounds.max.y += offset;
            Fill(draw, bounds, std::max(0.0f, radius + spread), color);
            coverage += (1 - coverage) * alpha;
        }
    };
    lobe(.07f, 1.44f, 0);
    lobe(.15f, 3.65f, 3.18f);
}

bool DrawFab(const Style& s, const char* id, Icon icon, FabColor color, bool disabled, const FabMetrics& metrics,
             const ExtendedMetrics* extended = nullptr, bool expanded = false) {
    if (SkipItems())
        return false;
    const auto size = s.Px(metrics.size);
    const auto radius = s.Px(metrics.radius);
    const auto symbolSize = s.Px(metrics.icon);
    const auto label = extended ? Label(id) : std::string_view{};
    const auto position = ImGui::GetCursorScreenPos();
    float width = size, phase = 0;
    if (extended) {
        const auto springId = ImGui::GetID(id);
        phase = Spring(s, springId, "fab width", expanded ? 1.0f : 0.0f, FastSpatialSpring(s));
        const float full = s.Px(extended->padding * 2 + extended->gap) + symbolSize + TextWidth(s, extended->type, label);
        width = std::max(size, Mix(size, full, phase));
    }
    const auto minimum = s.Px(48);
    const auto hit = At(position, std::max(minimum, width), std::max(minimum, size));
    const auto item = Interact(id, hit, disabled);
    if (!item.visible)
        return item.pressed;
    const auto box = Around(hit.Center(), width, size);
    auto* draw = ImGui::GetWindowDrawList();
    const auto palette = FabPalette(s.theme, color, item.disabled);
    if (!item.disabled)
        FabShadow(s, draw, box, radius, item.hovered && !item.held);
    Fill(draw, box, radius, palette.background);
    State(s, draw, item, box, radius, palette.ink);
    Focus(s, draw, item, box, radius);
    const float amount = std::clamp(phase, 0.0f, 1.0f);
    const float padding = extended ? Mix((size - symbolSize) / 2, s.Px(extended->padding), amount)
                                   : (size - symbolSize) / 2;
    const auto center = box.Center();
    Symbol(s, draw, icon, symbolSize, {box.min.x + padding + symbolSize / 2, center.y}, palette.ink);
    if (extended && amount > 0 && !label.empty()) {
        const float x = box.min.x + padding + symbolSize + s.Px(extended->gap);
        const float y = center.y - s.Px(extended->type.lineHeight) / 2;
        draw->PushClipRect(box.min, box.max, true);
        Text(s, draw, extended->type, {x, y}, Opacity(palette.ink, amount), label);
        draw->PopClipRect();
    }
    return item.pressed;
}

Item IconAction(const Style& s, const Choice& action, Rect hit, ImU32 ink, ImU32 selection, bool selected) {
    const auto item = Interact(action.label, hit, action.disabled, false);
    if (!item.visible)
        return item;
    const float boxSize = s.Px(40);
    const auto box = Around(hit.Center(), boxSize, boxSize);
    const float radius = boxSize / 2;
    const float phase = Spring(s, item.id, "toolbar selection", selected ? 1.0f : 0.0f, FastSpatialSpring(s));
    auto* draw = ImGui::GetWindowDrawList();
    if (!item.disabled)
        Fill(draw, box, radius, Opacity(selection, phase));
    const auto foreground = item.disabled ? Opacity(s.theme[Role::OnSurface], .38f) : ink;
    State(s, draw, item, box, radius, foreground);
    Focus(s, draw, item, box, radius);
    Symbol(s, draw, action.icon, s.Px(24), box.Center(), foreground, selected);
    return item;
}

float AvailableWidth(const Style& s, float requested) {
    return requested > 0 ? s.Px(requested) : std::max(0.0f, ImGui::GetContentRegionAvail().x);
}

float Fraction(std::optional<float> value, float fallback) {
    return value && std::isfinite(*value) ? std::clamp(*value, 0.0f, 1.0f) : fallback;
}

void RoundedLine(ImDrawList* draw, const ImVec2* points, int count, ImU32 ink, float thickness, bool closed = false) {
    if (count < 2)
        return;
    draw->AddPolyline(points, count, ink, closed ? ImDrawFlags_Closed : ImDrawFlags_None, thickness);
    if (!closed) {
        draw->AddCircleFilled(points[0], thickness / 2, ink, 12);
        draw->AddCircleFilled(points[count - 1], thickness / 2, ink, 12);
    }
}

void WaveSegment(ImDrawList* draw, const Style& s, float y, float x, float end, float amplitude, float wavelength,
                 float phase, ImU32 color) {
    if (end <= x)
        return;
    const float stroke = s.Px(4), length = end - x;
    if (amplitude <= 0) {
        Fill(draw, At({x - stroke / 2, y - stroke / 2}, length + stroke, stroke), stroke / 2, color);
        return;
    }
    ImVector<ImVec2> points;
    const int segments = std::max(2, static_cast<int>(std::min(4096.0f, std::ceil(length / s.Px(2)))));
    points.reserve(segments + 1);
    for (int i = 0; i <= segments; ++i) {
        const float offset = length * i / segments;
        const float edges = std::clamp(std::min(offset, length - offset) / (wavelength / 4), 0.0f, 1.0f);
        points.push_back({x + offset, y + edges * amplitude * std::sin(2 * Pi * (offset / wavelength - phase))});
    }
    RoundedLine(draw, points.Data, points.Size, color, stroke);
}

struct Segment { float start, end; };

void DrawLinearTrack(ImDrawList* draw, const Style& s, const Rect& bounds, Segment first, Segment second,
                     bool stop, bool wavy) {
    const float stroke = s.Px(4);
    const float start = bounds.min.x + stroke / 2;
    const float end = bounds.max.x - stroke / 2;
    const float y = bounds.Center().y;
    const float gap = s.Px(4) + stroke;
    if (first.start > second.start && second.end > second.start)
        std::swap(first, second);
    float available = start;
    for (const Segment active : {first, second}) {
        if (active.end <= active.start)
            continue;
        WaveSegment(draw, s, y, available, std::min(end, active.start - gap), 0, 1, 0, s.theme[Role::SecondaryContainer]);
        available = std::max(available, active.end + gap);
    }
    WaveSegment(draw, s, y, available, end, 0, 1, 0, s.theme[Role::SecondaryContainer]);
    if (stop && end >= start && available <= end - (wavy ? stroke : stroke / 2))
        draw->AddCircleFilled({end, y}, stroke / 2, s.theme[Role::Primary], 12);
}

void DrawRing(ImDrawList* draw, ImVec2 center, float radius, float start, float sweep, float wave, float phase,
              float thickness, ImU32 ink) {
    if (sweep <= 0)
        return;
    const bool closed = sweep >= 2 * Pi - .0001f;
    const int count = std::max(3, static_cast<int>(std::ceil(96 * sweep / (2 * Pi))));
    const float waves = std::max(1.0f, std::round(radius * 2 * Pi / (thickness * 3.75f)));
    ImVector<ImVec2> points;
    points.reserve(count + 1);
    const int segments = closed ? count - 1 : count;
    for (int i = 0; i <= segments; ++i) {
        const float angle = start + sweep * i / count;
        const float r = radius + wave * std::sin(waves * angle - 2 * Pi * phase);
        points.push_back({center.x + r * std::cos(angle), center.y + r * std::sin(angle)});
    }
    RoundedLine(draw, points.Data, points.Size, ink, thickness, closed);
}

} // namespace

bool FloatingActionButton(const Style& s, const char* id, Icon icon, const FabOptions& o) {
    return DrawFab(s, id, icon, o.color, o.disabled, FabSizes[static_cast<unsigned>(o.size)]);
}

bool ExtendedFloatingActionButton(const Style& s, const char* label, Icon icon, const ExtendedFabOptions& o) {
    const auto& metrics = ExtendedSizes[static_cast<unsigned>(o.size)];
    return DrawFab(s, label, icon, o.color, o.disabled, metrics.fab, &metrics, o.expanded);
}

int ShortNavigationBar(const Style& s, const char* id, const Choice* destinations, int count, int& selected,
                       const ShortNavigationOptions& o) {
    const float width = AvailableWidth(s, o.width);
    if (!destinations || count <= 0 || width <= 0 || SkipItems())
        return -1;
    const auto origin = ImGui::GetCursorScreenPos();
    const auto bar = At(origin, width, s.Px(64));
    ImGui::Dummy({bar.W(), bar.H()});
    auto* draw = ImGui::GetWindowDrawList();
    Fill(draw, bar, 0, s.theme[Role::SurfaceContainer]);
    draw->PushClipRect(bar.min, bar.max, true);
    const bool top = o.iconPosition == NavigationIconPosition::Top;
    const float share = width / count;
    const float pad = o.arrangement == NavigationArrangement::Centered && count >= 3 && count <= 6
        ? width * (100 - 10 * (count + 3)) / 200 : 0;
    float itemWidth = (width - 2 * pad) / count;
    if (pad > 0 && !top) {
        for (int i = 0; i < count; ++i)
            itemWidth = std::min(share, std::max(itemWidth, s.Px(60) + TextWidth(s, LabelMedium, Label(destinations[i].label))));
    }
    float x = origin.x + (width - itemWidth * count) / 2;
    int activated = -1;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        const auto& destination = destinations[i];
        const auto hit = At({x, origin.y}, itemWidth, bar.H());
        x += itemWidth;
        ImGui::PushID(i);
        const auto item = Interact(destination.label, hit, destination.disabled, false);
        if (item.pressed) {
            activated = i;
            if (selected != i) {
                selected = i;
                Edited(item.id);
            }
        }
        if (item.visible) {
            draw->PushClipRect(hit.min, hit.max, true);
            const auto label = Label(destination.label);
            const float labelWidth = TextWidth(s, LabelMedium, label);
            const auto center = hit.Center();
            const float height = s.Px(top ? 32 : 40);
            const float indicatorWidth = std::min(itemWidth, top ? s.Px(56) : s.Px(60) + labelWidth);
            const ImVec2 inkCenter{center.x, center.y - (top ? s.Px(10) : 0)};
            const float phase = item.disabled ? 0 : Spring(s, item.id, "navigation indicator",
                                                          selected == i ? 1.0f : 0.0f, FastSpatialSpring(s));
            const float fillWidth = indicatorWidth * std::max(0.0f, phase);
            const auto indicator = Around(inkCenter, indicatorWidth, height);
            Fill(draw, Around(inkCenter, fillWidth, height), height / 2, Opacity(s.theme[Role::SecondaryContainer], phase));
            const ImU32 idle = s.theme[Role::OnSurfaceVariant];
            const ImU32 iconInk = item.disabled ? Opacity(idle, .38f) : Blend(idle, s.theme[Role::OnSecondaryContainer], phase);
            const auto activeLabel = s.theme[top ? Role::Secondary : Role::OnSecondaryContainer];
            const ImU32 labelInk = item.disabled ? Opacity(idle, .38f) : Blend(idle, activeLabel, phase);
            State(s, draw, item, indicator, height / 2, iconInk, true, true);
            Focus(s, draw, item, indicator, height / 2, true);
            if (top) {
                Symbol(s, draw, destination.icon, s.Px(24), inkCenter, iconInk, selected == i);
                CenterText(s, draw, LabelMedium, {center.x, center.y + s.Px(18)}, labelInk, label);
            } else {
                const float start = center.x - (labelWidth + s.Px(28)) / 2;
                Symbol(s, draw, destination.icon, s.Px(24), {start + s.Px(12), center.y}, iconInk, selected == i);
                Text(s, draw, LabelMedium, {start + s.Px(28), center.y - s.Px(8)}, labelInk, label);
            }
            draw->PopClipRect();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    draw->PopClipRect();
    return activated;
}

int FloatingToolbar(const Style& s, const char* id, const Choice* actions, int count,
                    const bool* selected, const FloatingToolbarOptions& o) {
    if (!actions || count <= 0 || SkipItems())
        return -1;
    const auto p = ImGui::GetCursorScreenPos();
    const float cross = s.Px(64), cell = s.Px(48), padding = s.Px(8);
    const float length = 2 * padding + count * cell;
    const auto surface = At(p, o.vertical ? cross : length, o.vertical ? length : cross);
    ImGui::Dummy({surface.W(), surface.H()});
    const bool vibrant = o.color == ToolbarColor::Vibrant;
    Fill(ImGui::GetWindowDrawList(), surface, cross / 2, s.theme[vibrant ? Role::PrimaryContainer : Role::SurfaceContainer]);
    const ImU32 selectedFill = s.theme[vibrant ? Role::SurfaceContainer : Role::SecondaryContainer];
    int activated = -1;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        const bool active = selected && selected[i];
        const auto ink = vibrant ? (active ? Role::OnSurface : Role::OnPrimaryContainer)
                                 : (active ? Role::OnSecondaryContainer : Role::OnSurfaceVariant);
        const float along = padding + i * cell;
        const auto bounds = At({p.x + (o.vertical ? padding : along), p.y + (o.vertical ? along : padding)}, cell, cell);
        ImGui::PushID(i);
        if (IconAction(s, actions[i], bounds, s.theme[ink], selectedFill, active).pressed)
            activated = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return activated;
}

TopAppBarResult TopAppBar(const Style& s, const char* id, const char* title, const Choice* actions, int count,
                         const TopAppBarOptions& o) {
    TopAppBarResult result;
    const float width = AvailableWidth(s, o.width);
    if (width <= 0 || SkipItems())
        return result;
    const auto p = ImGui::GetCursorScreenPos();
    const auto box = At(p, width, s.Px(64));
    ImGui::Dummy({box.W(), box.H()});
    auto* draw = ImGui::GetWindowDrawList();
    const auto& t = s.theme;
    ImGui::PushID(id);
    const float tint = Animate(s, ImGui::GetID("##surface"), "app bar tint", o.scrolled ? 1.0f : 0.0f, .2f, Effects);
    Fill(draw, box, 0, Blend(t[Role::Surface], t[Role::SurfaceContainer], tint));
    const float cell = s.Px(48), inset = s.Px(4);
    const float top = p.y + (box.H() - cell) / 2;
    const bool navigation = o.navigationIcon && width >= cell + 2 * inset;
    const float leading = navigation ? 2 * inset + cell : s.Px(16);
    const int visibleActions = actions ? std::max(0, std::min(count, static_cast<int>((width - leading - inset) / cell))) : 0;
    const float trailing = box.max.x - inset - visibleActions * cell;
    draw->PushClipRect(box.min, box.max, true);
    if (navigation) {
        result.navigation = IconAction(s, {o.navigationLabel, o.navigationIcon}, At({p.x + inset, top}, cell, cell),
                                       t[Role::OnSurface], 0, false).pressed;
    }
    const ImVec2 textMin{p.x + leading, p.y};
    const ImVec2 textMax{trailing, box.max.y};
    if (textMax.x > textMin.x && title) {
        const auto text = Label(title);
        const auto subtitle = o.subtitle ? Label(o.subtitle) : std::string_view{};
        const float textWidth = std::max(TextWidth(s, TitleLarge, text), TextWidth(s, LabelMedium, subtitle));
        const float x = o.centerTitle ? std::max(textMin.x, std::min(box.Center().x - textWidth / 2, trailing - textWidth))
                                      : textMin.x;
        const float y = box.Center().y - s.Px(subtitle.empty() ? 14 : 22);
        draw->PushClipRect(textMin, textMax, true);
        Text(s, draw, TitleLarge, {x, y}, t[Role::OnSurface], text);
        if (!subtitle.empty())
            Text(s, draw, LabelMedium, {x, y + s.Px(28)}, t[Role::OnSurfaceVariant], subtitle);
        draw->PopClipRect();
    }
    for (int i = 0; i < visibleActions; ++i) {
        ImGui::PushID(i);
        if (IconAction(s, actions[i], At({trailing + i * cell, top}, cell, cell), t[Role::OnSurfaceVariant], 0, false).pressed)
            result.action = i;
        ImGui::PopID();
    }
    draw->PopClipRect();
    ImGui::PopID();
    return result;
}

void LinearProgress(const Style& s, std::optional<float> value, bool wavy, float width) {
    if (SkipItems() || !std::isfinite(width) || width <= 0)
        return;
    const auto p = ImGui::GetCursorScreenPos();
    const auto bounds = At(p, s.Px(width), s.Px(wavy ? 10 : 4));
    ImGui::Dummy({bounds.W(), bounds.H()});
    auto* draw = ImGui::GetWindowDrawList();
    const float start = bounds.min.x + s.Px(2), end = bounds.max.x - s.Px(2);
    if (end <= start)
        return;
    const float fraction = Fraction(value, 0);
    const float now = s.reducedMotion ? 0.0f : static_cast<float>(ImGui::GetTime());
    Segment first{start, Mix(start, end, fraction)}, second{start, start};
    if (!value) {
        if (s.reducedMotion) {
            first = {Mix(start, end, .32f), Mix(start, end, .62f)};
        } else {
            const float time = std::fmod(now, 1.75f);
            constexpr Curve emphasizedAccelerate{.3f, 0, .8f, .15f};
            auto endpoint = [&](float delay, float duration) {
                return Mix(start, end, Ease((time - delay) / duration, emphasizedAccelerate));
            };
            first = {endpoint(.25f, 1), endpoint(0, 1)};
            second = {endpoint(.90f, .85f), endpoint(.65f, .85f)};
        }
    }
    DrawLinearTrack(draw, s, bounds, first, second, value.has_value(), wavy);
    const float amplitude = wavy && (!value || (fraction > .10f && fraction < .95f)) ? s.Px(3) : 0;
    for (const Segment segment : {first, second})
        WaveSegment(draw, s, bounds.Center().y, segment.start, segment.end, amplitude, s.Px(value ? 40 : 20),
                    now, s.theme[Role::Primary]);
}

void CircularProgress(const Style& s, std::optional<float> value, bool wavy, float diameter) {
    if (SkipItems() || !std::isfinite(diameter))
        return;
    const float d = s.Px(diameter > 0 ? diameter : wavy ? 48.0f : 40.0f);
    const auto bounds = At(ImGui::GetCursorScreenPos(), d, d);
    ImGui::Dummy({d, d});
    const float thickness = s.Px(4);
    const float radius = (d - thickness) / 2 - (wavy ? s.Px(1.6f) : 0);
    if (radius <= thickness)
        return;
    const float now = s.reducedMotion ? 0.0f : static_cast<float>(ImGui::GetTime());
    float progress = Fraction(value, .25f);
    float start = -Pi / 2;
    if (!value && !s.reducedMotion) {
        const float chapter = std::fmod(now, 6.0f);
        const float pulse = .5f - .5f * std::cos(2 * Pi * chapter / 3.0f);
        progress = Mix(.10f, .87f, Ease(pulse, Standard));
        start += 2 * Pi * (chapter / 2.0f + Ease(std::fmod(chapter, 1.5f) / 1.5f, Standard));
    }
    auto* draw = ImGui::GetWindowDrawList();
    const auto center = bounds.Center();
    const float sweep = 2 * Pi * progress;
    const float gap = (s.Px(4) + thickness) / radius;
    const auto& theme = s.theme;
    if (progress <= 0) {
        DrawRing(draw, center, radius, start, 2 * Pi, 0, 0, thickness, theme[Role::SecondaryContainer]);
        return;
    }
    if (sweep + 2 * gap < 2 * Pi) {
        DrawRing(draw, center, radius, start + sweep + gap, 2 * Pi - sweep - 2 * gap,
                 0, 0, thickness, theme[Role::SecondaryContainer]);
    }
    const float wave = wavy && (!value || (progress > .10f && progress < .95f)) ? s.Px(1.6f) : 0;
    DrawRing(draw, center, radius, start, sweep, wave, now, thickness, theme[Role::Primary]);
}

} // namespace m3e::android
