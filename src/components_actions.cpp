#include "components_detail.hpp"
#include <m3e/android.hpp>
#include <cstdio>
#include <limits>

namespace m3e {
using namespace detail;
namespace {

constexpr ImGuiWindowFlags OverlayFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                                      | ImGuiWindowFlags_NoSavedSettings;

void OverlayStyle(const Style& s, Role background, Role ink, float rounding, ImVec2 padding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, s.Px(rounding));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, s.theme[background]);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, s.theme[background]);
    ImGui::PushStyleColor(ImGuiCol_Text, s.theme[ink]);
}

void EndOverlayStyle() {
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
}

int ActionItems(const Style& s, const Choice* actions, int count) {
    int result = -1;
    for (int i = 0; actions && i < count; ++i) {
        ImGui::PushID(i);
        MenuItemOptions item;
        item.icon = actions[i].icon;
        item.disabled = actions[i].disabled;
        if (MenuItem(s, actions[i].label, item))
            result = i;
        ImGui::PopID();
    }
    return result;
}
} // namespace

bool BeginBottomSheet(const Style& s, const char* id, bool& open, const SheetOptions& o) {
    const bool modal = o.mode == SheetMode::Modal;
    ImGui::PushID(id);
    const auto key = ImGui::GetID("sheet");
    if (!open && (!modal || !ImGui::IsPopupOpen("##sheet"))) {
        Animate(s, key, "show", 0, .24f, Standard);
        ImGui::PopID();
        return false;
    }
    const auto* view = ImGui::GetMainViewport();
    const auto w = std::min(view->WorkSize.x, s.Px(o.width > 0 ? o.width : 640));
    const auto h = std::min(view->WorkSize.y, s.Px(std::max(o.height, 64.0f)));
    if (modal && open && !ImGui::IsPopupOpen("##sheet")) {
        Animate(s, key, "show", 0, .24f, Standard);
        ImGui::OpenPopup("##sheet");
    }
    const float phase = Animate(s, key, "show", 1, .24f, Standard);
    ImGui::SetNextWindowPos({view->GetWorkCenter().x, view->WorkPos.y + view->WorkSize.y + h * (1 - phase)},
                            ImGuiCond_Always, {.5f, 1});
    ImGui::SetNextWindowSize({w, h});
    OverlayStyle(s, Role::SurfaceContainerLow, Role::OnSurface, 28, {s.Px(24), s.Px(o.dragHandle ? 4 : 24)});
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, Opacity(s.theme[Role::Scrim], .32f * phase));
    bool visible;
    if (modal) {
        visible = ImGui::BeginPopupModal("##sheet", nullptr, OverlayFlags);
    } else {
        char windowId[40];
        std::snprintf(windowId, sizeof(windowId), "##m3eSheet%08x", key);
        visible = ImGui::Begin(windowId, nullptr, OverlayFlags | ImGuiWindowFlags_NoFocusOnAppearing);
    }
    if (visible) {
        if (o.dragHandle) {
            const auto hit = Slot(s, 0, 32);
            auto* storage = ImGui::GetStateStorage();
            const auto drag = ImGui::GetID("sheet drag");
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                storage->SetBool(drag, ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(hit.min, hit.max));
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && storage->GetBool(drag)) {
                open &= ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y <= s.Px(48);
                storage->SetBool(drag, false);
            }
            Passive(hit);
            const auto grip = Around(hit.Center(), s.Px(32), s.Px(4));
            Fill(ImGui::GetWindowDrawList(), grip, s.Px(2), s.theme[Role::OnSurfaceVariant]);
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && EscapePressed())
            open = false;
        if (modal && ClickedOutsideCurrentPopup() && !ImGui::IsWindowAppearing())
            open = false;
        if (open)
            return true;
        if (modal)
            ImGui::CloseCurrentPopup();
        EndBottomSheet(o);
        return false;
    }
    if (!modal)
        ImGui::End();
    ImGui::PopStyleColor();
    EndOverlayStyle();
    ImGui::PopID();
    return false;
}

