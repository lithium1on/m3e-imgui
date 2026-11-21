#include "detail.hpp"
#include <algorithm>
#include <cstdio>

namespace m3e {
using namespace detail;

namespace {

void PushPopupStyle(ImVec2 padding, ImVec2 spacing) {
    ImGui::PushStyleColor(ImGuiCol_PopupBg, 0u);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
}

void PopPopupStyle() {
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

void PopupSurface(const Style& s, ImU32 bg, float radius, int elevation, float heightScale = 1) {
    const auto p = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
    const auto bounds = At(p, size.x, size.y * heightScale);
    auto* draw = ImGui::GetWindowDrawList();
    draw->PushClipRectFullScreen();
    Shadow(s, draw, bounds, s.Px(radius), elevation);
    Fill(draw, bounds, s.Px(radius), bg);
    draw->PopClipRect();
}

ImGuiID OpenedKey(const char* id) { return StateKey(ImGui::GetID(id), "opened"); }

void TimedOpen(const char* id) {
    if (ImGui::IsPopupOpen(id))
        return;
    ImGui::GetStateStorage()->SetFloat(OpenedKey(id), static_cast<float>(ImGui::GetTime()));
    ImGui::OpenPopup(id);
}

float OpenElapsed(const char* id) {
    const float now = static_cast<float>(ImGui::GetTime());
    return now - ImGui::GetStateStorage()->GetFloat(OpenedKey(id), now - 1);
}

void PopupMotion(float opacity, float scaleX, float scaleY, float pivotX, float pivotY) {
    auto* storage = ImGui::GetStateStorage();
    storage->SetFloat(ImGui::GetID("##m3eOpacity"), opacity);
    storage->SetFloat(ImGui::GetID("##m3eScaleX"), scaleX);
    storage->SetFloat(ImGui::GetID("##m3eScaleY"), scaleY);
    storage->SetFloat(ImGui::GetID("##m3ePivotX"), pivotX);
    storage->SetFloat(ImGui::GetID("##m3ePivotY"), pivotY);
}

void ApplyPopupMotion() {
    auto* storage = ImGui::GetStateStorage();
    const float opacity = storage->GetFloat(ImGui::GetID("##m3eOpacity"), 1);
    const float x = storage->GetFloat(ImGui::GetID("##m3eScaleX"), 1);
    const float y = storage->GetFloat(ImGui::GetID("##m3eScaleY"), 1);
    if (opacity == 1 && x == 1 && y == 1)
        return;
    const auto position = ImGui::GetWindowPos();
    const auto size = ImGui::GetWindowSize();
    const float px = position.x + size.x * storage->GetFloat(ImGui::GetID("##m3ePivotX"), 0);
    const float py = position.y + size.y * storage->GetFloat(ImGui::GetID("##m3ePivotY"), 0);
    auto* draw = ImGui::GetWindowDrawList();
    for (auto& vertex : draw->VtxBuffer) {
        vertex.pos.x = Mix(px, vertex.pos.x, x);
        vertex.pos.y = Mix(py, vertex.pos.y, y);
        vertex.col = Opacity(vertex.col, opacity);
    }
    for (auto& command : draw->CmdBuffer) {
        auto& clip = command.ClipRect;
        if (clip.x >= position.x - 1 && clip.z <= position.x + size.x + 1) {
            clip.x = Mix(px, clip.x, x); clip.z = Mix(px, clip.z, x);
            clip.y = Mix(py, clip.y, y); clip.w = Mix(py, clip.w, y);
        }
    }
}

void FadeVertices(ImDrawList* draw, int from, float alpha) {
    if (alpha >= 1)
        return;
    for (int i = from; i < draw->VtxBuffer.Size; ++i)
        draw->VtxBuffer[i].col = Opacity(draw->VtxBuffer[i].col, alpha);
}

void FinishDialogBody() {
    auto* storage = ImGui::GetStateStorage();
    const auto key = ImGui::GetID("##m3eDialogBodyStart");
    const int from = storage->GetInt(key, -1);
    if (from < 0)
        return;
    FadeVertices(ImGui::GetWindowDrawList(), from, storage->GetFloat(ImGui::GetID("##m3eDialogBodyAlpha"), 1));
    storage->SetInt(key, -1);
}

constexpr ImGuiWindowFlags MenuFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
                                      | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;

MenuVariant CurrentMenuVariant() {
    return static_cast<MenuVariant>(ImGui::GetStateStorage()->GetInt(ImGui::GetID("##m3eMenuVariant")));
}

bool MenuPopup(const Style& s, const char* id, const MenuOptions& o, bool submenu, ImVec2 anchor) {
    if (!ImGui::IsPopupOpen(id))
        return false;
    const float elapsed = OpenElapsed(id);
    const float w = s.Px(std::clamp(o.width, 112.0f, 280.0f));
    const auto* viewport = ImGui::GetMainViewport();
    if (anchor.x + w > viewport->WorkPos.x + viewport->WorkSize.x) {
        if (submenu)
            anchor.x = ImGui::GetItemRectMin().x - w - s.Px(8);
        else
            anchor.x = viewport->WorkPos.x + viewport->WorkSize.x - w;
    }
    anchor.x = std::max(anchor.x, viewport->WorkPos.x);
    anchor.y = std::clamp(anchor.y, viewport->WorkPos.y,
        viewport->WorkPos.y + std::max(0.0f, viewport->WorkSize.y - s.Px(56)));
    ImGui::SetNextWindowPos(anchor, ImGuiCond_Appearing);
    ImGui::SetNextWindowSizeConstraints({w, 0}, {w, s.Px(282)});
    const bool vibrant = o.variant == MenuVariant::Vibrant;
    PushPopupStyle({s.Px(4), s.Px(4)}, {0, s.Px(2)});
    if (!ImGui::BeginPopup(id, MenuFlags)) {
        PopPopupStyle();
        return false;
    }
    ImGui::GetStateStorage()->SetInt(ImGui::GetID("##m3eMenuVariant"), static_cast<int>(o.variant));
    ImGui::GetStateStorage()->SetInt(ImGui::GetID("##m3eMenuChain"), 1);
    PopupSurface(s, s.theme[vibrant ? Role::TertiaryContainer : Role::SurfaceContainer], 16, 3);
    const float alpha = s.reducedMotion ? 1 : Ease(elapsed / .10f, Standard);
    const float scale = s.reducedMotion || submenu ? 1 : Ease(elapsed / .35f, FastSpatial(s));
    PopupMotion(alpha, 1, scale, 0, 0);
    return true;
}

Item MenuRow(const Style& s, const char* label, const MenuItemOptions& o, bool submenu, bool submenuOpen = false) {
    const auto& t = s.theme;
    const bool vibrant = CurrentMenuVariant() == MenuVariant::Vibrant;
    const auto box = At(ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail().x, s.Px(44 + Density(s)));
    auto item = Interact(label, box, o.disabled);
    if (!item.visible)
        return item;
    auto* draw = ImGui::GetWindowDrawList();
    const auto color = item.disabled ? Opacity(t[Role::OnSurface], .38f)
                    : t[o.selected ? (vibrant ? Role::OnTertiary : Role::OnTertiaryContainer)
                                   : (vibrant ? Role::OnTertiaryContainer : Role::OnSurface)];
    const float radius = s.Px(o.selected ? 12 : 4);
    if (o.selected)
        Fill(draw, box, radius, t[vibrant ? Role::Tertiary : Role::TertiaryContainer]);
    item.hovered |= submenuOpen;
    State(s, draw, item, box, radius, color);
    Focus(s, draw, item, box, radius, true);
    const float y = box.Center().y;
    float x = box.min.x + s.Px(12);
    const auto icon = o.mark != MenuMark::None && o.selected ? icons::Check : o.icon;
    if (icon) {
        Symbol(s, draw, icon, s.Px(20), {x + s.Px(10), y}, color);
        x += s.Px(28);
    }
    float labelRight = box.max.x - s.Px(12);
    if (submenu) {
        const ImVec2 arrow[]{ {labelRight - s.Px(9), y - s.Px(4)}, {labelRight - s.Px(5), y},
                              {labelRight - s.Px(9), y + s.Px(4)} };
        draw->AddConvexPolyFilled(arrow, 3, color);
        labelRight -= s.Px(24);
    }
    if (o.shortcut) {
        const float width = TextWidth(s, BodySmall, o.shortcut);
        Text(s, draw, BodySmall, {labelRight - width, y - s.Px(8)}, color, o.shortcut);
        labelRight -= width + s.Px(8);
    }
    if (labelRight > x) {
        draw->PushClipRect({x, box.min.y}, {labelRight, box.max.y}, true);
        Text(s, draw, LabelLarge, {x, y - s.Px(10)}, color, Label(label));
        draw->PopClipRect();
    }
    return item;
}

void Wrapped(const Style& s, Type type, const char* text, ImU32 color) {
    const auto pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    auto* draw = ImGui::GetWindowDrawList();
    auto* font = Font(s, type);
    const std::string_view body(text);
    const auto* end = body.data() + body.size();
    float y = pos.y;
    for (auto* start = body.data(); start < end;) {
        const auto* hardEnd = std::find(start, end, '\n');
        auto* next = font->CalcWordWrapPosition(FontSize(s, type), start, hardEnd, width);
        if (next > hardEnd || next <= start)
            next = hardEnd;
        Text(s, draw, type, {pos.x, y}, color, {start, static_cast<std::size_t>(next - start)});
        y += s.Px(type.lineHeight);
        if (next == hardEnd)
            start = hardEnd < end ? hardEnd + 1 : end;
        else
            start = next;
        while (start < end && *start == ' ')
            ++start;
    }
    ImGui::Dummy({width, y - pos.y});
}

struct TipPosition { ImVec2 position, pivot; };

TipPosition TipAnchor(Rect anchor, float offset, TooltipPlacement placement) {
    const auto c = anchor.Center();
    switch (placement) {
        case TooltipPlacement::Above: return {{c.x, anchor.min.y - offset}, {.5f, 1}};
        case TooltipPlacement::Left: return {{anchor.min.x - offset, c.y}, {1, .5f}};
        case TooltipPlacement::Right: return {{anchor.max.x + offset, c.y}, {0, .5f}};
        case TooltipPlacement::Below: return {{c.x, anchor.max.y + offset}, {.5f, 0}};
    }
    return {};
}

ImVec2 FitTip(TipPosition anchor, ImVec2 size) {
    const auto* view = ImGui::GetMainViewport();
    const auto pos = view->WorkPos;
    return {std::clamp(anchor.position.x - size.x * anchor.pivot.x, pos.x, pos.x + std::max(0.0f, view->WorkSize.x - size.x)),
            std::clamp(anchor.position.y - size.y * anchor.pivot.y, pos.y, pos.y + std::max(0.0f, view->WorkSize.y - size.y))};
}

ImGuiID AttachedTipId(const char* channel) {
    auto id = ImGui::GetItemID();
    if (!id) {
        const auto p = ImGui::GetItemRectMin();
        ImGui::PushID(static_cast<int>(p.y - ImGui::GetWindowPos().y + ImGui::GetScrollY()));
        id = ImGui::GetID(channel);
        ImGui::PopID();
    }
    return StateKey(id, channel);
}

// TooltipElementBase defaults to immediate show and a 200 ms grace period.
float TipPhase(const Style& s, ImGuiID id, bool active) {
    auto* storage = ImGui::GetStateStorage();
    const auto key = StateKey(id, "lastVisible");
    const auto now = static_cast<float>(ImGui::GetTime());
    if (active)
        storage->SetFloat(key, now);
    const auto last = storage->GetFloat(key, -10);
    return Animate(s, id, "tooltipOpacity", active || now - last < .2f ? 1.0f : 0.0f, .15f, Standard);
}

} // namespace

void OpenMenu(const char* id) { TimedOpen(id); }

bool BeginMenu(const Style& s, const char* id, const MenuOptions& o) {
    const auto anchor = ImGui::GetItemRectMin();
    return MenuPopup(s, id, o, false, {anchor.x, ImGui::GetItemRectMax().y + s.Px(4)});
}

bool BeginSubmenu(const Style& s, const char* label, Icon icon, bool enabled) {
    MenuItemOptions options;
    options.icon = icon;
    options.disabled = !enabled;
    const bool open = ImGui::IsPopupOpen(label);
    const auto row = MenuRow(s, label, options, true, open);
    if (row.disabled)
        return false;
    const bool hoverOpen = row.hovered && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay);
    if (!open && (row.pressed || hoverOpen || NavIntoSubmenu()))
        TimedOpen(label);
    const auto rect = Rect{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
    const MenuOptions nested{CurrentMenuVariant(), 168};
    const bool begun = MenuPopup(s, label, nested, true, {rect.max.x + s.Px(8), rect.min.y});
    if (begun && SubmenuBack()) {
        ImGui::CloseCurrentPopup();
        EndMenu();
        return false;
    }
    return begun;
}

void EndMenu() {
    ApplyPopupMotion();
    ImGui::EndPopup();
    PopPopupStyle();
}

bool MenuItem(const Style& s, const char* label, const MenuItemOptions& o) {
    const auto item = MenuRow(s, label, o, false);
    if (item.hovered && ImGui::IsMousePosValid())
        CloseChildMenus();
    if (item.pressed && (o.mark == MenuMark::None || !ImGui::IsKeyDown(ImGuiKey_Space)))
        CloseMenuChain();
    return item.pressed;
}

void MenuDivider(const Style& s) {
    const auto start = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    Fill(ImGui::GetWindowDrawList(), At({start.x, start.y + s.Px(8)}, w, s.Px(1)), 0, s.theme[Role::OutlineVariant]);
    ImGui::Dummy({w, s.Px(17)});
}

void OpenDialog(const char* id) {
    TimedOpen(id);
}

bool BeginDialog(const Style& s, const char* id, const char* title, const DialogOptions& o) {
    if (!ImGui::IsPopupOpen(id))
        return false;
    const float elapsed = s.reducedMotion ? 1 : OpenElapsed(id);
    const float phase = Ease(elapsed / .5f, Standard);
    const auto* view = ImGui::GetMainViewport();
    const float maxWidth = std::max(s.Px(80), view->WorkSize.x - s.Px(48));
    const float width = std::min(maxWidth, s.Px(std::clamp(o.width, 280.0f, 560.0f)));
    const float maxHeight = std::max(s.Px(80), std::min(s.Px(560), view->WorkSize.y - s.Px(48)));
    auto center = view->GetWorkCenter();
    center.y -= s.Px(50) * (1 - phase);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, {.5f, .5f});
    ImGui::SetNextWindowSizeConstraints({width, 0}, {width, maxHeight});
    PushPopupStyle({s.Px(24), s.Px(24)}, {s.Px(8), s.Px(16)});
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, Opacity(s.theme[Role::Scrim], .32f * phase));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, s.Px(28));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (!ImGui::BeginPopupModal(id, nullptr, MenuFlags)) {
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        PopPopupStyle();
        return false;
    }
    PopupSurface(s, s.theme[Role::SurfaceContainerHigh], 28, 3, Mix(.35f, 1, phase));
    if ((o.dismissOnEscape && EscapePressed()) || (o.dismissOnOutsideClick && ClickedOutsideCurrentPopup())) {
        CloseDialog();
        EndDialog();
        return false;
    }
    auto* draw = ImGui::GetWindowDrawList();
    const int headerStart = draw->VtxBuffer.Size;
    if (o.icon) {
        const auto p = ImGui::GetCursorScreenPos();
        Symbol(s, ImGui::GetWindowDrawList(), o.icon, s.Px(24),
               {p.x + ImGui::GetContentRegionAvail().x / 2, p.y + s.Px(12)}, s.theme[Role::Secondary]);
        ImGui::Dummy({s.Px(24), s.Px(24)});
    }
    const auto p = ImGui::GetCursorScreenPos();
    const auto widthAvailable = ImGui::GetContentRegionAvail().x;
    if (o.closeButton) {
        ImGui::SetCursorScreenPos({p.x + widthAvailable - s.Px(40), p.y});
        if (IconButton(s, "Close##dialog", icons::Close)) {
            CloseDialog();
            EndDialog();
            return false;
        }
    }
    if (title && *title)
        Text(s, draw, HeadlineSmall, {p.x, p.y + s.Px(o.closeButton ? 4 : 0)},
             s.theme[Role::OnSurface], title);
    ReserveLayout(At(p, widthAvailable, s.Px(o.closeButton ? 40 : 32)));
    FadeVertices(draw, headerStart, std::clamp(elapsed / .25f, 0.0f, 1.0f));
    auto* storage = ImGui::GetStateStorage();
    storage->SetInt(ImGui::GetID("##m3eDialogBodyStart"), draw->VtxBuffer.Size);
    storage->SetFloat(ImGui::GetID("##m3eDialogBodyAlpha"), std::clamp((elapsed - .05f) / .25f, 0.0f, 1.0f));
    storage->SetFloat(ImGui::GetID("##m3eDialogActionsAlpha"), std::clamp((elapsed - .15f) / .30f, 0.0f, 1.0f));
    return true;
}

