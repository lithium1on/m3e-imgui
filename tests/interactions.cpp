#include <m3e/imgui.hpp>
#include <m3e/android.hpp>
#include <m3e/components.hpp>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {
using namespace m3e;

bool Check(bool condition, const char* message) {
    if (!condition)
        std::fprintf(stderr, "%s\n", message);
    return condition;
}

struct TestFrame {
    TestFrame() {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({680, 400});
        ImGui::Begin("Tests", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
    }
    ~TestFrame() {
        ImGui::End();
        ImGui::Render();
    }
    static void Pos(float x, float y) { ImGui::SetCursorScreenPos({x, y}); }
};

bool PointerControls(ImGuiIO& io, const Style& style) {
    bool checked = false, switched = false, clicked = false;
    CheckState mixed = CheckState::Mixed;
    float low = 30, high = 75;
    auto render = [&] {
        TestFrame frame;
        TestFrame::Pos(24, 24);
        clicked |= Button(style, "Click", {Variant::Filled});
        TestFrame::Pos(24, 80);
        Checkbox(style, "##normal", checked);
        TestFrame::Pos(88, 80);
        Checkbox(style, "##mixed", mixed);
        TestFrame::Pos(24, 140);
        Switch(style, "##disabled", switched, SwitchIcons::Selected, true);
        TestFrame::Pos(24, 200);
        RangeSlider(style, "range", low, high, 0, 100);
    };
    render();
    auto click = [&](float x, float y) {
        io.AddMousePosEvent(x, y);
        render();
        io.AddMouseButtonEvent(0, true);
        render();
        io.AddMouseButtonEvent(0, false);
        render();
        render();
    };
    click(50, 44);
    click(44, 100);
    click(108, 100);
    click(50, 151);
    io.AddMousePosEvent(165, 222);
    io.AddMouseButtonEvent(0, true);
    render();
    io.AddMousePosEvent(196, 222);
    render();
    io.AddMouseButtonEvent(0, false);
    render();
    return Check(clicked, "Pointer button activation") & Check(checked, "Pointer checkbox")
         & Check(mixed == CheckState::Checked, "Mixed checkbox resolves to checked")
         & Check(!switched, "Disabled switch ignores the pointer")
         & Check(low == 30 && high > 85 && high < 100, "Range slider tracks the nearest thumb");
}

bool SliderStopIndicator(ImGuiIO&, const Style& style) {
    TestFrame frame;
    SliderOptions options;
    options.length = 240;
    const auto markerInk = style.theme[Role::OnSecondaryContainer];
    auto hasMarker = [&](const char* label, float x, float y, float value) {
        TestFrame::Pos(x, y);
        const auto* draw = ImGui::GetWindowDrawList();
        const int first = draw->VtxBuffer.Size;
        Slider(style, label, value, 0, 100, options);
        for (int i = first; i < draw->VtxBuffer.Size; ++i) {
            if (draw->VtxBuffer[i].col == markerInk)
                return true;
        }
        return false;
    };
    const bool horizontal = hasMarker("horizontal", 24, 24, 62) && !hasMarker("horizontal end", 24, 80, 98);
    options.vertical = true;
    const bool vertical = hasMarker("vertical", 320, 24, 62) && !hasMarker("vertical end", 390, 24, 98);
    return Check(horizontal, "Horizontal slider hides its end dot when it would overlap the thumb gap")
         & Check(vertical, "Vertical slider hides its end dot when it would overlap the thumb gap");
}

bool CatalogSelection(ImGuiIO& io, const Style& style) {
    DateRange range{{2026, 11, 1}, {2026, 11, 1}};
    CalendarPage month{2026, 11};
    bool filter = false;
    int segment = 0;
    char query[32]{};
    int result = -1;
    const Choice choices[]{{"Notes"}, {"Library"}, {"Archive", 0, true}};
    auto render = [&] {
        TestFrame frame;
        TestFrame::Pos(24, 24);
        DateRangeCalendar(style, "Dates", month, range);
        TestFrame::Pos(340, 24);
        if (Chip(style, "Filter", {ChipKind::Filter, 0, filter}) == ChipAction::Activate)
            filter = !filter;
        TestFrame::Pos(340, 82);
        SegmentedButtons(style, "Mode", choices, 3, segment);
        TestFrame::Pos(340, 146);
        const int match = Autocomplete(style, "Find a destination", query, sizeof(query), choices, 3);
        if (match >= 0)
            result = match;
    };
    const auto click = [&](float x, float y) {
        io.AddMousePosEvent(x, y);
        render();
        io.AddMouseButtonEvent(0, true);
        render();
        io.AddMouseButtonEvent(0, false);
        render();
    };
    render();
    click(204, 116);
    click(84, 116);
    click(373, 48);
    click(476, 105);
    click(569, 105);
    click(435, 174);
    io.AddInputCharactersUTF8("lib");
    render();
    io.AddKeyEvent(ImGuiKey_Enter, true);
    render();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    render();
    return Check(range.start.day == 2 && range.end.day == 5 && !range.choosingEnd, "Dates order a reverse selection")
         & Check(filter, "Filter chip activation")
         & Check(segment == 1, "Segmented button skips a disabled choice")
         & Check(result == 1 && std::strcmp(query, "Library") == 0, "Autocomplete accepts the filtered result with Enter");
}

bool CatalogSurfaces(ImGuiIO& io, const Style& style) {
    bool open = true, sheetShown = false;
    SnackbarState snackbar;
    SnackbarResult result = SnackbarResult::None;
    float fraction = .5f;
    auto render = [&](bool start = false) {
        TestFrame frame;
        if (BeginBottomSheet(style, "Sheet", open, {SheetMode::Modal, 300, 160})) {
            sheetShown = true;
            Heading(style, "Sheet content");
            EndBottomSheet();
        }
        if (start)
            ShowSnackbar(snackbar, io.DeltaTime * 2.5);
        const auto thisFrame = Snackbar(style, "Message", snackbar, "Saved");
        if (thisFrame != SnackbarResult::None)
            result = thisFrame;
        TestFrame::Pos(24, 24);
        const SplitPaneOptions panes{320, 120, 64};
        BeginSplitPane(style, "Split", fraction, panes);
        Heading(style, "First");
        NextSplitPane(style, fraction, panes);
        Heading(style, "Second");
        EndSplitPane();
        TestFrame::Pos(24, 160);
        Badge(style);
    };
    render();
    render();
    io.AddKeyEvent(ImGuiKey_Escape, true);
    render();
    io.AddKeyEvent(ImGuiKey_Escape, false);
    render(true);
    render();
    render();
    render();
    io.AddMousePosEvent(184, 84);
    render();
    io.AddMouseButtonEvent(0, true);
    render();
    io.AddMousePosEvent(235, 84);
    render();
    io.AddMouseButtonEvent(0, false);
    render();
    return Check(sheetShown && !open, "Escape dismisses a modal bottom sheet")
         & Check(result == SnackbarResult::Dismissed && !snackbar.open, "Snackbar times out once")
         & Check(fraction > .6f && fraction <= .8f, "Split pane handle updates the caller's fraction");
}

bool CatalogListAlignment(ImGuiIO& io, const Style& style) {
    int selected = 0;
    const ListEntry entries[]{{"First"}, {"Second"}, {"Third"}};
    auto render = [&] {
        TestFrame frame;
        TestFrame::Pos(280, 24);
        List(style, "Results", entries, 3, selected, 200, true);
    };
    render();
    io.AddMousePosEvent(340, 102);
    render();
    io.AddMouseButtonEvent(0, true);
    render();
    io.AddMouseButtonEvent(0, false);
    render();
    return Check(selected == 1, "A list keeps every row aligned with its initial cursor position");
}

bool Editor(ImGuiIO& io, const Style& style) {
    char value[32] = "";
    bool edited = false;
    auto render = [&] {
        TestFrame frame;
        TestFrame::Pos(24, 24);
        FieldOptions options;
        options.variant = FieldVariant::Outlined;
        options.leadingIcon = icons::Search;
        edited |= TextField(style, "Search", value, sizeof(value), options);
    };
    render();
    io.AddMousePosEvent(90, 52);
    io.AddMouseButtonEvent(0, true);
    render();
    io.AddMouseButtonEvent(0, false);
    render();
    io.AddInputCharactersUTF8("M3E");
    render();
    return Check(edited && std::strcmp(value, "M3E") == 0, "Text field edits the caller's buffer");
}

bool Navigation(ImGuiIO& io, const Style& style) {
    bool enabled = false;
    int tab = 0;
    const Choice choices[]{{"First"}, {"Disabled", 0, true}, {"Last"}};
    auto render = [&] {
        TestFrame frame;
        TestFrame::Pos(24, 24);
        Switch(style, "Enabled", enabled);
        TestFrame::Pos(24, 96);
        Tabs(style, "tabs", choices, 3, tab);
    };
    render();
    io.AddKeyEvent(ImGuiKey_Tab, true);
    render();
    io.AddKeyEvent(ImGuiKey_Tab, false);
    render();
    render();
    io.AddKeyEvent(ImGuiKey_Space, true);
    render();
    io.AddKeyEvent(ImGuiKey_Space, false);
    render();
    render();
    const bool keyboard = enabled;
    io.AddKeyEvent(ImGuiKey_GamepadFaceDown, true);
    render();
    io.AddKeyEvent(ImGuiKey_GamepadFaceDown, false);
    render();
    render();
    const bool gamepad = keyboard && !enabled;
    io.AddMousePosEvent(47, 124);
    render();
    io.AddMouseButtonEvent(0, true);
    render();
    io.AddMouseButtonEvent(0, false);
    render();
    render();
    io.AddKeyEvent(ImGuiKey_RightArrow, true);
    render();
    io.AddKeyEvent(ImGuiKey_RightArrow, false);
    render();
    return Check(keyboard, "Keyboard activation") & Check(gamepad, "Gamepad activation")
         & Check(tab == 2, "Tab navigation skips disabled choices");
}

bool Modal(ImGuiIO& io, const Style& style) {
    bool opened = false, active = false;
    auto render = [&](bool open = false) {
        TestFrame frame;
        if (open)
            OpenDialog("dialog");
        if (BeginDialog(style, "dialog", "Confirm")) {
            opened = true;
            DialogText(style, "Dialog contents");
            EndDialog();
        }
        active = ImGui::IsPopupOpen("dialog");
    };
    render(true);
    render();
    render();
    io.AddKeyEvent(ImGuiKey_Escape, true);
    render();
    io.AddKeyEvent(ImGuiKey_Escape, false);
    render();
    return Check(opened && !active, "Escape dismisses the modal");
}

bool AndroidComponents(ImGuiIO& io, const Style& style) {
    int selection = 0, action = -1, toolbar = -1;
    bool fab = false, back = false, progressPassive = true;
    const Choice choices[]{{"First", icons::Home}, {"Second", icons::Search}, {"Disabled", icons::Close, true}};
    auto render = [&] {
        TestFrame frame;
        TestFrame::Pos(24, 24);
        fab |= android::FloatingActionButton(style, "Add", icons::Add, {android::FabSize::Small});
        TestFrame::Pos(24, 82);
        const int index = android::FloatingToolbar(style, "toolbar", choices, 3);
        if (index >= 0)
            toolbar = index;
        TestFrame::Pos(24, 152);
        android::ShortNavigationBar(style, "navigation", choices, 3, selection, {270});
        TestFrame::Pos(24, 222);
        const auto appBar = android::TopAppBar(style, "appbar", "Sample", choices, 2, {icons::Close, "Close", nullptr, 340});
        if (appBar.action >= 0)
            action = appBar.action;
        back |= appBar.navigation;
        TestFrame::Pos(24, 302);
        android::LinearProgress(style, std::nullopt, true);
        progressPassive &= !ImGui::IsItemActive();
        android::CircularProgress(style, .36f);
    };
    render();
    const auto click = [&](float x, float y) {
        io.AddMousePosEvent(x, y);
        render();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        render();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        render();
    };
    click(26, 48);
    click(105, 112);
    click(160, 112);
    click(170, 183);
    click(250, 183);
    click(48, 250);
    click(330, 250);
    click(120, 308);
    return Check(fab, "Small FAB respects its 48dp input target")
         & Check(toolbar == 1, "Floating toolbar returns the action and skips disabled buttons")
         & Check(selection == 1, "Short navigation selects an enabled destination")
         & Check(back && action == 1, "Top app bar routes leading and trailing actions")
         & Check(progressPassive, "Progress indicators are passive");
}
} // namespace

int main(int argc, char** argv) {
    bool passed = true;
    constexpr bool (*checks[])(ImGuiIO&, const Style&) = {
        PointerControls, SliderStopIndicator, CatalogSelection, CatalogSurfaces, CatalogListAlignment,
        Editor, Navigation, Modal, AndroidComponents
    };
    for (auto verify : checks) {
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = {680, 400};
        io.DeltaTime = 1.0f / 60;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures | ImGuiBackendFlags_HasGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
        Style style;
        style.reducedMotion = true;
        if (argc > 1)
            style.fonts = LoadFonts(argv[1]);
        passed &= verify(io, style);
        ImGui::DestroyContext();
    }
    return passed ? 0 : 1;
}
