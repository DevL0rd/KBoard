import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from keyboard import Report, current_line, focus_field, keymap, panel_geometry, tap, wait_for


def main():
    report = Report()
    focus_field("A")
    report.check(wait_for(lambda: keymap()["window"]["visible"]), f"the keyboard panel appeared: {panel_geometry()}")
    tap("q")
    report.check(wait_for(lambda: current_line("A").lower() == "q"), f"tapping the first key typed {current_line('A')!r}")
    report.finish()


if __name__ == "__main__":
    main()
