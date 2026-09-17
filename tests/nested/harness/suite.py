import sys
from pathlib import Path

from nested import run_runner

RUNNERS = Path(__file__).resolve().parent.parent / "runners"
NEVER_SPLIT = "[Look]\nsplitMode=Never\n"
SLOW_LONG_PRESS = "[Typing]\nlongPressDelay=1000\n"
SCHEMES = Path("/usr/share/color-schemes")


def scheme(name):
    icons = "breeze-dark" if "Dark" in name else "breeze"
    return f"[General]\nColorScheme={name}\n[Icons]\nTheme={icons}\n" + (SCHEMES / f"{name}.colors").read_text()


def showcase(prefix, colors, scenes="", **session):
    return {"runner": "showcase.py", "client": "textarea-client.qml", "kboardrc": session.pop("kboardrc", NEVER_SPLIT + SLOW_LONG_PRESS), "kdeglobals": scheme(colors),
            "timeout": 180, "env": {"KBOARD_SHOWCASE": prefix, "KBOARD_SHOWCASE_SCENES": scenes}, **session}


TESTS = {
    "smoke": {"runner": "smoke.py", "client": "textarea-client.qml", "timeout": 150},
    "typing": {"runner": "typing_flow.py", "client": "textarea-client.qml", "kboardrc": NEVER_SPLIT, "timeout": 240},
    "visibility": {"runner": "visibility.py", "client": "textarea-client.qml", "kboardrc": NEVER_SPLIT, "timeout": 180},
    "geometry": {"runner": "geometry.py", "client": "textarea-client.qml", "kboardrc": NEVER_SPLIT, "timeout": 180},
    "split": {"runner": "split.py", "client": "textarea-client.qml", "width": 3440, "height": 1440, "timeout": 180},
    "apprules": {"runner": "apprules.py", "client": "textarea-client.qml", "timeout": 150,
                 "kboardrc": NEVER_SPLIT + "[Behavior]\nappRules=org.qt-project.qml=desktoprow\n"},
    "screenshots-dark": showcase("keyboard-dark", "BreezeDark", "popup glide emoji gif clipboard voice edit", timeout=300),
    "screenshots-light": showcase("keyboard-light", "BreezeLight", "popup emoji"),
    "screenshots-split": showcase("split", "BreezeDark", kboardrc=SLOW_LONG_PRESS, width=3440, height=1440),
}


def run(name, keep=False):
    spec = dict(TESTS[name])
    runner = RUNNERS / spec.pop("runner")
    return run_runner(runner, keep=keep, **spec)


def main(name):
    sys.exit(run(name, keep="--keep" in sys.argv))
