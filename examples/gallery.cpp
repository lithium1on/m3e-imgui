#include "win32_dx11.hpp"
#include "catalog.hpp"
#include <m3e/android.hpp>
#include <m3e/imgui.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

namespace {
using namespace m3e;

constexpr int Width = 760, Height = 390;
constexpr int PhoneWidth = 360, PhoneHeight = 688;
constexpr int CatalogHeight = 810;
constexpr float Step = 1.0f / 60;
constexpr std::array Names{"button", "icon-button", "connected", "tabs", "menu", "menu-vibrant", "submenu",
                           "text-field", "checkbox", "switch", "slider", "dialog", "tooltip", "rich-tooltip",
                           "connected-icons", "rich-hover", "android-sample", "android-fab", "android-nav",
                           "android-toolbar", "android-app-bar", "android-progress", "android-phone", "selection",
                           "search-inputs", "date-time", "cards-lists", "navigation-layout", "panes-carousel",
                           "sheets-actions", "feedback-shapes", "android-layout", "android-catalog-phone"};
constexpr std::array Titles{"Buttons", "Icon buttons", "Connected buttons", "Tabs", "Menu", "Vibrant menu",
                            "Submenu", "Text fields", "Checkboxes", "Switches", "Sliders", "Dialog", "Tooltip",
                            "Rich tooltip", "Connected icons", "Hover tooltip", "App preview", "Floating action buttons",
                            "Short navigation", "Floating toolbars", "Top app bars", "Progress indicators", "Phone comparison",
                            "Selection controls", "Search and inputs", "Date and time", "Cards and lists", "Navigation layouts",
                            "Panes and carousel", "Sheets and actions", "Feedback and shapes", "Android screen layout",
                            "Phone components"};
constexpr int FirstAndroidCase = 16, PhoneCase = 22;
constexpr int FirstCatalogCase = PhoneCase + 1;
constexpr int AndroidLayoutCase = static_cast<int>(Names.size()) - 2;
constexpr int PhoneCatalogCase = AndroidLayoutCase + 1;
constexpr bool IsAndroidPage(int index) {
    return (index >= FirstAndroidCase && index <= PhoneCase) || index >= AndroidLayoutCase;
}
constexpr Variant Variants[]{Variant::Elevated, Variant::Filled, Variant::Tonal, Variant::Outlined, Variant::Text};
constexpr const char* VariantLabels[]{"Elevated", "Filled", "Tonal", "Outlined", "Text"};
constexpr Choice ConnectedChoices[]{{"Day"}, {"Week"}, {"Month"}};
constexpr Choice MultiChoices[]{{"Bold"}, {"Italic"}, {"Underline", 0, true}};
constexpr Choice TabChoices[]{{"Home", icons::Home}, {"Search", icons::Search}, {"Saved", icons::Favorite, true}};

struct Samples {
    CatalogState catalog;
    bool buttonSelected[5]{true, false, true, false, true};
    bool iconSelected[5]{true, false, true, false, true};
    int connected = 1;
    bool multi[3]{true, false, false};
    bool connectedIcons[3]{false, true, false};
    int tabs[4]{0, 1, 0, 1};
    char fields[6][160]{"", "Ada", "1.50", "", "Read only", "First line\nSecond line"};
    CheckState checks[9]{CheckState::Unchecked, CheckState::Checked, CheckState::Mixed,
                        CheckState::Unchecked, CheckState::Checked, CheckState::Mixed,
                        CheckState::Unchecked, CheckState::Checked, CheckState::Checked};
    bool switches[9]{false, true, false, true, false, true, false, true, true};
    float slider[9]{35, 45, 50, 55, 60, 40, 50, 30, 75};
    bool menuCheck = true;
    bool menuRadio = false;
    bool fabExpanded = true;
    bool toolbar[3]{true, false, false};
    bool appBarScrolled = false;
    int destinations[3]{0, 1, 0};
    int draftCount = 0;
    float progress = .62f;
    int phoneDestination = 0, phoneNotes = 0;
    bool phoneFavorite = true;
    float phoneSlider = .62f, phoneNearEndSlider = .98f;
};

struct Canvas {
    const Style& style;
    ImVec2 origin;
    void At(float x, float y) const {
        ImGui::SetCursorScreenPos({origin.x + style.Px(x), origin.y + style.Px(y)});
    }
};

void ShowButtons(const Canvas& c, Samples& v, bool iconOnly) {
    const auto& s = c.style;
    constexpr float x[]{24, 160, 296, 432, 568};
    for (int i = 0; i < 5; ++i) {
        ImGui::PushID(i);
        ButtonOptions o;
        o.variant = Variants[i];
        c.At(x[i], 24);
        if (iconOnly)
            IconButton(s, "variant", icons::Favorite, o);
        else
            Button(s, VariantLabels[i], o);
        c.At(x[i], 80);
        o.shape = Shape::Square;
        o.icon = icons::Add;
        if (iconOnly)
            ToggleIconButton(s, "toggle", icons::Favorite, v.iconSelected[i], o,
                             i == 1 ? IconWidth::Narrow : i == 2 ? IconWidth::Wide : IconWidth::Default, icons::Check);
        else
            ToggleButton(s, "Toggle", v.buttonSelected[i], o);
        c.At(x[i], 136);
        o.disabled = true;
        o.shape = Shape::Round;
        o.icon = 0;
        if (iconOnly)
            IconButton(s, "disabled", icons::Favorite, o);
        else
            Button(s, "Disabled", o);
        ImGui::PopID();
    }
    constexpr float sizesX[]{24, 118, 228, 353, 520};
    for (int i = 0; i < 5; ++i) {
        c.At(sizesX[i], 208);
        ImGui::PushID(i);
        ButtonOptions o;
        o.variant = Variant::Tonal;
        o.size = static_cast<Size>(i);
        if (iconOnly)
            IconButton(s, "size", icons::Add, o);
        else {
            if (i == 2)
                o.trailingIcon = icons::ChevronRight;
            Button(s, "Go", o);
        }
        ImGui::PopID();
    }
}

void ShowConnected(const Canvas& c, Samples& v) {
    c.At(24, 24);
    ConnectedButtons(c.style, "single", ConnectedChoices, 3, v.connected);
    c.At(24, 80);
    ConnectedButtonsMulti(c.style, "multi", MultiChoices, 3, v.multi, {Size::Small, Variant::Filled});
    c.At(24, 150);
    ConnectedButtons(c.style, "medium", ConnectedChoices, 3, v.connected, {Size::Medium, Variant::Outlined});
    c.At(24, 234);
    ConnectedButtons(c.style, "large", ConnectedChoices, 3, v.connected, {Size::Large, Variant::Tonal});
}

void ShowConnectedIcons(const Canvas& c, Samples& v) {
    const Choice choices[]{{"Home", icons::Home}, {"Search", icons::Search}, {"Favorite", icons::Favorite}};
    c.At(24, 24);
    ConnectedButtonsMulti(c.style, "icons", choices, 3, v.connectedIcons,
                          {Size::Small, Variant::Tonal, Shape::Round, true});
    c.At(24, 94);
    ConnectedButtons(c.style, "iconsWide", choices, 3, v.connected,
                     {Size::Medium, Variant::Filled, Shape::Square, true, IconWidth::Wide});
}

void ShowTabs(const Canvas& c, Samples& v) {
    c.At(24, 24);
    Tabs(c.style, "primary", TabChoices, 3, v.tabs[0]);
    c.At(24, 112);
    Tabs(c.style, "secondary", TabChoices, 3, v.tabs[1], {TabVariant::Secondary});
    c.At(24, 188);
    Tabs(c.style, "top", TabChoices, 3, v.tabs[2], {TabVariant::Secondary, false, true});
    c.At(24, 264);
    Tabs(c.style, "stretched", TabChoices, 3, v.tabs[3], {TabVariant::Primary, true, true, 560});
}

void ShowMenu(const Canvas& c, Samples& v, int index) {
    const auto& s = c.style;
    c.At(24, 24);
    if (Button(s, "Actions", {Variant::Filled}))
        OpenMenu("actionsPopup");
    if (BeginMenu(s, "actionsPopup", {index == 5 ? MenuVariant::Vibrant : MenuVariant::Standard, 168})) {
        MenuItem(s, "Edit", {icons::Edit});
        if (BeginSubmenu(s, "More", icons::MoreVert)) {
            MenuItem(s, "Settings", {icons::Settings});
            if (MenuItem(s, "Pinned", {0, nullptr, MenuMark::Radio, v.menuRadio}))
                v.menuRadio = !v.menuRadio;
            EndMenu();
        }
        MenuDivider(s);
        if (MenuItem(s, "Favorite", {0, nullptr, MenuMark::Checkbox, v.menuCheck}))
            v.menuCheck = !v.menuCheck;
        MenuItem(s, "Delete", {icons::Delete, nullptr, MenuMark::None, false, true});
        EndMenu();
    }
}

void ShowFields(const Canvas& c, Samples& v) {
    FieldOptions o;
    c.At(24, 32);
    TextField(c.style, "Name", v.fields[0], sizeof(v.fields[0]), o);
    c.At(336, 32);
    o.variant = FieldVariant::Outlined;
    TextField(c.style, "Name##outlined", v.fields[1], sizeof(v.fields[1]), o);
    c.At(24, 120);
    o.leadingIcon = icons::Edit;
    o.trailingIcon = icons::Info;
    o.prefix = "$";
    o.suffix = "USD";
    o.supportingText = "Enter an amount";
    TextField(c.style, "Amount", v.fields[2], sizeof(v.fields[2]), o);
    c.At(336, 120);
    o = {};
    o.error = true;
    o.required = true;
    o.supportingText = "Contact is required";
    TextField(c.style, "Contact", v.fields[3], sizeof(v.fields[3]), o);
    c.At(24, 228);
    o = {};
    o.variant = FieldVariant::Outlined;
    o.disabled = true;
    TextField(c.style, "Status", v.fields[4], sizeof(v.fields[4]), o);
    c.At(336, 228);
    o = {};
    o.multilineHeight = 100;
    TextField(c.style, "Notes", v.fields[5], sizeof(v.fields[5]), o);
}

void ShowChecks(const Canvas& c, Samples& v) {
    for (int i = 0; i < 9; ++i) {
        ImGui::PushID(i);
        c.At(24.0f + 64 * (i % 3), 24.0f + 56 * (i / 3));
        Checkbox(c.style, "##check", v.checks[i], i >= 6, i >= 3 && i < 6);
        ImGui::PopID();
    }
    c.At(24, 196);
    Checkbox(c.style, "Notifications", v.checks[8]);
}

void ShowSwitches(const Canvas& c, Samples& v) {
    constexpr SwitchIcons iconMode[]{SwitchIcons::None, SwitchIcons::None, SwitchIcons::Both, SwitchIcons::Both,
                                     SwitchIcons::Unselected, SwitchIcons::Selected, SwitchIcons::None,
                                     SwitchIcons::Selected};
    for (int i = 0; i < 8; ++i) {
        ImGui::PushID(i);
        c.At(24.0f + 80 * (i % 4), 24.0f + 62 * (i / 4));
        Switch(c.style, "##switch", v.switches[i], iconMode[i], i >= 6);
        ImGui::PopID();
    }
    c.At(24, 152);
    Switch(c.style, "Bluetooth", v.switches[8], SwitchIcons::Selected);
}

void ShowSliders(const Canvas& c, Samples& v) {
    constexpr float ys[]{24, 76, 128, 188, 266};
    SliderOptions o;
    o.length = 240;
    for (int i = 0; i < 5; ++i) {
        ImGui::PushID(i);
        c.At(24, ys[i]);
        o.size = static_cast<Size>(i);
        Slider(c.style, "size", v.slider[i], 0, 100, o);
        ImGui::PopID();
    }
    c.At(304, 24);
    o.size = Size::Small;
    o.ticks = true;
    o.step = 10;
    Slider(c.style, "ticks", v.slider[5], 0, 100, o);
    c.At(304, 76);
    o.disabled = true;
    Slider(c.style, "disabled", v.slider[6], 0, 100, o);
    c.At(304, 128);
    o.disabled = false;
    RangeSlider(c.style, "range", v.slider[7], v.slider[8], 0, 100, o);
    c.At(598, 24);
    o.vertical = true;
    Slider(c.style, "vertical", v.slider[5], 0, 100, o);
}

void ShowDialog(const Canvas& c) {
    const auto& s = c.style;
    c.At(24, 24);
    if (Button(s, "Open dialog", {Variant::Filled}))
        OpenDialog("exampleDialog");
    if (BeginDialog(s, "exampleDialog", "Discard changes?", {320, true, true, 0, true})) {
        DialogText(s, "Your edits will be lost.");
        constexpr Choice actions[]{{"Cancel"}, {"Discard"}};
        if (DialogActions(s, actions, 2) >= 0)
            CloseDialog();
        EndDialog();
    }
    c.At(172, 24);
    if (Button(s, "Info dialog", {Variant::Outlined}))
        OpenDialog("infoDialog");
    if (BeginDialog(s, "infoDialog", "Appearance", {320, true, false, icons::Info})) {
        DialogText(s, "Your preferences have been saved.");
        constexpr Choice actions[]{{"Done"}};
        if (DialogActions(s, actions, 1) >= 0)
            CloseDialog();
        EndDialog();
    }
}

void ShowTooltip(const Canvas& c, int mode) {
    const auto& s = c.style;
    c.At(148, 60);
    const bool activated = IconButton(s, "settings", icons::Settings);
    if (mode == 0) {
        Tooltip(s, "Settings");
        c.At(250, 60);
        IconButton(s, "above", icons::Info);
        Tooltip(s, "Above", TooltipPlacement::Above);
        c.At(352, 60);
        IconButton(s, "left", icons::Favorite);
        Tooltip(s, "Left", TooltipPlacement::Left);
        c.At(454, 60);
        IconButton(s, "right", icons::Search);
        Tooltip(s, "Right", TooltipPlacement::Right);
    } else if (mode == 1 && BeginInteractiveTooltip(s, "Settings", activated)) {
        DialogText(s, "Change your appearance and preferences.");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + s.Px(8));
        if (Button(s, "Learn more"))
            ImGui::CloseCurrentPopup();
        EndInteractiveTooltip();
    } else if (mode == 2 && BeginRichTooltip(s, "Settings")) {
        DialogText(s, "Change your appearance and preferences.");
        EndRichTooltip();
    }
}

