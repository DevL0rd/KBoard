import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "harness"))

from nested import run_script

SCRIPT = """
export QT_QPA_PLATFORM=wayland
konsole -e sh -c 'read line; printf "%s" "$line" > "$KBOARD_TEST_ROOT/typed.txt"; sleep 5' &
sleep 5
python3 {runner} > "$KBOARD_REPORT" 2>&1
"""

if __name__ == "__main__":
    runner = Path(__file__).resolve().parent / "runners" / "konsole.py"
    sys.exit(run_script(SCRIPT.format(runner=runner), timeout=180, keep="--keep" in sys.argv))
