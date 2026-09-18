#!/usr/bin/env bash
set -euo pipefail

REPO=$(cd "$(dirname "$0")/.." && pwd)

if [[ -z ${DBUS_SESSION_BUS_ADDRESS:-} || ${KBOARD_TEST_PRIVATE_BUS:-} != 1 ]]; then
    exec env KBOARD_TEST_PRIVATE_BUS=1 dbus-run-session -- "$0" "$@"
fi

ROOT=$(mktemp -d)
trap 'rm -rf "$ROOT"' EXIT
SHIMS="$ROOT/shims"
CALLS="$ROOT/calls.log"
export HOME="$ROOT/home"
export XDG_CONFIG_HOME="$HOME/.config"
export XDG_DATA_HOME="$HOME/.local/share"
export XDG_STATE_HOME="$HOME/.local/state"
export KBOARD_BUILD_DIR="${KBOARD_BUILD_DIR:-$REPO/build-packaging}"
export PATH="$SHIMS:$PATH"
export CALLS
export GIT_CONFIG_COUNT=1
export GIT_CONFIG_KEY_0=safe.directory
export GIT_CONFIG_VALUE_0="$REPO"

DESKTOP="$HOME/.local/share/applications/org.devl0rd.kboard.desktop"
PLASMA_KEYBOARD=/usr/share/applications/org.kde.plasma.keyboard.desktop
APPLETS="$XDG_CONFIG_HOME/plasma-org.kde.plasma.desktop-appletsrc"
PREVIOUS="$XDG_CONFIG_HOME/kboard/previous-input-method"
failures=0

shim() {
    {
        echo '#!/bin/bash'
        echo "echo \"$1 \$*\" >>\"\$CALLS\""
        cat
    } >"$SHIMS/$1"
    chmod +x "$SHIMS/$1"
}

make_shims() {
    mkdir -p "$SHIMS"
    shim systemctl </dev/null
    shim sudo <<'EOF'
cat >/dev/null
EOF
    shim gdbus </dev/null
    shim kpackagetool6 <<'EOF'
[[ $3 == -u && ! -f $CALLS.applet ]] && exit 1
[[ $3 == -i ]] && touch "$CALLS.applet"
[[ $3 == -r ]] && rm -f "$CALLS.applet"
exit 0
EOF
    shim kboard-voice-model <<'EOF'
models="$HOME/.local/share/kboard/models"
if [[ -f $models/$1 ]]; then
    echo "$1 is already downloaded"
else
    mkdir -p "$models" && touch "$models/$1"
fi
EOF
}

check() {
    local description=$1
    shift
    if "$@"; then
        echo "  ok    $description"
    else
        echo "  FAIL  $description"
        failures=$((failures + 1))
    fi
}

input_method() {
    kreadconfig6 --file kwinrc --group Wayland --key "${1:-InputMethod}"
}

equals() {
    [[ $1 == "$2" ]]
}

logged() {
    grep -qF -- "$1" "$CALLS"
}

hook_registered() {
    if (( EUID == 0 )); then
        test -f /etc/pacman.d/hooks/kboard-update.hook
    else
        logged "sudo install -Dm644 $REPO/packaging/kboard-update.hook /etc/pacman.d/hooks/kboard-update.hook"
    fi
}

contains() {
    grep -qF -- "$2" "$1"
}

seed_session() {
    mkdir -p "$XDG_CONFIG_HOME" "$HOME/.local/share/kboard/models"
    printf '[Wayland]\nInputMethod=%s\nVirtualKeyboardMode=1\n' "$PLASMA_KEYBOARD" >"$XDG_CONFIG_HOME/kwinrc"
    printf '[General]\nsoundVolume=0.4\n' >"$XDG_CONFIG_HOME/kboardrc"
    touch "$HOME/.local/share/kboard/learned-words.json"
    cat >"$APPLETS" <<'EOF'
[Containments][2]
plugin=org.kde.panel

[Containments][2][Applets][7]
immutability=1
plugin=dev.devl0rd.keyboardtoggle

[Containments][2][Applets][7][Configuration]
PreloadWeight=42

[Containments][2][Applets][70]
immutability=1
plugin=org.kde.plasma.kickoff

[Containments][2][Applets][70][Configuration]
PreloadWeight=100

[Containments][2][General]
AppletOrder=7;70
EOF
}

run() {
    local log=$1
    shift
    : >"$CALLS"
    if ! "$@" >"$log" 2>&1; then
        echo "  FAIL  $* exited with an error:"
        tail -20 "$log"
        exit 1
    fi
}