void EndBottomSheet(const SheetOptions& o) {
    if (o.mode == SheetMode::Modal)
        ImGui::EndPopup();
    else
        ImGui::End();
    ImGui::PopStyleColor();
    EndOverlayStyle();
    ImGui::PopID();
}

bool BeginModalNavigationDrawer(const Style& s, const char* id, bool& open, float width) {
    ImGui::PushID(id);
    constexpr const char* popup = "##drawer";
    const auto key = ImGui::GetID(popup);
    if (!ImGui::IsPopupOpen(popup)) {
        Animate(s, key, "show", 0, .24f, Standard);
        if (!open) {
            ImGui::PopID();
            return false;
        }
        ImGui::OpenPopup(popup);
    }
    const auto* view = ImGui::GetMainViewport();
    const float w = std::min(view->WorkSize.x, s.Px(std::max(120.0f, width)));
    const float phase = Animate(s, key, "show", 1, .24f, Standard);
    ImGui::SetNextWindowPos({view->WorkPos.x - w * (1 - phase), view->WorkPos.y});
    ImGui::SetNextWindowSize({w, view->WorkSize.y});
    OverlayStyle(s, Role::SurfaceContainerLow, Role::OnSurface, 16, {s.Px(12), s.Px(20)});
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, Opacity(s.theme[Role::Scrim], .32f * phase));
    if (ImGui::BeginPopupModal(popup, nullptr, OverlayFlags)) {
        if (EscapePressed() || (ClickedOutsideCurrentPopup() && !ImGui::IsWindowAppearing()))
            open = false;
        if (open)
            return true;
        ImGui::CloseCurrentPopup();
        EndModalNavigationDrawer();
        return false;
    }
    ImGui::PopStyleColor();
    EndOverlayStyle();
    ImGui::PopID();
    return false;
}

void EndModalNavigationDrawer() {
    ImGui::EndPopup();
    ImGui::PopStyleColor();
    EndOverlayStyle();
    ImGui::PopID();
}

void ShowSnackbar(SnackbarState& state, double duration) {
    state = {duration > 0 ? ImGui::GetTime() + duration : std::numeric_limits<double>::infinity(), true};
}

SnackbarResult Snackbar(const Style& s, const char* id, SnackbarState& state, const char* message,
                        const char* action, bool closeButton, float width) {
    if (!state.open)
        return SnackbarResult::None;
    if (ImGui::GetTime() >= state.expires) {
        state.open = false;
        return SnackbarResult::Dismissed;
    }
    const auto* view = ImGui::GetMainViewport();
    const float available = std::max(s.Px(1), view->WorkSize.x - s.Px(32));
    const float w = std::min(available, s.Px(std::max(width, 160.0f)));
    const float margin = s.Px(16);
    const float h = s.Px(48);
    char windowId[40];
    std::snprintf(windowId, sizeof(windowId), "##m3eSnackbar%08x", ImGui::GetID(id));
    ImGui::SetNextWindowPos({view->GetWorkCenter().x, view->WorkPos.y + view->WorkSize.y - margin}, ImGuiCond_Always, {.5f, 1});
    ImGui::SetNextWindowSize({w, h});
    OverlayStyle(s, Role::InverseSurface, Role::InverseOnSurface, 4, {0, 0});
    SnackbarResult result = SnackbarResult::None;
    if (ImGui::Begin(windowId, nullptr, OverlayFlags | ImGuiWindowFlags_NoFocusOnAppearing)) {
        const auto rect = At(ImGui::GetCursorScreenPos(), w, h);
        auto* draw = ImGui::GetWindowDrawList();
        const auto ink = s.theme[Role::InverseOnSurface];
        float right = rect.max.x;
        auto control = [&](const char* label, float size, Icon icon, ImU32 color, SnackbarResult event) {
            right -= size;
            const auto box = At({right, rect.min.y}, size, rect.H());
            const auto item = Interact(label, box, false, false);
            State(s, draw, item, box, s.Px(4), color);
            Focus(s, draw, item, box, s.Px(4), true);
            if (icon)
                Symbol(s, draw, icon, s.Px(24), box.Center(), color);
            else
                CenterText(s, draw, LabelLarge, box.Center(), color, Label(label));
            if (item.pressed)
                result = event;
        };
        Passive(rect);
        if (closeButton)
            control("Dismiss", std::min(s.Px(48), w / 3), icons::Close, ink, SnackbarResult::Dismissed);
        if (action)
            control(action, std::min(TextWidth(s, LabelLarge, Label(action)) + s.Px(24), w * .5f), 0,
                    s.theme[Role::InversePrimary], SnackbarResult::Action);
        if (right > rect.min.x + margin) {
            draw->PushClipRect(rect.min, {right, rect.max.y}, true);
            Text(s, draw, BodyMedium, {rect.min.x + margin, rect.Center().y - s.Px(10)}, ink, Label(message));
            draw->PopClipRect();
        }
    }
    ImGui::End();
    EndOverlayStyle();
    state.open = result == SnackbarResult::None;
    return result;
}

