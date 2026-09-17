import os
import subprocess
import sys
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from fakepointer import script
from keyboard import Keys, Report, call, focus_field, keymap, panel_geometry, path, type_keys, wait_for
from screenshot import capture_workspace

MEDIA = Path(os.environ.get("KBOARD_MEDIA_DIR", Path(__file__).resolve().parents[3] / "docs" / "media"))
SCRATCH = Path(os.environ.get("KBOARD_TEST_ROOT", "/tmp"))


def shot(headroom=0.18):
    geometry = panel_geometry()
    image = capture_workspace(str(SCRATCH / "frame.png"))
    top = max(0, int(geometry["y"] - geometry["height"] * headroom))
    return image.crop((0, top, image.width, image.height))


def capture(name, headroom=0.18):
    shot(headroom).save(MEDIA / f"{name}.png")
    print("  saved", MEDIA / f"{name}.png")


def play(commands):
    worker = threading.Thread(target=script, args=(commands,))
    worker.start()
    return worker


def hold(label, seconds):
    x, y = Keys().center(label)
    worker = play([f"down 0 {x} {y}", "frame", f"sleep {int(seconds * 1000)}", "up 0", "frame"])
    time.sleep(0.4)
    return worker


def scene_keys(prefix):
    type_keys("keyb")
    time.sleep(1.0)
    print("  before hold", [entry["text"] for entry in keymap()["suggestions"]])
    worker = hold("k", 2.0)
    capture(prefix)
    capture(f"{prefix}-suggestions", headroom=0.05)
    worker.join()


def scene_popup(prefix):
    worker = hold("e", 2.4)
    time.sleep(1.5)
    capture(f"{prefix}-longpress")
    worker.join()


def scene_glide(prefix):
    keys = Keys()
    worker = play(path([keys.center(letter) for letter in "keyboard"], steps=5, delay=45))
    frames = []
    for _ in range(8):
        frames.append(shot())
        time.sleep(0.05)
    worker.join()
    frames[0].save(MEDIA / f"{prefix}-glide.gif", save_all=True, append_images=frames[1:], duration=180, loop=0, optimize=True)
    print("  saved", MEDIA / f"{prefix}-glide.gif")


def scene_panel(prefix, panel):
    call("OpenPanel", panel)
    time.sleep(2.2)
    capture(f"{prefix}-{panel}", headroom=0.05)
    call("OpenPanel", "keys")
    time.sleep(0.6)


SCENES = {"popup": scene_popup, "glide": scene_glide}


def main():
    report = Report()
    MEDIA.mkdir(parents=True, exist_ok=True)
    prefix = os.environ.get("KBOARD_SHOWCASE", "keyboard")
    focus_field("A")
    report.check(wait_for(lambda: keymap()["window"]["visible"]), "keyboard visible")
    time.sleep(1.0)
    scene_keys(prefix)
    for scene in os.environ.get("KBOARD_SHOWCASE_SCENES", "").split():
        SCENES.get(scene, lambda name, panel=scene: scene_panel(name, panel))(prefix)
    report.finish()


if __name__ == "__main__":
    main()
