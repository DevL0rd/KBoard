import json
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

from keyboard import Report, focus_field, keymap, output_geometry, panel_geometry, wait_for

LANDSCAPE = 0.36
PORTRAIT = 0.3


def output_name():
    listing = subprocess.run(["kscreen-doctor", "-j"], capture_output=True, text=True, check=True).stdout
    return json.loads(listing)["outputs"][0]["name"]


def fits(report, label):
    def matches():
        output = output_geometry()
        panel = panel_geometry()
        if not panel or not panel["shown"]:
            return None
        factor = PORTRAIT if output["height"] > output["width"] else LANDSCAPE
        expected = round(output["height"] * factor)
        good = (abs(panel["width"] - output["width"]) < 1 and abs(panel["x"] - output["x"]) < 1
                and abs(panel["y"] + panel["height"] - output["y"] - output["height"]) < 1 and abs(panel["height"] - expected) <= 1)
        return (good, output, panel, keymap()["window"])
    result = wait_for(lambda: (lambda value: value if value and value[0] else None)(matches()), timeout=6) or matches()
    if result is None:
        report.check(False, f"{label}: keyboard not shown")
        return
    good, output, panel, window = result
    report.check(good, f"{label}: panel {panel} fits output {output}, window {window}")


def apply(setting):
    subprocess.run(["kscreen-doctor", f"output.{output_name()}.{setting}"], capture_output=True, text=True, check=True)
    time.sleep(2.5)


def main():
    report = Report()
    focus_field("A")
    fits(report, "initial 1920x1080")
    apply("rotation.left")
    fits(report, "rotated to portrait")
    apply("rotation.normal")
    fits(report, "rotated back to landscape")
    apply("scale.1.5")
    fits(report, "scale 1.5")
    apply("scale.1")
    fits(report, "scale 1")
    report.finish()


if __name__ == "__main__":
    main()
