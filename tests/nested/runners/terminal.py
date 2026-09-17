import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "harness"))

import keyboard as kb
from kwinsession import qdbus


def typed():
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        lines = [line.strip().split("kboard-test-keys:", 1)[1] for line in log if "kboard-test-keys:" in line]
    return lines[-1].split(":", 1)[1] if lines else ""


def main():
    problems = []
    qdbus("org.devl0rd.KBoard", "/KBoard", "org.devl0rd.KBoard.Show")
    time.sleep(3)
    for label in ("t", "e", "h"):
        kb.tap(label)
    time.sleep(1)
    kb.tap("space")
    time.sleep(2)
    result = typed()
    print("typed after autocorrect:", repr(result))
    if result.strip() != "The":
        problems.append(f"autocorrect in an app without surrounding text produced {result!r}, expected 'The '")
    for problem in problems:
        print("  PROBLEM " + problem)
    print("RESULT:", "FAIL" if problems else "PASS")


if __name__ == "__main__":
    main()