constexpr Choice Destinations[]{{"Library", icons::Home}, {"Explore", icons::Search}, {"Saved", icons::Favorite}};
constexpr Choice ToolbarActions[]{{"Favorite", icons::Favorite}, {"Filter", icons::Settings}, {"Edit", icons::Edit}};
constexpr Choice AppBarActions[]{{"Search", icons::Search}, {"More options", icons::MoreVert}};
constexpr Icon ArrowBack = 0xe5c4;

void ShowAndroidApp(const Canvas& c, Samples& v) {
    auto& s = c.style;
    const char* tasks[] = {"Library", "Explore", "Saved"};
    ImGui::GetWindowDrawList()->AddRectFilled({c.origin.x + s.Px(24), c.origin.y + s.Px(16)},
                                              {c.origin.x + s.Px(524), c.origin.y + s.Px(382)},
                                              s.theme[Role::Surface]);
    c.At(24, 16);
    const auto actions = android::TopAppBar(s, "reader bar", tasks[v.destinations[0]], AppBarActions, 2,
                                           {ArrowBack, "Return to library", nullptr, 500, false, true});
    if (actions.navigation)
        v.destinations[0] = 0;
    if (actions.action == 0)
        v.destinations[0] = 1;
    if (actions.action == 1)
        OpenMenu("reader options");
    if (BeginMenu(s, "reader options")) {
        if (MenuItem(s, "Saved items", {icons::Favorite}))
            v.destinations[0] = 2;
        EndMenu();
    }
    c.At(40, 108);
    ImGui::TextUnformatted("Reading progress");
    c.At(40, 142);
    android::LinearProgress(s, v.progress, true, 305);
    ImGui::SameLine(0, s.Px(12));
    ImGui::Text("%.0f%%", v.progress * 100);
    c.At(40, 184);
    if (Button(s, "Mark chapter read", {Variant::Tonal}))
        v.progress = std::min(1.0f, v.progress + .1f);
    c.At(40, 249);
    const int toolbar = android::FloatingToolbar(s, "reader tools", ToolbarActions, 3, v.toolbar,
                                                 {android::ToolbarColor::Vibrant});
    if (toolbar >= 0)
        v.toolbar[toolbar] = !v.toolbar[toolbar];
    c.At(346, 253);
    if (android::ExtendedFloatingActionButton(s, "New note", icons::Edit,
                                              {android::ExtendedFabSize::Regular, android::FabColor::Primary, v.fabExpanded}))
        ++v.draftCount;
    if (v.draftCount) {
        c.At(286, 218);
        ImGui::Text("Draft notes: %d", v.draftCount);
    }
    c.At(24, 318);
    android::ShortNavigationBar(s, "reader navigation", Destinations, 3, v.destinations[0], {500});
}

