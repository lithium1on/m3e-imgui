// Converts the pinned Matraic shape polygons to normalized integer coordinates.
// Usage: node --experimental-strip-types tools/prepare_shapes.mjs .cache/m3e src/shape_tokens.hpp
import { readFileSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";
import { pathToFileURL } from "node:url";

const [upstream, output] = process.argv.slice(2);
const root = resolve(upstream, "packages/web/src");
const utility = pathToFileURL(resolve(root, "core/shared/utils/generateClipPaths.ts")).href;
const { generateClipPaths } = await import(utility);
const input = readFileSync(resolve(root, "shape/ShapePolygon.ts"), "utf8");
const shapes = Object.fromEntries(
    [...input.matchAll(/^  (?:"([a-z0-9-]+)"|([a-z]+)):\s*"([^"]+)",$/gm)]
        .map(([, quoted, unquoted, path]) => [quoted || unquoted, path])
);

// This order matches ExpressiveShape in components.hpp.
const names = [
    "circle", "square", "pill", "diamond", "4-leaf-clover", "burst", "heart",
    "4-sided-cookie", "6-sided-cookie", "7-sided-cookie", "8-leaf-clover",
    "9-sided-cookie", "12-sided-cookie", "arch", "arrow", "boom", "bun", "fan",
    "flower", "gem", "ghost-ish", "hexagon", "oval", "pentagon", "pixel-circle",
    "pixel-triangle", "puffy", "puffy-diamond", "semicircle", "slanted", "soft-boom",
    "soft-burst", "sunny", "triangle", "very-sunny"
];
const sourceOrder = Object.keys(shapes);
if (sourceOrder.length !== names.length || names.some(name => !shapes[name]))
    throw new Error("The upstream shape names differ from ExpressiveShape.");
const paths = generateClipPaths(Object.values(shapes), 300);
const rows = names.map(name => {
    const points = paths[sourceOrder.indexOf(name)].split(", ").map(point =>
        point.split(" ").map(percent => Math.round(Number.parseFloat(percent) * 100))
    );
    if (points.length === 299)
        points.push(points[0]);
    if (points.length !== 300 || points.some(point => point.length !== 2 || point.some(x => !Number.isInteger(x))))
        throw new Error(`Invalid normalized polygon: ${name}`);
    const lines = [];
    for (let i = 0; i < points.length; i += 8)
        lines.push("        " + points.slice(i, i + 8).map(point => `{${point.join(", ")}}`).join(", ") + ",");
    return `    {"${name}", {\n${lines.join("\n")}\n    }},`;
});

writeFileSync(output,
    `// Generated from Matraic M3E ShapePolygon.ts. See licenses/Matraic-M3E-MIT.txt.\n` +
    `// Pinned revision: 489ad4c65b9ee348ee859cda214311cf39bcc842.\n` +
    `#pragma once\n\n#include <cstdint>\n\nnamespace m3e::detail {\n\n` +
    `struct ShapePoint { std::uint16_t x, y; };\n` +
    `struct ShapeToken { const char* name; ShapePoint points[300]; };\n` +
    `inline constexpr ShapeToken MaterialShapeTokens[]{\n${rows.join("\n")}\n};\n\n} // namespace m3e::detail\n`
);
