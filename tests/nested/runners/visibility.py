import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from fakepointer import script
from keyboard import Keys, Report, call, focus_field, keymap, panel_geometry, typed, wait_for


def visible():
    geometry = panel_geometry()
    return call("IsVisible") == "true" and keymap()["window"]["visible"] and geometry is not None and geometry["shown"]


def hidden():
    geometry = panel_geometry()
    return call("IsVisible") == "false" and not keymap()["window"]["visible"] and (geometry is None or not geometry["shown"])


def check_dbus(report):
    call("Show")
    report.check(wait_for(visible), "D-Bus Show without a focused field shows the keyboard")
    call("Hide")
    report.check(wait_for(hidden), "D-Bus Hide unmaps the keyboard")
    call("Toggle")
    report.check(wait_for(visible), "Toggle shows it again")
    call("Toggle")
    report.check(wait_for(hidden), "Toggle hides it again")


def check_focus(report):
    focus_field("A")
    report.check(wait_for(visible), "focusing a text field shows the keyboard")
    call("Hide")
    report.check(wait_for(hidden), "Hide works while a field is focused")
    focus_field("A")
    report.check(wait_for(visible), "tapping the field again brings the keyboard back")


def check_emoji(report):
    call("OpenPanel", "emoji")
    report.check(wait_for(lambda: call("CurrentPanel") == "emoji" and visible()), "OpenPanel emoji opens the emoji panel")
    cells = wait_for(lambda: keymap()["cells"], timeout=6)
    report.check(bool(cells), f"emoji grid shows cells: {[cell['text'] for cell in cells or []]}")
    if cells:
        cell = cells[0]
        keys = Keys()
        x, y = keys.screen(cell["x"] + cell["width"] / 2, cell["y"] + cell["height"] / 2)
        script([f"down 0 {x} {y}", "frame", "sleep 50", "up 0", "frame", "sleep 300"])
        report.check(wait_for(lambda: typed("A").endswith(cell["text"])), f"tapping {cell['text']} committed it, field has {typed('A')!r}")
    call("OpenPanel", "keys")
    report.check(wait_for(lambda: keymap()["panel"] == "keys"), "OpenPanel keys returns to the letters")


def main():
    report = Report()
    for step in (check_dbus, check_focus, check_emoji):
        try:
            step(report)
        except Exception as error:
            report.check(False, f"{step.__name__} raised {error!r}")
    report.finish()


if __name__ == "__main__":
    main()