void ShowAndroidFabs(const Canvas& c, Samples& v) {
    auto& s = c.style;
    constexpr const char* labels[]{"Small", "Regular", "Medium", "Large"};
    constexpr android::FabColor colors[]{android::FabColor::Primary, android::FabColor::Secondary,
                                         android::FabColor::Tertiary, android::FabColor::Surface};
    for (int i = 0; i < 4; ++i) {
        const float x = 24.0f + 122 * i;
        c.At(x, 16);
        ImGui::TextUnformatted(labels[i]);
        c.At(x, 44);
        if (android::FloatingActionButton(s, labels[i], icons::Add, {static_cast<android::FabSize>(i), colors[i]}))
            ++v.draftCount;
    }
    c.At(512, 44);
    android::FloatingActionButton(s, "Disabled", icons::Edit, {android::FabSize::Regular, android::FabColor::Primary, true});
    c.At(512, 16);
    ImGui::TextUnformatted("Disabled");
    c.At(24, 169);
    Switch(s, "Expanded", v.fabExpanded);
    c.At(24, 235);
    if (android::ExtendedFloatingActionButton(s, "Compose", icons::Edit,
                                              {android::ExtendedFabSize::Regular, colors[0], v.fabExpanded}))
        ++v.draftCount;
    c.At(205, 235);
    if (android::ExtendedFloatingActionButton(s, "New note", icons::Add,
                                              {android::ExtendedFabSize::Medium, colors[1], v.fabExpanded}))
        ++v.draftCount;
    c.At(416, 235);
    if (android::ExtendedFloatingActionButton(s, "Add item", icons::Add,
                                              {android::ExtendedFabSize::Large, colors[2], v.fabExpanded}))
        ++v.draftCount;
    if (v.draftCount) {
        c.At(24, 349);
        ImGui::Text("Actions: %d", v.draftCount);
    }
}

