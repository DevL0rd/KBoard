import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

import keyboard as kb
from kwinsession import qdbus
from keyboard import Report, focus_field, keymap, wait_for

CTRL_KEY = 0x01000021
META_KEY = 0x01000022
ALT_KEY = 0x01000023
SHIFT_KEY = 0x01000020
KEY_C = 0x43
KEY_D = 0x44


def events():
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        return [line.strip().split("kboard-test-mod:", 1)[1].split(":") for line in log if "kboard-test-mod:" in line]


def text():
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        lines = [line.strip().split("kboard-test-text:", 1)[1] for line in log if "kboard-test-text:" in line]
    return lines[-1] if lines else ""


def since(mark):
    return events()[mark:]


def seen(mark, phase, key, mods):
    return any(event[0] == phase and int(event[1]) == key and event[2] == mods for event in since(mark))


def shortcut_fired(marker):
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        return log.read().count(marker)


def global_shortcuts(report, keys):
    script = Path(os.environ["KBOARD_TEST_ROOT"]) / "shortcut.js"
    script.write_text('registerShortcut("KBoardTest", "KBoard test", "Meta+J", function () { print("KBOARD-META-J-FIRED"); });\n'
                      'registerShortcut("KBoardTestMeta", "KBoard test Meta alone", "Meta", function () { print("KBOARD-META-ALONE-FIRED"); });')
    script_id = qdbus("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting.loadScript", str(script), "kboard-shortcut")
    qdbus("org.kde.KWin", f"/Scripting/Script{script_id}", "org.kde.kwin.Script.run")
    time.sleep(1)
    before = shortcut_fired("KBOARD-META-J-FIRED")
    kb.tap("Super", keys=keys)
    kb.tap("j", keys=keys)
    time.sleep(1)
    report.check(shortcut_fired("KBOARD-META-J-FIRED") > before, "a KWin global shortcut on Meta+J fires from the keyboard")
    before = shortcut_fired("KBOARD-META-ALONE-FIRED")
    kb.tap("Super", keys=keys)
    kb.tap("Super", keys=keys)
    time.sleep(1)
    report.check(shortcut_fired("KBOARD-META-ALONE-FIRED") > before, "Meta alone reaches KWin as a modifier-only shortcut")


def main():
    report = Report()
    focus_field("A")
    report.check(wait_for(lambda: keymap()["window"]["visible"]), "keyboard visible")
    report.check(wait_for(lambda: "fn" in [key["type"] for key in keymap()["keys"]], timeout=8), "the function row is on")
    time.sleep(1)
    kb.tap("fn")
    report.check(wait_for(lambda: "modifier" in [key["type"] for key in keymap()["keys"]], timeout=4), "the Fn key switches to the desktop row")
    keys = kb.Keys()

    mark = len(events())
    kb.tap("Ctrl", keys=keys)
    kb.tap("c", keys=keys)
    time.sleep(0.5)
    report.check(seen(mark, "down", KEY_C, "C"), f"Ctrl+c reaches the client as Ctrl plus c with no Shift: {since(mark)}")
    report.check(seen(mark, "down", CTRL_KEY, "C"), "the Ctrl press reaches the client")
    report.check(seen(mark, "up", CTRL_KEY, ""), "the Ctrl release reaches the client")

    mark = len(events())
    kb.tap("Alt", keys=keys)
    kb.tap("c", keys=keys)
    time.sleep(0.5)
    report.check(seen(mark, "down", KEY_C, "A"), f"Alt+c reaches the client: {since(mark)}")

    mark = len(events())
    kb.tap("Super", keys=keys)
    kb.tap("d", keys=keys)
    time.sleep(0.5)
    if not os.environ.get("KBOARD_GLOBAL_SHORTCUTS"):
        report.check(seen(mark, "down", KEY_D, "M"), f"Meta+d reaches the client: {since(mark)}")

    mark = len(events())
    kb.tap("Super", keys=keys)
    kb.tap("Super", keys=keys)
    time.sleep(0.5)
    report.check(seen(mark, "down", META_KEY, "M") and seen(mark, "up", META_KEY, ""), f"Meta alone is a press and a release: {since(mark)}")

    kb.tap("shift", hold_ms=900, keys=keys)
    time.sleep(0.5)
    kb.tap("c", keys=keys)
    time.sleep(0.5)
    report.check(text().endswith("C"), f"caps is on, a plain c types a capital: {text()!r}")
    mark = len(events())
    kb.tap("Ctrl", keys=keys)
    kb.tap("c", keys=keys)
    time.sleep(0.5)
    shifted = [e for e in since(mark) if e[0] == "down" and int(e[1]) == KEY_C]
    report.check(bool(shifted) and all("S" not in e[2] and "C" in e[2] for e in shifted), f"Ctrl+c with caps on has no Shift: {since(mark)}")
    if os.environ.get("KBOARD_GLOBAL_SHORTCUTS"):
        global_shortcuts(report, keys)
    report.finish()


if __name__ == "__main__":
    main()
