import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from keyboard import Report, focus_field, keymap, wait_for


def main():
    report = Report()
    focus_field("A")
    report.check(wait_for(lambda: keymap()["window"]["visible"]), "keyboard visible")
    application = wait_for(lambda: keymap()["application"], timeout=6)
    report.check(bool(application), f"the host learns the focused application: {application!r}")
    types = wait_for(lambda: [key["type"] for key in keymap()["keys"]], timeout=4)
    report.check("modifier" in types, "the app rule forced the desktop row on")
    report.finish()


if __name__ == "__main__":
    main()
