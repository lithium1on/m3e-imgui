# Browser and native parity

Open the [comparison gallery](parity/index.html) for 32 matched captures and 32 paired motion clips. The gallery shows the browser render, the native render, and the pixel difference for each light and dark case. The [compact DPI report](parity/compact/index.html) adds seven dark samples at 1.25× scale, density −2, and reduced motion.

## Baseline and method

- The baseline uses a 760 × 390 logical viewport, 1× framebuffer scale, density 0, and standard motion. Chromium Edge 157 rendered the pinned Matraic 2.13.5 package. The native Win32 gallery rendered to a Direct3D 11 framebuffer using WARP.
- Both fixtures use the same component labels, canvas positions, layout widths, theme, and font families. The browser fixture renders Matraic’s custom elements with their shadow DOM styles. It imports the same npm version as the live showcase at the time of capture.
- `browser.png` and `native.png` are the original screenshots. `comparison.png` combines them with the amplified difference image. The percentage below counts pixels whose maximum absolute RGB-channel difference exceeds 12 on a 0–255 scale. It uses the whole viewport, so inspect the highlighted regions directly for local geometry discrepancies. `metrics.json` also includes browser element bounds and mean RGB differences.
- The 12 fps motion clips pair a browser recording with the nearest native sample from a 60 Hz input replay. The recordings use the same logical pointer positions and gesture schedule. Browser frame labels estimate screenshot times, so individual frames can differ by a sample when measuring transition timing.

## Static differences and remaining gaps

| Sample | Light | Dark | Remaining differences |
| --- | ---: | ---: | --- |
| Button | 2.870% | 2.417% | Roboto rasterization and fractional label advances produce small width and baseline differences. Elevated variants use a layered native shadow. |
| Icon button | 0.974% | 0.806% | Symbol antialiasing and elevated shadows differ. |
| Connected buttons | 1.614% | 1.654% | Adjacent corners use native tessellation. The native API offers controlled single selection. The motion capture uses the matching multi-select row. |
| Connected icon buttons | 0.365% | 0.371% | Symbol edges and intermediate corner morphs differ. |
| Tabs | 1.804% | 1.862% | Indicator and divider rasterization can differ by a pixel. Panel transitions, swipe gestures, and overflow pagination belong to the application. |
| Menu | 1.588% | 0.736% | CSS shadow blur, selected-row corner transitions, and text edges differ. |
| Vibrant menu | 1.588% | 0.738% | The standard menu differences also apply to the vibrant palette. |
| Submenu | 2.368% | 1.198% | Popup alignment and highlight timing differ at the join. Narrow viewports use native anchor clamping. |
| Text field | 2.138% | 2.205% | The native editor uses ImGui line spacing, cursor, selection, and IME. The browser textarea shows a resize grip. Label motion and outline-notch antialiasing differ. |
| Checkbox | 0.254% | 0.278% | The native polygon reproduces the check and mixed geometry with different antialiasing. |
| Switch | 0.474% | 0.443% | Handle growth and small icon edges differ. The native switch treats a pointer activation as a toggle. |
| Slider | 0.310% | 0.395% | Tick, track, and handle edges differ. Pointer capture and value editing use ImGui input and an application-owned value. |
| Dialog | 1.967% | 1.709% | Shadow falloff and action text alignment differ. The native entry animates the surface, content, actions, and scrim. Closing releases the ImGui modal immediately. |
| Tooltip | 0.325% | 0.339% | Text width and rasterization differ. The native tooltip clamps to the viewport edges. |
| Rich tooltip | 2.516% | 1.289% | Text wrapping, action padding, and outline shadow differ. |
| Rich hover tooltip | 2.976% | 1.869% | Surface width, padding, and shadow differ. |

The clips show the same main event sequence for the sampled presses, toggles, drag, and popup opens. CSS blur and native layered ripples differ visibly around their edges. Menu scaling operates on the native draw list while hit rectangles use their resting bounds. Switch and connected-button geometry can settle on a different frame. The gallery covers pointer input, and the renderer-free interaction checks also exercise keyboard and synthetic gamepad input. A physical gamepad and browser screen-reader behavior were outside the capture run.

## Reproduce

Run the gallery build on Windows. The comparison script also needs Node.js, Python, Chromium via Playwright or an installed Edge channel, and FFmpeg for motion exports.

```powershell
npm.cmd ci --prefix tools/parity
python -m pip install -r tools/parity/requirements.txt
python tools/parity/capture.py --gallery build/Release/m3e_gallery.exe --output docs/parity --channel msedge --motion
```

For Playwright-managed Chromium, run `python -m playwright install chromium` and omit `--channel msedge`. Use repeated `--case` or `--scheme` flags to select a subset. Add `--update` to preserve other cases when recapturing into the same report. The script supports `--scale`, `--density`, and `--reduced-motion`. For a compact report, write to a separate output and use `--scheme dark --scale 1.25 --density -2 --reduced-motion`.
