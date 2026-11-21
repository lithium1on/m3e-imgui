#include "components_detail.hpp"
#include <cstdio>

namespace m3e {
using namespace detail;
namespace {

constexpr Icon Back = 0xe5cb, Expand = 0xe5cf, Collapse = 0xe5ce;

const RowStyle PlainRow{Role::Count, Role::OnSurface, 0};
const RowStyle ActiveRow{Role::SecondaryContainer, Role::OnSecondaryContainer, 28};

int NavigationList(const Style& s, const char* id, const Choice* choices, int count, int& selected,
                   float width, bool labelsOnly) {
    if (!choices || SkipItems())
        return -1;
    const float x = ImGui::GetCursorScreenPos().x;
    const float logicalWidth = ContentWidth(s, width) / s.scale;
    ImGui::PushID(id);
    int action = -1;
    for (int i = 0; i < count; ++i) {
        ImGui::SetCursorScreenPos({x, ImGui::GetCursorScreenPos().y});
        ImGui::PushID(i);
        const auto& choice = choices[i];
        const auto row = Row(s, "destination", Slot(s, logicalWidth, labelsOnly ? 40 : 56),
                             {choice.label, nullptr, labelsOnly ? 0u : choice.icon, 0, choice.disabled},
                             selected == i ? ActiveRow : RowStyle{Role::Count, Role::OnSurfaceVariant, 28});
        if (row.pressed)
            selected = action = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return action;
}
} // namespace

bool BeginCard(const Style& s, const char* id, CardVariant variant, float width, float height) {
    const ImVec2 size{ContentWidth(s, width), height > 0 ? s.Px(height) : s.Px(120)};
    if (variant == CardVariant::Elevated && !SkipItems())
        Shadow(s, ImGui::GetWindowDrawList(), At(ImGui::GetCursorScreenPos(), size.x, size.y), s.Px(12), 1);
    const auto color = variant == CardVariant::Filled ? Role::SurfaceContainerHighest
                     : variant == CardVariant::Elevated ? Role::SurfaceContainerLow : Role::Surface;
    return BeginSurface(s, id, size, color, 12, variant == CardVariant::Outlined);
}
void EndCard() { EndSurface(); }

bool BeginContentPane(const Style& s, const char* id, float width, float height) {
    return BeginSurface(s, id, {ContentWidth(s, width), height > 0 ? s.Px(height) : 0.0f}, Role::SurfaceContainerLow, 24);
}
void EndContentPane() { EndSurface(); }

bool ExpansionPanel(const Style& s, const char* label, bool& expanded, Icon icon, float width) {
    if (SkipItems())
        return expanded;
    const auto state = expanded ? Collapse : Expand;
    if (Row(s, label, Slot(s, width, 56), {label, nullptr, icon, state}, {Role::SurfaceContainerLow}).pressed)
        expanded = !expanded;
    return expanded;
}

bool ListItem(const Style& s, const char* id, const ListEntry& entry, bool selected, float width) {
    if (SkipItems())
        return false;
    const float height = entry.supporting ? 72 : 56;
    return Row(s, id, Slot(s, width, height + Density(s)), entry, selected ? ActiveRow : PlainRow).pressed;
}

int List(const Style& s, const char* id, const ListEntry* entries, int count, int& selected, float width, bool segmented) {
    if (!entries || SkipItems())
        return -1;
    int action = -1;
    const float x = ImGui::GetCursorScreenPos().x;
    const float logicalWidth = ContentWidth(s, width) / s.scale;
    ImGui::PushID(id);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {ImGui::GetStyle().ItemSpacing.x, segmented ? s.Px(2) : 0.0f});
    for (int i = 0; i < count; ++i) {
        ImGui::SetCursorScreenPos({x, ImGui::GetCursorScreenPos().y});
        const auto& entry = entries[i];
        ImGui::PushID(i);
        const auto height = (entry.supporting ? 72.0f : 56.0f) + Density(s);
        const auto row = Row(s, "item", Slot(s, logicalWidth, height), entry, i == selected ? ActiveRow
                          : segmented ? RowStyle{Role::SurfaceContainerLow, Role::OnSurface, 8} : PlainRow);
        if (row.pressed)
            selected = action = i;
        ImGui::PopID();
    }
    ImGui::PopStyleVar();
    ImGui::PopID();
    return action;
}

int NavigationMenu(const Style& s, const char* id, const Choice* choices, int count, int& selected, float width) {
    return NavigationList(s, id, choices, count, selected, width, false);
}

int NavigationRail(const Style& s, const char* id, const Choice* choices, int count, int& selected, bool expanded) {
    if (!choices || SkipItems())
        return -1;
    if (expanded)
        return NavigationMenu(s, id, choices, count, selected, 220);
    const float x = ImGui::GetCursorScreenPos().x;
    int action = -1;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        ImGui::SetCursorScreenPos({x, ImGui::GetCursorScreenPos().y});
        ImGui::PushID(i);
        const auto box = Slot(s, 80, 64);
        const auto item = Interact("destination", box, choices[i].disabled);
        if (item.pressed)
            selected = action = i;
        if (item.visible) {
            auto* draw = ImGui::GetWindowDrawList();
            const auto pill = Around({box.Center().x, box.min.y + s.Px(21)}, s.Px(56), s.Px(32));
            const auto color = s.theme[i == selected ? Role::OnSecondaryContainer : Role::OnSurfaceVariant];
            const auto ink = item.disabled ? Opacity(color, .38f) : color;
            if (selected == i)
                Fill(draw, pill, s.Px(16), s.theme[Role::SecondaryContainer]);
            State(s, draw, item, pill, s.Px(16), ink, true, true);
            Focus(s, draw, item, pill, s.Px(16), true);
            Symbol(s, draw, choices[i].icon, s.Px(24), pill.Center(), ink, selected == i);
            draw->PushClipRect(box.min, box.max, true);
            CenterText(s, draw, LabelMedium, {box.Center().x, box.min.y + s.Px(47)}, ink, Label(choices[i].label));
            draw->PopClipRect();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    return action;
}

int Breadcrumbs(const Style& s, const char* id, const Choice* entries, int count) {
    if (!entries || SkipItems())
        return -1;
    int activated = -1;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        if (i) {
            ImGui::SameLine(0, 0);
            const auto box = Slot(s, 24, 40);
            Passive(box);
            Symbol(s, ImGui::GetWindowDrawList(), icons::ChevronRight, s.Px(18), box.Center(), s.theme[Role::OnSurfaceVariant]);
            ImGui::SameLine(0, 0);
        }
        ImGui::PushID(i);
        if (Button(s, entries[i].label, {Variant::Text, Size::ExtraSmall, Shape::Round, entries[i].icon, 0, entries[i].disabled}))
            activated = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return activated;
}

int TableOfContents(const Style& s, const char* id, const Choice* entries, int count, int& selected, float width) {
    return NavigationList(s, id, entries, count, selected, width, true);
}

bool Paginator(const Style& s, const char* id, int& page, int count) {
    if (count <= 0 || SkipItems())
        return false;
    const int old = page;
    page = std::clamp(page, 0, count - 1);
    ImGui::PushID(id);
    ButtonOptions arrow;
    arrow.variant = Variant::Text;
    arrow.disabled = page == 0;
    if (IconButton(s, "Previous page", Back, arrow))
        --page;
    ImGui::SameLine();
    char caption[40];
    std::snprintf(caption, sizeof(caption), "Page %d of %d", page + 1, count);
    const auto box = Slot(s, 116, 48);
    Passive(box);
    CenterText(s, ImGui::GetWindowDrawList(), LabelLarge, box.Center(), s.theme[Role::OnSurface], caption);
    ImGui::SameLine();
    arrow.disabled = page == count - 1;
    if (IconButton(s, "Next page", icons::ChevronRight, arrow))
        ++page;
    ImGui::PopID();
    return page != old;
}

bool Stepper(const Style& s, const char* id, const Choice* entries, int count, int& step) {
    if (!entries || SkipItems())
        return false;
    const int old = step;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        if (i)
            ImGui::SameLine(0, s.Px(4));
        ImGui::PushID(i);
        char number[16];
        std::snprintf(number, sizeof(number), "%d", i + 1);
        std::string label = number;
        label += " ";
        label += Label(entries[i].label);
        ButtonOptions option;
        option.size = Size::Small;
        option.variant = i == step ? Variant::Filled : Variant::Outlined;
        option.icon = i < step ? icons::Check : entries[i].icon;
        option.disabled = entries[i].disabled;
        if (Button(s, label.c_str(), option))
            step = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return old != step;
}

int Tree(const Style& s, const char* id, const TreeEntry* entries, int count, int& selected, bool* expanded, float width) {
    if (!entries || !expanded || SkipItems())
        return -1;
    ImGui::PushID(id);
    int activated = -1;
    const float x = ImGui::GetCursorScreenPos().x;
    const float fullWidth = ContentWidth(s, width);
    int collapsedDepth = -1;
    for (int i = 0; i < count; ++i) {
        const auto& node = entries[i];
        const int depth = std::max(0, node.depth);
        if (collapsedDepth >= 0 && depth > collapsedDepth)
            continue;
        const bool children = i + 1 < count && entries[i + 1].depth > depth;
        const ImVec2 start{x, ImGui::GetCursorScreenPos().y};
        ImGui::SetCursorScreenPos(start);
        const float inset = std::min(s.Px(16.0f * depth), fullWidth / 2);
        const auto whole = At(start, fullWidth, s.Px(42));
        Passive(whole);
        const auto disclosure = At({start.x + inset, start.y}, std::min(s.Px(32), fullWidth - inset), whole.H());
        ImGui::PushID(i);
        if (children) {
            const auto toggle = Interact("expand", disclosure, node.disabled, false);
            if (toggle.pressed)
                expanded[i] = !expanded[i];
            if (toggle.visible) {
                auto* draw = ImGui::GetWindowDrawList();
                const auto ink = s.theme[Role::OnSurfaceVariant];
                State(s, draw, toggle, disclosure, s.Px(12), ink);
                Focus(s, draw, toggle, disclosure, s.Px(12), true);
                Symbol(s, draw, expanded[i] ? Expand : icons::ChevronRight, s.Px(18), disclosure.Center(), ink);
            }
        }
        const auto row = Rect{{disclosure.max.x, start.y}, whole.max};
        if (row.W() > 0 && Row(s, "node", row, {node.label, nullptr, node.icon, 0, node.disabled},
                               selected == i ? ActiveRow : PlainRow, false).pressed)
            selected = activated = i;
        ImGui::PopID();
        collapsedDepth = children && !expanded[i] ? depth : -1;
    }
    ImGui::PopID();
    return activated;
}

int SlideGroup(const Style& s, const char* id, const Choice* choices, int count, int& selected, float width) {
    if (!choices || count <= 0 || SkipItems())
        return -1;
    const auto box = Slot(s, width, 48);
    const float arrow = std::min(s.Px(40), box.W() / 4);
    int result = -1;
    ImGui::PushID(id);
    Passive(box);
    auto* store = ImGui::GetStateStorage();
    const auto leftKey = ImGui::GetID("has previous"), rightKey = ImGui::GetID("has next");
    auto control = [&](const char* label, float x, Icon icon, bool enabled) {
        const auto hit = At({x, box.min.y}, arrow, box.H());
        const auto item = Interact(label, hit, !enabled, false);
        auto* draw = ImGui::GetWindowDrawList();
        const auto ink = s.theme[Role::OnSurfaceVariant];
        State(s, draw, item, hit, arrow / 2, ink, true, true);
        Focus(s, draw, item, hit, arrow / 2, true);
        if (item.visible)
            Symbol(s, draw, icon, s.Px(20), hit.Center(), enabled ? ink : Opacity(ink, .38f));
        return item.pressed;
    };
    const int offset = static_cast<int>(control("Next items", box.max.x - arrow, icons::ChevronRight, store->GetBool(rightKey)))
                     - static_cast<int>(control("Previous items", box.min.x, Back, store->GetBool(leftKey)));
    ImGui::SetCursorScreenPos({box.min.x + arrow, box.min.y});
    if (BeginSurface(s, "options", {box.W() - 2 * arrow, box.H()}, Role::Count, 0, false,
                     ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollbar, false)) {
        const float scroll = ImGui::GetScrollX(), maxScroll = ImGui::GetScrollMaxX();
        store->SetBool(leftKey, scroll > 0);
        store->SetBool(rightKey, scroll < maxScroll);
        if (offset)
            ImGui::SetScrollX(std::clamp(scroll + offset * s.Px(112), 0.0f, maxScroll));
        for (int i = 0; i < count; ++i) {
            if (i)
                ImGui::SameLine();
            ImGui::PushID(i);
            const auto event = Chip(s, choices[i].label, {ChipKind::Filter, choices[i].icon, i == selected, false, choices[i].disabled});
            if (event == ChipAction::Activate)
                selected = result = i;
            ImGui::PopID();
        }
    }
    EndSurface();
    ImGui::PopID();
    return result;
}

bool BeginCarousel(const Style& s, const char* id, int& active, int count, const CarouselOptions& o) {
    if (count > 0)
        active = std::clamp(active, 0, count - 1);
    const float width = ContentWidth(s, o.width);
    const bool visible = BeginSurface(s, id, {width, s.Px(std::max(56.0f, o.height))}, Role::SurfaceContainerLow, 20);
    if (!visible || count <= 0)
        return false;
    ButtonOptions arrow;
    arrow.variant = Variant::Tonal;
    arrow.size = Size::ExtraSmall;
    arrow.disabled = count == 1 || (!o.loop && active == 0);
    if (IconButton(s, "Previous slide", Back, arrow))
        active = active == 0 ? count - 1 : active - 1;
    ImGui::SameLine();
    char caption[36];
    std::snprintf(caption, sizeof(caption), "%d / %d", active + 1, count);
    const auto label = Slot(s, 64, 40);
    Passive(label);
    CenterText(s, ImGui::GetWindowDrawList(), LabelMedium, label.Center(), s.theme[Role::OnSurface], caption);
    ImGui::SameLine();
    arrow.disabled = count == 1 || (!o.loop && active == count - 1);
    if (IconButton(s, "Next slide", icons::ChevronRight, arrow))
        active = active == count - 1 ? 0 : active + 1;
    if (ImGui::IsWindowHovered()) {
        auto* storage = ImGui::GetStateStorage();
        const auto dragging = ImGui::GetID("swiping");
        const auto offset = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        const auto p = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            const auto pos = ImGui::GetIO().MousePos;
            storage->SetBool(dragging, pos.y > p.y + s.Px(56) && pos.y < p.y + size.y);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && storage->GetBool(dragging)) {
            const int step = offset > s.Px(42) ? -1 : offset < -s.Px(42) ? 1 : 0;
            const int next = active + step;
            active = o.loop ? (next < 0 ? count - 1 : next == count ? 0 : next) : std::clamp(next, 0, count - 1);
            storage->SetBool(dragging, false);
        }
    }
    return true;
}
void EndCarousel() { EndSurface(); }

void BeginSplitPane(const Style& s, const char* id, float& fraction, const SplitPaneOptions& o) {
    const float width = ContentWidth(s, o.width);
    const float height = s.Px(std::max(16.0f, o.height));
    BeginSurface(s, id, {width, height}, Role::Count, 0, false, ImGuiWindowFlags_NoScrollbar, false);
    const float axis = o.vertical ? height : width;
    const float gutter = std::min(s.Px(12), axis / 3);
    const float travel = axis - gutter;
    const float min = std::clamp(s.Px(o.minimum) / travel, 0.0f, .5f);
    fraction = std::clamp(std::isfinite(fraction) ? fraction : .5f, min, 1 - min);
    const auto size = o.vertical ? ImVec2{width, travel * fraction} : ImVec2{travel * fraction, height};
    auto* storage = ImGui::GetStateStorage();
    storage->SetFloat(ImGui::GetID("width"), width);
    storage->SetFloat(ImGui::GetID("height"), height);
    storage->SetFloat(ImGui::GetID("gutter"), gutter);
    BeginSurface(s, "first", size, Role::SurfaceContainerLow);
}

void NextSplitPane(const Style& s, float& fraction, const SplitPaneOptions& o) {
    EndSurface();
    auto* storage = ImGui::GetStateStorage();
    const float width = storage->GetFloat(ImGui::GetID("width"));
    const float height = storage->GetFloat(ImGui::GetID("height"));
    const float gutter = storage->GetFloat(ImGui::GetID("gutter"));
    const float travel = (o.vertical ? height : width) - gutter;
    const auto rect = ImGui::GetItemRectMax();
    const auto start = ImGui::GetItemRectMin();
    const auto handle = o.vertical ? At({start.x, rect.y}, width, gutter) : At({rect.x, start.y}, gutter, height);
    const auto item = Interact("Resize panes", handle, false, false, true);
    auto* draw = ImGui::GetWindowDrawList();
    if (item.visible) {
        const auto grab = Around(handle.Center(), o.vertical ? s.Px(28) : s.Px(4), o.vertical ? s.Px(4) : s.Px(28));
        Fill(draw, grab, s.Px(2), s.theme[item.held || item.hovered ? Role::Primary : Role::Outline]);
        Focus(s, draw, item, handle, s.Px(6), true);
    }
    const float old = fraction;
    if (item.held && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        const auto delta = ImGui::GetIO().MouseDelta;
        fraction += (o.vertical ? delta.y : delta.x) / travel;
    }
    float direction = 0;
    bool boundary = false;
    if (item.focused && NavStep(o.vertical, direction, boundary))
        fraction += direction * (boundary ? 1 : .05f);
    const float min = std::clamp(s.Px(o.minimum) / travel, 0.0f, .5f);
    fraction = std::clamp(fraction, min, 1 - min);
    const auto after = o.vertical ? ImVec2{start.x, rect.y + gutter} : ImVec2{rect.x + gutter, start.y};
    ImGui::SetCursorScreenPos(after);
    const auto size = o.vertical ? ImVec2{width, travel * (1 - old)} : ImVec2{travel * (1 - old), height};
    BeginSurface(s, "second", size, Role::SurfaceContainer);
}

void EndSplitPane() {
    EndSurface();
    EndSurface();
}

bool BeginDrawerLayout(const Style& s, const char* id, bool open, float drawerWidth, float height) {
    BeginSurface(s, id, {0, s.Px(std::max(48.0f, height))}, Role::Count, 0, false, ImGuiWindowFlags_NoScrollbar, false);
    auto* storage = ImGui::GetStateStorage();
    storage->SetBool(ImGui::GetID("drawer open"), open);
    const auto p = ImGui::GetCursorScreenPos();
    storage->SetFloat(ImGui::GetID("drawer x"), p.x);
    storage->SetFloat(ImGui::GetID("drawer y"), p.y);
    const float width = std::clamp(s.Px(drawerWidth), s.Px(1), std::max(s.Px(1), ImGui::GetContentRegionAvail().x * .7f));
    const bool visible = BeginSurface(s, "drawer", open ? ImVec2{width, 0} : ImVec2{1, 1},
                                      open ? Role::SurfaceContainerLow : Role::Count, 16);
    return open && visible;
}

void NextDrawerPane(const Style& s) {
    EndSurface();
    auto* storage = ImGui::GetStateStorage();
    if (storage->GetBool(ImGui::GetID("drawer open")))
        ImGui::SameLine(0, s.Px(8));
    else
        ImGui::SetCursorScreenPos({storage->GetFloat(ImGui::GetID("drawer x")), storage->GetFloat(ImGui::GetID("drawer y"))});
    BeginSurface(s, "body", {}, Role::SurfaceContainer, 16);
}

void EndDrawerLayout() {
    EndSurface();
    EndSurface();
}

} // namespace m3e
