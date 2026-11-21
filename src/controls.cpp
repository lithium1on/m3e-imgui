#include "detail.hpp"
#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <string>

namespace m3e {
using namespace detail;

namespace {

struct ButtonMetrics {
    float height, padding, gap, icon, iconOnly, outline, square, pressed, connected, connectedPressed;
    Type label;
    float iconPadding[3];
};

// ButtonSizeToken, IconButtonSizeToken and the connected ButtonGroupElement overrides.
constexpr ButtonMetrics ButtonsBySize[] = {
    {32, 12, 8, 20, 20, 1, 12, 8, 8, 4, LabelLarge, {4, 6, 10}},
    {40, 20, 8, 20, 24, 1, 12, 8, 8, 4, LabelLarge, {4, 8, 14}},
    {56, 24, 8, 24, 24, 1, 16, 12, 8, 4, BodyLarge, {12, 16, 24}},
    {96, 48, 12, 32, 32, 2, 28, 16, 16, 12, HeadlineSmall, {16, 32, 48}},
    {136, 64, 16, 40, 40, 3, 28, 16, 20, 16, HeadlineLarge, {24, 48, 72}}
};

struct ButtonColors { ImU32 background, foreground, outline; };

ButtonColors UseButtonColors(const Theme& t, Variant variant, bool iconOnly, bool toggle, bool selected, bool disabled) {
    if (disabled) {
        return {iconOnly && variant == Variant::Text ? 0 : Opacity(t[Role::OnSurface], .10f),
                Opacity(t[Role::OnSurface], .38f), 0};
    }
    switch (variant) {
        case Variant::Elevated:
            return selected ? ButtonColors{t[Role::Primary], t[Role::OnPrimary], 0}
                            : ButtonColors{t[Role::SurfaceContainerLow], t[Role::Primary], 0};
        case Variant::Filled:
            return toggle && !selected ? ButtonColors{t[Role::SurfaceContainer], t[Role::OnSurfaceVariant], 0}
                                       : ButtonColors{t[Role::Primary], t[Role::OnPrimary], 0};
        case Variant::Tonal:
            return selected ? ButtonColors{t[Role::Secondary], t[Role::OnSecondary], 0}
                            : ButtonColors{t[Role::SecondaryContainer], t[Role::OnSecondaryContainer], 0};
        case Variant::Outlined:
            return selected ? ButtonColors{t[Role::InverseSurface], t[Role::InverseOnSurface], 0}
                            : ButtonColors{0, t[Role::OnSurfaceVariant], t[Role::OutlineVariant]};
        case Variant::Text:
            return {0, t[iconOnly && !selected ? Role::OnSurfaceVariant : Role::Primary], 0};
    }
    return {};
}

bool DrawButton(const Style& s, const char* label, const ButtonOptions& o, bool* selected,
                bool iconOnly, IconWidth width, Icon selectedIcon, bool innerLeft = false, bool innerRight = false,
                bool staySelected = false) {
    const auto& m = ButtonsBySize[Index(o.size)];
    const auto text = iconOnly ? std::string_view{} : Label(label);
    const float h = s.Px(m.height + Density(s));
    const float iconSize = s.Px(iconOnly ? m.iconOnly : m.icon);
    const float gap = s.Px(m.gap);
    const float textWidth = TextWidth(s, m.label, text);
    if (!selectedIcon)
        selectedIcon = o.selectedIcon;
    const float leadingSpace = (o.icon || (selected && selectedIcon)) ? iconSize + (iconOnly ? 0 : gap) : 0;
    const float trailingSpace = o.trailingIcon && !iconOnly ? iconSize + gap : 0;
    const float iconPadding = std::max(0.0f, m.iconPadding[static_cast<unsigned>(width)] + Density(s) / 2);
    const float w = iconOnly ? iconSize + 2 * s.Px(iconPadding) : s.Px(2 * m.padding) + textWidth + leadingSpace + trailingSpace;
    const Rect bounds = At(ImGui::GetCursorScreenPos(), w, h);
    const auto item = Interact(label, bounds, o.disabled);
    if (item.pressed && selected) {
        *selected = staySelected || !*selected;
        Edited(item.id);
    }
    if (!item.visible)
        return item.pressed;
    const bool checked = selected && *selected;
    const float selection = Animate(s, item.id, "buttonSelection", checked ? 1.0f : 0.0f, .2f, Standard);
    const float pressed = Animate(s, item.id, "buttonPress", item.held ? 1.0f : 0.0f, .15f, FastEffects);
    const bool connected = innerLeft || innerRight;
    const bool round = o.shape == Shape::Round;
    const float unselectedRadius = round ? h / 2 : s.Px(m.square);
    const float selectedRadius = connected || !round ? h / 2 : s.Px(m.square);
    const float restRadius = Mix(unselectedRadius, selectedRadius, selection);
    const float outer = Mix(restRadius, s.Px(m.pressed), pressed);
    const float inner = Mix(Mix(s.Px(m.connected), selectedRadius, selection), s.Px(m.connectedPressed), pressed);
    const float l = innerLeft ? inner : outer;
    const float r = innerRight ? inner : outer;
    const Corners corners{l, r, r, l};
    auto* draw = ImGui::GetWindowDrawList();
    const auto a = UseButtonColors(s.theme, o.variant, iconOnly, selected != nullptr, false, item.disabled);
    const auto b = selected ? UseButtonColors(s.theme, o.variant, iconOnly, true, true, item.disabled) : a;
    const ButtonColors c{Blend(a.background, b.background, selection), Blend(a.foreground, b.foreground, selection),
                         Blend(a.outline, b.outline, selection)};
    if (o.variant == Variant::Elevated && !item.disabled)
        Shadow(s, draw, bounds, corners, item.hovered && !item.held ? 2 : 1);
    Fill(draw, bounds, corners, c.background);
    Stroke(draw, bounds, corners, c.outline, s.Px(m.outline));
    State(s, draw, item, bounds, corners, c.foreground);
    Focus(s, draw, item, bounds, corners);
    const auto center = bounds.Center();
    const Icon glyph = checked && selectedIcon ? selectedIcon : o.icon;
    if (iconOnly) {
        Symbol(s, draw, glyph, iconSize, center, c.foreground, o.filledIcon);
    } else {
        const float x = bounds.min.x + s.Px(m.padding);
        Symbol(s, draw, glyph, iconSize, {x + iconSize / 2, center.y}, c.foreground, o.filledIcon);
        Text(s, draw, m.label, {x + leadingSpace, center.y - s.Px(m.label.lineHeight) / 2}, c.foreground, text);
        Symbol(s, draw, o.trailingIcon, iconSize,
               {x + leadingSpace + textWidth + gap + iconSize / 2, center.y}, c.foreground);
    }
    return item.pressed;
}

int DrawConnected(const Style& s, const char* id, const Choice* items, int count, int* single, bool* multi,
                  const ConnectedOptions& o) {
    if (count <= 0 || SkipItems())
        return -1;
    int changed = -1;
    ImGui::PushID(id);
    ImGui::BeginGroup();
    for (int i = 0; i < count; ++i) {
        if (i)
            ImGui::SameLine(0, s.Px(2));
        ImGui::PushID(i);
        bool value = multi ? multi[i] : *single == i;
        ButtonOptions button;
        button.size = o.size;
        button.variant = o.variant;
        button.shape = o.shape;
        button.icon = items[i].icon;
        button.disabled = items[i].disabled;
        if (DrawButton(s, items[i].label, button, &value, o.iconsOnly, o.iconWidth, 0, i > 0, i + 1 < count,
                       single != nullptr)) {
            changed = i;
            if (multi)
                multi[i] = value;
            else
                *single = i;
        }
        ImGui::PopID();
    }
    ImGui::EndGroup();
    ImGui::PopID();
    return changed;
}

void ControlLabel(const Style& s, ImDrawList* draw, Rect bounds, float controlWidth, const Item& item,
                  std::string_view label) {
    auto color = s.theme[Role::OnSurface];
    if (item.disabled)
        color = Opacity(color, .38f);
    Text(s, draw, BodyLarge, {bounds.min.x + controlWidth + s.Px(8), bounds.Center().y - s.Px(12)}, color, label);
}

} // namespace

bool Button(const Style& s, const char* label, const ButtonOptions& o) {
    return DrawButton(s, label, o, nullptr, false, IconWidth::Default, 0);
}

bool ToggleButton(const Style& s, const char* label, bool& selected, const ButtonOptions& o) {
    return DrawButton(s, label, o, &selected, false, IconWidth::Default, 0);
}

bool IconButton(const Style& s, const char* id, Icon icon, const ButtonOptions& o, IconWidth width) {
    auto options = o;
    options.icon = icon;
    return DrawButton(s, id, options, nullptr, true, width, 0);
}

bool ToggleIconButton(const Style& s, const char* id, Icon icon, bool& selected, const ButtonOptions& o,
                      IconWidth width, Icon selectedIcon) {
    auto options = o;
    options.icon = icon;
    return DrawButton(s, id, options, &selected, true, width, selectedIcon);
}

int ConnectedButtons(const Style& s, const char* id, const Choice* items, int count, int& selected,
                     const ConnectedOptions& o) {
    return DrawConnected(s, id, items, count, &selected, nullptr, o);
}

bool ConnectedButtonsMulti(const Style& s, const char* id, const Choice* items, int count, bool* selected,
                           const ConnectedOptions& o) {
    return DrawConnected(s, id, items, count, nullptr, selected, o) >= 0;
}

bool Tabs(const Style& s, const char* id, const Choice* tabs, int count, int& selected, const TabOptions& o) {
    if (count <= 0 || SkipItems())
        return false;
    const int previous = selected;
    const bool primary = o.variant == TabVariant::Primary;
    const float h = s.Px((primary ? 64.0f : 48.0f) + Density(s));
    const float divider = s.Px(1);
    const auto stripStart = ImGui::GetCursorScreenPos();
    const auto stripId = ImGui::GetID(id);
    const float available = o.width > 0 ? s.Px(o.width) : ImGui::GetContentRegionAvail().x;
    const auto widthOf = [&](const Choice& tab) {
        const float label = TextWidth(s, TitleSmall, Label(tab.label));
        const float icon = tab.icon ? s.Px(24) : 0;
        return s.Px(48) + (primary ? std::max(label, icon) : label + (icon ? icon + s.Px(8) : 0));
    };
    float growth = 0;
    if (o.stretch) {
        float intrinsicWidth = 0;
        for (int i = 0; i < count; ++i)
            intrinsicWidth += widthOf(tabs[i]);
        growth = std::max(0.0f, available - intrinsicWidth) / count;
    }
    const auto navKey = StateKey(stripId, "tabPendingFocus");
    auto* storage = ImGui::GetStateStorage();
    const int focus = storage->GetInt(navKey, -1);
    storage->SetInt(navKey, -1);
    Rect indicator{};
    float right = stripStart.x;
    auto* draw = ImGui::GetWindowDrawList();
    ImGui::PushID(id);
    ImGui::BeginGroup();
    for (int i = 0; i < count; ++i) {
        if (i)
            ImGui::SameLine(0, 0);
        ImGui::PushID(i);
        const auto label = Label(tabs[i].label);
        const float labelWidth = TextWidth(s, TitleSmall, label);
        const float iconWidth = tabs[i].icon ? s.Px(24) : 0;
        const float contentWidth = primary ? std::max(labelWidth, iconWidth)
                                           : labelWidth + (iconWidth ? iconWidth + s.Px(8) : 0);
        const float width = widthOf(tabs[i]) + growth;
        const auto p = ImGui::GetCursorScreenPos();
        const Rect bounds = At({p.x, p.y + (o.indicatorAtTop ? divider : 0)}, width, h);
        const auto item = Interact(tabs[i].label, bounds, tabs[i].disabled);
        right = bounds.max.x;
        if (i == focus && item.visible && !item.disabled)
            FocusCurrentItem();
        if (item.pressed)
            selected = i;
        float direction = 0;
        bool boundary = false;
        if (!item.disabled && NavStep(false, direction, boundary)) {
            const int step = direction > 0 ? 1 : -1;
            int next = boundary ? (step > 0 ? count - 1 : 0) : (i + step + count) % count;
            for (int j = 0; j < count && tabs[next].disabled; ++j)
                next = (next + (boundary ? -step : step) + count) % count;
            if (!tabs[next].disabled) {
                selected = next;
                storage->SetInt(navKey, next);
            }
        }
        if (item.visible) {
            const auto center = bounds.Center();
            const auto color = item.disabled ? Opacity(s.theme[Role::OnSurface], .38f)
                                : i == selected ? s.theme[primary ? Role::Primary : Role::OnSurface] : s.theme[Role::OnSurfaceVariant];
            State(s, draw, item, bounds, 0, i == selected ? s.theme[Role::Primary] : s.theme[Role::OnSurface]);
            Focus(s, draw, item, bounds.Inset(s.Px(2)), s.Px(16), true);
            if (primary && iconWidth) {
                const float sign = o.indicatorAtTop ? -1.0f : 1.0f;
                Symbol(s, draw, tabs[i].icon, iconWidth, {center.x, center.y - sign * s.Px(10)}, color);
                CenterText(s, draw, TitleSmall, {center.x, center.y + sign * s.Px(12)}, color, label);
            } else {
                const float left = center.x - contentWidth / 2;
                Symbol(s, draw, tabs[i].icon, iconWidth, {left + iconWidth / 2, center.y}, color);
                Text(s, draw, TitleSmall, {left + contentWidth - labelWidth, center.y - s.Px(10)}, color, label);
            }
        }
        const float thickness = s.Px(primary ? 3.0f : 2.0f);
        const float indicatorWidth = primary ? std::max(s.Px(24), labelWidth) - s.Px(4) : width;
        if (i == selected) {
            const float y = o.indicatorAtTop ? bounds.min.y : bounds.max.y - thickness;
            indicator = At({bounds.Center().x - indicatorWidth / 2, y}, indicatorWidth, thickness);
        }
        ImGui::PopID();
    }
    ImGui::EndGroup();
    ImGui::PopID();
    const float dividerY = o.indicatorAtTop ? stripStart.y : stripStart.y + h;
    Fill(draw, At({stripStart.x, dividerY}, right - stripStart.x, divider), 0, s.theme[Role::OutlineVariant]);
    if (indicator.W() > 0) {
        const float x = Animate(s, stripId, "tabIndicatorX", indicator.min.x - stripStart.x, .35f, FastSpatial(s)) + stripStart.x;
        const float w = Animate(s, stripId, "tabIndicatorW", indicator.W(), .35f, FastSpatial(s));
        const float radius = primary ? s.Px(3) : 0;
        const Corners corners = o.indicatorAtTop ? Corners{0, 0, radius, radius} : Corners{radius, radius, 0, 0};
        Fill(draw, At({x, indicator.min.y}, w, indicator.H()), corners, s.theme[Role::Primary]);
    }
    return previous != selected;
}

bool Checkbox(const Style& s, const char* label, CheckState& value, bool disabled, bool error) {
    const auto text = Label(label);
    const float d = s.Px(40 + Density(s));
    const float width = text.empty() ? d : d + s.Px(8) + TextWidth(s, BodyLarge, text);
    const auto bounds = At(ImGui::GetCursorScreenPos(), width, d);
    auto item = Interact(label, bounds, disabled);
    if (item.pressed) {
        value = value == CheckState::Checked ? CheckState::Unchecked : CheckState::Checked;
        Edited(item.id);
    }
    if (!item.visible)
        return item.pressed;
    auto* draw = ImGui::GetWindowDrawList();
    const auto area = At(bounds.min, d, d);
    const auto box = Around(area.Center(), s.Px(18), s.Px(18));
    const bool marked = value != CheckState::Unchecked;
    const auto& t = s.theme;
    const auto color = item.disabled ? Opacity(t[Role::OnSurface], .38f)
                    : error && !marked ? t[Role::Error] : t[marked && !error ? Role::Primary : Role::OnSurfaceVariant];
    State(s, draw, item, area, d / 2, t[error ? (marked ? Role::OnError : Role::Error)
                                          : marked ? Role::Primary : Role::OnSurface],
          true, true);
    const float selection = Animate(s, item.id, "checkFill", marked ? 1.0f : 0.0f, .20f, Standard);
    const float inkProgress = Animate(s, item.id, "checkInk", marked ? 1.0f : 0.0f, .4f, Decelerate);
    if (!error || item.disabled)
        Fill(draw, box, s.Px(2), Opacity(item.disabled ? color : t[Role::Primary], selection));
    if (marked) {
        if (error && !item.disabled)
            Stroke(draw, box, s.Px(2), color, s.Px(2));
        const ImVec2 center = box.Center();
        const auto ink = t[item.disabled ? Role::Surface : error ? Role::OnSurfaceVariant : Role::OnPrimary];
        const auto point = [&](float x, float y) { return ImVec2{center.x + s.Px(x), center.y + s.Px(y)}; };
        const ImVec2 path[] = {point(-6.11f, .23f), point(-4.99f, -.84f), point(-1.84f, 2.36f),
                               point(4.99f, -4.52f), point(6.11f, -3.45f), point(-1.84f, 4.5f)};
        if (value == CheckState::Mixed) {
            Fill(draw, Around(center, s.Px(9), s.Px(1.5f)), 0, ink);
        } else {
            draw->PushClipRect(box.min, {Mix(box.min.x, box.max.x, inkProgress), box.max.y}, true);
            draw->AddConcavePolyFilled(path, 6, Opacity(ink, Mix(.8f, 1, inkProgress)));
            draw->PopClipRect();
        }
    } else {
        Stroke(draw, box, s.Px(2), color, s.Px(2));
    }
    Focus(s, draw, item, box, s.Px(2));
    ControlLabel(s, draw, bounds, d, item, text);
    return item.pressed;
}

bool Checkbox(const Style& s, const char* label, bool& value, bool disabled, bool error) {
    auto state = value ? CheckState::Checked : CheckState::Unchecked;
    const bool changed = Checkbox(s, label, state, disabled, error);
    if (changed)
        value = state == CheckState::Checked;
    return changed;
}

bool Switch(const Style& s, const char* label, bool& value, SwitchIcons icons, bool disabled) {
    const auto text = Label(label);
    const float density = Density(s);
    const float w = s.Px(52 + density), h = s.Px(32 + density);
    const float width = text.empty() ? w : w + s.Px(8) + TextWidth(s, BodyLarge, text);
    const auto bounds = At(ImGui::GetCursorScreenPos(), width, h);
    const auto item = Interact(label, bounds, disabled);
    if (item.pressed) {
        value = !value;
        Edited(item.id);
    }
    if (!item.visible)
        return item.pressed;
    auto* draw = ImGui::GetWindowDrawList();
    const auto track = At(bounds.min, w, h);
    const auto& t = s.theme;
    const float travel = Animate(s, item.id, "switchTravel", value ? 1.0f : 0.0f, .35f, FastSpatial(s));
    const auto trackInk = item.disabled ? (value ? Opacity(t[Role::OnSurface], .12f) : Opacity(t[Role::SurfaceContainerHighest], .12f))
                                         : Blend(t[Role::SurfaceContainerHighest], t[Role::Primary], travel);
    Fill(draw, track, h / 2, trackInk);
    Stroke(draw, track, h / 2, Opacity(item.disabled ? t[Role::OnSurface] : t[Role::Outline],
           (item.disabled ? .12f : 1.0f) * (1 - std::clamp(travel, 0.0f, 1.0f))), s.Px(2));
    const bool iconOn = value && (icons == SwitchIcons::Both || icons == SwitchIcons::Selected);
    const bool iconOff = !value && (icons == SwitchIcons::Both || icons == SwitchIcons::Unselected);
    const float handle = Animate(s, item.id, "switchSize", s.Px((item.held ? 28 : value || (iconOff && !item.disabled) ? 24 : 16) + density),
                                 .15f, FastEffects);
    const ImVec2 center{Mix(track.min.x + s.Px(16 + density / 2), track.max.x - s.Px(16 + density / 2), travel), track.Center().y};
    const auto size = s.Px(40 + density);
    State(s, draw, item, Around(center, size, size), size / 2, t[value ? Role::Primary : Role::OnSurface], false);
    const bool emphasis = item.hovered || item.focused || item.held;
    ImU32 handleInk = Blend(t[emphasis ? Role::OnSurfaceVariant : Role::Outline],
                            t[emphasis ? Role::PrimaryContainer : Role::OnPrimary], travel);
    if (item.disabled)
        handleInk = value ? t[Role::Surface] : Opacity(t[Role::OnSurface], .38f);
    Fill(draw, Around(center, handle, handle), handle / 2, handleInk);
    const auto symbolInk = item.disabled ? Opacity(t[Role::OnSurface], .38f)
                        : t[value ? (emphasis ? Role::OnPrimaryContainer : Role::Primary) : Role::SurfaceContainerHighest];
    Symbol(s, draw, iconOn ? icons::Check : iconOff ? icons::Close : 0, s.Px(16), center, symbolInk);
    Focus(s, draw, item, track, h / 2);
    ControlLabel(s, draw, bounds, w, item, text);
    return item.pressed;
}

bool TextField(const Style& s, const char* label, char* value, std::size_t capacity, const FieldOptions& o,
               ImGuiInputTextCallback callback, void* userData) {
    if (SkipItems() || !value || capacity == 0 || capacity > INT_MAX)
        return false;
    const auto& t = s.theme;
    const bool filled = o.variant == FieldVariant::Filled;
    const bool multiline = o.multilineHeight > 0;
    const float density = Density(s);
    const float h = s.Px(std::max(32.0f, std::max(56.0f + density, o.multilineHeight)));
    const float w = s.Px(std::max(96.0f, o.width));
    const auto p = ImGui::GetCursorScreenPos();
    const auto bounds = At(p, w, h);
    const auto id = ImGui::GetID(label);
    auto* storage = ImGui::GetStateStorage();
    const auto activeKey = StateKey(id, "fieldActive");
    const bool previouslyActive = storage->GetBool(activeKey);
    const float left = s.Px(o.leadingIcon ? 48.0f : 16.0f);
    const float right = s.Px(o.trailingIcon ? 48.0f : 16.0f);
    const auto visibleLabel = Label(label);
    std::string requiredLabel;
    if (o.required && !visibleLabel.empty()) {
        requiredLabel = visibleLabel;
        requiredLabel += " *";
    }
    const std::string_view labelText = requiredLabel.empty() ? visibleLabel : std::string_view(requiredLabel);
    const std::string_view prefix = o.prefix ? o.prefix : "";
    const std::string_view suffix = o.suffix ? o.suffix : "";
    const bool auxiliary = previouslyActive || *value || o.floatLabel || labelText.empty();
    const float prefixSpace = auxiliary ? TextWidth(s, BodyLarge, prefix) : 0;
    const float suffixSpace = auxiliary ? TextWidth(s, BodyLarge, suffix) : 0;
    const float inputH = multiline ? h - s.Px(filled ? 24.0f : 12.0f) : s.Px(20);
    float inputY = filled && auxiliary ? std::max(0.0f, s.Px(24 + density)) : (h - inputH) / 2;
    if (filled && !auxiliary && multiline)
        inputY = s.Px(16);
    const float inputW = std::max(s.Px(24), w - left - right - prefixSpace - suffixSpace);
    auto* draw = ImGui::GetWindowDrawList();
    ImDrawListSplitter layers;
    layers.Split(draw, 2);
    layers.SetCurrentChannel(draw, 1);
    const float inputSize = FontSize(s, BodyLarge);
    ImGui::SetCursorScreenPos({p.x + left + prefixSpace, p.y + inputY + (s.Px(16) - inputSize) / 2});
    ImGui::PushFont(Font(s, BodyLarge), BaseFontSize(inputSize));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, s.Px(2)));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, .38f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, t[Role::OnSurface]);
    ImGui::PushStyleColor(ImGuiCol_InputTextCursor, t[Role::Primary]);
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, t[Role::PrimaryContainer]);
    ImGui::PushStyleColor(ImGuiCol_NavCursor, IM_COL32(0, 0, 0, 0));
    ImGui::BeginDisabled(o.disabled);
    ImGui::PushID(label);
    ImGui::SetNextItemWidth(inputW);
    bool changed;
    if (multiline) {
        changed = ImGui::InputTextMultiline("##input", value, capacity, {inputW, inputH}, o.flags, callback, userData);
    } else if (o.placeholder && auxiliary) {
        changed = ImGui::InputTextWithHint("##input", o.placeholder, value, capacity, o.flags, callback, userData);
    } else {
        changed = ImGui::InputText("##input", value, capacity, o.flags, callback, userData);
    }
    const bool active = ImGui::IsItemActive() || (ImGui::IsItemFocused() && NavVisible());
    ImGui::PopID();
    ImGui::EndDisabled();
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(3);
    ImGui::PopFont();
    storage->SetBool(activeKey, active);
    const bool hovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(bounds.min, bounds.max);
    if (!o.disabled && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered())
        ImGui::SetKeyboardFocusHere(-1);
    // Reserve the container and supporting line after the narrower editor has been submitted.
    ReserveLayout(At(p, w, h + (o.supportingText ? s.Px(20) : 0)));

    const bool floating = active || *value || o.floatLabel || labelText.empty();
    const float phase = Animate(s, id, "floatLabel", floating ? 1.0f : 0.0f, .2f, Standard);
    const float labelCenter = Mix(h / 2, filled ? std::max(s.Px(8), s.Px(16 + density)) : 0, phase);
    const Type labelType{Mix(16, 12, phase), Mix(24, 16, phase), Mix(.5f, .4f, phase)};
    const ImU32 labelInk = o.disabled ? Opacity(t[Role::OnSurface], .38f)
                       : t[o.error ? Role::Error : active ? Role::Primary : Role::OnSurfaceVariant];
    const ImU32 outlineInk = o.disabled ? Opacity(t[Role::OnSurface], .38f)
                       : t[o.error ? Role::Error : active ? Role::Primary : hovered || filled ? Role::OnSurfaceVariant : Role::Outline];
    layers.SetCurrentChannel(draw, 0);
    if (filled) {
        const Corners corners{s.Px(4), s.Px(4), 0, 0};
        Fill(draw, bounds, corners, o.disabled ? Opacity(t[Role::OnSurface], .04f) : t[Role::SurfaceContainerHighest]);
        if (hovered && !active && !o.disabled)
            Fill(draw, bounds, corners, Opacity(t[Role::OnSurface], HoverOpacity));
        const float thickness = s.Px(active ? 3 : 1);
        Fill(draw, At({p.x, bounds.max.y - thickness}, w, thickness), 0, outlineInk);
    } else {
        const float thickness = s.Px(active ? 2 : 1);
        const Corners corners{s.Px(4)};
        if (phase > .55f && !labelText.empty()) {
            const float x0 = p.x + left - s.Px(4);
            const float x1 = p.x + left + TextWidth(s, labelType, labelText) + s.Px(4);
            draw->PushClipRect({p.x, p.y + thickness + .5f}, bounds.max, true);
            Stroke(draw, bounds, corners, outlineInk, thickness);
            draw->PopClipRect();
            draw->PushClipRect({p.x, p.y}, {x0, p.y + thickness + .5f}, true);
            Stroke(draw, bounds, corners, outlineInk, thickness);
            draw->PopClipRect();
            draw->PushClipRect({x1, p.y}, {bounds.max.x, p.y + thickness + .5f}, true);
            Stroke(draw, bounds, corners, outlineInk, thickness);
            draw->PopClipRect();
        } else {
            Stroke(draw, bounds, corners, outlineInk, thickness);
        }
    }
    layers.SetCurrentChannel(draw, 1);
    draw->PushClipRect({p.x, p.y - s.Px(10)}, {bounds.max.x, bounds.max.y}, true);
    Text(s, draw, labelType, {p.x + left, p.y + labelCenter - s.Px(labelType.lineHeight) / 2}, labelInk, labelText);
    if (auxiliary) {
        const float y = p.y + inputY - s.Px(2);
        Text(s, draw, BodyLarge, {p.x + left, y}, labelInk, prefix);
        Text(s, draw, BodyLarge, {p.x + w - right - suffixSpace, y}, labelInk, suffix);
    }
    const auto iconInk = o.disabled ? Opacity(t[Role::OnSurface], .38f) : t[Role::OnSurfaceVariant];
    Symbol(s, draw, o.leadingIcon, s.Px(24), {p.x + s.Px(24), bounds.Center().y}, iconInk);
    Symbol(s, draw, o.trailingIcon, s.Px(24), {bounds.max.x - s.Px(24), bounds.Center().y}, labelInk);
    draw->PopClipRect();
    if (o.supportingText)
        Text(s, draw, BodySmall, {p.x + s.Px(16), bounds.max.y + s.Px(4)},
             t[o.error ? Role::Error : Role::OnSurfaceVariant], o.supportingText);
    layers.Merge(draw);
    return changed;
}