void DialogText(const Style& s, const char* body) { Wrapped(s, BodyMedium, body, s.theme[Role::OnSurfaceVariant]); }

int DialogActions(const Style& s, const Choice* actions, int count, bool alignEnd) {
    if (count <= 0 || SkipItems())
        return -1;
    FinishDialogBody();
    auto* draw = ImGui::GetWindowDrawList();
    const int actionsStart = draw->VtxBuffer.Size;
    auto position = ImGui::GetCursorScreenPos();
    const float gap = s.Px(8);
    if (alignEnd) {
        float width = gap * (count - 1);
        for (int i = 0; i < count; ++i)
            width += s.Px(actions[i].icon ? 68.0f : 40.0f) + TextWidth(s, LabelLarge, Label(actions[i].label));
        position.x += std::max(0.0f, ImGui::GetContentRegionAvail().x - width);
    }
    ImGui::SetCursorScreenPos({position.x, position.y + s.Px(8)});
    int pressed = -1;
    for (int i = 0; i < count; ++i) {
        if (i)
            ImGui::SameLine(0, gap);
        ImGui::PushID(i);
        ButtonOptions options;
        options.icon = actions[i].icon;
        options.disabled = actions[i].disabled;
        if (Button(s, actions[i].label, options))
            pressed = i;
        ImGui::PopID();
    }
    FadeVertices(draw, actionsStart, ImGui::GetStateStorage()->GetFloat(ImGui::GetID("##m3eDialogActionsAlpha"), 1));
    return pressed;
}