void ShowAndroidNavigation(const Canvas& c, Samples& v) {
    auto& s = c.style;
    constexpr Choice available[]{{"Library", icons::Home}, {"Explore", icons::Search},
                                 {"Saved", icons::Favorite}, {"Settings", icons::Settings, true}};
    c.At(24, 16);
    ImGui::TextUnformatted("Leading icons with equal weights");
    c.At(24, 44);
    android::ShortNavigationBar(s, "equal", available, 4, v.destinations[0], {560});
    c.At(24, 133);
    ImGui::TextUnformatted("Leading icons centered");
    c.At(24, 161);
    android::ShortNavigationBar(s, "centered", Destinations, 3, v.destinations[1],
                                 {560, android::NavigationIconPosition::Start, android::NavigationArrangement::Centered});
    c.At(24, 250);
    ImGui::TextUnformatted("Icons above labels");
    c.At(24, 278);
    android::ShortNavigationBar(s, "top", Destinations, 3, v.destinations[2], {560, android::NavigationIconPosition::Top});
}

void ShowAndroidToolbars(const Canvas& c, Samples& v) {
    auto& s = c.style;
    constexpr Choice vertical[]{{"Favorite", icons::Favorite}, {"Filter", icons::Settings}, {"Edit", icons::Edit, true}};
    c.At(24, 16);
    ImGui::TextUnformatted("Standard");
    c.At(24, 44);
    auto activated = android::FloatingToolbar(s, "standard", ToolbarActions, 3, v.toolbar);
    if (activated >= 0)
        v.toolbar[activated] = !v.toolbar[activated];
    c.At(24, 139);
    ImGui::TextUnformatted("Vibrant");
    c.At(24, 167);
    activated = android::FloatingToolbar(s, "vibrant", ToolbarActions, 3, v.toolbar, {android::ToolbarColor::Vibrant});
    if (activated >= 0)
        v.toolbar[activated] = !v.toolbar[activated];
    c.At(258, 16);
    ImGui::TextUnformatted("Vertical");
    c.At(258, 44);
    activated = android::FloatingToolbar(s, "vertical", vertical, 3, v.toolbar, {android::ToolbarColor::Vibrant, true});
    if (activated >= 0)
        v.toolbar[activated] = !v.toolbar[activated];
}

