import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

import keyboard as kb
from kwinsession import qdbus, run_script


def shell_line():
    printed = run_script('for (const w of workspace.windowList()) { if (w.resourceClass.includes("konsole")) { print("MARK|" + w.caption); } }')
    return printed[0] if printed else ""


def main():
    problems = []
    qdbus("org.devl0rd.KBoard", "/KBoard", "org.devl0rd.KBoard.Show")
    time.sleep(3)
    for label in ("h", "e", "l", "k", "o"):
        kb.tap(label)
    time.sleep(1)
    kb.tap("space")
    time.sleep(2)
    result = Path(os.environ["KBOARD_TEST_ROOT"], "typed.txt")
    kb.tap("enter")
    time.sleep(2)
    text = result.read_text().strip() if result.exists() else "<no file>"
    print("konsole received:", repr(text))
    if text != "helko":
        problems.append(f"Konsole received {text!r}; a terminal must get the keys typed with no autocorrect or capitals")
    for problem in problems:
        print("  PROBLEM " + problem)
    print("RESULT:", "FAIL" if problems else "PASS")


if __name__ == "__main__":
    main()
