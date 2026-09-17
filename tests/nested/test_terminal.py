import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "harness"))

from nested import run_runner

if __name__ == "__main__":
    sys.exit(run_runner(Path(__file__).resolve().parent / "runners" / "terminal.py", client="keys-client.qml", keep="--keep" in sys.argv))
