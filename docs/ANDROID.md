# Android-referenced Material 3 Expressive

The Windows gallery opens on **App preview**, an interactive example that combines the new widgets. The first group in the sidebar shows their individual variants. The earlier Matraic-based widgets are in the second group. These are native Windows gallery captures. [Light preview](android-previews/light.png) · [Dark preview](android-previews/dark.png)

## Components

| Widget | Coverage |
| --- | --- |
| Floating action buttons | Small, regular, medium, and large icon FABs. Three extended sizes with animated collapse, four color-role choices, and a disabled state. |
| Short navigation bar | Leading or stacked icons, equal-weight or centered layout, controlled selection, and disabled destinations. |
| Floating toolbar | Horizontal and vertical layouts, standard and vibrant colors, and caller-controlled toggle states. |
| Top app bar | Small leading or centered title, optional subtitle, navigation icon, trailing actions, and a scrolled container color. |
| Progress indicators | Straight or wavy linear and circular indicators, with determinate and indeterminate modes. Reduced motion freezes the indeterminate animation. |

## Integrate

Link the existing `m3e::imgui` target and include `<m3e/android.hpp>`. Android-referenced components live in `m3e::android` and take the same `m3e::Style` as the original widgets. Their dimensions use Compose logical dp scaled with `Style::scale`. Optional widths and diameters also use logical units. The fixed Android component sizes stay independent of the Matraic density setting.

```cpp
#include <m3e/android.hpp>

// Set these fields when configuring the style for your application.
material.theme = m3e::android::DefaultTheme(m3e::Scheme::Light);
material.motion = m3e::MotionPreset::Expressive;

// Submit inside the current ImGui window.
if (m3e::android::FloatingActionButton(material, "Add note", m3e::icons::Add))
    AddNote();
```

Keep the selected destination, toolbar toggles, progress values, and scroll state in the host application. Return values identify actions without taking over navigation or document state. Set `Style::reducedMotion` to respect the host preference.

Use `SliderTrackStyle::Android` in `SliderOptions::trackStyle` to select the Compose-style 16 dp track, 44 dp thumb and 48 dp cross-axis control size. The Android-style track and its stop indicator disappear together when the thumb approaches the end.

## Upstream reference

The reference is AndroidX Compose Material 3 at [`ebe7964173e100eb8833f6d4d5e59cba3656db0d`](https://github.com/androidx/androidx/tree/ebe7964173e100eb8833f6d4d5e59cba3656db0d/compose/material3/material3/src/commonMain/kotlin/androidx/compose/material3). The port uses Compose’s default dp and color roles, its expressive light and default dark palettes, and its fast spatial spring parameters for indicator and FAB transitions. The [AndroidX Apache 2.0 notice](../licenses/AndroidX-Apache-2.0.txt) accompanies the project and gallery.

Official examples and visual references are available for [short navigation bars](https://developer.android.com/develop/ui/compose/components/navigation-bar), [floating toolbars](https://developer.android.com/develop/ui/compose/components/floating-toolbar), and [app bars](https://developer.android.com/develop/ui/compose/components/app-bars).

## Verification and remaining differences

The native gallery rendered all Android component pages in both themes. The interaction checks verify small FAB input bounds, action dispatch from top bars, short-navigation selection, disabled controls, and passive progress indicators.

### Physical Android comparison

A physical Android 16 phone ran the [Compose reference app](../tools/android-reference/src/main/kotlin/dev/m3e/reference/MainActivity.kt) with Compose Material 3 1.5.0. The Windows gallery rendered the matching 360 × 688 dp layout at the phone’s 3× display density and 0.90 text scale. Both apps loaded the same bundled Google Sans Flex files. The paired captures use Compose’s default light and dark palettes, and both wavy progress indicators held a fixed phase.

| Theme | Full-frame pixels over threshold | Foreground pixels over threshold | Capture |
| --- | ---: | ---: | --- |
| Light | 1.105% | 3.835% | [Side-by-side and pixel diff](android-device/light/comparison.png) |
| Dark | 1.173% | 4.269% | [Side-by-side and pixel diff](android-device/dark/comparison.png) |

The threshold is a maximum RGB-channel difference greater than 12 out of 255. Foreground percentages use pixels that differ from the flat canvas in either image. Font rasterization, shadow corners, and wavy progress outlines still differ. Under the center of the regular light-mode FAB, the mean red-channel error fell from 13 to 0.9 across eight points spaced 1 dp apart below the button.

In both themes, the two apps draw the slider stop dot at 62% and hide it at 98%. ADB taps selected Explore, toggled Favorite, activated the extended FAB, and took the slider to 100% on the phone. A wallpaper-based color scheme also produced different primary tones from the default palette. The Windows gallery uses the pinned default palettes and accepts system-derived color roles from the host. Its scrollbars use the active palette’s surface and outline roles. Native haptics, broader touch gestures, safe-area insets, and platform accessibility still require host integration. [Capture metrics](android-device/metrics.json) · [Windows scrollbars in both themes](android-device/scrollbars.png)

### Reproduce

Install Android SDK Platform 37.0, Build Tools 37.0.0, and Platform Tools, then authorize a USB-connected phone in ADB. Set `ANDROID_HOME` to the SDK directory and use a JDK compatible with AGP 9.5. The Android fixture uses its pinned Gradle wrapper.

```powershell
.\tools\android-reference\gradlew.bat -p tools/android-reference assembleDebug
python tools/android-reference/compare.py --adb "$env:ANDROID_HOME/platform-tools/adb.exe" --apk tools/android-reference/build/outputs/apk/debug/m3e-android-reference-debug.apk --gallery build/Release/m3e_gallery.exe --output docs/android-device
```

Keep the phone unlocked while capturing. The comparison tool checks that the reference app is in the foreground and saves only its fixture rectangle.

Use the existing gallery capture command for individual frames. For example:

```powershell
.\build\Release\m3e_gallery.exe --capture android-sample light out/android-sample
```
