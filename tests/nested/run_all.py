#!/usr/bin/env python3
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "harness"))

from suite import TESTS, run


def main(names):
    results = {}
    for name in names:
        print(f"\n===== {name} =====")
        started = time.monotonic()
        results[name] = (run(name), time.monotonic() - started)
    print("\n===== summary =====")
    for name, (code, seconds) in results.items():
        print(f"  {'PASS' if code == 0 else 'FAIL'}  {name:<18} {seconds:5.1f}s")
    failed = [name for name, (code, _) in results.items() if code != 0]
    print(f"\n{len(results) - len(failed)}/{len(results)} suites passed")
    return 1 if failed else 0


if __name__ == "__main__":
    selected = [name for name in sys.argv[1:] if not name.startswith("-")]
    unknown = [name for name in selected if name not in TESTS]
    if unknown:
        print("unknown suites:", ", ".join(unknown), "\navailable:", ", ".join(TESTS))
        sys.exit(2)
    sys.exit(main(selected or [name for name in TESTS if not name.startswith("screenshots")]))
