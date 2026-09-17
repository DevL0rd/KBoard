import os
import shutil
import signal
import subprocess
import tempfile
import textwrap
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]


def build_dir():
    return Path(os.environ.get("KBOARD_BUILD_DIR", REPO / "build"))


class NestedSession:
    def __init__(self, width=1920, height=1080, extra_kwinrc="", kboardrc="", xwayland=False, output_count=1):
        self.width = width
        self.height = height
        self.output_count = output_count
        self.xwayland = xwayland
        self.root = Path(tempfile.mkdtemp(prefix="kboard-test-"))
        self.socket = f"kboard-test-{os.getpid()}"
        self.config_home = self.root / "config"
        self.data_home = self.root / "data"
        self.state_home = self.root / "state"
        for path in (self.config_home, self.data_home, self.state_home):
            path.mkdir()
        (self.config_home / "kwinrc").write_text(textwrap.dedent(f"""\
            [Plugins]
            slideEnabled=false
            [Wayland]
            VirtualKeyboardEnabled=true
            VirtualKeyboardMode=2
            {extra_kwinrc}
            """))
        (self.config_home / "kboardrc").write_text(kboardrc)
        self.proc = None
        self.log_path = self.root / "kwin.log"

    def env(self):
        drop = ("WAYLAND_DISPLAY", "DISPLAY", "DBUS_SESSION_BUS_ADDRESS", "QT_QPA_PLATFORM", "KDE_FULL_SESSION", "XDG_CURRENT_DESKTOP", "SESSION_MANAGER")
        env = {k: v for k, v in os.environ.items() if k not in drop}
        env["XDG_CONFIG_HOME"] = str(self.config_home)
        env["XDG_DATA_HOME"] = str(self.data_home)
        env["XDG_STATE_HOME"] = str(self.state_home)
        env["KBOARD_USE_BUILD_TREE"] = "1"
        env["KWIN_SCREENSHOT_NO_PERMISSION_CHECKS"] = "1"
        env["KWIN_WAYLAND_NO_PERMISSION_CHECKS"] = "1"
        env["QT_LOGGING_RULES"] = "kwin_*.debug=false"
        env["QT_FORCE_STDERR_LOGGING"] = "1"
        if os.environ.get("KBOARD_WAYLAND_DEBUG"):
            env["KWIN_IM_WAYLAND_DEBUG"] = "1"
        return env

    def start(self, session_script):
        script = self.root / "session.sh"
        script.write_text("#!/bin/sh\n" + session_script)
        script.chmod(0o755)
        log = open(self.log_path, "w")
        command = ["dbus-run-session", "--", "kwin_wayland", "--virtual", "--no-lockscreen", "--no-global-shortcuts",
                   "--inputmethod", str(build_dir() / "bin" / "kboard")]
        if self.xwayland:
            command.append("--xwayland")
        command += ["--socket", self.socket, "--width", str(self.width), "--height", str(self.height),
                    "--output-count", str(self.output_count), "--exit-with-session", str(script)]
        self.proc = subprocess.Popen(command, env=self.env(), stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        return self.proc

    def wait(self, timeout):
        try:
            return self.proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(self.proc.pid, signal.SIGTERM)
            self.proc.wait(timeout=10)
            raise

    def log(self):
        return self.log_path.read_text(errors="replace")

    def cleanup(self):
        shutil.rmtree(self.root, ignore_errors=True)


CLIENT_SCRIPT = """
export QT_QPA_PLATFORM=wayland
qml6 {client} -- A 1200 700 &
sleep 3
python3 {runner} > "$KBOARD_REPORT" 2>&1
"""


def run_runner(runner, timeout=120, client="text-client.qml", keep=False, **session):
    script = CLIENT_SCRIPT.format(client=REPO / "tests" / "nested" / "clients" / client, runner=runner)
    return run_script(script, timeout, keep=keep, **session)


def run_script(script, timeout, keep=False, **session_args):
    session = NestedSession(**session_args)
    report = session.root / "report.txt"
    session.start(f'export KBOARD_REPORT="{report}"\nexport KBOARD_KWIN_LOG="{session.log_path}"\nexport KBOARD_TEST_ROOT="{session.root}"\n' + script)
    try:
        session.wait(timeout=timeout)
    except Exception as error:
        print(f"nested session did not finish: {error}")
    if not report.exists():
        print("the nested session produced no report")
        print("\n".join(session.log().splitlines()[-40:]))
        return 2
    text = report.read_text()
    print(text.strip())
    if "RESULT: PASS" not in text:
        print("---- kwin log tail ----")
        print("\n".join(session.log().splitlines()[-40:]))
    if not keep:
        session.cleanup()
    else:
        print("kept", session.root)
    return 0 if "RESULT: PASS" in text else 1