void ShowAndroidAppBars(const Canvas& c, Samples& v) {
    auto& s = c.style;
    c.At(24, 28);
    const auto first = android::TopAppBar(s, "standard app bar", "Your library", AppBarActions, 2,
                                          {ArrowBack, "Back", nullptr, 520, false, v.appBarScrolled});
    if (first.navigation)
        v.appBarScrolled = false;
    if (first.action == 0)
        v.appBarScrolled = true;
    if (first.action == 1)
        OpenMenu("app bar overflow");
    if (BeginMenu(s, "app bar overflow")) {
        if (MenuItem(s, "Toggle scrolled color"))
            v.appBarScrolled = !v.appBarScrolled;
        EndMenu();
    }
    c.At(24, 122);
    android::TopAppBar(s, "centered app bar", "Discover", AppBarActions, 1,
                        {ArrowBack, "Back", "Recommendations", 520, true, true});
    c.At(24, 220);
    Switch(s, "Scrolled container color", v.appBarScrolled);
}

void ShowAndroidProgress(const Canvas& c, Samples& v) {
    auto& s = c.style;
    c.At(24, 21);
    ImGui::TextUnformatted("Determinate");
    c.At(325, 21);
    ImGui::TextUnformatted("Indeterminate");
    c.At(24, 62);
    android::LinearProgress(s, v.progress);
    c.At(325, 62);
    android::LinearProgress(s, std::nullopt);
    c.At(24, 104);
    android::LinearProgress(s, v.progress, true);
    c.At(325, 104);
    android::LinearProgress(s, std::nullopt, true);
    c.At(24, 152);
    android::CircularProgress(s, v.progress);
    c.At(86, 148);
    android::CircularProgress(s, v.progress, true);
    c.At(325, 152);
    android::CircularProgress(s, std::nullopt);
    c.At(387, 148);
    android::CircularProgress(s, std::nullopt, true);
    c.At(24, 244);
    Slider(s, "Progress", v.progress, 0, 1,
           {Size::ExtraSmall, 240, .01f, false, false, false, false, 2, SliderTrackStyle::Android});
    ImGui::SameLine(0, s.Px(12));
    ImGui::Text("%.0f%%", v.progress * 100);
}

void ShowPhoneComparison(const Canvas& c, Samples& v) {
    const auto& s = c.style;
    constexpr Choice actions[]{{"Search", icons::Search}};
    c.At(16, 16);
    const auto bar = android::TopAppBar(s, "phone header", Destinations[v.phoneDestination].label, actions, 1,
                                        {ArrowBack, "Back", nullptr, 328});
    if (bar.navigation || bar.action == 0)
        v.phoneDestination = bar.navigation ? 0 : 1;
    c.At(16, 104);
    android::ShortNavigationBar(s, "phone navigation", Destinations, 3, v.phoneDestination, {328});
    c.At(16, 192);
    const bool toggles[]{v.phoneFavorite, false, false};
    const auto toolbar = android::FloatingToolbar(s, "phone toolbar", ToolbarActions, 3, toggles,
                                                   {android::ToolbarColor::Vibrant});
    if (toolbar == 0)
        v.phoneFavorite = !v.phoneFavorite;
    if (toolbar == 1)
        v.phoneDestination = 1;
    if (toolbar == 2)
        ++v.phoneNotes;

    c.At(16, 284);
    v.phoneNotes += android::FloatingActionButton(s, "Small add", icons::Add, {android::FabSize::Small});
    c.At(80, 280);
    v.phoneNotes += android::FloatingActionButton(s, "Add", icons::Add, {android::FabSize::Regular, android::FabColor::Secondary});
    c.At(152, 268);
    v.phoneNotes += android::FloatingActionButton(s, "Medium add", icons::Add, {android::FabSize::Medium, android::FabColor::Tertiary});
    c.At(248, 260);
    v.phoneNotes += android::FloatingActionButton(s, "Large add", icons::Add, {android::FabSize::Large, android::FabColor::Surface});
    c.At(16, 380);
    v.phoneNotes += android::ExtendedFloatingActionButton(s, "New note", icons::Edit);
    if (v.phoneNotes) {
        c.At(175, 399);
        ImGui::Text("Draft notes: %d", v.phoneNotes);
    }
    c.At(16, 482);
    android::LinearProgress(s, .62f);
    c.At(16, 514);
    android::LinearProgress(s, .62f, true);
    c.At(278, 478);
    android::CircularProgress(s, .62f);
    c.At(274, 520);
    android::CircularProgress(s, .62f, true);
    const SliderOptions slider{Size::ExtraSmall, 240, .01f, false, false, false, false, 2, SliderTrackStyle::Android};
    c.At(16, 580);
    Slider(s, "Normal progress", v.phoneSlider, 0, 1, slider);
    c.At(268, 592);
    ImGui::Text("%.0f%%", v.phoneSlider * 100);
    c.At(16, 636);
    Slider(s, "Near-end progress", v.phoneNearEndSlider, 0, 1, slider);
    c.At(268, 648);
    ImGui::Text("%.0f%%", v.phoneNearEndSlider * 100);
}

