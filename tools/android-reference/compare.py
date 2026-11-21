"""Capture the dedicated Compose reference app on a USB phone and compare the Windows gallery."""
import argparse
from io import BytesIO
import json
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import time

from PIL import Image, ImageChops, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[2]
PACKAGE = "dev.m3e.reference"
LOG_TAG = "M3EReference"
LOGICAL_SIZE = (360, 688)


class Phone:
    def __init__(self, path, fixture):
        self.command = [str(path), "-d"]
        self.fixture = fixture

    def adb(self, *args, binary=False):
        return subprocess.check_output([*self.command, *args], text=not binary, timeout=30)

    def require_foreground(self):
        windows = self.adb("shell", "dumpsys", "window", "displays")
        focus = next((line for line in windows.splitlines() if "mCurrentFocus=" in line), "")
        if PACKAGE not in focus:
            raise RuntimeError("Unlock the phone and allow the M3E reference app to be in the foreground.")

    def logs(self):
        pid = self.adb("shell", "pidof", "-s", PACKAGE).strip()
        return self.adb("logcat", "-d", "--pid", pid, "-s", f"{LOG_TAG}:I", "*:S")

    def start(self, palette, keep_awake=True):
        self.adb("shell", "am", "start", "-W", "-S", "-n", f"{PACKAGE}/.MainActivity", "--es", "palette", palette,
                 "--es", "fixture", self.fixture, "--ez", "capture", str(keep_awake).lower())
        deadline = time.monotonic() + 12
        while time.monotonic() < deadline:
            try:
                self.require_foreground()
                logs = self.logs()
                bounds = re.findall(r"captureBounds=(\d+),(\d+),(\d+),(\d+)", logs)
                color = re.search(rf"palette={palette} primary=(-?\d+) container=(-?\d+) fontScale=([\d.]+)", logs)
                if bounds and color:
                    return tuple(map(int, bounds[-1])), color.groups()[:2], float(color.group(3))
            except (RuntimeError, subprocess.CalledProcessError):
                pass
            time.sleep(.3)
        self.require_foreground()
        raise RuntimeError("The Compose reference activity did not publish fixture bounds.")

    def capture(self, bounds):
        self.require_foreground()
        result = Image.open(BytesIO(self.adb("exec-out", "screencap", "-p", binary=True))).convert("RGB")
        self.require_foreground()
        x0, y0, x1, y1 = bounds
        if not (0 <= x0 < x1 <= result.width and 0 <= y0 < y1 <= result.height):
            raise RuntimeError("The Compose fixture is clipped on this display.")
        return result.crop(bounds)

    def tap(self, bounds, x, y):
        self.require_foreground()
        x0, y0, x1, y1 = bounds
        self.adb("shell", "input", "tap", str(round(x0 + x * (x1 - x0) / LOGICAL_SIZE[0])),
                 str(round(y0 + y * (y1 - y0) / LOGICAL_SIZE[1])))

    def verify_touch(self, bounds):
        if self.fixture == "catalog":
            interactions = [(152, 40, "filter=false"), (53, 112, "segment=0"), (42, 608, "bottom=Search"),
                            (70, 480, "sheet=true"), (208, 480, "snackbar=true"), (308, 655, "snackbar=undo")]
        else:
            interactions = [(180, 136, "destination=Explore"), (46, 224, "favorite=false"),
                            (68, 406, "notes=1"), (255, 660, "slider=100")]
        for x, y, marker in interactions:
            self.tap(bounds, x, y)
            deadline = time.monotonic() + 2
            while marker not in self.logs() and time.monotonic() < deadline:
                time.sleep(.15)
            if marker not in self.logs():
                raise RuntimeError(f"Reference did not report touch event {marker}")
            if marker == "sheet=true":
                time.sleep(.5)
                self.require_foreground()
                self.adb("shell", "input", "keyevent", "KEYCODE_BACK")
                time.sleep(.5)
        return [marker for _, _, marker in interactions]


def strongest_delta(a, b):
    r, g, blue = ImageChops.difference(a, b).split()
    return ImageChops.lighter(r, ImageChops.lighter(g, blue))


