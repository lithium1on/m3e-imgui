"""Build the native static font instances from the pinned upstream TTF files."""
import argparse
from pathlib import Path

from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont


def instantiate(source, destination, axes, update_names=True):
    with TTFont(source, recalcTimestamp=False) as font:
        instantiateVariableFont(font, axes, inplace=True, updateFontNames=update_names)
        font.save(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("roboto", type=Path)
    parser.add_argument("symbols", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--google-sans-flex", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    for weight, name in ((400, "Regular"), (500, "Medium")):
        instantiate(args.roboto, args.output / f"Roboto-{name}.ttf", {"wdth": 100, "wght": weight})
        if args.google_sans_flex:
            instantiate(
                args.google_sans_flex,
                args.output / f"GoogleSansFlex-{name}.ttf",
                {"opsz": 18, "wdth": 100, "wght": weight, "ROND": 0, "slnt": 0},
                update_names=False,
            )
    for fill, name in ((0, ""), (1, "-Filled")):
        instantiate(
            args.symbols,
            args.output / f"MaterialSymbolsOutlined{name}.ttf",
            {"FILL": fill, "GRAD": 0, "opsz": 24, "wght": 400},
            update_names=False,
        )


if __name__ == "__main__":
    main()