void ShowCase(const Canvas& c, Samples& values, int index) {
    if (index >= FirstCatalogCase) {
        ShowCatalog(c.style, c.origin, values.catalog, index - FirstCatalogCase);
        c.At((index == PhoneCatalogCase ? PhoneWidth : Width) - 1,
             (index == PhoneCatalogCase ? PhoneHeight : CatalogHeight) - 1);
        ImGui::Dummy({c.style.Px(1), c.style.Px(1)});
        return;
    }
    switch (index) {
        case 0: ShowButtons(c, values, false); break;
        case 1: ShowButtons(c, values, true); break;
        case 2: ShowConnected(c, values); break;
        case 3: ShowTabs(c, values); break;
        case 4: case 5: case 6: ShowMenu(c, values, index); break;
        case 7: ShowFields(c, values); break;
        case 8: ShowChecks(c, values); break;
        case 9: ShowSwitches(c, values); break;
        case 10: ShowSliders(c, values); break;
        case 11: ShowDialog(c); break;
        case 12: case 13: ShowTooltip(c, index - 12); break;
        case 14: ShowConnectedIcons(c, values); break;
        case 15: ShowTooltip(c, 2); break;
        case 16: ShowAndroidApp(c, values); break;
        case 17: ShowAndroidFabs(c, values); break;
        case 18: ShowAndroidNavigation(c, values); break;
        case 19: ShowAndroidToolbars(c, values); break;
        case 20: ShowAndroidAppBars(c, values); break;
        case 21: ShowAndroidProgress(c, values); break;
        case PhoneCase: ShowPhoneComparison(c, values); break;
    }
    c.At((index == PhoneCase ? PhoneWidth : Width) - 1, (index == PhoneCase ? PhoneHeight : Height) - 1);
    ImGui::Dummy({c.style.Px(1), c.style.Px(1)});
}

void OpenCanvas(const Style& s, const char* name, ImGuiWindowFlags extraFlags = 0) {
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, s.theme[Role::SurfaceContainerLowest]);
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                               ImGuiWindowFlags_NoMove | extraFlags);
}