void EndDialog() {
    FinishDialogBody();
    ImGui::EndPopup();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    PopPopupStyle();
}

void CloseDialog() { ImGui::CloseCurrentPopup(); }

void Tooltip(const Style& s, const char* text, TooltipPlacement placement) {
    if (!text || !*text || SkipItems())
        return;
    const bool active = ImGui::IsItemHovered() || (ImGui::IsItemFocused() && NavVisible());
    const float phase = TipPhase(s, AttachedTipId("plainTooltip"), active);
    if (phase <= .001f)
        return;
    const Rect item{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
    const float maxWidth = s.Px(184);
    auto* font = Font(s, BodySmall);
    const float size = FontSize(s, BodySmall);
    const auto textSize = font->CalcTextSizeA(size, 1e7f, maxWidth, text);
    const float w = std::clamp(TextWidth(s, BodySmall, text) + s.Px(16), s.Px(40), s.Px(200));
    const float h = s.Px(8) + std::max(s.Px(16), textSize.y / size * s.Px(16));
    const auto p = FitTip(TipAnchor(item, s.Px(Mix(10, 4, phase)), placement), {w, h});
    const Rect box = At(p, w, h);
    auto* draw = ImGui::GetForegroundDrawList();
    Fill(draw, box, s.Px(4), Opacity(s.theme[Role::InverseSurface], phase));
    if (textSize.y <= size)
        CenterText(s, draw, BodySmall, box.Center(), Opacity(s.theme[Role::InverseOnSurface], phase), text);
    else
        draw->AddText(font, size, {p.x + s.Px(8), p.y + s.Px(5)},
                      Opacity(s.theme[Role::InverseOnSurface], phase), text, nullptr, maxWidth);
}

bool BeginRichTooltip(const Style& s, const char* title, TooltipPlacement placement) {
    if (SkipItems())
        return false;
    const Rect item{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
    const auto id = AttachedTipId("richTooltip");
    auto* storage = ImGui::GetStateStorage();
    const auto xKey = StateKey(id, "tipX"), yKey = StateKey(id, "tipY"),
               wKey = StateKey(id, "tipW"), hKey = StateKey(id, "tipH");
    const Rect previous = At({storage->GetFloat(xKey), storage->GetFloat(yKey)}, storage->GetFloat(wKey),
                             storage->GetFloat(hKey));
    const bool insideTip = previous.W() > 0 && ImGui::IsMouseHoveringRect(previous.min, previous.max, false);
    const bool active = insideTip || ImGui::IsItemHovered() || (ImGui::IsItemFocused() && NavVisible());
    const float phase = TipPhase(s, id, active);
    if (phase <= .001f) {
        storage->SetFloat(wKey, 0);
        return false;
    }
    const float width = std::min(s.Px(280), ImGui::GetMainViewport()->WorkSize.x);
    const float height = previous.H() > 0 ? previous.H() : s.Px(88);
    ImGui::SetNextWindowPos(FitTip(TipAnchor(item, s.Px(Mix(14, 8, phase)), placement), {width, height}));
    ImGui::SetNextWindowSizeConstraints({width, 0}, {width, ImGui::GetMainViewport()->WorkSize.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, s.Px(12));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(s.Px(16), s.Px(12)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(s.Px(8), s.Px(4)));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, Opacity(s.theme[Role::SurfaceContainer], phase));
    ImGui::PushStyleColor(ImGuiCol_Text, Opacity(s.theme[Role::OnSurfaceVariant], phase));
    char name[40]{};
    std::snprintf(name, sizeof(name), "##m3e_rich_tip_%08x", id);
    const bool visible = ImGui::Begin(name, nullptr, MenuFlags | ImGuiWindowFlags_NoFocusOnAppearing
                                                       | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoCollapse);
    if (!visible) {
        EndRichTooltip();
        return false;
    }
    const auto p = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
    storage->SetFloat(xKey, p.x); storage->SetFloat(yKey, p.y);
    storage->SetFloat(wKey, size.x); storage->SetFloat(hKey, size.y);
    if (title && *title)
        Wrapped(s, TitleSmall, title, Opacity(s.theme[Role::OnSurfaceVariant], phase));
    return true;
}

void EndRichTooltip() {
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);
}

bool BeginInteractiveTooltip(const Style& s, const char* title, bool activated, TooltipPlacement placement, float width) {
    if (SkipItems())
        return false;
    const Rect item{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
    const auto trigger = AttachedTipId("interactiveTip");
    char name[40]{};
    std::snprintf(name, sizeof(name), "##m3e_interactive_tip_%08x", trigger);
    if (activated && !ImGui::IsPopupOpen(name))
        TimedOpen(name);
    if (!ImGui::IsPopupOpen(name))
        return false;
    width = std::clamp(s.Px(width), s.Px(80), std::max(s.Px(80), ImGui::GetMainViewport()->WorkSize.x));
    const auto anchor = TipAnchor(item, s.Px(4), placement);
    const float phase = s.reducedMotion ? 1 : Ease(OpenElapsed(name) / .15f, Standard);
    ImGui::SetNextWindowPos(anchor.position, ImGuiCond_Appearing, anchor.pivot);
    ImGui::SetNextWindowSizeConstraints({width, 0}, {width, ImGui::GetMainViewport()->WorkSize.y});
    PushPopupStyle({s.Px(16), s.Px(12)}, {s.Px(8), s.Px(4)});
    if (!ImGui::BeginPopup(name, MenuFlags)) {
        PopPopupStyle();
        return false;
    }
    PopupSurface(s, s.theme[Role::SurfaceContainer], 12, 3);
    PopupMotion(phase, Mix(.8f, 1, phase), Mix(.8f, 1, phase), .5f, .5f);
    if (title && *title)
        Wrapped(s, TitleSmall, title, s.theme[Role::OnSurfaceVariant]);
    return true;
}

void EndInteractiveTooltip() {
    ApplyPopupMotion();
    ImGui::EndPopup();
    PopPopupStyle();
}

} // namespace m3e