namespace {

bool DrawSlider(const Style& s, const char* label, float& first, float* second, float min, float max, const SliderOptions& o) {
    if (SkipItems())
        return false;
    const auto index = Index(o.size);
    const bool androidTrack = o.trackStyle == SliderTrackStyle::Android;
    constexpr float trackSizes[] = {16, 24, 40, 56, 96};
    constexpr float crossSizes[] = {44, 44, 52, 68, 108};
    constexpr float radii[] = {8, 8, 12, 16, 28};
    const float length = s.Px(std::max(androidTrack ? 48.0f : 200.0f, o.length));
    const float cross = s.Px(androidTrack ? 48.0f : crossSizes[index]);
    const float axisInset = s.Px(androidTrack ? 2.0f : 8.0f);
    const float travel = length - 2 * axisInset;
    const float range = max - min;
    const bool usable = std::isfinite(min) && std::isfinite(max) && std::isfinite(range) && range > 0;
    const auto pos = [&](float value) {
        return axisInset + travel * (usable && std::isfinite(value) ? std::clamp((value - min) / range, 0.0f, 1.0f) : 0.0f);
    };
    const auto origin = ImGui::GetCursorScreenPos();
    const auto rect = [&](float a, float b, float thickness) {
        if (o.vertical)
            return At({origin.x + (cross - thickness) / 2, origin.y + length - b}, thickness, b - a);
        return At({origin.x + a, origin.y + (cross - thickness) / 2}, b - a, thickness);
    };
    const auto full = rect(0, length, cross);
    const float divider = second ? (pos(first) + pos(*second)) / 2 : length;
    const float split = std::clamp(divider, axisInset, length - axisInset);
    const bool disabled = o.disabled || !usable;
    ImGui::PushID(label);
    // Submit the full-size layout first; each thumb then owns a separate focusable item.
    ImGui::Dummy({full.W(), full.H()});
    Item lower = Interact("##lower", rect(0, second ? split : length, cross), disabled, false, true);
    const float step = std::isfinite(o.step) && o.step > 0 ? o.step : usable ? range / 100 : 1;
    auto* storage = ImGui::GetStateStorage();
    auto adjust = [&](const Item& item, float& target, float low, float high) {
        if (!item.visible || item.disabled)
            return false;
        const auto key = StateKey(item.id, "sliderOffset");
        float next = std::isfinite(target) ? target : min;
        bool interacting = false;
        if (item.held && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            const auto pointer = ImGui::GetIO().MousePos;
            const float mouse = o.vertical ? length - pointer.y + origin.y : pointer.x - origin.x;
            if (item.pressed) {
                const float offset = mouse - pos(next);
                storage->SetFloat(key, std::fabs(offset) <= s.Px(8) ? offset : 0);
            }
            const float fraction = std::clamp((mouse - storage->GetFloat(key) - axisInset) / travel, 0.0f, 1.0f);
            next = min + range * fraction;
            interacting = true;
        }
        float direction = 0;
        bool boundary = false;
        if (NavStep(o.vertical, direction, boundary)) {
            next = boundary ? (direction < 0 ? low : high) : next + direction * step;
            interacting = true;
        }
        if (!interacting || low > high)
            return false;
        if (next > low && next < high)
            next = min + std::round((next - min) / step) * step;
        next = std::clamp(next, low, high);
        if (next == target)
            return false;
        target = next;
        Edited(item.id);
        return true;
    };
    const float lowerMax = second && usable && std::isfinite(*second) ? std::clamp(*second, min, max) : max;
    bool changed = adjust(lower, first, min, lowerMax);
    Item upper;
    if (second) {
        upper = Interact("##upper", rect(split, length, cross), disabled, false, true);
        const float upperMin = std::isfinite(first) && usable ? std::clamp(first, min, max) : min;
        changed |= adjust(upper, *second, upperMin, max);
    }
    ImGui::PopID();
    if (!lower.visible && !upper.visible)
        return changed;
    const bool off = disabled || lower.disabled || (second && upper.disabled);
    auto* draw = ImGui::GetWindowDrawList();
    const auto& t = s.theme;
    const float track = s.Px(androidTrack ? 16.0f : trackSizes[index]);
    const float round = s.Px(androidTrack ? 8.0f : radii[index]);
    const float insideRound = androidTrack ? s.Px(2) : 0;
    const auto active = off ? Opacity(t[Role::OnSurface], .38f) : t[Role::Primary];
    const auto inactive = off ? Opacity(t[Role::OnSurface], .12f) : t[Role::SecondaryContainer];
    const float a = pos(first);
    const float b = second ? pos(*second) : a;
    const auto handleWidth = [&](const Item& item) {
        return Animate(s, item.id, "sliderThumb", s.Px(item.held || item.focused ? 2.0f : 4.0f), .1f, Standard);
    };
    const float lowerWidth = handleWidth(lower);
    const float upperWidth = second ? handleWidth(upper) : lowerWidth;
    const float lowerGap = androidTrack ? s.Px(6) + lowerWidth / 2 : s.Px(8);
    const float upperGap = androidTrack ? s.Px(6) + upperWidth / 2 : s.Px(8);
    const float trackStart = androidTrack ? axisInset : 0;
    const float trackEnd = androidTrack ? length - axisInset : length;
    const float leftEnd = a - lowerGap, rightStart = b + upperGap;
    const float outerThreshold = androidTrack ? round : 0;
    const bool leftTrack = leftEnd - trackStart > outerThreshold;
    const bool rightTrack = trackEnd - rightStart > outerThreshold;
    auto trackSection = [&](float begin, float end, float beginRadius, float endRadius, ImU32 ink) {
        const Corners corners = o.vertical ? Corners{endRadius, endRadius, beginRadius, beginRadius}
                                           : Corners{beginRadius, endRadius, endRadius, beginRadius};
        Fill(draw, rect(begin, end, track), corners, ink);
    };
    if (second) {
        if (leftTrack)
            trackSection(trackStart, leftEnd, round, insideRound, inactive);
        trackSection(a + lowerGap, b - upperGap, insideRound, insideRound, active);
    } else if (leftTrack) {
        trackSection(trackStart, leftEnd, round, insideRound, active);
    }
    if (rightTrack)
        trackSection(rightStart, trackEnd, insideRound, round, inactive);
    const auto tick = [&](float x, ImU32 color) {
        const auto center = rect(x, x, track).Center();
        draw->AddCircleFilled(center, s.Px(2), color, 12);
    };
    if (o.ticks && usable) {
        const int count = static_cast<int>(std::min(200.0, std::floor(static_cast<double>(range) / step)));
        for (int i = 0; i <= count; ++i) {
            const float x = pos(min + i * step);
            if (std::fabs(x - a) < lowerGap || (second && std::fabs(x - b) < upperGap))
                continue;
            const bool on = second ? x > a && x < b : x < a;
            tick(x, off ? t[on ? Role::InverseOnSurface : Role::OnSurface]
                        : t[on ? Role::OnPrimary : Role::OnSecondaryContainer]);
        }
    } else {
        const float markerInset = androidTrack ? round : s.Px(6);
        const float leftDot = trackStart + markerInset;
        const float rightDot = trackEnd - markerInset;
        const float radius = s.Px(2);
        const auto ink = androidTrack ? active : t[off ? Role::OnSurface : Role::OnSecondaryContainer];
        if (rightTrack && rightDot - radius >= rightStart)
            tick(rightDot, ink);
        if (second && leftTrack && leftDot + radius <= leftEnd)
            tick(leftDot, ink);
    }
    auto thumb = [&](const Item& item, float x, float value, float width) {
        const float height = androidTrack ? s.Px(44) : std::max(s.Px(4), cross - s.Px(2));
        const auto bar = rect(x - width / 2, x + width / 2, height);
        Fill(draw, bar, width / 2, active);
        Focus(s, draw, item, bar, width / 2);
        const bool show = !off && o.showValue && (lower.hovered || upper.hovered || lower.held || upper.held
                                                   || lower.focused || upper.focused);
        const float phase = Animate(s, item.id, "sliderLabel", show ? 1.0f : 0.0f,
                                    .15f, Standard);
        if (phase < .01f)
            return;
        char buffer[64]{};
        std::snprintf(buffer, sizeof(buffer), "%.*f", std::clamp(o.valueDecimals, 0, 8), static_cast<double>(value));
        const auto center = bar.Center();
        const ImVec2 bubbleCenter = o.vertical ? ImVec2{bar.min.x - s.Px(28), center.y}
                                               : ImVec2{center.x, bar.min.y - s.Px(26)};
        const auto bubble = Around(bubbleCenter, s.Px(48) * phase, s.Px(44) * phase);
        Fill(draw, bubble, s.Px(24), Opacity(t[Role::InverseSurface], phase));
        CenterText(s, draw, {12 * phase, 16 * phase, .5f * phase, true}, bubbleCenter,
                   Opacity(t[Role::InverseOnSurface], phase), buffer);
    };
    thumb(lower, a, first, lowerWidth);
    if (second)
        thumb(upper, b, *second, upperWidth);
    return changed;
}

} // namespace

bool Slider(const Style& s, const char* id, float& value, float min, float max, const SliderOptions& o) {
    return DrawSlider(s, id, value, nullptr, min, max, o);
}

bool RangeSlider(const Style& s, const char* id, float& lower, float& upper, float min, float max, const SliderOptions& o) {
    return DrawSlider(s, id, lower, &upper, min, max, o);
}

} // namespace m3e
