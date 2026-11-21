#pragma once

#include <imgui.h>
#include <cstddef>
#include <cstdint>

namespace m3e {

enum class Role : std::uint8_t {
    Primary, OnPrimary, PrimaryContainer, OnPrimaryContainer,
    PrimaryFixed, PrimaryFixedDim, OnPrimaryFixed, OnPrimaryFixedVariant,
    Secondary, OnSecondary, SecondaryContainer, OnSecondaryContainer,
    SecondaryFixed, SecondaryFixedDim, OnSecondaryFixed, OnSecondaryFixedVariant,
    Tertiary, OnTertiary, TertiaryContainer, OnTertiaryContainer,
    TertiaryFixed, TertiaryFixedDim, OnTertiaryFixed, OnTertiaryFixedVariant,
    Error, OnError, ErrorContainer, OnErrorContainer, Surface, OnSurface, OnSurfaceVariant,
    SurfaceContainerLowest, SurfaceContainerLow, SurfaceContainer, SurfaceContainerHigh,
    SurfaceContainerHighest, SurfaceDim, SurfaceBright, SurfaceVariant, InverseSurface,
    InverseOnSurface, InversePrimary, Outline, OutlineVariant, Shadow, Scrim, SurfaceTint, Count
};

enum class Scheme { Light, Dark };

struct Theme {
    ImU32 colors[static_cast<std::size_t>(Role::Count)]{};
    ImU32& operator[](Role role) { return colors[static_cast<std::size_t>(role)]; }
    ImU32 operator[](Role role) const { return colors[static_cast<std::size_t>(role)]; }

