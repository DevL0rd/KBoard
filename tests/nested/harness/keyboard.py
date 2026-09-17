import json
import os
import time

from fakepointer import click, script
from kwinsession import qdbus, run_script

SERVICE = ("org.devl0rd.KBoard", "/KBoard")


def call(method, *args):
    return qdbus(*SERVICE, "org.devl0rd.KBoard." + method, *args)


def keymap():
    return json.loads(call("KeyMap"))


def panel_geometry():
    printed = run_script('for (const w of workspace.stackingOrder) { if (w.inputMethod) { const g = w.frameGeometry;'
                         ' print("MARK|" + g.x + "|" + g.y + "|" + g.width + "|" + g.height + "|" + !w.hidden); } }')
    if not printed:
        return None
    x, y, width, height, shown = printed[0].split("|")
    return {"x": float(x), "y": float(y), "width": float(width), "height": float(height), "shown": shown == "true"}


def window_geometry(title):
    printed = run_script(f'for (const w of workspace.windowList()) {{ if (w.caption == "{title}") {{ const g = w.frameGeometry;'
                         ' print("MARK|" + g.x + "|" + g.y + "|" + g.width + "|" + g.height); } }')
    x, y, width, height = (float(v) for v in printed[0].split("|"))
    return {"x": x, "y": y, "width": width, "height": height}


def output_geometry():
    printed = run_script('const o = workspace.activeScreen.geometry; print("MARK|" + o.x + "|" + o.y + "|" + o.width + "|" + o.height);')
    x, y, width, height = (float(v) for v in printed[0].split("|"))
    return {"x": x, "y": y, "width": width, "height": height}


class Keys:
    def __init__(self):
        self.map = keymap()
        self.panel = panel_geometry()
        left = min(rect["x"] for rect in self.map["rects"])
        top = min(rect["y"] for rect in self.map["rects"])
        self.offset = (self.panel["x"] - left, self.panel["y"] - top)

    def screen(self, x, y):
        return x + self.offset[0], y + self.offset[1]

    def find(self, label):
        matches = [key for key in self.map["keys"] if key["label"] == label or key["type"] == label]
        if not matches:
            raise KeyError(f"no key {label!r} on page {self.map['page']}")
        return matches[0]

    def center(self, label):
        key = self.find(label)
        return self.screen(key["x"] + key["width"] / 2, key["y"] + key["height"] / 2)


def tap(label, hold_ms=40, keys=None):
    x, y = (keys or Keys()).center(label)
    tap_at(x, y, hold_ms)


def tap_at(x, y, hold_ms=40):
    script([f"down 0 {x} {y}", "frame", f"sleep {hold_ms}", "up 0", "frame", "sleep 120"])


def type_keys(labels, keys=None):
    for label in labels:
        tap(label, keys=keys)
        keys = None


def path(points, steps=6, delay=12):
    lines = [f"down 0 {points[0][0]} {points[0][1]}", "frame", "sleep 30"]
    for (x0, y0), (x1, y1) in zip(points, points[1:]):
        for step in range(1, steps + 1):
            t = step / steps
            lines += [f"move 0 {x0 + (x1 - x0) * t} {y0 + (y1 - y0) * t}", "frame", f"sleep {delay}"]
    lines += ["up 0", "frame", "sleep 150"]
    return lines


def glide(word):
    keys = Keys()
    script(path([keys.center(letter) for letter in word]))


def typed(label):
    with open(os.environ["KBOARD_KWIN_LOG"], errors="replace") as log:
        entries = [line.strip().split(f"kboard-test-typed:{label}:", 1)[1] for line in log if f"kboard-test-typed:{label}:" in line]
    if not entries:
        return ""
    last = entries[-1]
    return json.loads(last) if last.startswith('"') else last


def current_line(label):
    return typed(label).split("\n")[-1]


def focus_field(title):
    geometry = window_geometry(title)
    click(geometry["x"] + geometry["width"] / 2, geometry["y"] + 60)
    time.sleep(2.5)


def wait_for(predicate, timeout=4.0, interval=0.2):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = predicate()
        if value:
            return value
        time.sleep(interval)
    return predicate()


class Report:
    def __init__(self):
        self.problems = []

    def known(self, condition, message, limitation):
        print(("  ok      " if condition else "  KNOWN   ") + message + ("" if condition else f" ({limitation})"))
        return condition

    def check(self, condition, message):
        print(("  ok      " if condition else "  PROBLEM ") + message)
        if not condition:
            self.problems.append(message)
        return condition

    def finish(self):
        print("RESULT:", "FAIL" if self.problems else "PASS")
