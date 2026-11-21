#include "components_detail.hpp"
#include <cctype>
#include <climits>
#include <cstring>

namespace m3e {
using namespace detail;

ChipAction Chip(const Style& s, const char* label, const ChipOptions& o) {
    if (SkipItems())
        return ChipAction::None;
    ImGui::PushID(label);
    const auto text = Label(label);
    const Icon leading = o.kind == ChipKind::Filter && o.selected ? icons::Check : o.icon;
    const bool removable = o.kind == ChipKind::Input && o.removable;
    const float width = TextWidth(s, LabelLarge, text) + s.Px(32 + (leading ? 18 : 0) + (removable ? 24 : 0));
    const auto hit = At(ImGui::GetCursorScreenPos(), width, s.Px(48));
    const auto box = Around(hit.Center(), width, s.Px(std::max(24.0f, 32 + Density(s))));
    const float removeWidth = removable ? s.Px(34) : 0;
    Passive(hit);
    const auto action = Interact("activate", At(hit.min, width - removeWidth, hit.H()), o.disabled, false);
    const auto remove = removable ? Interact("remove", At({hit.max.x - removeWidth, hit.min.y}, removeWidth, hit.H()),
                                             o.disabled, false) : Item{};
    if (action.visible || remove.visible) {
        auto* draw = ImGui::GetWindowDrawList();
        const auto ink = o.disabled ? Opacity(s.theme[Role::OnSurface], .38f)
                                    : s.theme[o.selected ? Role::OnSecondaryContainer : Role::OnSurfaceVariant];
        if (o.elevated && !o.disabled)
            Shadow(s, draw, box, s.Px(8), 1);
        if (o.selected || o.elevated)
            Fill(draw, box, s.Px(8), o.disabled ? Opacity(s.theme[Role::OnSurface], .12f)
                                               : s.theme[o.selected ? Role::SecondaryContainer : Role::SurfaceContainerLow]);
        else
            Stroke(draw, box, s.Px(8), o.disabled ? Opacity(s.theme[Role::OnSurface], .12f) : s.theme[Role::OutlineVariant], s.Px(1));
        State(s, draw, action, box, s.Px(8), ink);
        Focus(s, draw, action, box, s.Px(8));
        float x = box.min.x + s.Px(16);
        if (leading) {
            const auto iconInk = !o.selected && !o.disabled && o.kind == ChipKind::Assist ? s.theme[Role::Primary] : ink;
            Symbol(s, draw, leading, s.Px(18), {x + s.Px(2), box.Center().y}, iconInk);
            x += s.Px(18);
        }
        Text(s, draw, LabelLarge, {x, box.Center().y - s.Px(10)}, ink, text);
        if (removable) {
            const ImVec2 center{hit.max.x - s.Px(16), box.Center().y};
            const auto removeBox = Around(center, s.Px(24), s.Px(24));
            State(s, draw, remove, removeBox, s.Px(12), ink);
            Focus(s, draw, remove, removeBox, s.Px(12));
            Symbol(s, draw, icons::Close, s.Px(18), center, ink);
        }
    }
    ImGui::PopID();
    return remove.pressed ? ChipAction::Remove : action.pressed ? ChipAction::Activate : ChipAction::None;
}

bool RadioButton(const Style& s, const char* label, bool selected, bool disabled) {
    if (SkipItems())
        return false;
    const auto text = Label(label);
    const float textSpace = text.empty() ? 0 : TextWidth(s, BodyLarge, text) + s.Px(8);
    const auto hit = At(ImGui::GetCursorScreenPos(), s.Px(48) + textSpace, s.Px(48));
    const auto item = Interact(label, hit, disabled);
    if (!item.visible)
        return item.pressed;
    const ImVec2 center{hit.min.x + s.Px(24), hit.Center().y};
    const auto target = Around(center, s.Px(40), s.Px(40));
    auto* draw = ImGui::GetWindowDrawList();
    const auto ink = item.disabled ? Opacity(s.theme[Role::OnSurface], .38f)
                                   : s.theme[selected ? Role::Primary : Role::OnSurfaceVariant];
    State(s, draw, item, target, s.Px(20), ink, true, true);
    Focus(s, draw, item, target, s.Px(20));
    draw->AddCircle(center, s.Px(9), ink, 24, s.Px(2));
    const float fill = Animate(s, item.id, "radio", selected ? 1.0f : 0.0f, .15f, FastEffects);
    if (fill > 0)
        draw->AddCircleFilled(center, s.Px(5) * fill, ink, 20);
    Text(s, draw, BodyLarge, {hit.min.x + s.Px(48), center.y - s.Px(12)},
         item.disabled ? ink : s.theme[Role::OnSurface], text);
    return item.pressed;
}

bool RadioGroup(const Style& s, const char* id, const Choice* choices, int count, int& selected, bool horizontal) {
    if (!choices || SkipItems())
        return false;
    const int old = selected;
    const float x = ImGui::GetCursorScreenPos().x;
    ImGui::PushID(id);
    for (int i = 0; i < count; ++i) {
        if (i && horizontal)
            ImGui::SameLine(0, s.Px(8));
        if (!horizontal)
            ImGui::SetCursorScreenPos({x, ImGui::GetCursorScreenPos().y});
        ImGui::PushID(i);
        if (RadioButton(s, choices[i].label, selected == i, choices[i].disabled))
            selected = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return old != selected;
}

namespace {
bool Segments(const Style& s, const char* id, const Choice* choices, int count, int* single, bool* multi, bool check) {
    if (!choices || count <= 0 || SkipItems())
        return false;
    float width = s.Px(48);
    for (int i = 0; i < count; ++i) {
        width = std::max(width, TextWidth(s, LabelLarge, Label(choices[i].label))
                              + s.Px((check || choices[i].icon ? 26 : 0) + 24));
    }
    const float height = s.Px(std::max(28.0f, 40 + Density(s)));
    const auto startPosition = ImGui::GetCursorScreenPos();
    ImGui::PushID(id);
    bool changed = false;
    for (int i = 0; i < count; ++i) {
        if (i) {
            ImGui::SameLine(0, 0);
            ImGui::SetCursorScreenPos({startPosition.x + i * (width - s.Px(1)), startPosition.y});
        }
        ImGui::PushID(i);
        const auto hit = At(ImGui::GetCursorScreenPos(), width, s.Px(48));
        const auto box = Around(hit.Center(), width, height);
        const auto item = Interact("segment", hit, choices[i].disabled);
        if (item.pressed) {
            changed |= multi || *single != i;
            if (multi)
                multi[i] = !multi[i];
            else
                *single = i;
        }
        const bool selected = multi ? multi[i] : *single == i;
        if (item.visible) {
            auto* draw = ImGui::GetWindowDrawList();
            const float start = i == 0 ? height / 2 : 0;
            const float end = i == count - 1 ? height / 2 : 0;
            const Corners corners{start, end, end, start};
            const auto ink = item.disabled ? Opacity(s.theme[Role::OnSurface], .38f)
                                           : s.theme[selected ? Role::OnSecondaryContainer : Role::OnSurface];
            if (selected)
                Fill(draw, box, corners, s.theme[Role::SecondaryContainer]);
            Stroke(draw, box, corners, s.theme[Role::Outline], s.Px(1));
            State(s, draw, item, box, corners, ink);
            Focus(s, draw, item, box, corners, true);
            const auto text = Label(choices[i].label);
            const auto icon = check && selected ? icons::Check : choices[i].icon;
            const float x = box.Center().x - (TextWidth(s, LabelLarge, text) + s.Px(icon ? 26 : 0)) / 2;
            if (icon)
                Symbol(s, draw, icon, s.Px(18), {x + s.Px(9), box.Center().y}, ink);
            Text(s, draw, LabelLarge, {x + s.Px(icon ? 26 : 0), box.Center().y - s.Px(10)}, ink, text);
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    return changed;
}

bool Matches(std::string_view label, std::string_view query) {
    const auto same = [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); };
    return query.empty() || std::search(label.begin(), label.end(), query.begin(), query.end(), same) != label.end();
}

int Query(const Style& s, const char* label, char* buffer, std::size_t capacity, const Choice* choices, int count,
          float width, bool search, bool expanded) {
    if (!buffer || capacity == 0 || capacity > INT_MAX || SkipItems())
        return -1;
    ImGui::PushID(label);
    const auto editor = Slot(s, width, 56);
    auto* draw = ImGui::GetWindowDrawList();
    auto* storage = ImGui::GetStateStorage();
    const auto state = ImGui::GetID("open");
    const auto dismissed = ImGui::GetID("dismissed");
    bool open = storage->GetBool(state);
    const float inset = s.Px(search ? 50 : 16);
    const auto tint = s.theme[search ? Role::SurfaceContainerHigh : Role::SurfaceContainerHighest];
    Fill(draw, editor, s.Px(search ? 28 : 4), tint);
    if (search)
        Symbol(s, draw, icons::Search, s.Px(24), {editor.min.x + s.Px(26), editor.Center().y}, s.theme[Role::OnSurfaceVariant]);
    else
        Stroke(draw, editor, s.Px(4), s.theme[Role::Outline], s.Px(1));
    const auto fontSize = FontSize(s, BodyLarge);
    ImGui::SetCursorScreenPos({editor.min.x + inset, editor.min.y + (editor.H() - fontSize) / 2});
    ImGui::PushFont(Font(s, BodyLarge), BaseFontSize(fontSize));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0u);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, 0u);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, 0u);
    ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
    ImGui::SetNextItemWidth(std::max(s.Px(12), editor.W() - inset - s.Px(16)));
    const auto hint = std::string(Label(label));
    const bool submitted = ImGui::InputTextWithHint("##query", hint.c_str(), buffer, capacity, ImGuiInputTextFlags_EnterReturnsTrue);
    const bool editing = ImGui::IsItemActive();
    if (ImGui::IsItemActivated() || ImGui::IsItemEdited()) {
        storage->SetBool(dismissed, false);
        open = true;
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()
        && ImGui::IsMouseHoveringRect(editor.min, editor.max) && !ImGui::IsItemHovered()) {
        ImGui::SetKeyboardFocusHere(-1);
        storage->SetBool(dismissed, false);
        open = true;
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
    ImGui::PopFont();
    ReserveLayout(editor);
    if (open && EscapePressed()) {
        open = false;
        storage->SetBool(dismissed, true);
    }
    int accepted = -1;
    if (open && choices && !storage->GetBool(dismissed)) {
        const bool result = BeginSurface(s, "##results", {editor.W(), s.Px(expanded ? 216 : 132)},
                                         Role::SurfaceContainer, 16, false, 0, false);
        if (result) {
            int first = -1;
            for (int i = 0; i < count; ++i) {
                if (!Matches(Label(choices[i].label), buffer))
                    continue;
                if (first < 0 && !choices[i].disabled)
                    first = i;
                ImGui::PushID(i);
                const ListEntry entry{choices[i].label, nullptr, choices[i].icon, 0, choices[i].disabled};
                if (Row(s, "result", Slot(s, 0, 44), entry, {Role::Count}).pressed)
                    accepted = i;
                ImGui::PopID();
            }
            if (submitted && accepted < 0)
                accepted = first;
            if (first < 0)
                Heading(s, "No matches", Size::ExtraSmall);
        }
        EndSurface();
        const ImVec2 resultsEnd{editor.max.x, ImGui::GetItemRectMax().y};
        if (!editing && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && !ImGui::IsMouseHoveringRect(editor.min, resultsEnd, false))
            open = false;
    }
    if (accepted >= 0) {
        const auto value = Label(choices[accepted].label);
        auto size = std::min(value.size(), capacity - 1);
        while (size && size < value.size() && (static_cast<unsigned char>(value[size]) & 0xc0) == 0x80)
            --size;
        std::memcpy(buffer, value.data(), size);
        buffer[size] = '\0';
        open = false;
        storage->SetBool(dismissed, true);
    }
    storage->SetBool(state, open);
    ImGui::PopID();
    return accepted;
}
} // namespace

bool SegmentedButtons(const Style& s, const char* id, const Choice* choices, int count, int& selected, bool check) {
    return Segments(s, id, choices, count, &selected, nullptr, check);
}

bool SegmentedButtonsMulti(const Style& s, const char* id, const Choice* choices, int count, bool* selected, bool check) {
    return selected && Segments(s, id, choices, count, nullptr, selected, check);
}

int ButtonGroup(const Style& s, const char* id, const Choice* choices, int count, const ButtonOptions& options) {
    if (!choices)
        return -1;
    ImGui::PushID(id);
    int action = -1;
    for (int i = 0; i < count; ++i) {
        if (i)
            ImGui::SameLine(0, s.Px(8));
        ImGui::PushID(i);
        auto o = options;
        o.icon = choices[i].icon;
        o.disabled |= choices[i].disabled;
        const bool iconOnly = Label(choices[i].label).empty() && o.icon;
        if (iconOnly ? IconButton(s, "action", o.icon, o) : Button(s, choices[i].label, o))
            action = i;
        ImGui::PopID();
    }
    ImGui::PopID();
    return action;
}

bool Select(const Style& s, const char* label, const Choice* choices, int count, int& selected, float width) {
    if (!choices || count <= 0 || SkipItems())
        return false;
    ImGui::PushID(label);
    auto box = Slot(s, width, 56 + Density(s));
    const auto entry = ListEntry{selected >= 0 && selected < count ? choices[selected].label : label,
                                 nullptr, 0, 0xe5cf};
    if (Row(s, "select", box, entry, {Role::Count, Role::OnSurface, 4, 16, 0, true}).pressed)
        OpenMenu("options");
    const int old = selected;
    if (BeginMenu(s, "options", {MenuVariant::Standard, box.W() / s.scale})) {
        for (int i = 0; i < count; ++i) {
            ImGui::PushID(i);
            if (MenuItem(s, choices[i].label, {choices[i].icon, nullptr, MenuMark::Radio, i == selected, choices[i].disabled}))
                selected = i;
            ImGui::PopID();
        }
        EndMenu();
    }
    ImGui::PopID();
    return old != selected;
}

int Autocomplete(const Style& s, const char* label, char* query, std::size_t capacity, const Choice* choices, int count, float width) {
    return Query(s, label, query, capacity, choices, count, width, false, false);
}

int SearchBar(const Style& s, const char* label, char* query, std::size_t capacity, const Choice* choices, int count,
              const SearchOptions& options) {
    const bool expanded = options.presentation == SearchPresentation::Expanded;
    const float width = expanded ? 0 : options.width;
    return Query(s, label, query, capacity, choices, count, width, true, expanded);
}

bool AutoTextarea(const Style& s, const char* label, char* value, std::size_t capacity,
                  int minimumLines, int maximumLines, float width) {
    if (!value || !capacity)
        return false;
    char* end = std::find(value, value + capacity, '\0');
    if (end == value + capacity)
        return false;
    const auto lines = 1 + std::count(value, end, '\n');
    const int min = std::clamp(minimumLines, 1, 64);
    const int max = std::clamp(maximumLines, min, 64);
    FieldOptions field;
    field.width = width;
    field.multilineHeight = 32.0f + 24.0f * static_cast<float>(std::clamp<std::ptrdiff_t>(lines, min, max));
    field.floatLabel = true;
    return TextField(s, label, value, capacity, field);
}

} // namespace m3e