    static Theme TonalSpot(Scheme scheme);
};

struct Fonts {
    ImFont* regular = nullptr;
    ImFont* medium = nullptr;
    ImFont* symbols = nullptr;
    ImFont* symbolsFilled = nullptr;
    // Ratios of the loader's ascent-minus-descent size to the CSS em size.
    float textEmScale = 1.0f;
    float symbolEmScale = 1.0f;
};

// Call with the intended ImGui context current and its atlas unlocked.
Fonts LoadFonts(const char* directory);
enum class Typeface { Roboto, GoogleSansFlex };
Fonts LoadFonts(const char* directory, Typeface typeface);

enum class MotionPreset { Standard, Expressive };

struct Style {
    Theme theme = Theme::TonalSpot(Scheme::Light);
    Fonts fonts;
    float scale = 1.0f;
    float density = 0.0f;
    bool reducedMotion = false;
    MotionPreset motion = MotionPreset::Standard;
    float Px(float logicalPixels) const { return logicalPixels * scale; }
};

enum class Size { ExtraSmall, Small, Medium, Large, ExtraLarge };
enum class Shape { Round, Square };
enum class Variant { Elevated, Filled, Tonal, Outlined, Text };
enum class IconWidth { Narrow, Default, Wide };

using Icon = std::uint32_t; // Material Symbols Outlined Unicode code point. Zero hides the icon.
namespace icons {
inline constexpr Icon Add = 0xe145, Check = 0xe5ca, Close = 0xe5cd, Favorite = 0xe87d,
    MoreVert = 0xe5d4, Search = 0xe8b6, Settings = 0xe8b8, Delete = 0xe872, Home = 0xe88a,
    Star = 0xe838, Info = 0xe88e, Edit = 0xe3c9, Remove = 0xe15b, ChevronRight = 0xe5cc;
}

struct ButtonOptions {
    Variant variant = Variant::Text; // Text is the standard variant for icon-only buttons.
    Size size = Size::Small;
    Shape shape = Shape::Round;
    Icon icon = 0;
    Icon trailingIcon = 0;
    bool disabled = false;
    bool filledIcon = false;
    Icon selectedIcon = 0;
};

bool Button(const Style& style, const char* label, const ButtonOptions& options = {});
bool ToggleButton(const Style& style, const char* label, bool& selected, const ButtonOptions& options = {});
bool IconButton(const Style& style, const char* id, Icon icon, const ButtonOptions& options = {},
                IconWidth width = IconWidth::Default);
bool ToggleIconButton(const Style& style, const char* id, Icon icon, bool& selected,
                      const ButtonOptions& options = {}, IconWidth width = IconWidth::Default, Icon selectedIcon = 0);

struct Choice {
    const char* label;
    Icon icon = 0;
    bool disabled = false;
};

struct ConnectedOptions {
    Size size = Size::Small;
    Variant variant = Variant::Tonal;
    Shape shape = Shape::Round;
    bool iconsOnly = false;
    IconWidth iconWidth = IconWidth::Default;
};
int ConnectedButtons(const Style& style, const char* id, const Choice* items, int count, int& selected,
                     const ConnectedOptions& options = {});
bool ConnectedButtonsMulti(const Style& style, const char* id, const Choice* items, int count, bool* selected,
                           const ConnectedOptions& options = {});

enum class TabVariant { Primary, Secondary };
struct TabOptions {
    TabVariant variant = TabVariant::Primary;
    bool stretch = false;
    bool indicatorAtTop = false;
    float width = 0; // Uses the available width when stretched and zero.
};
// Submit the content for selected immediately before or after this strip.
bool Tabs(const Style& style, const char* id, const Choice* tabs, int count, int& selected,
          const TabOptions& options = {});

enum class CheckState { Unchecked, Checked, Mixed };
bool Checkbox(const Style& style, const char* label, CheckState& value, bool disabled = false, bool error = false);
bool Checkbox(const Style& style, const char* label, bool& value, bool disabled = false, bool error = false);

enum class SwitchIcons { None, Selected, Unselected, Both };
bool Switch(const Style& style, const char* label, bool& value, SwitchIcons icons = SwitchIcons::None,
            bool disabled = false);

enum class FieldVariant { Filled, Outlined };
struct FieldOptions {
    FieldVariant variant = FieldVariant::Filled;
    float width = 270.0f;
    float multilineHeight = 0.0f;
    Icon leadingIcon = 0;
    Icon trailingIcon = 0;
    const char* supportingText = nullptr;
    const char* placeholder = nullptr;
    const char* prefix = nullptr;
    const char* suffix = nullptr;
    bool floatLabel = false;
    bool required = false;
    bool disabled = false;
    bool error = false;
    ImGuiInputTextFlags flags = 0;
};
bool TextField(const Style& style, const char* label, char* text, std::size_t capacity, const FieldOptions& options = {},
               ImGuiInputTextCallback callback = nullptr, void* userData = nullptr);

enum class SliderTrackStyle { Matraic, Android };
struct SliderOptions {
    Size size = Size::ExtraSmall;
    float length = 200.0f;
    float step = 1.0f;
    bool vertical = false;
    bool ticks = false;
    bool disabled = false;
    bool showValue = true;
    int valueDecimals = 0;
    SliderTrackStyle trackStyle = SliderTrackStyle::Matraic;
};
bool Slider(const Style& style, const char* id, float& value, float min, float max, const SliderOptions& options = {});
bool RangeSlider(const Style& style, const char* id, float& lower, float& upper, float min, float max,
                 const SliderOptions& options = {});

enum class MenuVariant { Standard, Vibrant };
enum class MenuMark { None, Checkbox, Radio };
struct MenuOptions {
    MenuVariant variant = MenuVariant::Standard;
    float width = 168.0f;
};
struct MenuItemOptions {
    Icon icon = 0;
    const char* shortcut = nullptr;
    MenuMark mark = MenuMark::None;
    bool selected = false;
    bool disabled = false;
};

void OpenMenu(const char* id);
bool BeginMenu(const Style& style, const char* id, const MenuOptions& options = {});
bool BeginSubmenu(const Style& style, const char* label, Icon icon = 0, bool enabled = true);
void EndMenu();
bool MenuItem(const Style& style, const char* label, const MenuItemOptions& options = {});
void MenuDivider(const Style& style);

struct DialogOptions {
    float width = 320.0f;
    bool dismissOnEscape = true;
    bool dismissOnOutsideClick = false;
    Icon icon = 0;
    bool closeButton = false;
};
void OpenDialog(const char* id);
bool BeginDialog(const Style& style, const char* id, const char* title, const DialogOptions& options = {});
void DialogText(const Style& style, const char* body);
int DialogActions(const Style& style, const Choice* actions, int count, bool alignEnd = true);
void EndDialog();
void CloseDialog();

enum class TooltipPlacement { Above, Below, Left, Right };
// Attach to the most recently submitted ImGui item.
void Tooltip(const Style& style, const char* text, TooltipPlacement placement = TooltipPlacement::Below);
bool BeginRichTooltip(const Style& style, const char* title, TooltipPlacement placement = TooltipPlacement::Below);
void EndRichTooltip();
// Pass the trigger's activation result for a tooltip containing actions.
bool BeginInteractiveTooltip(const Style& style, const char* title, bool activated,
                             TooltipPlacement placement = TooltipPlacement::Below, float width = 280);
void EndInteractiveTooltip();

} // namespace m3e