test_first_install() {
    echo "First install over plasma-keyboard"
    run "$ROOT/install1.log" "$REPO/install.sh"
    check "InputMethod points at the installed desktop file" equals "$(input_method)" "$DESKTOP"
    check "VirtualKeyboardEnabled is true" equals "$(input_method VirtualKeyboardEnabled)" true
    check "VirtualKeyboardMode is kept" equals "$(input_method VirtualKeyboardMode)" 1
    check "previous input method is saved" equals "$(<"$PREVIOUS")" "$PLASMA_KEYBOARD"
    check "kboard binary is installed" test -x "$HOME/.local/bin/kboard"
    check "kboard-input-method is installed" test -x "$HOME/.local/bin/kboard-input-method"
    check "desktop file is installed" test -f "$DESKTOP"
    check "install manifest is kept" test -s "$XDG_CONFIG_HOME/kboard/install-manifest"
    check "voice model is downloaded" test -f "$HOME/.local/share/kboard/models/parakeet-tdt-0.6b-v3-q4_0"
    check "plasmoid is installed" logged "kpackagetool6 -t Plasma/Applet -i $REPO/plasmoids/org.devl0rd.kboard"
    check "shared QML is staged into the plasmoid" test -f "$REPO/plasmoids/org.devl0rd.kboard/contents/ui/lib/PopCard.qml"
    check "pacman hook is registered" hook_registered
    check "update unit is written" contains "$XDG_CONFIG_HOME/systemd/user/kboard-update.service" "ExecStart=$REPO/install.sh --system-update"
    check "plasmashell is stopped and started" logged "systemctl --user start plasma-plasmashell.service"
    check "Keyboard Toggle is replaced by KBoard" contains "$APPLETS" "plugin=org.devl0rd.kboard"
    check "old widget id is gone" bash -c "! grep -q keyboardtoggle '$APPLETS'"
    check "old widget config is dropped" bash -c "! grep -q 'PreloadWeight=42' '$APPLETS'"
    check "other applets are untouched" contains "$APPLETS" "PreloadWeight=100"
}

test_second_install() {
    echo "Second install with nothing changed"
    local applets_before
    applets_before=$(<"$APPLETS")
    run "$ROOT/install2.log" "$REPO/install.sh"
    check "skips the rebuild" contains "$ROOT/install2.log" "KBoard is up to date."
    check "does not build" bash -c "! grep -q 'Building KBoard' '$ROOT/install2.log'"
    check "does not restart the active keyboard" contains "$ROOT/install2.log" "already the on-screen keyboard"
    check "previous input method is unchanged" equals "$(<"$PREVIOUS")" "$PLASMA_KEYBOARD"
    check "voice model download is skipped" contains "$ROOT/install2.log" "already downloaded"
    check "plasmoid is upgraded" logged "kpackagetool6 -t Plasma/Applet -u"
    check "appletsrc is unchanged" equals "$(<"$APPLETS")" "$applets_before"
}

test_system_update() {
    echo "System update with nothing changed"
    mkdir -p "$XDG_STATE_HOME/kboard"
    touch "$XDG_STATE_HOME/kboard/update-pending"
    run "$ROOT/update.log" "$REPO/install.sh" --system-update
    check "no notification" bash -c "! grep -q gdbus '$CALLS'"
    check "plasmashell is left alone" bash -c "! grep -q plasma-plasmashell '$CALLS'"
    check "pending flag is cleared" test ! -e "$XDG_STATE_HOME/kboard/update-pending"
}

test_uninstall() {
    echo "Uninstall"
    run "$ROOT/uninstall.log" "$REPO/uninstall.sh"
    check "InputMethod is restored" equals "$(input_method)" "$PLASMA_KEYBOARD"
    check "saved input method is removed" test ! -e "$PREVIOUS"
    check "kboard binary is removed" test ! -e "$HOME/.local/bin/kboard"
    check "kboard-input-method is removed" test ! -e "$HOME/.local/bin/kboard-input-method"
    check "desktop file is removed" test ! -e "$DESKTOP"
    check "KBoard QML modules are removed" test ! -e "$HOME/.local/lib/qml/org/devl0rd/kboard"
    check "plasmoid is removed" logged "kpackagetool6 -t Plasma/Applet -r org.devl0rd.kboard"
    check "update unit is removed" test ! -e "$XDG_CONFIG_HOME/systemd/user/kboard-update.service"
    check "kboardrc is kept" test -f "$XDG_CONFIG_HOME/kboardrc"
    check "learned words are kept" test -f "$HOME/.local/share/kboard/learned-words.json"
    check "voice models are kept" test -f "$HOME/.local/share/kboard/models/parakeet-tdt-0.6b-v3-q4_0"
}

test_without_previous_keyboard() {
    echo "Install and uninstall without a previous keyboard"
    rm -f "$XDG_CONFIG_HOME/kwinrc"
    run "$ROOT/install3.log" "$REPO/install.sh"
    check "InputMethod points at KBoard" equals "$(input_method)" "$DESKTOP"
    check "an empty previous input method is saved" equals "$(<"$PREVIOUS")" ""
    run "$ROOT/uninstall2.log" "$REPO/uninstall.sh"
    check "InputMethod key is removed" bash -c "! grep -q InputMethod '$XDG_CONFIG_HOME/kwinrc'"
}

make_shims
seed_session
test_first_install
test_second_install
test_system_update
test_uninstall
test_without_previous_keyboard

if (( failures )); then
    echo "$failures check(s) failed; logs in $ROOT"
    trap - EXIT
    exit 1
fi
echo "All install checks passed."