SplitButtonResult SplitButton(const Style& s, const char* label, const Choice* actions, int count, const ButtonOptions& options) {
    SplitButtonResult result;
    if (SkipItems())
        return result;
    ImGui::PushID(label);
    auto o = options;
    o.shape = Shape::Square;
    result.primary = Button(s, label, o);
    ImGui::SameLine(0, s.Px(2));
    o.disabled |= count <= 0 || !actions;
    if (IconButton(s, "More actions", 0xe5cf, o))
        OpenMenu("split actions");
    if (BeginMenu(s, "split actions")) {
        result.secondary = ActionItems(s, actions, count);
        EndMenu();
    }
    ImGui::PopID();
    return result;
}

int FloatingActionButtonMenu(const Style& s, const char* id, Icon icon, const Choice* choices, int count) {
    if (SkipItems())
        return -1;
    ImGui::PushID(id);
    int result = -1;
    if (android::FloatingActionButton(s, "Actions", icon, {android::FabSize::Regular, android::FabColor::Primary, !choices || count <= 0}))
        OpenMenu("FAB actions");
    if (BeginMenu(s, "FAB actions", {MenuVariant::Vibrant, 192})) {
        result = ActionItems(s, choices, count);
        EndMenu();
    }
    ImGui::PopID();
    return result;
}

int Toolbar(const Style& s, const char* id, const Choice* choices, int count, const ToolbarOptions& o) {
    if (!choices || count <= 0 || SkipItems())
        return -1;
    const float height = std::max(40.0f, 64 + Density(s));
    float width = 64;
    if (!o.vertical) {
        width = 8;
        for (int i = 0; i < count; ++i) {
            width += std::max(48.0f, TextWidth(s, LabelLarge, Label(choices[i].label)) / s.scale
                                      + (choices[i].icon ? 52.0f : 32.0f));
        }
    }
    const auto box = Slot(s, width, o.vertical ? 8 + height * count : height);
    const float rounding = o.rounded ? s.Px(24) : 0;
    if (o.elevated)
        Shadow(s, ImGui::GetWindowDrawList(), box, rounding, 2);
    int action = -1;
    if (BeginSurface(s, id, {box.W(), box.H()}, o.vibrant ? Role::PrimaryContainer : Role::SurfaceContainer,
                     o.rounded ? 24.0f : 0.0f, false, ImGuiWindowFlags_NoScrollbar, false)) {
        auto p = ImGui::GetCursorScreenPos();
        p.x += s.Px(o.vertical ? 8 : 4);
        p.y += s.Px(std::max(0.0f, (height - 48) * .5f));
        for (int i = 0; i < count; ++i) {
            ImGui::SetCursorScreenPos(p);
            ImGui::PushID(i);
            ButtonOptions option;
            option.variant = o.vibrant ? Variant::Tonal : Variant::Text;
            option.icon = choices[i].icon;
            option.disabled = choices[i].disabled;
            const bool iconOnly = o.vertical || Label(choices[i].label).empty();
            if (iconOnly ? IconButton(s, choices[i].label, choices[i].icon, option) : Button(s, choices[i].label, option))
                action = i;
            if (o.vertical)
                p.y += s.Px(height);
            else
                p.x = ImGui::GetItemRectMax().x + s.Px(4);
            ImGui::PopID();
        }
    }
    EndSurface();
    return action;
}

} // namespace m3e