void CloseCanvas() {
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

struct Gesture {
    float x, y, x2, y2;
    bool clicks;
};

Gesture CaseGesture(int index) {
    constexpr Gesture gestures[]{
        {56, 44, 56, 44, true}, {44, 44, 44, 44, true}, {130, 100, 130, 100, true},
        {143, 54, 143, 54, true}, {60, 44, 748, 378, true}, {60, 44, 748, 378, true},
        {60, 44, 70, 136, true}, {80, 59, 80, 59, true}, {44, 44, 44, 44, true},
        {44, 40, 44, 40, true}, {111, 46, 196, 46, true}, {75, 44, 748, 378, true},
        {168, 80, 168, 80, false}, {168, 80, 748, 378, true},
        {44, 44, 44, 44, true}, {168, 80, 168, 80, false},
        {430, 355, 430, 355, true}, {44, 184, 44, 184, true}, {231, 70, 231, 70, true},
        {106, 76, 106, 76, true}, {477, 58, 477, 58, true}, {150, 266, 150, 266, true},
        {180, 136, 180, 136, true}
    };
    static_assert(std::size(gestures) == FirstCatalogCase);
    if (index < FirstCatalogCase)
        return gestures[index];
    constexpr Gesture catalogGestures[]{
        {160, 77, 160, 77, true}, {140, 80, 140, 80, true}, {283, 72, 283, 72, true},
        {66, 333, 66, 333, true}, {400, 73, 400, 73, true}, {566, 276, 566, 276, true},
        {109, 78, 109, 78, true}, {748, 778, 748, 778, false}, {200, 74, 200, 74, true},
        {145, 39, 145, 39, true}
    };
    static_assert(std::size(catalogGestures) == Names.size() - FirstCatalogCase);
    return catalogGestures[index - FirstCatalogCase];
}

bool Capture(Window& window, const Style& s, const std::filesystem::path& directory, int index, bool motion) {
    std::filesystem::create_directories(directory);
    auto& io = ImGui::GetIO();
    Samples values;
    const auto g = CaseGesture(index);
    const bool overlay = (index >= 4 && index <= 6) || (index >= 11 && index <= 13) || index == 15;
    constexpr int warmup = 16;
    const int last = motion ? 59 : 68;
    for (int i = -warmup; i <= last; ++i) {
        if (i == -warmup)
            io.AddMousePosEvent(s.Px(index == PhoneCase || index == PhoneCatalogCase ? 355 : 748), s.Px(378));
        if ((motion || overlay) && i == 0)
            io.AddMousePosEvent(s.Px(g.x), s.Px(g.y));
        if ((motion || overlay) && g.clicks && i == 6)
            io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        if (motion && index == 10 && i == 12)
            io.AddMousePosEvent(s.Px(g.x2), s.Px(g.y2));
        if ((motion || overlay) && g.clicks && i == 15)
            io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        if (((motion && index != 10) || overlay) && i == 20)
            io.AddMousePosEvent(s.Px(g.x2), s.Px(g.y2));
        window.BeginFrame(true, Step);
        OpenCanvas(s, "##parity", ImGuiWindowFlags_NoScrollbar);
        if (index >= FirstAndroidCase) {
            ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
            ImGui::PushFont(s.fonts.regular, s.Px(16) * s.fonts.textEmScale);
        }
        ShowCase({s, {}}, values, index);
        if (index >= FirstAndroidCase) {
            ImGui::PopFont();
            ImGui::PopStyleColor();
        }
        CloseCanvas();
        std::string file;
        if (motion && i >= 0 && i % 5 == 0) {
            char filename[32];
            std::snprintf(filename, sizeof(filename), "frame-%02d.bmp", i / 5);
            file = (directory / filename).string();
        } else if (!motion && i == last) {
            file = (directory / "native.bmp").string();
        }
        if (!window.Render(file))
            return false;
    }
    return true;
}

struct GalleryState {
    Samples values;
    int index = FirstAndroidCase;
    bool dark = false, webExpressive = false, androidExpressive = true;
};

void DrawGallery(Style& s, GalleryState& state) {
    auto& index = state.index;
    auto& dark = state.dark;
    auto& androidExpressive = state.androidExpressive;
    auto& webExpressive = state.webExpressive;
    auto applyTheme = [&] {
        const auto scheme = dark ? Scheme::Dark : Scheme::Light;
        const bool androidPage = IsAndroidPage(index);
        s.theme = androidPage ? android::DefaultTheme(scheme) : Theme::TonalSpot(scheme);
        s.motion = (androidPage ? androidExpressive : webExpressive) ? MotionPreset::Expressive : MotionPreset::Standard;
    };
    applyTheme();
    OpenCanvas(s, "##gallery", ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, s.theme[Role::SurfaceContainer]);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, s.theme[Role::OutlineVariant]);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, s.theme[Role::Outline]);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, s.theme[Role::Secondary]);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, s.Px(12));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, s.Px(6));
    ImGui::PushFont(s.fonts.regular, s.Px(16) * s.fonts.textEmScale);
    const float rail = s.Px(196);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, s.theme[Role::SurfaceContainerLow]);
    ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
    ImGui::BeginChild("Navigation", {rail, 0}, ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::SetCursorPos({s.Px(16), s.Px(20)});
    ImGui::TextUnformatted("Android-inspired");
    ImGui::Dummy({0, s.Px(8)});
    for (int row = 0; row < static_cast<int>(Names.size()); ++row) {
        const int initialAndroidCount = PhoneCase - FirstAndroidCase + 1;
        const int androidCount = initialAndroidCount + 2;
        const int catalogCount = AndroidLayoutCase - FirstCatalogCase;
        const int i = row < initialAndroidCount ? FirstAndroidCase + row
                    : row < androidCount ? AndroidLayoutCase + row - initialAndroidCount
                    : row < androidCount + catalogCount ? FirstCatalogCase + row - androidCount
                    : row - androidCount - catalogCount;
        if (i == FirstCatalogCase) {
            ImGui::Dummy({0, s.Px(8)});
            ImGui::SetCursorPosX(s.Px(16));
            ImGui::TextUnformatted("More components");
        }
        if (i == 0) {
            ImGui::Dummy({0, s.Px(8)});
            ImGui::SetCursorPosX(s.Px(16));
            ImGui::TextUnformatted("Matraic reference");
        }
        ImGui::SetCursorPosX(s.Px(12));
        ButtonOptions o;
        o.variant = i == index ? Variant::Tonal : Variant::Text;
        o.size = Size::ExtraSmall;
        if (Button(s, Titles[i], o))
            index = i;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    applyTheme();
    ImGui::SameLine(0, 0);
    ImGui::SetNextWindowContentSize({s.Px(Width), 0});
    ImGui::PushStyleColor(ImGuiCol_Text, s.theme[Role::OnSurface]);
    ImGui::BeginChild("Content", {0, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SetCursorPos({s.Px(24), s.Px(20)});
    ImGui::PushFont(s.fonts.regular, s.Px(28) * s.fonts.textEmScale);
    ImGui::TextUnformatted(Titles[index]);
    ImGui::PopFont();
    ImGui::SetCursorPosX(s.Px(24));
    if (Switch(s, "Dark theme", dark))
        applyTheme();
    ImGui::SameLine();
    Switch(s, "Reduce motion", s.reducedMotion);
    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(s.Px(8), s.Px(8)));
    ImGui::SetNextItemWidth(s.Px(130));
    ImGui::SliderFloat("Density", &s.density, -3, 0, "%.0f");
    ImGui::PopStyleVar();
    ImGui::SetCursorPosX(s.Px(24));
    if (Checkbox(s, "Expressive motion", IsAndroidPage(index) ? androidExpressive : webExpressive))
        applyTheme();
    ImGui::Dummy({0, s.Px(12)});
    const auto origin = ImGui::GetCursorScreenPos();
    ImGui::PushID(index);
    ShowCase({s, origin}, state.values, index);
    ImGui::PopID();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);
    CloseCanvas();
}

void Interactive(Window& window, Style& s) {
    GalleryState state;
    while (window.Poll()) {
        s.scale = window.DpiScale();
        window.BeginFrame(false, 0);
        DrawGallery(s, state);
        if (!window.Render())
            break;
    }
}

bool CaptureGallery(Window& window, Style& s, bool dark, const std::filesystem::path& directory) {
    std::filesystem::create_directories(directory);
    GalleryState state;
    state.dark = dark;
    state.index = 21;
    for (int frame = 0; frame < 4; ++frame) {
        window.BeginFrame(true, Step);
        DrawGallery(s, state);
        if (!window.Render(frame == 3 ? (directory / "native.bmp").string() : std::string{}))
            return false;
    }
    return true;
}

} // namespace

