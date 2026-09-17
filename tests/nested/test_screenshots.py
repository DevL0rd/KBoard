import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "harness"))

from suite import run

if __name__ == "__main__":
    names = [name for name in sys.argv[1:] if not name.startswith("--")] or ["screenshots-dark", "screenshots-light"]
    sys.exit(max(run(name, keep="--keep" in sys.argv) for name in names))
