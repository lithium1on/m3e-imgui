# Upstream revisions and notices

| Source | Pinned revision | License |
| --- | --- | --- |
| Matraic M3E | [`489ad4c65b9ee348ee859cda214311cf39bcc842`](https://github.com/matraic/m3e/tree/489ad4c65b9ee348ee859cda214311cf39bcc842) | [MIT](../licenses/Matraic-M3E-MIT.txt) |
| Dear ImGui 1.92.5 | [`6d910d5487d11ca567b61c7824b0c78c569d62f0`](https://github.com/ocornut/imgui/tree/6d910d5487d11ca567b61c7824b0c78c569d62f0) | [MIT](../licenses/Dear-ImGui-MIT.txt) |
| AndroidX Compose Material 3 | [`ebe7964173e100eb8833f6d4d5e59cba3656db0d`](https://github.com/androidx/androidx/tree/ebe7964173e100eb8833f6d4d5e59cba3656db0d/compose/material3/material3) | [Apache License 2.0](../licenses/AndroidX-Apache-2.0.txt) |
| Roboto | [`06bfe0d62a98375cb09c942fa18e68bd32cff057`](https://github.com/google/fonts/tree/06bfe0d62a98375cb09c942fa18e68bd32cff057/ofl/roboto) | [SIL Open Font License 1.1](../licenses/Roboto-OFL.txt) |
| Google Sans Flex | [`06bfe0d62a98375cb09c942fa18e68bd32cff057`](https://github.com/google/fonts/tree/06bfe0d62a98375cb09c942fa18e68bd32cff057/ofl/googlesansflex) | [SIL Open Font License 1.1](../licenses/GoogleSansFlex-OFL.txt) |
| Material Symbols Outlined | [`10ecc14790612ca8fd164e3f4ffe9c16fc50d5b7`](https://github.com/google/material-design-icons/tree/10ecc14790612ca8fd164e3f4ffe9c16fc50d5b7/variablefont) | [Apache License 2.0](../licenses/MaterialSymbolsOutlined-Apache-2.0.txt) |

Matraic retains copyright in its source and the ported portions. The pinned revision is the npm publication’s `gitHead`. The browser fixture uses `@m3e/web@2.13.5` with the integrity and transitive dependencies in `tools/parity/package-lock.json`. The live showcase informed the fixtures and component choices.

The 35 normalized shape polygons are generated from the same pinned Matraic source. Run `node --experimental-strip-types tools/prepare_shapes.mjs <matraic-checkout> src/shape_tokens.hpp` to regenerate the table.

The files under `assets/fonts` include the original Roboto variable font for Chromium and static Roboto 400 and 500 instances for native drawing. Google Sans Flex uses static weights 400 and 500 with optical size 18, width 100, and zero slant and roundness. Both reference apps use those same files. Material Symbols uses optical size 24, weight 400, grade 0, and fill 0 or 1. `tools/prepare_fonts.py` reproduces the static instances using fontTools with the original variable TTFs as inputs. Licenses accompany the Windows gallery and are bundled with the Android fixture.
