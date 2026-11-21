# Integration and API notes

## Context and ownership

Make your application’s Dear ImGui context current before calling the library. The library owns no context, platform window, renderer, or persistent global widget state. Pass application-owned values by reference. Text fields use caller-owned UTF-8 buffers. `FieldOptions::flags`, the callback, and callback user data forward to Dear ImGui’s editor. Provide unique labels or ImGui ID suffixes such as `"Name##billing"` for repeated labels in a window. Use `PushID` for repeated arrays and component instances.

`LoadFonts` loads regular and medium text weights and both Material Symbols Outlined fill instances. The default text face is Roboto. Pass `Typeface::GoogleSansFlex` to use the gallery’s font instead. Call it with the atlas unlocked. Font pointers belong to that atlas. Check each pointer when the app depends on the bundled font files. An empty `Fonts` uses the current ImGui font for labels. Provide symbol fonts to draw icons. When supplying other fonts, set the two font scale factors for the font loader and typeface you use. The bundled factors match Dear ImGui’s default stb loader.

Render with the application’s normal calls to `ImGui::NewFrame`, its window API, and `ImGui::Render`. The gallery uses ImGui’s Win32 and DX11 backends. The library target itself links only ImGui. A renderer that supports ImGui 1.92’s texture updates can load glyphs and type sizes on demand. Applications with a fixed atlas can preload their sizes and glyphs using their renderer’s normal font workflow.

## Themes, sizing, and motion

`Theme::TonalSpot` returns the light or dark palette for Matraic’s default seed `#6750A4`. Apply your own palette by replacing individual `Role` entries. `Style::scale` converts logical sizes to the units of the current ImGui viewport. Keep it finite and positive. Use the monitor DPI ratio when ImGui uses device-pixel window coordinates, as the Win32 gallery does. With a backend that expresses window coordinates in logical pixels, keep the style at the logical scale and let the backend handle framebuffer scaling.

`Style::density` applies the upstream 4 px step down to the per-control floor of −3. Pass widget dimensions such as text-field width and slider length in logical units. Follow the application or operating system’s motion preference using `Style::reducedMotion`. `Style::motion` selects the standard or expressive spring curves.

## Selection and overlays

- `Button` and `IconButton` return activation. The toggle overloads also update selection. `ConnectedButtons` returns the activated index or `-1`. The multi-select overload updates the caller’s bool array.
- `Tabs` updates the selected index and reports a change. The application submits the corresponding panel contents. Place the strip before or after the panel and set `indicatorAtTop` for a strip under the content.
- Submit the menu trigger, call `OpenMenu` on activation, then call `BeginMenu` in the same window and ID scope. Pair successful begins with `EndMenu`. Use the same pattern for `BeginSubmenu`. Toggle the application’s checkbox or radio value when the menu item returns true.
- Call `OpenDialog` from the trigger, then submit `BeginDialog` each frame. Successful begins require `EndDialog`. `DialogActions` returns the pressed index or `-1`. Close the dialog with `CloseDialog` while its scope is active.
- `Tooltip` and `BeginRichTooltip` attach to the preceding item. Pair a successful rich hover tooltip with `EndRichTooltip`. Pass the trigger’s activation result to `BeginInteractiveTooltip` for content with actions, and pair it with `EndInteractiveTooltip`. Use `ImGui::CloseCurrentPopup` from an action to dismiss it.

## Dear ImGui version boundary

`src/imgui_bridge.cpp` is the only translation unit that includes `imgui_internal.h`. It registers custom items with normal ImGui navigation and visibility, invokes `ButtonBehavior`, publishes edited IDs, handles directional input in composite widgets, and restores popup focus via ImGui’s popup stack. Composite widgets preserve ImGui’s layout and use the active window’s storage for animation state. The adapter checks the 1.92.5 version number at compile time. When upgrading ImGui, validate this adapter with the interaction checks and regenerate relevant comparisons.

```sh
cmake -S . -B build -DM3E_BUILD_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```
