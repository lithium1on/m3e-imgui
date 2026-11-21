#include "components_detail.hpp"
#include "shape_tokens.hpp"
#include <array>

namespace m3e::detail {

Item Row(const Style& s, const char* id, Rect rect, const ListEntry& entry, const RowStyle& style, bool layout) {
    const Item item = Interact(id, rect, entry.disabled, layout);
    if (!item.visible)
        return item;
    auto* draw = ImGui::GetWindowDrawList();
    const float rounding = s.Px(style.rounding);
    const auto ink = item.disabled ? Opacity(s.theme[Role::OnSurface], .38f) : s.theme[style.ink];
    if (style.elevation > 0 && !item.disabled)
        Shadow(s, draw, rect, rounding, static_cast<int>(style.elevation));
    if (style.background != Role::Count)
        Fill(draw, rect, rounding, s.theme[style.background]);
    if (style.border)
        Stroke(draw, rect, rounding, s.theme[Role::Outline], s.Px(1));
    State(s, draw, item, rect, rounding, ink);
    Focus(s, draw, item, rect, rounding, true);
    float left = rect.min.x + s.Px(style.padding);
    float right = rect.max.x - s.Px(style.padding);
    if (entry.leading) {
        Symbol(s, draw, entry.leading, s.Px(24), {left + s.Px(12), rect.Center().y}, ink);
        left += s.Px(40);
    }
    if (entry.trailing) {
        Symbol(s, draw, entry.trailing, s.Px(24), {right - s.Px(12), rect.Center().y}, ink);
        right -= s.Px(36);
    }
    if (left < right) {
        draw->PushClipRect({left, rect.min.y}, {right, rect.max.y}, true);
        Text(s, draw, BodyLarge, {left, rect.Center().y - s.Px(entry.supporting ? 22 : 12)}, ink, Label(entry.label));
        if (entry.supporting)
            Text(s, draw, BodyMedium, {left, rect.Center().y + s.Px(2)},
                 item.disabled ? ink : s.theme[Role::OnSurfaceVariant], Label(entry.supporting));
        draw->PopClipRect();
    }
    return item;
}

bool BeginSurface(const Style& s, const char* id, ImVec2 size, Role background, float rounding,
                  bool border, ImGuiWindowFlags flags, bool padding) {
    const float inset = s.Px(padding ? 16 : 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {inset, inset});
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, s.Px(rounding));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, border ? s.Px(1) : 0);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, background == Role::Count ? 0u : s.theme[background]);
    ImGui::PushStyleColor(ImGuiCol_Border, s.theme[Role::OutlineVariant]);
    ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
    ImGuiChildFlags childFlags = ImGuiChildFlags_AlwaysUseWindowPadding;
    if (border)
        childFlags |= ImGuiChildFlags_Borders;
    return ImGui::BeginChild(id, size, childFlags, flags);
}

void EndSurface() {
    ImGui::EndChild();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
}

} // namespace m3e::detail

