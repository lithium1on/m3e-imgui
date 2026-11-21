#pragma once

#include <m3e/imgui.hpp>
#include <optional>

namespace m3e::android {

// Compose expressiveLightColorScheme() and darkColorScheme() defaults.
// A host can replace these roles with its platform's dynamic color scheme.
Theme DefaultTheme(Scheme scheme);

enum class FabSize { Small, Regular, Medium, Large };
enum class FabColor { Primary, Secondary, Tertiary, Surface };
struct FabOptions {
    FabSize size = FabSize::Regular;
    FabColor color = FabColor::Primary;
    bool disabled = false;
};
bool FloatingActionButton(const Style& style, const char* id, Icon icon, const FabOptions& options = {});

enum class ExtendedFabSize { Regular, Medium, Large };
struct ExtendedFabOptions {
    ExtendedFabSize size = ExtendedFabSize::Regular;
    FabColor color = FabColor::Primary;
    bool expanded = true;
    bool disabled = false;
};
bool ExtendedFloatingActionButton(const Style& style, const char* label, Icon icon, const ExtendedFabOptions& options = {});

enum class NavigationIconPosition { Start, Top };
enum class NavigationArrangement { EqualWeight, Centered };
struct ShortNavigationOptions {
    float width = 0; // Logical units. Zero uses the available content width.
    NavigationIconPosition iconPosition = NavigationIconPosition::Start;
    NavigationArrangement arrangement = NavigationArrangement::EqualWeight;
};
// Returns the activated index or -1 and updates selected.
int ShortNavigationBar(const Style& style, const char* id, const Choice* destinations, int count, int& selected,
                       const ShortNavigationOptions& options = {});

enum class ToolbarColor { Standard, Vibrant };
struct FloatingToolbarOptions {
    ToolbarColor color = ToolbarColor::Standard;
    bool vertical = false;
};
// Each true entry in selected displays the corresponding action as toggled.
int FloatingToolbar(const Style& style, const char* id, const Choice* actions, int count,
                    const bool* selected = nullptr, const FloatingToolbarOptions& options = {});

struct TopAppBarOptions {
    Icon navigationIcon = 0;
    const char* navigationLabel = "Navigate back";
    const char* subtitle = nullptr;
    float width = 0; // Logical units. Zero uses the available content width.
    bool centerTitle = false;
    bool scrolled = false;
};
struct TopAppBarResult {
    bool navigation = false;
    int action = -1;
};
TopAppBarResult TopAppBar(const Style& style, const char* id, const char* title, const Choice* actions, int count,
                         const TopAppBarOptions& options = {});

// Values are fractions in [0, 1]. Pass std::nullopt for indeterminate progress.
// Width and diameter in logical units. Zero diameter uses the Compose default.
void LinearProgress(const Style& style, std::optional<float> value, bool wavy = false, float width = 240);
void CircularProgress(const Style& style, std::optional<float> value, bool wavy = false, float diameter = 0);

} // namespace m3e::android
