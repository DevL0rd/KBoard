import os
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from fakepointer import script
from kwinsession import activate, for_window, run_script
from keyboard import Keys, Report, current_line, focus_field, keymap, output_geometry, panel_geometry, type_keys, wait_for

CLIENTS = Path(__file__).resolve().parent.parent / "clients"


def taps(label):
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        return [line for line in log if f"kboard-test-tap:{label}:" in line]


def main():
    report = Report()
    output = output_geometry()
    subprocess.Popen(["qml6", str(CLIENTS / "tap-client.qml"), "--", "B", str(int(output["width"])), str(int(output["height"]))],
                     stdout=subprocess.DEVNULL, stderr=open(os.environ["KBOARD_KWIN_LOG"], "a"))
    time.sleep(3)
    run_script(for_window("B", "w.frameGeometry = {x: 0, y: 120, width: w.frameGeometry.width, height: w.frameGeometry.height - 120};"))
    activate("A")
    time.sleep(0.5)
    focus_field("A")
    report.check(wait_for(lambda: keymap()["split"], timeout=6), "the keyboard splits on an ultrawide output")
    time.sleep(1.0)
    state = keymap()
    report.check(len(state["rects"]) == 2, f"input mask has two halves: {state['rects']}")
    panel = panel_geometry()
    report.check(abs(panel["width"] - output["width"]) < 1, f"panel spans the output width: {panel}")
    type_keys("split")
    time.sleep(0.6)
    report.check(current_line("A").lower() == "split", f"typing on both halves typed {current_line('A')!r}")
    keys = Keys()
    left, right = sorted(state["rects"], key=lambda rect: rect["x"])
    above = len(taps("B"))
    script(["down 0 400 800", "frame", "sleep 60", "up 0", "frame", "sleep 400"])
    report.check(wait_for(lambda: len(taps("B")) > above), "a tap in the headroom above the keys reaches the app underneath")
    activate("A")
    focus_field("A")
    wait_for(lambda: keymap()["split"])
    gap_x, gap_y = keys.screen((left["x"] + left["width"] + right["x"]) / 2, right["y"] + right["height"] * 0.7)
    before = len(taps("B"))
    script([f"down 0 {gap_x} {gap_y}", "frame", "sleep 60", "up 0", "frame", "sleep 400"])
    report.known(wait_for(lambda: len(taps("B")) > before), f"a tap in the gap at {gap_x:.0f},{gap_y:.0f} reaches the app underneath",
                 "KWin hit-tests the input panel by the bounding box of its input region")
    report.finish()


if __name__ == "__main__":
    main()