namespace m3e {
using namespace detail;

void Divider(const Style& s, float length, bool vertical) {
    if (SkipItems())
        return;
    const auto box = vertical ? At(ImGui::GetCursorScreenPos(), s.Px(1), s.Px(length > 0 ? length : 32))
                              : Slot(s, length, 1);
    Passive(box);
    Fill(ImGui::GetWindowDrawList(), box, 0, s.theme[Role::OutlineVariant]);
}

void Heading(const Style& s, const char* text, Size size) {
    if (SkipItems())
        return;
    constexpr Type types[]{TitleSmall, {20, 28, 0}, HeadlineSmall, HeadlineLarge, {40, 48, 0}};
    const auto type = types[Index(size)];
    const auto label = Label(text);
    const auto box = At(ImGui::GetCursorScreenPos(), TextWidth(s, type, label), s.Px(type.lineHeight));
    Passive(box);
    Text(s, ImGui::GetWindowDrawList(), type, box.min, s.theme[Role::OnSurface], label);
}

void Avatar(const Style& s, const char* initials, float diameter, Icon icon) {
    if (SkipItems() || diameter <= 0)
        return;
    const auto size = s.Px(diameter);
    const auto box = At(ImGui::GetCursorScreenPos(), size, size);
    Passive(box);
    auto* draw = ImGui::GetWindowDrawList();
    Fill(draw, box, size / 2, s.theme[Role::PrimaryContainer]);
    const auto ink = s.theme[Role::OnPrimaryContainer];
    if (icon) {
        Symbol(s, draw, icon, size * .6f, box.Center(), ink);
    } else {
        draw->PushClipRect(box.min, box.max, true);
        CenterText(s, draw, {diameter * .4f, diameter * .55f, 0, true}, box.Center(), ink, Label(initials));
        draw->PopClipRect();
    }
}

void Badge(const Style& s, const char* text) {
    if (SkipItems())
        return;
    const auto label = Label(text);
    const auto size = s.Px(label.empty() ? 6 : 16);
    const auto width = std::max(size, TextWidth(s, LabelMedium, label) + (label.empty() ? 0 : s.Px(8)));
    const auto box = At(ImGui::GetCursorScreenPos(), width, size);
    Passive(box);
    auto* draw = ImGui::GetWindowDrawList();
    Fill(draw, box, size / 2, s.theme[Role::Error]);
    CenterText(s, draw, LabelMedium, box.Center(), s.theme[Role::OnError], label);
}

void Callout(const Style& s, const char* title, const char* message, MessageTone tone, float width) {
    if (SkipItems())
        return;
    const auto box = Slot(s, width, 72);
    Passive(box);
    auto* draw = ImGui::GetWindowDrawList();
    constexpr Role containers[]{Role::SecondaryContainer, Role::PrimaryContainer, Role::TertiaryContainer, Role::ErrorContainer};
    constexpr Role labels[]{Role::OnSecondaryContainer, Role::OnPrimaryContainer, Role::OnTertiaryContainer, Role::OnErrorContainer};
    constexpr Icon symbols[]{icons::Info, icons::Check, 0xe002, 0xe000};
    const auto index = static_cast<unsigned>(tone);
    const auto ink = s.theme[labels[index]];
    Fill(draw, box, s.Px(12), s.theme[containers[index]]);
    draw->PushClipRect(box.min, box.max, true);
    Symbol(s, draw, symbols[index], s.Px(24), {box.min.x + s.Px(28), box.Center().y}, ink);
    Text(s, draw, TitleSmall, {box.min.x + s.Px(52), box.min.y + s.Px(12)}, ink, Label(title));
    Text(s, draw, BodyMedium, {box.min.x + s.Px(52), box.min.y + s.Px(36)}, ink, Label(message));
    draw->PopClipRect();
}

namespace {
constexpr float Pi = 3.14159265358979323846f;

const ShapeToken& ShapeData(ExpressiveShape shape) {
    static_assert(std::size(MaterialShapeTokens) == static_cast<std::size_t>(ExpressiveShape::Count));
    const auto index = static_cast<std::size_t>(shape);
    return MaterialShapeTokens[index < std::size(MaterialShapeTokens) ? index : 0];
}

void DrawShape(ImDrawList* draw, ImVec2 center, float size, ExpressiveShape from, ExpressiveShape to,
               float fraction, float rotation, ImU32 ink) {
    const auto& start = ShapeData(from).points;
    const auto& end = ShapeData(to).points;
    std::array<ImVec2, std::size(MaterialShapeTokens[0].points)> path;
    const float si = std::sin(rotation), co = std::cos(rotation);
    const float unit = size / 10000;
    int count = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        const float x = (Mix(start[i].x, end[i].x, fraction) - 5000) * unit;
        const float y = (Mix(start[i].y, end[i].y, fraction) - 5000) * unit;
        const ImVec2 p{x * co - y * si, x * si + y * co};
        if (!count || std::hypot(p.x - path[count - 1].x, p.y - path[count - 1].y) > .005f)
            path[count++] = p;
    }
    if (count > 1 && std::hypot(path[count - 1].x - path[0].x, path[count - 1].y - path[0].y) <= .005f)
        --count;
    float area = 0;
    for (int i = 0; i < count; ++i) {
        const auto a = path[i], b = path[(i + 1) % count];
        area += a.x * b.y - a.y * b.x;
    }
    if (area < 0)
        std::reverse(path.begin(), path.begin() + count);
    for (int i = 0; i < count; ++i) {
        path[i].x += center.x;
        path[i].y += center.y;
    }
    draw->AddConcavePolyFilled(path.data(), count, ink);
}
} // namespace

