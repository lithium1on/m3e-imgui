// Dear ImGui 1.92.5 is the only version supported by this private adapter.
#include "detail.hpp"
#include <imgui_internal.h>

static_assert(IMGUI_VERSION_NUM == 19250, "m3e requires Dear ImGui 1.92.5");

namespace m3e::detail {

Item Interact(const char* label, Rect hit, bool disabled, bool layout, bool pressOnClick) {
    Item result;
    auto* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return result;
    result.id = window->GetID(label);
    const ImRect bounds(hit.min, hit.max);
    if (layout)
        ImGui::ItemSize(bounds);
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, disabled || (GImGui->CurrentItemFlags & ImGuiItemFlags_Disabled));
    result.visible = ImGui::ItemAdd(bounds, result.id);
    result.disabled = (GImGui->LastItemData.ItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (result.visible && !result.disabled) {
        auto flags = pressOnClick ? ImGuiButtonFlags_PressedOnClick : ImGuiButtonFlags_None;
        if (pressOnClick)
            flags |= ImGuiButtonFlags_NoKeyModsAllowed;
        result.pressed = ImGui::ButtonBehavior(bounds, result.id, &result.hovered, &result.held, flags);
        result.focused = ImGui::IsItemFocused() && NavVisible();
    }
    ImGui::PopItemFlag();
    return result;
}

bool SkipItems() { return ImGui::GetCurrentWindowRead()->SkipItems; }
bool NavVisible() { return GImGui->NavCursorVisible; }
void Edited(ImGuiID id) { ImGui::MarkItemEdited(id); }
ImGuiID StateKey(ImGuiID id, const char* channel) { return ImHashStr(channel, 0, id); }

void ReserveLayout(Rect rect) {
    ImGui::SetCursorScreenPos(rect.min);
    ImGui::ItemSize(ImRect(rect.min, rect.max));
}

void FocusCurrentItem() {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const auto& item = g.LastItemData;
    ImGui::SetNavID(item.ID, window->DC.NavLayerCurrent, g.CurrentFocusScopeId,
                    ImGui::WindowRectAbsToRel(window, item.NavRect));
}

bool NavIntoSubmenu() {
    if (!ImGui::IsItemFocused() || !GImGui->NavMoveSubmitted || GImGui->NavMoveDir != ImGuiDir_Right)
        return false;
    ImGui::NavMoveRequestCancel();
    return true;
}

bool SubmenuBack() {
    if (!ImGui::IsWindowFocused() || !GImGui->NavMoveSubmitted || GImGui->NavMoveDir != ImGuiDir_Left)
        return false;
    ImGui::NavMoveRequestCancel();
    return true;
}

void CloseMenuChain() {
    auto& g = *GImGui;
    int root = g.BeginPopupStack.Size - 1;
    while (root > 0) {
        auto* parent = g.BeginPopupStack[root - 1].Window;
        if (!parent || !parent->StateStorage.GetInt(parent->GetID("##m3eMenuChain")))
            break;
        --root;
    }
    if (root >= 0 && root < g.OpenPopupStack.Size)
        ImGui::ClosePopupToLevel(root, true);
}

void CloseChildMenus() {
    auto& g = *GImGui;
    const int level = g.BeginPopupStack.Size;
    if (level < g.OpenPopupStack.Size) {
        auto* child = g.OpenPopupStack[level].Window;
        if (child && child->StateStorage.GetInt(child->GetID("##m3eMenuChain")))
            ImGui::ClosePopupToLevel(level, false);
    }
}

bool NavStep(bool vertical, float& direction, bool& boundary) {
    if (!ImGui::IsItemFocused())
        return false;
    auto previous = vertical ? ImGuiKey_DownArrow : ImGuiKey_LeftArrow;
    auto next = vertical ? ImGuiKey_UpArrow : ImGuiKey_RightArrow;
    auto padPrevious = vertical ? ImGuiKey_GamepadDpadDown : ImGuiKey_GamepadDpadLeft;
    auto padNext = vertical ? ImGuiKey_GamepadDpadUp : ImGuiKey_GamepadDpadRight;
    boundary = ImGui::IsKeyPressed(ImGuiKey_Home) || ImGui::IsKeyPressed(ImGuiKey_End);
    if (boundary)
        direction = ImGui::IsKeyPressed(ImGuiKey_Home) ? -1.0f : 1.0f;
    else if (ImGui::IsKeyPressed(previous) || ImGui::IsKeyPressed(padPrevious))
        direction = -1;
    else if (ImGui::IsKeyPressed(next) || ImGui::IsKeyPressed(padNext))
        direction = 1;
    else
        return false;
    ImGui::NavMoveRequestCancel();
    return true;
}

bool EscapePressed() {
    return ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false);
}

bool ClickedOutsideCurrentPopup() {
    const ImGuiWindow* window = ImGui::GetCurrentWindowRead();
    return ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !window->Rect().Contains(ImGui::GetIO().MousePos)
           && GImGui->OpenPopupStack.Size == GImGui->BeginPopupStack.Size;
}

} // namespace m3e::detail
