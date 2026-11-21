# Expanded widget catalog

Link `m3e::imgui` and include `<m3e/components.hpp>`. The widgets use the current ImGui context and the same theme, fonts, DPI scale, and motion preference as the original API. Selections, strings, dates, split positions, open state, and refresh operations belong to the host.

| Gallery page | Controls |
| --- | --- |
| Selection controls | Assist, filter, suggestion, and removable input chips. Single and multi-select segments, radio groups, standard button groups. |
| Search and inputs | Autocomplete, select, docked and expanded search, auto-growing multiline text. |
| Date and time | Calendar, single-date and date-range pickers, ISO date input, clock dial, typed time, and drag-adjustable time input. |
| Cards and lists | Filled, outlined, and elevated cards. Content panes, expansion panels, segmented lists, and an inline navigation drawer. |
| Navigation layouts | Navigation menu and rail, table of contents, tree, breadcrumbs, paginator, and stepper. |
| Panes and carousel | Horizontal and vertical split panes, a paged carousel, and a scrollable slide group. |
| Sheets and actions | Modal and persistent bottom sheets, split button, FAB menu, toolbar, snackbar with Undo. |
| Feedback and shapes | Avatar, badge, callout, heading, divider, skeleton, loading indicators, all 35 Matraic shapes, and shape interpolation. |
| Android screen layout | Medium and large collapsing app bars, bottom action and navigation bars, modal navigation drawer, scaffold, pull-to-refresh, and swipe-to-dismiss. |
| Phone components | Compose comparison fixture for chips, segments, cards, lists, radio, badge, loading indicator, bottom app bar, sheet, and snackbar. |

## Using the widgets

```cpp
#include <m3e/components.hpp>

// Keep these values in your application state.
bool sheetOpen = false;
m3e::SnackbarState notice;

// Submit once per frame inside the current ImGui window.
if (m3e::Button(style, "Actions", {m3e::Variant::Filled}))
    sheetOpen = true;
if (m3e::BeginBottomSheet(style, "Item actions", sheetOpen)) {
    if (m3e::Button(style, "Archive")) {
        ArchiveItem();
        m3e::ShowSnackbar(notice);
        sheetOpen = false;
    }
    m3e::EndBottomSheet();
}
if (m3e::Snackbar(style, "Archive notice", notice, "Item archived", "Undo") == m3e::SnackbarResult::Action)
    RestoreItem();
```

Always close child layouts with their matching End call even when the child is clipped. Popup scopes require End only after Begin returns true. Pass the same sheet options to both calls for persistent sheets. In a split pane, drawer layout, or scaffold, submit each Next call in order every frame. The gallery includes examples of these scopes. [Public declarations](../include/m3e/components.hpp) · [Gallery code](../examples/catalog.cpp)

## Comparisons

The browser fixture uses Matraic 2.13.5 and the pinned Roboto samples at 760 × 810. It includes light and dark stills, pixel differences, and short paired recordings for the eight expanded Matraic gallery pages. [Browser comparisons](parity/catalog/index.html)

The Android fixture ran on a physical Android 16 phone with Compose Material 3 1.5.0. Windows rendered the matching 360 × 688 dp frame at 3× density and 90% text scale. Both loaded the same Google Sans Flex files. The physical-device tap replay verified filter toggling, segmented selection, bottom-bar action dispatch, sheet opening, and snackbar Undo.

| Theme | Full-frame pixels above RGB delta 12 | Capture |
| --- | ---: | --- |
| Light | 2.970% | [Phone, Windows, and diff](android-catalog/light/comparison.png) |
| Dark | 3.015% | [Phone, Windows, and diff](android-catalog/dark/comparison.png) |

The renderer-free checks cover a disabled segment, autocomplete acceptance by Enter, an out-of-order date range, modal Escape, snackbar expiry, and split-pane dragging. The native gallery renders all ten new pages in both themes.

### Remaining differences

- Browser captures compare representative states. The date-and-time page juxtaposes the browser's inline range calendar and the native range-picker trigger. The carousel comparison uses the browser's hero layout and the native paged layout. Open date and time popups, embedded badge anchors, and the full set of sheet and menu transitions still need individual matched captures.
- Search currently shows results below the field within the available ImGui content area. The browser's fullscreen search transition and the Android search expansion transitions differ. The calendar uses English month names, Sunday-first weeks, and ISO date entry. The dial selects minutes in five-minute steps, with one-minute values available through numeric entry.
- The carousel implements paged content with arrows and pointer swipes. Matraic's multi-browse and uncontained layouts and their scroll physics remain gaps. The table of contents exposes selection for the host to scroll to a section.
- Android comparisons cover the dedicated phone fixture and its scripted interactions. Other Android gesture behavior, platform insets, keyboard/IME behavior, accessibility, and haptics depend on host integration. The current phone differences include typography, segmented sizing, outlines, and the loading indicator's polygon interpolation.

### Reproduce

```powershell
python tools/parity/capture.py --catalog --motion --channel msedge --gallery build/Release/m3e_gallery.exe --output docs/parity/catalog
python tools/android-reference/compare.py --fixture catalog --adb "$env:ANDROID_HOME/platform-tools/adb.exe" --apk tools/android-reference/build/outputs/apk/debug/m3e-android-reference-debug.apk --gallery build/Release/m3e_gallery.exe --output docs/android-catalog
```

The browser command uses the same pinned npm installation as the original parity report. Build the APK first for the phone command. [Browser prerequisites](PARITY.md) · [APK build and phone setup](ANDROID.md#reproduce)
