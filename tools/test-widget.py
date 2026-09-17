#!/usr/bin/env python3
import os
import shutil
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
HARNESS = REPO / "tests" / "nested" / "harness"
sys.path.insert(0, str(HARNESS))

from nested import run_script

RUNNER = """
import os, subprocess, sys, time
sys.path.insert(0, {harness!r})
from fakepointer import click
from kwinsession import run_script
from screenshot import capture_workspace

out = {out!r}
package = {package!r}
problems = []


def viewer_geometry():
    for _ in range(40):
        found = run_script('for (const w of workspace.windowList()) {{ if (w.resourceName == "plasmoidviewer") {{ const g = w.frameGeometry; print("MARK|" + g.x + "|" + g.y + "|" + g.width + "|" + g.height); }} }}')
        if found:
            return [float(v) for v in found[0].split("|")]
        time.sleep(0.25)
    return None


def kboard(method, *args):
    return subprocess.run(["qdbus6", "org.devl0rd.KBoard", "/KBoard", "org.devl0rd.KBoard." + method, *args], capture_output=True, text=True).stdout.strip()


def calls():
    with open(os.environ["KBOARD_FAKE_CALLS"]) as log:
        return log.read().split()


def shot(name):
    time.sleep(1.5)
    capture_workspace(out + "/" + name + ".png")


def expect(description, condition):
    print(("  ok    " if condition else "  FAIL  ") + description)
    if not condition:
        problems.append(description)


def viewer(formfactor, location, size):
    containment = ["-c", "org.kde.panel"] if formfactor == "horizontal" else []
    process = subprocess.Popen(["plasmoidviewer", *containment, "-a", package, "-f", formfactor, "-l", location, "-s", size])
    time.sleep(6)
    return process, viewer_geometry()


card, geometry = viewer("planar", "floating", "460x760")
expect("desktop card window opens", geometry is not None)
shot("card-hidden")
kboard("Show")
shot("card-shown")
kboard("OpenPanel", "emoji")
shot("card-emoji")
kboard("Hide")
shot("card-hidden-again")
card.terminate()
card.wait()

panel, geometry = viewer("horizontal", "bottomedge", "400x160")
expect("panel window opens", geometry is not None)
if geometry:
    x, y, w, h = geometry
    icon_x, icon_y = x + 84, y + 80
    before = len(calls())
    click(icon_x, icon_y)
    time.sleep(1)
    expect("clicking the panel icon calls Toggle", "Toggle" in calls()[before:])
    shot("panel-shown")
    before = len(calls())
    click(icon_x, icon_y)
    time.sleep(1)
    expect("clicking again hides the keyboard", "Toggle" in calls()[before:] and kboard("IsVisible") == "false")
    shot("panel-hidden")
panel.terminate()
panel.wait()

subprocess.run(["kwriteconfig6", "--file", "kwinrc", "--group", "Wayland", "--key", "InputMethod", "/usr/share/applications/org.kde.plasma.keyboard.desktop"])
card, geometry = viewer("planar", "floating", "460x760")
shot("card-inactive")
card.terminate()
card.wait()

for problem in problems:
    print("  PROBLEM " + problem)
print("RESULT:", "FAIL" if problems else "PASS")
"""


def stage_package(staging):
    package = staging / "org.devl0rd.kboard"
    shutil.copytree(REPO / "plasmoids" / "org.devl0rd.kboard", package, ignore=shutil.ignore_patterns("lib"))
    lib = package / "contents" / "ui" / "lib"
    lib.mkdir()
    for pattern in ("*.qml", "*.js"):
        for source in (REPO / "shared" / "common").glob(pattern):
            shutil.copy(source, lib)
    return package


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp(prefix="kboard-widget-")).resolve()
    out.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix="kboard-widget-test-"))
    fake_build = staging / "build"
    (fake_build / "bin").mkdir(parents=True)
    os.symlink(REPO / "tools" / "fake-kboard.py", fake_build / "bin" / "kboard")
    os.environ["KBOARD_BUILD_DIR"] = str(fake_build)
    os.environ["KBOARD_FAKE_CALLS"] = str(staging / "calls.log")
    desktop = staging / "org.devl0rd.kboard.desktop"
    shutil.copy(REPO / "src" / "keyboard" / "org.devl0rd.kboard.desktop", desktop)
    runner = staging / "runner.py"
    runner.write_text(RUNNER.format(harness=str(HARNESS), out=str(out), package=str(stage_package(staging))))
    status = run_script(f'export QT_QPA_PLATFORM=wayland\npython3 {runner} > "$KBOARD_REPORT" 2>&1\n', 180,
                        extra_kwinrc=f"InputMethod={desktop}")
    shutil.rmtree(staging, ignore_errors=True)
    print("screenshots in", out)
    return status


if __name__ == "__main__":
    sys.exit(main())