def compare(android, native):
    if android.size != native.size:
        raise RuntimeError(f"Phone frame {android.size} differs from Windows frame {native.size}")
    delta = strongest_delta(android, native)
    context = android.convert("L").point(lambda value: 18 + value // 6).convert("RGB")
    diffs = Image.merge("RGB", (delta.point(lambda value: min(255, value * 4)),
                                delta.point(lambda value: min(180, value * 2)),
                                delta.point(lambda value: min(120, value))))
    heatmap = ImageChops.add(context, diffs)
    def foreground(frame):
        solid = Image.new("RGB", frame.size, frame.getpixel((0, 0)))
        return strongest_delta(frame, solid).point(lambda value: 255 if value > 12 else 0)
    mask = ImageChops.lighter(foreground(android), foreground(native))
    changed = delta.point(lambda value: 255 if value > 12 else 0)
    pixels_changed = changed.histogram()[255]
    ink_changed = ImageChops.multiply(mask, changed).histogram()[255]
    return heatmap, {
        "pixels_above_12_percent": round(100 * pixels_changed / (delta.width * delta.height), 3),
        "foreground_pixels_above_12_percent": round(100 * ink_changed / max(1, mask.histogram()[255]), 3)
    }


def join(images):
    columns = ("Android phone / Jetpack Compose", "Windows / Dear ImGui", "Pixel difference")
    w, h = images[0].size
    header = 44
    figure = Image.new("RGB", (w * 3, h + header), "#242128")
    pen = ImageDraw.Draw(figure)
    font = ImageFont.truetype(str(ROOT / "assets/fonts/GoogleSansFlex-Medium.ttf"), 25)
    for i, (image, heading) in enumerate(zip(images, columns)):
        figure.paste(image, (w * i, header))
        pen.text((w * i + 12, 7), heading, font=font, fill="white")
    return figure


def stop_indicators(image, primary):
    scale = image.width / LOGICAL_SIZE[0]
    rgb = bytes.fromhex(f"{int(primary) & 0xffffff:06x}")
    poses = (604, 660)
    present = []
    for y in poses:
        crop = image.crop(tuple(round(v * scale) for v in (245, y - 4, 247, y + 4)))
        colors = crop.getcolors(maxcolors=crop.width * crop.height)
        present.append(any(max(abs(a - b) for a, b in zip(color, rgb)) <= 3 for _, color in colors))
    return present


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--adb", type=Path, default=Path("adb"))
    parser.add_argument("--apk", type=Path, help="Install the debug reference app before capturing")
    parser.add_argument("--gallery", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--fixture", choices=("original", "catalog"), default="original")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    phone = Phone(args.adb, args.fixture)
    if phone.adb("get-state").strip() != "device":
        raise RuntimeError("Connect and authorize one Android phone over USB.")
    if args.apk:
        print(phone.adb("install", "-r", str(args.apk.resolve(strict=True))).strip(), flush=True)
    report = {"android_release": phone.adb("shell", "getprop", "ro.build.version.release").strip(),
              "fixture": args.fixture,
              "compose_material3": "1.5.0", "typeface": "Google Sans Flex", "logical_size_dp": LOGICAL_SIZE, "samples": {}}
    default_colors = {}
    for scheme in ("light", "dark"):
        print(f"Capturing {scheme} on the phone and Windows", flush=True)
        bounds, default_colors[scheme], font_scale = phone.start(scheme)
        time.sleep(.6)
        android = phone.capture(bounds)
        scale = android.width / LOGICAL_SIZE[0]
        out = args.output / scheme
        out.mkdir(exist_ok=True)
        with TemporaryDirectory() as path:
            native_case = "android-catalog-phone" if args.fixture == "catalog" else "android-phone"
            subprocess.run([str(args.gallery.resolve(strict=True)), "--capture", native_case, scheme, path,
                            f"--scale={scale}", f"--font-scale={font_scale}", "--reduced-motion"], check=True, timeout=35)
            with Image.open(Path(path) / "native.bmp") as bitmap:
                native = bitmap.convert("RGB")
        diff, metrics = compare(android, native)
        for name, image in (("phone", android), ("native", native), ("diff", diff),
                            ("comparison", join((android, native, diff)))):
            image.save(out / f"{name}.png", optimize=True)
        sample = {"scale": scale, "font_scale": font_scale, "pixels": android.size, **metrics}
        if args.fixture == "original":
            android_dots = stop_indicators(android, default_colors[scheme][0])
            native_dots = stop_indicators(native, default_colors[scheme][0])
            sample["stop_indicator_at_62_and_98_percent"] = {"phone": android_dots, "windows": native_dots}
            if android_dots != [True, False] or native_dots != android_dots:
                raise RuntimeError(f"Slider end-dot mismatch in {scheme}: phone {android_dots}, Windows {native_dots}")
        report["samples"][scheme] = sample
    report["touch_events"] = phone.verify_touch(bounds)
    _, dynamic, _ = phone.start("dynamic-light")
    report["wallpaper_palette_differs_from_default"] = dynamic != default_colors["light"]
    phone.start("light", keep_awake=False)
    (args.output / "metrics.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
