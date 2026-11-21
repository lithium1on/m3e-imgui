// Material 3 Expressive layout dimensions adapted from AndroidX Compose.
// See licenses/AndroidX-Apache-2.0.txt.
#include "components_detail.hpp"
#include <m3e/android.hpp>

namespace m3e::android {
using namespace detail;

int CollapsingTopAppBar(const Style& s, const char* id, const char* title, const Choice* choices, int count,
                        const CollapsingAppBarOptions& o) {
    if (SkipItems())
        return -1;
    const float collapsed = std::isfinite(o.collapse) ? std::clamp(o.collapse, 0.0f, 1.0f) : 0;
    const auto expandedHeight = o.size == AppBarSize::Large ? 152 : o.size == AppBarSize::Medium ? 112 : 64;
    const float height = Mix(static_cast<float>(expandedHeight), 64, collapsed);
    const auto box = Slot(s, o.width, height);
    auto* draw = ImGui::GetWindowDrawList();
    const auto surface = Blend(s.theme[Role::Surface], s.theme[Role::SurfaceContainer], collapsed);
    Fill(draw, box, 0, surface);
    Passive(box);
    ImGui::PushID(id);
    const float small = o.size == AppBarSize::Small ? 1 : collapsed;
    const Type type{Mix(o.size == AppBarSize::Large ? 32.0f : 28.0f, 22, small),
                    Mix(o.size == AppBarSize::Large ? 40.0f : 36.0f, 28, small), 0};
    const float centerY = Mix(box.max.y - s.Px(34), box.min.y + s.Px(32), small);
    const float inset = s.Px(16);
    const float actionWidth = s.Px(48);
    int activated = -1;
    float actionX = box.max.x - s.Px(4);
    for (int i = count - 1; choices && i >= 0 && actionX - actionWidth >= box.min.x; --i) {
        actionX -= actionWidth;
        const auto target = At({actionX, box.min.y + s.Px(8)}, actionWidth, actionWidth);
        ImGui::PushID(i);
        const auto item = Interact(choices[i].label, target, choices[i].disabled, false);
        const auto primary = s.theme[Role::OnSurfaceVariant];
        const auto ink = item.disabled ? Opacity(primary, .38f) : primary;
        if (item.visible) {
            State(s, draw, item, target, actionWidth / 2, ink);
            Focus(s, draw, item, target, actionWidth / 2, true);
            Symbol(s, draw, choices[i].icon, s.Px(24), target.Center(), ink);
        }
        if (item.pressed)
            activated = i;
        ImGui::PopID();
    }
    const auto label = Label(title);
    const float right = Mix(box.max.x - inset, actionX, small);
    if (right > box.min.x + inset) {
        draw->PushClipRect({box.min.x + inset, box.min.y}, {right, box.max.y}, true);
        const float start = o.centered ? std::max(box.min.x + inset, box.Center().x - TextWidth(s, type, label) / 2) : box.min.x + inset;
        Text(s, draw, type, {start, centerY - s.Px(type.lineHeight / 2)}, s.theme[Role::OnSurface], label);
        draw->PopClipRect();
    }
    ImGui::PopID();
    return activated;
}

int BottomAppBar(const Style& s, const char* id, const Choice* choices, int count, float width) {
    if (SkipItems() || !choices)
        return -1;
    const auto box = Slot(s, width, 80);
    Passive(box);
    auto* draw = ImGui::GetWindowDrawList();
    Fill(draw, box, 0, s.theme[Role::SurfaceContainer]);
    const auto ink = s.theme[Role::OnSurface];
    int activated = -1;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        const float left = box.min.x + s.Px(4 + 48.0f * i);
        if (left + s.Px(48) > box.max.x)
            break;
        const auto hit = At({left, box.min.y + s.Px(16)}, s.Px(48), s.Px(48));
        ImGui::PushID(i);
        const auto item = Interact(choices[i].label, hit, choices[i].disabled, false);
        if (item.visible) {
            State(s, draw, item, hit, s.Px(24), ink);
            Focus(s, draw, item, hit, s.Px(24), true);
            Symbol(s, draw, choices[i].icon, s.Px(24), hit.Center(), item.disabled ? Opacity(ink, .38f) : ink);
        }
        if (item.pressed)
            activated = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return activated;
}

int NavigationBar(const Style& s, const char* id, const Choice* choices, int count, int& selected, float width) {
    if (!choices || count <= 0 || SkipItems())
        return -1;
    const auto box = Slot(s, width, 80);
    const auto share = box.W() / count;
    Passive(box);
    auto* draw = ImGui::GetWindowDrawList();
    Fill(draw, box, 0, s.theme[Role::SurfaceContainer]);
    int activated = -1;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        ImGui::PushID(i);
        const auto hit = At({box.min.x + share * i, box.min.y}, share, box.H());
        const auto item = Interact(choices[i].label, hit, choices[i].disabled, false);
        if (item.pressed)
            selected = activated = i;
        if (item.visible) {
            const bool current = selected == i;
            const auto pill = Around({hit.Center().x, hit.min.y + s.Px(28)}, std::min(share, s.Px(64)), s.Px(32));
            const auto ink = s.theme[current ? Role::OnSecondaryContainer : Role::OnSurfaceVariant];
            const auto glyph = item.disabled ? Opacity(ink, .38f) : ink;
            if (current)
                Fill(draw, pill, s.Px(16), s.theme[Role::SecondaryContainer]);
            State(s, draw, item, pill, s.Px(16), glyph, true, true);
            Focus(s, draw, item, hit, s.Px(16), true);
            draw->PushClipRect(hit.min, hit.max, true);
            Symbol(s, draw, choices[i].icon, s.Px(24), pill.Center(), glyph, current);
            CenterText(s, draw, LabelMedium, {hit.Center().x, hit.min.y + s.Px(57)}, glyph, Label(choices[i].label));
            draw->PopClipRect();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    return activated;
}

bool SwipeToDismiss(const Style& s, const char* id, const ListEntry& entry, float width) {
    if (SkipItems())
        return false;
    const auto box = Slot(s, width, entry.supporting ? 72 : 56);
    const auto item = Interact(id, box, entry.disabled, true, true);
    if (!item.visible)
        return false;
    auto* store = ImGui::GetStateStorage();
    const auto key = StateKey(item.id, "swiping");
    if (item.held && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        store->SetBool(key, true);
    const bool tracking = store->GetBool(key);
    const float offset = tracking && !item.disabled ? std::clamp(ImGui::GetMouseDragDelta().x, -box.W(), box.W()) : 0;
    const bool released = tracking && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        store->SetBool(key, false);
    const bool dismiss = released && std::abs(offset) >= box.W() * .45f;
    const bool keyDismiss = item.focused && !item.disabled && ImGui::IsKeyPressed(ImGuiKey_Delete);
    auto* draw = ImGui::GetWindowDrawList();
    auto shifted = box;
    shifted.min.x += offset;
    shifted.max.x += offset;
    draw->PushClipRect(box.min, box.max, true);
    if (std::abs(offset) > s.Px(1)) {
        Fill(draw, box, s.Px(12), s.theme[Role::ErrorContainer]);
        const auto x = offset > 0 ? box.min.x + s.Px(24) : box.max.x - s.Px(24);
        Symbol(s, draw, icons::Delete, s.Px(24), {x, box.Center().y}, s.theme[Role::OnErrorContainer]);
    }
    Fill(draw, shifted, s.Px(12), s.theme[Role::SurfaceContainerLow]);
    const auto ink = item.disabled ? Opacity(s.theme[Role::OnSurface], .38f) : s.theme[Role::OnSurface];
    float x = shifted.min.x + s.Px(16);
    if (entry.leading) {
        Symbol(s, draw, entry.leading, s.Px(24), {x + s.Px(12), shifted.Center().y}, ink);
        x += s.Px(40);
    }
    Text(s, draw, BodyLarge, {x, shifted.Center().y - s.Px(entry.supporting ? 22 : 12)}, ink, Label(entry.label));
    if (entry.supporting)
        Text(s, draw, BodyMedium, {x, shifted.Center().y + s.Px(2)}, ink, Label(entry.supporting));
    Focus(s, draw, item, shifted, s.Px(12), true);
    draw->PopClipRect();
    return dismiss || keyDismiss;
}

bool PullToRefresh(const Style& s, const char* id, float distance, bool released, bool refreshing, float width) {
    if (SkipItems())
        return false;
    const float fraction = std::isfinite(distance) ? std::clamp(distance / 80, 0.0f, 1.0f) : 0;
    if (refreshing || fraction > 0) {
        const auto box = Slot(s, width, 48);
        const auto target = Around(box.Center(), s.Px(48), s.Px(48));
        ImGui::SetCursorScreenPos(target.min);
        ImGui::PushID(id);
        LoadingIndicator(s, refreshing ? std::nullopt : std::optional<float>(fraction), true);
        ImGui::PopID();
        ReserveLayout(box);
    }
    return !refreshing && released && fraction >= 1;
}

bool BeginScaffold(const Style& s, const char* id, float topHeight, float bottomHeight, float width, float height) {
    const float available = height > 0 ? s.Px(height) : ImGui::GetContentRegionAvail().y;
    const float total = std::max(s.Px(48), available);
    const float top = std::clamp(s.Px(topHeight), s.Px(1), total - s.Px(2));
    const float bottom = std::clamp(s.Px(bottomHeight), s.Px(1), total - top - s.Px(1));
    BeginSurface(s, id, {ContentWidth(s, width), total}, Role::Surface, 0, false, ImGuiWindowFlags_NoScrollbar, false);
    auto* state = ImGui::GetStateStorage();
    state->SetFloat(ImGui::GetID("content height"), total - top - bottom);
    state->SetFloat(ImGui::GetID("bottom height"), bottom);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
    return BeginSurface(s, "top", {0, top}, Role::Count, 0, false, ImGuiWindowFlags_NoScrollbar, false);
}

bool NextScaffoldContent(const Style& s) {
    EndSurface();
    const auto height = ImGui::GetStateStorage()->GetFloat(ImGui::GetID("content height"));
    return BeginSurface(s, "content", {0, height}, Role::Count, 0);
}

bool NextScaffoldBottom(const Style& s) {
    EndSurface();
    const auto height = ImGui::GetStateStorage()->GetFloat(ImGui::GetID("bottom height"));
    return BeginSurface(s, "bottom", {0, height}, Role::Count, 0, false, ImGuiWindowFlags_NoScrollbar, false);
}

void EndScaffold() {
    EndSurface();
    ImGui::PopStyleVar();
    EndSurface();
}

} // namespace m3e::android
