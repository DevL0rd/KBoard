import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "fixtures"))

from fakepointer import script
from words import MISSPELLED, PREFIX
from keyboard import Keys, Report, current_line, focus_field, glide, keymap, tap, type_keys, typed, wait_for


def new_line():
    tap("enter")
    time.sleep(0.3)


def settle(seconds=0.6):
    time.sleep(seconds)


def check_letters(report):
    type_keys("hello")
    settle()
    report.check(current_line("A") == "Hello", f"letters with sentence auto-capitalisation typed {current_line('A')!r}")
    tap("space")
    tap("backspace")
    settle()
    report.check(current_line("A") == "Hello", f"space then backspace left {current_line('A')!r}")
    new_line()
    report.check(typed("A").endswith("\n"), "enter inserted a new line")


def check_shift(report):
    type_keys(["x", "shift", "w", "o"])
    settle()
    report.check(current_line("A") == "XWo", f"one-shot shift typed {current_line('A')!r}")
    new_line()
    tap("a")
    tap("shift", hold_ms=700)
    type_keys(["b", "c", "shift", "d"])
    settle()
    report.check(current_line("A") == "ABCd", f"caps lock typed {current_line('A')!r}")
    new_line()


def check_symbols(report):
    tap("a")
    tap("symbols")
    settle(0.4)
    report.check(keymap()["page"] == "symbols", "?123 opened the symbols page")
    type_keys(["1", "@"])
    tap("letters")
    settle(0.4)
    report.check(keymap()["page"] == "letters", "ABC returned to letters")
    report.check(current_line("A") == "A1@", f"symbols typed {current_line('A')!r}")
    new_line()


def check_long_press(report):
    tap("a")
    keys = Keys()
    x, y = keys.center("e")
    step = min(keys.find("e")["width"] * 0.92, keys.find("e")["height"] * 0.9)
    script([f"down 0 {x} {y}", "frame", "sleep 700", f"move 0 {x + step} {y}", "frame", "sleep 60",
            f"move 0 {x + step * 2} {y}", "frame", "sleep 120", "up 0", "frame", "sleep 200"])
    settle()
    report.check(current_line("A") == "Aé", f"long-press accent slide typed {current_line('A')!r}")
    new_line()


def check_autocorrect(report):
    type_keys(list(MISSPELLED) + ["space"])
    settle()
    report.check(current_line("A") == "The ", f"autocorrect turned {MISSPELLED!r} into {current_line('A')!r}")
    tap("backspace")
    settle()
    report.check(current_line("A") == MISSPELLED.capitalize(), f"backspace after autocorrect reverted to {current_line('A')!r}")
    new_line()
    type_keys(["o", "k", "space", "space"])
    settle()
    report.check(current_line("A") == "Ok. ", f"double space typed {current_line('A')!r}")
    new_line()


def check_suggestions(report):
    type_keys(PREFIX)
    settle(0.8)
    suggestions = keymap()["suggestions"]
    middle = suggestions[1]
    report.check(middle["text"] != "", f"suggestions shown for {PREFIX!r}: {[s['text'] for s in suggestions]}")
    if middle["text"]:
        keys = Keys()
        x, y = keys.screen(middle["x"] + middle["width"] / 2, middle["y"] + middle["height"] / 2)
        script([f"down 0 {x} {y}", "frame", "sleep 40", "up 0", "frame", "sleep 200"])
        settle()
        report.check(current_line("A") == middle["text"] + " ", f"tapping the suggestion typed {current_line('A')!r}")
    new_line()


def check_glide(report):
    glide("hello")
    wait_for(lambda: current_line("A").strip() != "", timeout=5)
    report.check(current_line("A").lower() == "hello ", f"gliding h-e-l-l-o typed {current_line('A')!r}")
    new_line()


def check_multitouch(report):
    keys = Keys()
    (tx, ty), (hx, hy), (ex, ey) = keys.center("t"), keys.center("h"), keys.center("e")
    script([f"down 0 {tx} {ty}", "frame", "sleep 25", f"down 1 {hx} {hy}", "frame", "sleep 25", "up 0", "frame", "sleep 10",
            f"down 0 {ex} {ey}", "frame", "sleep 25", "up 1", "frame", "sleep 25", "up 0", "frame", "sleep 200"])
    settle()
    report.check(current_line("A").lower() == "the", f"overlapping fingers typed {current_line('A')!r}")
    new_line()


def main():
    report = Report()
    focus_field("A")
    report.check(wait_for(lambda: keymap()["window"]["visible"]), "keyboard became visible on focus")
    for step in (check_letters, check_shift, check_symbols, check_long_press, check_autocorrect, check_suggestions, check_glide, check_multitouch):
        try:
            step(report)
        except Exception as error:
            report.check(False, f"{step.__name__} raised {error!r}")
    report.finish()


if __name__ == "__main__":
    main()
