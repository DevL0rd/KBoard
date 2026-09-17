import os
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from fakepointer import click, touch
from kwinsession import run_script


def panel():
    printed = run_script('for (const w of workspace.stackingOrder) { if (w.inputMethod) { const g = w.frameGeometry; print("MARK|" + g.x + "|" + g.y + "|" + g.width + "|" + g.height + "|" + !w.hidden); } }')
    return tuple(printed[0].split("|")) if printed else None


def typed():
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        return [line.strip().split("kboard-test-typed:", 1)[1] for line in log if "kboard-test-typed:" in line]


def main():
    problems = []
    window = run_script('for (const w of workspace.windowList()) { if (w.caption == "A") { const g = w.frameGeometry; print("MARK|" + g.x + "|" + g.y + "|" + g.width + "|" + g.height); } }')
    x, y, width, height = (float(v) for v in window[0].split("|"))
    click(x + width / 2, y + height - 35)
    time.sleep(3)
    geometry = panel()
    print("panel", geometry)
    if not geometry:
        problems.append("keyboard panel never appeared")
    else:
        px, py, pw, ph = (float(v) for v in geometry[:4])
        target = os.environ.get("KBOARD_SMOKE_TARGET")
        tx, ty = (float(v) for v in target.split(",")) if target else (px + pw * 0.08, py + ph * 0.3)
        touch(1, px + tx if target else tx, py + ty if target else ty)
        time.sleep(1.5)
    print("typed", typed())
    if not any(entry.endswith(":q") for entry in typed()):
        problems.append("tapping the first key did not type q")
    for problem in problems:
        print("  PROBLEM " + problem)
    print("RESULT:", "FAIL" if problems else "PASS")


if __name__ == "__main__":
    main()