const char* ShapeName(ExpressiveShape shape) { return ShapeData(shape).name; }

void ShapePreview(const Style& s, ExpressiveShape shape, float size, Role color) {
    ShapeMorph(s, shape, shape, 0, size, color);
}

void ShapeMorph(const Style& s, ExpressiveShape from, ExpressiveShape to, float fraction, float size, Role color) {
    if (SkipItems() || !std::isfinite(size) || size <= 0 || color == Role::Count)
        return;
    const auto box = At(ImGui::GetCursorScreenPos(), s.Px(size), s.Px(size));
    Passive(box);
    const auto phase = std::isfinite(fraction) ? std::clamp(fraction, 0.0f, 1.0f) : 0;
    DrawShape(ImGui::GetWindowDrawList(), box.Center(), box.W(), from, to, phase, 0, s.theme[color]);
}

void Skeleton(const Style& s, float width, float height, float rounding) {
    if (SkipItems() || height <= 0)
        return;
    const auto box = Slot(s, width, height);
    Passive(box);
    const float pulse = s.reducedMotion ? .5f : .5f + .5f * std::sin(static_cast<float>(ImGui::GetTime()) * 3.2f);
    Fill(ImGui::GetWindowDrawList(), box, s.Px(rounding),
         Blend(s.theme[Role::SurfaceContainerHigh], s.theme[Role::SurfaceContainerHighest], pulse));
}

void LoadingIndicator(const Style& s, std::optional<float> progress, bool contained) {
    if (SkipItems())
        return;
    const auto box = At(ImGui::GetCursorScreenPos(), s.Px(48), s.Px(48));
    Passive(box);
    auto* draw = ImGui::GetWindowDrawList();
    const float time = s.reducedMotion ? 0 : static_cast<float>(ImGui::GetTime());
    if (contained)
        Fill(draw, box, box.W() / 2, s.theme[Role::PrimaryContainer]);
    const auto ink = s.theme[contained ? Role::OnPrimaryContainer : Role::Primary];
    if (progress) {
        const float fraction = std::isfinite(*progress) ? std::clamp(*progress, 0.0f, 1.0f) : 0;
        DrawShape(draw, box.Center(), s.Px(38), ExpressiveShape::Circle, ExpressiveShape::SoftBurst,
                  fraction, -fraction * Pi, ink);
    } else {
        constexpr ExpressiveShape shapes[]{ExpressiveShape::SoftBurst, ExpressiveShape::Cookie9, ExpressiveShape::Pentagon,
            ExpressiveShape::Pill, ExpressiveShape::Sunny, ExpressiveShape::Cookie4, ExpressiveShape::Oval};
        const float t = std::fmod(time / .65f, static_cast<float>(std::size(shapes)));
        const int current = static_cast<int>(t);
        const int next = (current + 1) % std::size(shapes);
        const float fraction = std::clamp(Ease(t - current, FastSpatial(s)), 0.0f, 1.0f);
        const float rotation = Pi / 2 * t + Pi * 2 * std::fmod(time / 4.666f, 1.0f);
        DrawShape(draw, box.Center(), s.Px(38), shapes[current], shapes[next], fraction, rotation, ink);
    }
}

} // namespace m3e
