#include "catalog.hpp"
#include <m3e/android.hpp>

namespace {
using namespace m3e;
constexpr Choice Files[]{{"Notes", icons::Edit}, {"Library", icons::Home}, {"Saved", icons::Favorite},
                         {"Archive", 0xe149}, {"Shared", 0xe80d, true}};
constexpr Choice Actions[]{{"Search", icons::Search}, {"Settings", icons::Settings}, {"Save", icons::Check}};
constexpr Choice Segments[]{{"Today"}, {"Week"}, {"Month"}};
constexpr ListEntry Rows[]{{"Today's notes", "Two items edited today", icons::Edit, icons::ChevronRight},
                           {"Favorites", "Pinned to your library", icons::Favorite, icons::ChevronRight},
                           {"Archived", nullptr, 0xe149, 0, true}};

void FeedBack(const Style& s, CatalogState& v, const char* message = "Action completed") {
    if (Snackbar(s, "status", v.snackbar, message, "Undo") == SnackbarResult::Action)
        v.dismissed = false;
}
} // namespace

void ShowCatalog(const m3e::Style& s, ImVec2 origin, CatalogState& v, int page) {
    using namespace m3e;
    const auto at = [&](float x, float y) {
        ImGui::SetCursorScreenPos({origin.x + s.Px(x), origin.y + s.Px(y)});
    };
    switch (page) {
    case 0:
        at(24, 16); Heading(s, "Chips", Size::Small);
        at(24, 52); Chip(s, "Assist", {ChipKind::Assist, icons::Info});
        ImGui::SameLine();
        if (Chip(s, "Filter", {ChipKind::Filter, 0, v.filter}) == ChipAction::Activate)
            v.filter = !v.filter;
        ImGui::SameLine(); Chip(s, "Suggestion", {ChipKind::Suggestion, 0, false, true});
        ImGui::SameLine(); Chip(s, "Disabled", {ChipKind::Assist, icons::Info, false, false, true});
        at(24, 110);
        if (v.token) {
            if (Chip(s, "Saved tag", {ChipKind::Input, icons::Favorite, false, false, false, true}) == ChipAction::Remove)
                v.token = false;
        } else if (Button(s, "Restore tag", {Variant::Text})) {
            v.token = true;
        }
        at(24, 185); Heading(s, "Radio group", Size::Small);
        at(24, 222); RadioGroup(s, "Destination", Files, 5, v.radio, true);
        at(24, 306); Heading(s, "Single selection", Size::Small);
        at(24, 342); SegmentedButtons(s, "Period", Segments, 3, v.segmented);
        at(24, 414); Heading(s, "Multiple selection", Size::Small);
        at(24, 450); SegmentedButtonsMulti(s, "Periods", Segments, 3, v.segments, false);
        at(24, 530); Heading(s, "Standard button group", Size::Small);
        at(24, 566); ButtonGroup(s, "Actions", Actions, 3, {Variant::Filled});
        break;
    case 1:
        at(24, 16); Heading(s, "Autocomplete", Size::Small);
        at(24, 52); Autocomplete(s, "Find a destination", v.query[0], sizeof(v.query[0]), Files, 5, 320);
        at(382, 16); Heading(s, "Docked search", Size::Small);
        at(382, 52); SearchBar(s, "Search notes", v.query[1], sizeof(v.query[1]), Files, 5);
        at(24, 288); Heading(s, "Select", Size::Small);
        at(24, 324); Select(s, "Destination", Files, 5, v.selected);
        at(382, 288); Heading(s, "Auto-growing textarea", Size::Small);
        at(382, 324); AutoTextarea(s, "Notes", v.notes, sizeof(v.notes));
        at(24, 500); Heading(s, "Expanded search", Size::Small);
        at(24, 536); SearchBar(s, "Search your library", v.query[2], sizeof(v.query[2]), Files, 5,
                              {320, SearchPresentation::Expanded});
        break;
    case 2:
        at(24, 16); Heading(s, "Calendar", Size::Small);
        at(24, 52); Calendar(s, "Calendar", v.months[0], v.dates[0]);
        at(382, 16); Heading(s, "Date and date-range pickers", Size::Small);
        at(382, 52); DatePicker(s, "Pick a date", v.months[1], v.dates[1]);
        at(382, 130); DateRangePicker(s, "Pick a date range", v.months[2], v.range);
        at(382, 226); DateInput(s, "Type a date", v.dates[2], v.dateText, sizeof(v.dateText));
        at(24, 380); TimePicker(s, "Clock dial", v.times[0]);
        at(382, 380); TimePicker(s, "Time input (24-hour)", v.times[1], TimePickerMode::Input, true);
        at(382, 490); TimePicker(s, "Scrollable time input", v.times[2], TimePickerMode::Scroll);
        break;
    case 3:
        at(24, 16); Heading(s, "Cards", Size::Small);
        for (int i = 0; i < 3; ++i) {
            at(24.0f + 230.0f * i, 52);
            ImGui::PushID(i);
            if (BeginCard(s, "card", static_cast<CardVariant>(i), 208, 128)) {
                const char* titles[]{"Filled", "Outlined", "Elevated"};
                Heading(s, titles[i], Size::Small);
                ImGui::TextUnformatted("Notebook preview");
                if (Button(s, "Open", {Variant::Text}))
                    ShowSnackbar(v.snackbar);
            }
            EndCard();
            ImGui::PopID();
        }
        at(24, 220);
        if (BeginContentPane(s, "notes", 324, 200)) {
            Heading(s, "Content pane", Size::Small);
            Divider(s);
            if (ExpansionPanel(s, "Details", v.expansion, icons::Info))
                ImGui::TextWrapped("The expansion panel shows additional information.");
        }
        EndContentPane();
        at(382, 218); Heading(s, "Segmented list", Size::Small);
        at(382, 254); List(s, "list", Rows, 3, v.list, 320, true);
        at(24, 473); Checkbox(s, "Show drawer", v.drawer);
        at(24, 521);
        if (BeginDrawerLayout(s, "drawer layout", v.drawer, 224, 230))
            NavigationMenu(s, "drawer", Files, 3, v.navigation, 0);
        NextDrawerPane(s);
        Heading(s, Files[v.navigation].label, Size::Small);
        ImGui::TextUnformatted("Drawer content");
        EndDrawerLayout();
        FeedBack(s, v, "Notebook opened");
        break;
    case 4: {
        at(24, 16); Heading(s, "Navigation rail", Size::Small);
        at(24, 52); NavigationRail(s, "rail", Files, 3, v.rail);
        at(154, 16); Heading(s, "Table of contents", Size::Small);
        at(154, 52); TableOfContents(s, "contents", Files, 3, v.toc, 180);
        at(382, 16); Heading(s, "Tree", Size::Small);
        constexpr TreeEntry nodes[]{{"Library", 0, icons::Home}, {"Notes", 1, icons::Edit},
                                    {"Drafts", 2}, {"Recipes", 2}, {"Saved", 1, icons::Favorite}, {"Shared", 0, 0, true}};
        at(382, 52); Tree(s, "library tree", nodes, 6, v.tree, v.branches);
        at(24, 319); Heading(s, "Breadcrumbs", Size::Small);
        at(24, 352); Breadcrumbs(s, "path", Files, 3);
        at(24, 440); Heading(s, "Paginator", Size::Small);
        at(24, 476); Paginator(s, "pages", v.page, 8);
        at(382, 440); Heading(s, "Stepper", Size::Small);
        at(382, 476); Stepper(s, "Steps", Segments, 3, v.step);
        break;
    }
    case 5: {
        const SplitPaneOptions horizontal{680, 160, 96, false};
        const SplitPaneOptions vertical{304, 176, 48, true};
        at(24, 16); Heading(s, "Split panes", Size::Small);
        at(24, 52); BeginSplitPane(s, "Horizontal", v.split, horizontal);
        Heading(s, "Left");
        NextSplitPane(s, v.split, horizontal);
        Heading(s, "Right");
        EndSplitPane();
        at(24, 246); BeginSplitPane(s, "Vertical", v.vertical, vertical);
        Heading(s, "Top");
        NextSplitPane(s, v.vertical, vertical);
        Heading(s, "Bottom");
        EndSplitPane();
        at(382, 246);
        if (BeginCarousel(s, "Carousel", v.carousel, 3, {316, 176, true})) {
            Heading(s, Segments[v.carousel].label);
            ImGui::TextUnformatted("Drag to change slides.");
        }
        EndCarousel();
        at(24, 458); Heading(s, "Slide group", Size::Small);
        at(24, 490); SlideGroup(s, "slides", Files, 5, v.slide, 290);
        break;
    }
    case 6: {
        constexpr SheetOptions persistent{SheetMode::Persistent, 380, 213, true};
        at(24, 16); Heading(s, "Sheets", Size::Small);
        at(24, 52);
        if (Button(s, "Open modal sheet", {Variant::Filled})) {
            v.persistent = false;
            v.modal = true;
        }
        ImGui::SameLine();
        if (Button(s, "Toggle persistent sheet", {Variant::Tonal}))
            v.persistent = !v.persistent;
        if (BeginBottomSheet(s, "Modal", v.modal)) {
            Heading(s, "Move note");
            ImGui::TextUnformatted("Choose a destination, or drag the handle down to close.");
            if (NavigationMenu(s, "destinations", Files, 3, v.navigation, 0) >= 0)
                v.modal = false;
            EndBottomSheet();
        }
        if (BeginBottomSheet(s, "Persistent", v.persistent, persistent)) {
            Heading(s, "Note details");
            ImGui::TextUnformatted("You can still use the gallery behind this sheet.");
            if (Button(s, "Close", {Variant::Text}))
                v.persistent = false;
            EndBottomSheet(persistent);
        }
        at(24, 139); Heading(s, "Split button and FAB menu", Size::Small);
        at(24, 175);
        const auto split = SplitButton(s, "Save", Actions, 3, {Variant::Filled});
        ImGui::SameLine(0, s.Px(32));
        if (FloatingActionButtonMenu(s, "Create", icons::Add, Actions, 3) >= 0 || split.primary || split.secondary >= 0)
            ShowSnackbar(v.snackbar);
        at(24, 272); Heading(s, "Toolbar", Size::Small);
        at(24, 307); Toolbar(s, "Standard", Actions, 3);
        at(24, 391); Toolbar(s, "Vibrant", Actions, 3, {true, false, true, true});
        at(382, 307); Toolbar(s, "Vertical", Actions, 3, {false, true, true, false});
        at(24, 510);
        if (Button(s, "Show snackbar", {Variant::Tonal}))
            ShowSnackbar(v.snackbar, 0);
        FeedBack(s, v, "Saved to your library");
        break;
    }
    case 7:
        at(24, 16); Heading(s, "Avatar and badge", Size::Small);
        at(24, 52); Avatar(s, "AB");
        ImGui::SameLine(); Avatar(s, nullptr, 40, 0xe7fd);
        ImGui::SameLine(); Badge(s, "12");
        ImGui::SameLine(); Badge(s);
        at(24, 130); Callout(s, "Information", "Notes sync across your devices.", MessageTone::Information, 320);
        at(382, 130); Callout(s, "Saved", "Your changes are up to date.", MessageTone::Success, 320);
        at(24, 218); Callout(s, "Offline", "These changes will sync later.", MessageTone::Warning, 320);
        at(382, 218); Callout(s, "Could not sync", "Check your connection.", MessageTone::Error, 320);
        at(24, 328); Heading(s, "Shapes", Size::Small);
        for (int i = 0; i < static_cast<int>(ExpressiveShape::Count); ++i) {
            at(24.0f + 55.0f * (i % 12), 364.0f + 55.0f * (i / 12));
            const auto shape = static_cast<ExpressiveShape>(i);
            ShapePreview(s, shape, 42);
            Tooltip(s, ShapeName(shape));
        }
        at(24, 548); Heading(s, "Loading indicators", Size::Small);
        at(24, 584); LoadingIndicator(s);
        ImGui::SameLine(); LoadingIndicator(s, .65f, true);
        at(382, 548); Heading(s, "Skeleton", Size::Small);
        at(382, 584); Skeleton(s, 44, 44, 22);
        ImGui::SameLine(); Skeleton(s, 180, 14);
        at(435, 608); Skeleton(s, 120, 10);
        at(24, 654); Divider(s, 678);
        at(24, 674); Heading(s, "Shape morph", Size::Small);
        at(24, 710); Slider(s, "Morph", v.morph, 0, 1, {Size::ExtraSmall, 240, .01f});
        at(290, 709); ShapeMorph(s, ExpressiveShape::Circle, ExpressiveShape::SoftBurst, v.morph);
        break;
    case 8: {
        using namespace m3e::android;
        at(24, 16); Heading(s, "Collapsing app bars", Size::Small);
        at(24, 52); Slider(s, "Collapse", v.collapse, 0, 1, {Size::ExtraSmall, 250, .01f});
        at(24, 109); CollapsingTopAppBar(s, "Medium", "Medium", Actions, 3, {AppBarSize::Medium, v.collapse, 328});
        at(382, 109); CollapsingTopAppBar(s, "Large", "Large", Actions, 3, {AppBarSize::Large, v.collapse, 328});
        at(24, 282); Heading(s, "Scaffold", Size::Small);
        at(24, 318);
        if (BeginScaffold(s, "screen", 64, 80, 360, 330))
            TopAppBar(s, "Library", "Library", Actions, 3, {});
        if (NextScaffoldContent(s)) {
            const bool refreshing = ImGui::GetTime() < v.refreshedUntil;
            const bool activated = ListItem(s, "Refresh content", {"Drag down to refresh", "Or focus and press Enter", 0xe5d5});
            v.pulling |= ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            const bool released = v.pulling && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
            const float pull = v.pulling ? ImGui::GetMouseDragDelta().y / s.scale : 0;
            if (PullToRefresh(s, "Refresh", pull, released, refreshing) || (activated && !released))
                v.refreshedUntil = ImGui::GetTime() + 1.5;
            if (released)
                v.pulling = false;
            if (!v.dismissed && SwipeToDismiss(s, "swipe", {"Swipe this note", "Or press Delete when focused", icons::Edit})) {
                v.dismissed = true;
                ShowSnackbar(v.snackbar, 0);
            }
        }
        if (NextScaffoldBottom(s))
            BottomAppBar(s, "Bottom actions", Actions, 3);
        EndScaffold();
        at(416, 282); Heading(s, "Navigation bar", Size::Small);
        at(416, 318); NavigationBar(s, "Standard navigation", Files, 3, v.navigation, 304);
        at(416, 440);
        if (Button(s, "Open modal drawer", {Variant::Tonal}))
            v.drawerModal = true;
        if (BeginModalNavigationDrawer(s, "Android navigation drawer", v.drawerModal)) {
            Heading(s, "Library");
            if (NavigationMenu(s, "Drawer actions", Files, 3, v.navigation, 0) >= 0)
                v.drawerModal = false;
            EndModalNavigationDrawer();
        }
        FeedBack(s, v, "Note removed");
        break;
    }
    case 9: {
        using namespace m3e::android;
        at(16, 16); Chip(s, "Assist", {ChipKind::Assist, icons::Info});
        at(116, 16);
        if (Chip(s, "Filter", {ChipKind::Filter, 0, v.filter}) == ChipAction::Activate)
            v.filter = !v.filter;
        at(220, 16); Chip(s, "Suggest", {ChipKind::Suggestion});
        at(16, 88); SegmentedButtons(s, "Range", Segments, 3, v.segmented);
        at(16, 154);
        if (BeginCard(s, "Filled", CardVariant::Filled, 156, 104)) {
            Heading(s, "Notebook", Size::Small);
            ImGui::TextUnformatted("Filled card");
        }
        EndCard();
        at(188, 154);
        if (BeginCard(s, "Outlined", CardVariant::Outlined, 156, 104)) {
            Heading(s, "Library", Size::Small);
            ImGui::TextUnformatted("Outlined card");
        }
        EndCard();
        at(16, 282); ListItem(s, "row", Rows[0], false, 328);
        at(16, 380);
        if (RadioButton(s, "Default", v.radio == 0))
            v.radio = 0;
        at(160, 396); Badge(s, "12");
        at(246, 380); LoadingIndicator(s, .62f, true);
        at(16, 464);
        if (Button(s, "Open sheet", {Variant::Filled}))
            v.modal = true;
        at(174, 464);
        if (Button(s, "Notify", {Variant::Tonal}))
            ShowSnackbar(v.snackbar, 0);
        at(16, 568); BottomAppBar(s, "Actions", Actions, 3, 328);
        if (BeginBottomSheet(s, "Catalog sheet", v.modal, {SheetMode::Modal, 360, 244})) {
            Heading(s, "Note details");
            if (Button(s, "Close", {Variant::Text}))
                v.modal = false;
            EndBottomSheet();
        }
        FeedBack(s, v, "Action completed");
        break;
    }
    }
}
