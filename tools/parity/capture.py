"""Capture matched M3E browser and DX11 samples and produce a pixel-diff report."""
import argparse
from functools import partial
from contextlib import contextmanager
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
from threading import Thread
import time
from urllib.parse import urlencode

from PIL import Image, ImageChops, ImageDraw
from playwright.sync_api import sync_playwright


ROOT = Path(__file__).resolve().parents[2]
WIDTH, HEIGHT = 760, 390
CATALOG_HEIGHT = 810
REVISION = "489ad4c65b9ee348ee859cda214311cf39bcc842"
GESTURES = {
    "button": (56, 44, 56, 44, True),
    "icon-button": (44, 44, 44, 44, True),
    "connected": (130, 100, 130, 100, True),
    "tabs": (143, 54, 143, 54, True),
    "menu": (60, 44, 748, 378, True),
    "menu-vibrant": (60, 44, 748, 378, True),
    "submenu": (60, 44, 70, 136, True),
    "text-field": (80, 59, 80, 59, True),
    "checkbox": (44, 44, 44, 44, True),
    "switch": (44, 40, 44, 40, True),
    "slider": (111, 46, 196, 46, True),
    "dialog": (75, 44, 748, 378, True),
    "tooltip": (168, 80, 168, 80, False),
    "rich-tooltip": (168, 80, 748, 378, True),
    "connected-icons": (44, 44, 44, 44, True),
    "rich-hover": (168, 80, 168, 80, False),
}
OVERLAYS = {"menu", "menu-vibrant", "submenu", "dialog", "tooltip", "rich-tooltip", "rich-hover"}
CATALOG_GESTURES = {
    "selection": (160, 77, 160, 77, True),
    "search-inputs": (140, 80, 140, 80, True),
    "date-time": (283, 72, 283, 72, True),
    "cards-lists": (66, 333, 66, 333, True),
    "navigation-layout": (400, 73, 400, 73, True),
    "panes-carousel": (566, 276, 566, 276, True),
    "sheets-actions": (109, 78, 109, 78, True),
    "feedback-shapes": (748, 778, 748, 778, False),
}


