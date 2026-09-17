#!/usr/bin/env python3
import argparse
import os
import shutil
import signal
import subprocess
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SCHEMES = {"dark": "BreezeDark", "light": "BreezeLight"}


def scheme_config(theme, root):
    config = root / "config"
    config.mkdir(parents=True)
    scheme = Path(f"/usr/share/color-schemes/{SCHEMES[theme]}.colors").read_text()
    (config / "kdeglobals").write_text(scheme + "\n[General]\nColorScheme=" + SCHEMES[theme] + "\n[Icons]\nTheme=" + ("breeze-dark" if theme == "dark" else "breeze") + "\n")
    return config


def session_script(root, args, theme):
    suffix = "" if theme == args.plain_theme else f"-{theme}"
    shot = [str(Path(args.build) / "bin" / "settingsshot"), "--out", args.out, "--suffix", suffix]
    if args.pages:
        shot += ["--pages", args.pages]
    if args.reveal:
        shot += ["--reveal", args.reveal]
    if args.search:
        shot += ["--search", args.search]
    script = root / "session.sh"
    script.write_text("#!/bin/sh\n" + " ".join(f"'{part}'" for part in shot) + "\necho $? > " + str(root / "status") + "\n")
    script.chmod(0o755)
    return script


def run(theme, args):
    root = Path(tempfile.mkdtemp(prefix=f"kboard-shot-{theme}-"))
    try:
        env = dict(os.environ)
        for key in ("DBUS_SESSION_BUS_ADDRESS", "WAYLAND_DISPLAY", "DISPLAY", "QT_QPA_PLATFORM", "KDE_FULL_SESSION", "SESSION_MANAGER"):
            env.pop(key, None)
        env.update({
            "XDG_CONFIG_HOME": str(scheme_config(theme, root)),
            "XDG_DATA_HOME": str(root / "data"),
            "XDG_CACHE_HOME": str(root / "cache"),
            "KBOARD_USE_BUILD_TREE": "1",
            "QT_FORCE_STDERR_LOGGING": "1",
            "QT_QPA_PLATFORMTHEME": "kde",
            "KWIN_WAYLAND_NO_PERMISSION_CHECKS": "1",
        })
        command = ["dbus-run-session", "--", "kwin_wayland", "--virtual", "--no-lockscreen", "--no-global-shortcuts",
                   "--socket", f"kboard-shot-{os.getpid()}", "--width", "1600", "--height", "1100",
                   "--exit-with-session", str(session_script(root, args, theme))]
        quiet = None if args.verbose else subprocess.DEVNULL
        process = subprocess.Popen(command, env=env, stdout=quiet, stderr=quiet, start_new_session=True)
        try:
            process.wait(timeout=args.timeout)
        finally:
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
        status = (root / "status").read_text().strip() if (root / "status").exists() else "missing"
        if status != "0":
            raise SystemExit(f"settingsshot failed for the {theme} theme (status {status})")
    finally:
        shutil.rmtree(root, ignore_errors=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", default=os.environ.get("KBOARD_BUILD_DIR", str(REPO / "build")))
    parser.add_argument("--out", default=str(REPO / "docs" / "media"))
    parser.add_argument("--themes", default="dark,light")
    parser.add_argument("--pages", default="")
    parser.add_argument("--reveal", default="")
    parser.add_argument("--search", default="")
    parser.add_argument("--plain-theme", default="", help="theme whose files are saved without a suffix")
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--timeout", type=int, default=240)
    args = parser.parse_args()
    for theme in args.themes.split(","):
        run(theme, args)


if __name__ == "__main__":
    main()
