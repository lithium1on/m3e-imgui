#pragma once

#include <m3e/imgui.hpp>
#include <optional>
#include <string>

namespace m3e {

// Dimensions are in logical units. A width of zero uses available content width.
void Divider(const Style&, float length = 0, bool vertical = false);
void Heading(const Style&, const char* text, Size = Size::Medium);
void Avatar(const Style&, const char* initials, float diameter = 40, Icon icon = 0);
void Badge(const Style&, const char* text = nullptr);
enum class MessageTone { Information, Success, Warning, Error };
void Callout(const Style&, const char* title, const char* message, MessageTone = MessageTone::Information,
             float width = 0);
enum class ExpressiveShape {
    Circle, Square, Pill, Diamond, Clover, Burst, Heart, Cookie4, Cookie6, Cookie7, Clover8, Cookie9, Cookie12,
    Arch, Arrow, Boom, Bun, Fan, Flower, Gem, Ghost, Hexagon, Oval, Pentagon, PixelCircle, PixelTriangle, Puffy,
    PuffyDiamond, Semicircle, Slanted, SoftBoom, SoftBurst, Sunny, Triangle, VerySunny, Count
};
const char* ShapeName(ExpressiveShape);
void ShapePreview(const Style&, ExpressiveShape, float size = 48, Role color = Role::Primary);
void ShapeMorph(const Style&, ExpressiveShape from, ExpressiveShape to, float fraction, float size = 48,
                Role color = Role::Primary);
void Skeleton(const Style&, float width, float height, float rounding = 8);
void LoadingIndicator(const Style&, std::optional<float> progress = std::nullopt, bool contained = false);

enum class ChipKind { Assist, Filter, Input, Suggestion };
struct ChipOptions {
    ChipKind kind = ChipKind::Assist;
    Icon icon = 0;
    bool selected = false, elevated = false, disabled = false, removable = false;
};
enum class ChipAction { None, Activate, Remove };
ChipAction Chip(const Style&, const char* label, const ChipOptions& = {});
bool RadioButton(const Style&, const char* label, bool selected, bool disabled = false);
bool RadioGroup(const Style&, const char* id, const Choice*, int count, int& selected, bool horizontal = false);
bool SegmentedButtons(const Style&, const char* id, const Choice*, int count, int& selected, bool showCheck = true);
bool SegmentedButtonsMulti(const Style&, const char* id, const Choice*, int count, bool* selected, bool showCheck = true);
int ButtonGroup(const Style&, const char* id, const Choice*, int count, const ButtonOptions& = {});

bool Select(const Style&, const char* label, const Choice*, int count, int& selected, float width = 240);
// Search and autocomplete return the accepted option index or -1. The buffer holds the query.
int Autocomplete(const Style&, const char* label, char* query, std::size_t capacity,
                 const Choice*, int count, float width = 280);
enum class SearchPresentation { Docked, Expanded };
struct SearchOptions { float width = 320; SearchPresentation presentation = SearchPresentation::Docked; };
int SearchBar(const Style&, const char* label, char* query, std::size_t capacity,
              const Choice*, int count, const SearchOptions& = {});
bool AutoTextarea(const Style&, const char* label, char* value, std::size_t capacity,
                  int minimumLines = 2, int maximumLines = 8, float width = 300);

struct Date { int year = 1970, month = 1, day = 1; };
struct DateRange { Date start, end; bool choosingEnd = false; };
struct CalendarPage { int year = 1970, month = 1; };
bool DateInput(const Style&, const char* label, Date&, char* buffer, std::size_t capacity, float width = 200);
bool Calendar(const Style&, const char* id, CalendarPage&, Date& selected);
bool DateRangeCalendar(const Style&, const char* id, CalendarPage&, DateRange&);
bool DatePicker(const Style&, const char* label, CalendarPage&, Date&);
bool DateRangePicker(const Style&, const char* label, CalendarPage&, DateRange&);
struct Time { int hour = 0, minute = 0; };
enum class TimePickerMode { Dial, Input, Scroll };
bool TimePicker(const Style&, const char* label, Time&, TimePickerMode = TimePickerMode::Dial, bool use24Hour = false);

enum class CardVariant { Filled, Outlined, Elevated };
// Begin/End child pairs always require End, even when Begin returns false.
bool BeginCard(const Style&, const char* id, CardVariant = CardVariant::Filled, float width = 0, float height = 0);
void EndCard();
bool BeginContentPane(const Style&, const char* id, float width = 0, float height = 0);
void EndContentPane();
bool ExpansionPanel(const Style&, const char* label, bool& expanded, Icon = 0, float width = 0);
struct ListEntry { const char* label; const char* supporting = nullptr; Icon leading = 0, trailing = 0; bool disabled = false; };
bool ListItem(const Style&, const char* id, const ListEntry&, bool selected = false, float width = 0);
int List(const Style&, const char* id, const ListEntry*, int count, int& selected, float width = 0, bool segmented = false);
int NavigationMenu(const Style&, const char* id, const Choice*, int count, int& selected, float width = 220);
int NavigationRail(const Style&, const char* id, const Choice*, int count, int& selected, bool expanded = false);
int Breadcrumbs(const Style&, const char* id, const Choice*, int count);
int TableOfContents(const Style&, const char* id, const Choice*, int count, int& selected, float width = 220);
bool Paginator(const Style&, const char* id, int& page, int pageCount);
bool Stepper(const Style&, const char* id, const Choice*, int count, int& step);
struct TreeEntry { const char* label; int depth = 0; Icon icon = 0; bool disabled = false; };
int Tree(const Style&, const char* id, const TreeEntry*, int count, int& selected, bool* expanded, float width = 240);
int SlideGroup(const Style&, const char* id, const Choice*, int count, int& selected, float width = 0);
// Carousel receives an application-owned active page. Render content inside a visible page.
struct CarouselOptions { float width = 0, height = 180; bool loop = false; };
bool BeginCarousel(const Style&, const char* id, int& active, int pageCount, const CarouselOptions& = {});
void EndCarousel();
// Split panes render two children using the current axis and an application-owned fraction.
struct SplitPaneOptions { float width = 0, height = 220, minimum = 64; bool vertical = false; };
void BeginSplitPane(const Style&, const char* id, float& fraction, const SplitPaneOptions& = {});
void NextSplitPane(const Style&, float& fraction, const SplitPaneOptions& = {});
void EndSplitPane();
bool BeginDrawerLayout(const Style&, const char* id, bool open, float drawerWidth = 224, float height = 240);
void NextDrawerPane(const Style&);
void EndDrawerLayout();

// Popup scopes require End only when Begin returns true.
enum class SheetMode { Modal, Persistent };
struct SheetOptions { SheetMode mode = SheetMode::Modal; float width = 400, height = 272; bool dragHandle = true; };
bool BeginBottomSheet(const Style&, const char* id, bool& open, const SheetOptions& = {});
void EndBottomSheet(const SheetOptions& = {});
bool BeginModalNavigationDrawer(const Style&, const char* id, bool& open, float width = 320);
void EndModalNavigationDrawer();
struct SnackbarState { double expires = 0; bool open = false; };
enum class SnackbarResult { None, Action, Dismissed };
void ShowSnackbar(SnackbarState&, double duration = 4);
SnackbarResult Snackbar(const Style&, const char* id, SnackbarState&, const char* message,
                        const char* action = nullptr, bool closeButton = true, float width = 380);
struct SplitButtonResult { bool primary = false; int secondary = -1; };
SplitButtonResult SplitButton(const Style&, const char* label, const Choice* actions, int count,
                              const ButtonOptions& = {});
int FloatingActionButtonMenu(const Style&, const char* id, Icon icon, const Choice*, int count);
struct ToolbarOptions { bool vibrant = false, vertical = false, rounded = false, elevated = false; };
int Toolbar(const Style&, const char* id, const Choice*, int count, const ToolbarOptions& = {});

namespace android {
enum class AppBarSize { Small, Medium, Large };
struct CollapsingAppBarOptions { AppBarSize size = AppBarSize::Medium; float collapse = 0, width = 0; bool centered = false; };
int CollapsingTopAppBar(const Style&, const char* id, const char* title, const Choice*, int count,
                        const CollapsingAppBarOptions& = {});
int BottomAppBar(const Style&, const char* id, const Choice*, int count, float width = 0);
int NavigationBar(const Style&, const char* id, const Choice*, int count, int& selected, float width = 0);
bool SwipeToDismiss(const Style&, const char* id, const ListEntry&, float width = 0);
// Report downward drag distance from a host scroll container when it is at its top edge.
bool PullToRefresh(const Style&, const char* id, float pullDistance, bool released, bool refreshing, float width = 0);
// Top, content, then bottom. Call both Next functions and End on every Begin.
bool BeginScaffold(const Style&, const char* id, float topHeight, float bottomHeight, float width = 0, float height = 0);
bool NextScaffoldContent(const Style&);
bool NextScaffoldBottom(const Style&);
void EndScaffold();
} // namespace android

} // namespace m3e