class QuietHandler(SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


@contextmanager
def fixture_server():
    handler = partial(QuietHandler, directory=str(ROOT))
    with ThreadingHTTPServer(("127.0.0.1", 0), handler) as server:
        thread = Thread(target=server.serve_forever, daemon=True)
        thread.start()
        try:
            yield f"http://127.0.0.1:{server.server_port}"
        finally:
            server.shutdown()
            thread.join()


def launch_native(args, case, scheme, output, motion=False):
    command = [str(args.gallery), "--capture", case, scheme, str(output),
               f"--scale={args.scale}", f"--density={args.density}"]
    if motion:
        command.append("--motion")
    if args.reduced_motion:
        command.append("--reduced-motion")
    subprocess.run(command, check=True, timeout=45)


def navigate(page, base, case, scheme, density):
    query = urlencode(dict(case=case, scheme=scheme, density=density))
    page.goto(f"{base}/tools/parity/reference.html?{query}")
    page.wait_for_function("window.referenceReady === true", timeout=10000)
    page.mouse.move(748, 378)
    page.wait_for_timeout(480)


def browser_static(page, case, output):
    if case in OVERLAYS:
        x, y, next_x, next_y, clicks = GESTURES[case]
        page.mouse.move(x, y)
        if clicks:
            page.mouse.click(x, y)
            page.wait_for_timeout(85)
        page.mouse.move(next_x, next_y)
        if case == "submenu":
            # Matraic opens a submenu on pointer hover. Click is its other entry point.
            page.wait_for_timeout(180)
        page.wait_for_timeout(680)
        selector = {"menu": "#actions", "menu-vibrant": "#actions", "submenu": "#sub",
                    "dialog": "#dialog", "tooltip": "m3e-tooltip", "rich-tooltip": "m3e-rich-tooltip",
                    "rich-hover": "m3e-rich-tooltip"}[case]
        page.wait_for_function(
            """selector => {
                const element = document.querySelector(selector);
                return element.matches(':popover-open') || !!element.shadowRoot?.querySelector('dialog[open], :popover-open');
            }""", arg=selector, timeout=2500)
    page.screenshot(path=str(output / "browser.png"), scale="device")
    return page.evaluate("""() => Array.from(document.querySelectorAll('.sample > *, m3e-menu, m3e-dialog, m3e-tooltip, m3e-rich-tooltip'))
        .flatMap(element => {
          const shadow = element.shadowRoot;
          const shown = shadow?.querySelector('dialog[open], :popover-open');
          const box = (shown || element).getBoundingClientRect();
          return box.width && box.height ? [{
            tag: element.localName, variant: element.variant, size: element.size,
            x: box.x, y: box.y, width: box.width, height: box.height
          }] : [];
        })""")


def browser_motion(page, case, output):
    x, y, next_x, next_y, clicks = (GESTURES | CATALOG_GESTURES)[case]
    t0 = time.perf_counter()
    events = [(0, lambda: page.mouse.move(x, y))]
    if clicks:
        events.extend([(0.1, page.mouse.down), (.25, page.mouse.up)])
    if case == "slider":
        events.append((.2, lambda: page.mouse.move(next_x, next_y)))
    else:
        events.append((1 / 3, lambda: page.mouse.move(next_x, next_y)))
    events.sort(key=lambda entry: entry[0])
    captures = []
    for frame in range(12):
        deadline = frame / 12
        while events and events[0][0] <= deadline:
            moment, action = events.pop(0)
            time.sleep(max(0, t0 + moment - time.perf_counter()))
            action()
        time.sleep(max(0, t0 + deadline - time.perf_counter()))
        start = time.perf_counter() - t0
        page.screenshot(path=str(output / f"browser-{frame:02}.png"), scale="device")
        end = time.perf_counter() - t0
        captures.append(round((start + end) * 500))
    return captures


def comparison(browser, native):
    if browser.size != native.size:
        raise RuntimeError(f"Size mismatch {browser.size} and {native.size}")
    raw = ImageChops.difference(browser, native)
    r, g, b = raw.split()
    strongest = ImageChops.lighter(r, ImageChops.lighter(g, b))
    histogram = strongest.histogram()
    count = browser.width * browser.height
    different = round(100 * sum(histogram[13:]) / count, 3)
    mean = round(sum(level * pixels for level, pixels in enumerate(histogram)) / count, 2)
    # Composite residual over grayscale context for a legible overlay in both schemes.
    context = browser.convert("L").point(lambda value: 22 + value // 6).convert("RGB")
    heat = Image.merge("RGB", (strongest.point(lambda value: min(255, value * 4)),
                               strongest.point(lambda value: min(180, value * 2)),
                               strongest.point(lambda value: min(120, value))))
    return ImageChops.add(context, heat), different, mean


def strip(images, labels):
    width, height = images[0].size
    header = 22
    figure = Image.new("RGB", (width * len(images), height + header), "#242128")
    draw = ImageDraw.Draw(figure)
    for index, (image, label) in enumerate(zip(images, labels)):
        x = index * width
        figure.paste(image, (x, header))
        draw.text((x + 9, 5), label, fill="white")
    return figure


def save_static(output):
    browser = Image.open(output / "browser.png").convert("RGB")
    native_bmp = output / "native.bmp"
    native = Image.open(native_bmp).convert("RGB")
    native.save(output / "native.png", optimize=True)
    native_bmp.unlink()
    diff, changed, mean = comparison(browser, native)
    diff.save(output / "diff.png", optimize=True)
    strip((browser, native, diff), ("Matraic / Chromium", "Dear ImGui / DX11", "Absolute RGB difference, amplified")).save(
        output / "comparison.png", optimize=True)
    return {"pixels_above_12_percent": changed, "mean_max_rgb_delta": mean}


def save_motion(frames, output, times, ffmpeg):
    for i, time_ms in enumerate(times):
        native_index = min(11, int(round(time_ms * 12 / 1000)))
        browser = Image.open(frames / f"browser-{i:02}.png").convert("RGB")
        native = Image.open(frames / f"frame-{native_index:02}.bmp").convert("RGB")
        strip((browser, native), (f"Matraic / Chromium   ~{time_ms} ms", f"Dear ImGui / DX11   {round(native_index / 12 * 1000)} ms")).save(
            frames / f"joined-{i:02}.png")
    subprocess.run([ffmpeg, "-loglevel", "error", "-y", "-framerate", "12", "-i", str(frames / "joined-%02d.png"),
                    "-vf", "pad=ceil(iw/2)*2:ceil(ih/2)*2", "-c:v", "libx264", "-crf", "22",
                    "-pix_fmt", "yuv420p", "-movflags", "+faststart", str(output / "motion.mp4")], check=True, timeout=45)


def write_index(output, metadata):
    rows = []
    for row in metadata["samples"]:
        case, scheme = row["case"], row["scheme"]
        prefix = f"{scheme}/{case}"
        video = f'<video controls preload="none" src="{prefix}/motion.mp4"></video>' if row.get("motion_ms") else ""
        rows.append(f"""<section><h2>{case} · {scheme}</h2><p>{row['pixels_above_12_percent']:.3f}%
            of pixels exceed a maximum RGB-channel delta of 12.</p>
            <a href="{prefix}/comparison.png"><img loading="lazy" src="{prefix}/comparison.png"
            alt="Browser, native and pixel difference for {case} {scheme}"></a>{video}</section>""")
    (output / "index.html").write_text("""<!doctype html><html lang="en"><meta charset="utf-8">
      <title>M3E browser and native comparisons</title>
      <style>body{font:16px system-ui;margin:2rem auto;padding:0 1rem;max-width:1280px;color:#332c39;background:#faf7fc}
      section{margin-block:1.5rem;padding:1.25rem;background:white;border-radius:16px}h2{margin:0;text-transform:capitalize}
      img,video{width:100%;display:block;margin-top:.5rem}</style>
      <h1>M3E browser and native comparisons</h1>
      <p>Open a comparison for full-resolution pixels. See the capture notes for remaining differences.</p>"""
      + "".join(rows) + "</html>", encoding="utf-8")
    (output / "metrics.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gallery", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--channel", help="Installed Chromium channel, e.g. msedge")
    parser.add_argument("--case", action="append", choices=(*GESTURES, *CATALOG_GESTURES))
    parser.add_argument("--catalog", action="store_true", help="Capture the expanded widget catalog")
    parser.add_argument("--scheme", action="append", choices=("light", "dark"))
    parser.add_argument("--scale", type=float, default=1.0)
    parser.add_argument("--density", type=float, default=0.0)
    parser.add_argument("--reduced-motion", action="store_true")
    parser.add_argument("--motion", action="store_true")
    parser.add_argument("--update", action="store_true", help="Preserve other samples in an existing matching report")
    parser.add_argument("--ffmpeg", default="ffmpeg")
    args = parser.parse_args()
    cases = CATALOG_GESTURES if args.catalog else GESTURES
    height = CATALOG_HEIGHT if args.catalog else HEIGHT
    if any(case not in cases for case in args.case or ()):
        parser.error("Use --catalog for expanded widgets and omit it for the original cases.")
    args.gallery = args.gallery.resolve(strict=True)
    args.output = args.output.resolve()
    if not (ROOT / "tools/parity/node_modules/@m3e/web/dist/all.min.js").exists():
        parser.error("Install the pinned fixture with npm ci --prefix tools/parity")
    metadata = dict(upstream_revision=REVISION, upstream_version="2.13.5", logical_size=[WIDTH, height],
                    scale=args.scale, density=args.density, reduced_motion=args.reduced_motion, samples=[])
    with fixture_server() as base, sync_playwright() as playwright:
        browser = playwright.chromium.launch(channel=args.channel, args=["--hide-scrollbars"])
        metadata["chromium_version"] = browser.version
        existing = args.output / "metrics.json"
        if args.update and existing.exists():
            previous = json.loads(existing.read_text(encoding="utf-8"))
            for key, value in metadata.items():
                if key != "samples" and previous.get(key) != value:
                    parser.error(f"Existing report has a different {key}")
            metadata["samples"] = previous["samples"]
        for scheme in args.scheme or ("light", "dark"):
            context = browser.new_context(viewport={"width": WIDTH, "height": height}, device_scale_factor=args.scale,
                                          color_scheme=scheme, reduced_motion="reduce" if args.reduced_motion else "no-preference")
            for case in args.case or cases:
                print(f"{scheme}/{case}", flush=True)
                output = args.output / scheme / case
                output.mkdir(parents=True, exist_ok=True)
                page = context.new_page()
                js_errors = []
                page.on("pageerror", lambda error: js_errors.append(str(error)))
                try:
                    navigate(page, base, case, scheme, args.density)
                    geometry = browser_static(page, case, output)
                    launch_native(args, case, scheme, output)
                    stats = dict(case=case, scheme=scheme, browser_boxes=geometry, **save_static(output))
                    if args.motion:
                        with TemporaryDirectory() as temporary:
                            frames = Path(temporary)
                            launch_native(args, case, scheme, frames, motion=True)
                            navigate(page, base, case, scheme, args.density)
                            times = browser_motion(page, case, frames)
                            save_motion(frames, output, times, args.ffmpeg)
                            stats["motion_ms"] = times
                    if js_errors:
                        raise RuntimeError("; ".join(js_errors))
                    rows = metadata["samples"]
                    rows[:] = [row for row in rows if (row["scheme"], row["case"]) != (scheme, case)]
                    rows.append(stats)
                    rows.sort(key=lambda row: (row["scheme"] == "dark", tuple(cases).index(row["case"])))
                    write_index(args.output, metadata)
                finally:
                    page.close()
            context.close()
        browser.close()


if __name__ == "__main__":
    main()