int main(int argc, char** argv) {
    const bool capture = argc >= 5 && std::string(argv[1]) == "--capture";
    const bool galleryCapture = capture && std::string(argv[2]) == "gallery-shell";
    const auto find = capture ? std::find_if(Names.begin(), Names.end(), [&](const char* candidate) {
        return std::string(candidate) == argv[2];
    }) : Names.end();
    if (capture && find == Names.end() && !galleryCapture) {
        std::fprintf(stderr, "Unknown gallery comparison case\n");
        return 2;
    }
    m3e::Style style;
    bool motion = false;
    float fontScale = 1.0f;
    if (capture) {
        const auto scheme = std::string(argv[3]) == "dark" ? m3e::Scheme::Dark : m3e::Scheme::Light;
        const bool androidPage = galleryCapture || IsAndroidPage(static_cast<int>(find - Names.begin()));
        style.theme = androidPage ? m3e::android::DefaultTheme(scheme) : m3e::Theme::TonalSpot(scheme);
        if (androidPage)
            style.motion = m3e::MotionPreset::Expressive;
        for (int i = 5; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--motion")
                motion = true;
            else if (option == "--reduced-motion")
                style.reducedMotion = true;
            else if (option.rfind("--scale=", 0) == 0)
                style.scale = std::stof(option.substr(8));
            else if (option.rfind("--font-scale=", 0) == 0)
                fontScale = std::stof(option.substr(13));
            else if (option.rfind("--density=", 0) == 0)
                style.density = std::stof(option.substr(10));
        }
        if (!(style.scale >= .5f && style.scale <= 4) || !(fontScale > 0 && fontScale <= 4)
            || !std::isfinite(style.density))
            return 2;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    const auto fonts = std::filesystem::absolute(argv[0]).parent_path() / "fonts";
    const bool googleSans = !capture || galleryCapture || IsAndroidPage(static_cast<int>(find - Names.begin()));
    style.fonts = m3e::LoadFonts(fonts.string().c_str(), googleSans ? Typeface::GoogleSansFlex : Typeface::Roboto);
    style.fonts.textEmScale *= fontScale;
    int result = 0;
    if (!style.fonts.regular || !style.fonts.medium || !style.fonts.symbols || !style.fonts.symbolsFilled) {
        std::fprintf(stderr, "Gallery font files are missing from %s\n", fonts.string().c_str());
        result = 1;
    } else {
        const auto index = find - Names.begin();
        const bool phone = index == PhoneCase || index == PhoneCatalogCase;
        const int width = galleryCapture ? 1080 : phone ? PhoneWidth : Width;
        const int height = galleryCapture ? 540 : phone ? PhoneHeight
                         : find - Names.begin() >= FirstCatalogCase ? CatalogHeight : Height;
        Window window(capture ? static_cast<int>(std::lround(width * style.scale)) : 1080,
                      capture ? static_cast<int>(std::lround(height * style.scale)) : 720, capture);
        if (!window.Valid()) {
            std::fprintf(stderr, "Could not initialize the Win32/DX11 gallery\n");
            result = 1;
        } else if (galleryCapture) {
            result = CaptureGallery(window, style, std::string(argv[3]) == "dark", argv[4]) ? 0 : 1;
        } else if (capture) {
            result = Capture(window, style, argv[4], static_cast<int>(find - Names.begin()), motion) ? 0 : 1;
        } else {
            Interactive(window, style);
        }
    }
    ImGui::DestroyContext();
    return result;
}
