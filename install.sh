#!/bin/bash
set -euo pipefail

REPO_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
if [[ ! -f "$REPO_DIR/src/keyboard/main.cpp" ]]; then
    echo "Repository files are incomplete."
    exit 1
fi
source "$REPO_DIR/packaging/lib.sh"

AUR=false
SYSTEM_UPDATE=false
[[ ${KBOARD_AUR:-} == @(1|true|yes) ]] && AUR=true
for argument in "$@"; do
    case "$argument" in
    --aur) AUR=true ;;
    --system-update) SYSTEM_UPDATE=true ;;
    -h | --help)
        echo "Usage: ./install.sh [--aur]"
        echo "Builds and installs KBoard, makes it KWin's on-screen keyboard and, for a git checkout on a pacman system, updates it with every system update."
        echo "  --aur  Installed by a package (also KBOARD_AUR=true); no update hook is registered."
        exit 0
        ;;
    *) echo "Unknown option: $argument"; exit 1 ;;
    esac
done

missing=0
for command_name in cmake ninja c++ pkg-config python3 kreadconfig6 kwriteconfig6 kpackagetool6 busctl; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "Missing required command: $command_name"
        missing=1
    fi
done
for module in Qt6Core Qt6Quick Qt6WaylandClient Qt6Multimedia wayland-client wayland-protocols xkbcommon whisper parakeet ggml sdl3 hunspell libudev; do
    if ! pkg-config --exists "$module" 2>/dev/null; then
        echo "Missing required library: $module"
        missing=1
    fi
done
for package in ECM KF6Config KF6CoreAddons KF6I18n KF6GuiAddons KF6WindowSystem KF6DBusAddons KF6Kirigami KF6Crash KF6Service; do
    found=0
    for directory in /usr/lib/cmake /usr/lib64/cmake /usr/local/lib/cmake /usr/share/"$package"/cmake /usr/share/cmake; do
        if [[ -f "$directory/$package/${package}Config.cmake" || -f "$directory/${package}Config.cmake" ]]; then
            found=1
            break
        fi
    done
    if (( ! found )); then
        echo "Missing required CMake package: $package"
        missing=1
    fi
done
if (( missing )); then
    echo "Install the missing Qt 6/KDE Frameworks/build dependencies, then run this again."
    exit 1
fi

if [[ ! -e "$REPO_DIR/shared/common/PopupShell.qml" ]]; then
    echo "shared/common (Plasma-Shared submodule) is empty."
    echo "Run: git submodule update --init --recursive"
    exit 1
fi

PREFIX="$HOME/.local"
BIN_DIR="$PREFIX/bin"
BUILD_DIR="${KBOARD_BUILD_DIR:-$REPO_DIR/build}"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/kboard"
STAMP="$CONFIG_DIR/installed-revision"
MANIFEST="$CONFIG_DIR/install-manifest"
PLASMOID="$REPO_DIR/plasmoids/org.devl0rd.kboard"
VOICE_MODEL="parakeet-tdt-0.6b-v3-q4_0"

mkdir -p "$CONFIG_DIR"

cmake_options=(-DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="$PREFIX" -DKDE_INSTALL_USE_QT_SYS_PATHS=OFF -DBUILD_TESTING=OFF -DKBOARD_WERROR=OFF)
[[ -n ${KBOARD_KLIPY_API_KEY:-} ]] && cmake_options+=(-DKBOARD_KLIPY_API_KEY="$KBOARD_KLIPY_API_KEY")

REVISION=$( {
    printf '%s\n' "${cmake_options[@]}" "$PREFIX"
    pkg-config --modversion Qt6Core Qt6WaylandClient whisper ggml
    find "$REPO_DIR/CMakeLists.txt" "$REPO_DIR/src" "$REPO_DIR/data" "$REPO_DIR/packaging/kboard-input-method" -type f -not -path '*/__pycache__/*' -print0 | sort -z | xargs -0 sha256sum
} | sha256sum | cut -d' ' -f1)

REBUILT=false
if [[ -f "$STAMP" && "$(<"$STAMP")" == "$REVISION" && -f "$MANIFEST" && -x "$BIN_DIR/kboard" ]]; then
    echo "KBoard is up to date."
else
    echo "Building KBoard..."
    cmake -S "$REPO_DIR" -B "$BUILD_DIR" -G Ninja "${cmake_options[@]}" >/dev/null
    cmake --build "$BUILD_DIR"
    cmake --install "$BUILD_DIR" >/dev/null
    if [[ -f "$MANIFEST" ]]; then
        while IFS= read -r installed || [[ -n $installed ]]; do
            if ! grep -qxF -- "$installed" "$BUILD_DIR/install_manifest.txt"; then
                rm -f "$installed"
            fi
        done <"$MANIFEST"
    fi
    cp "$BUILD_DIR/install_manifest.txt" "$MANIFEST"
    printf '%s\n' "$REVISION" > "$STAMP"
    REBUILT=true
    echo "Installed KBoard to $PREFIX"
fi

if $REBUILT; then
    "$REPO_DIR/packaging/kboard-input-method" restart
else
    "$REPO_DIR/packaging/kboard-input-method" enable
fi

export PATH="$PATH:$BIN_DIR"
if ! command -v kboard-voice-model >/dev/null 2>&1; then
    echo "The build did not install kboard-voice-model into $BIN_DIR."
    exit 1
fi
if kboard-voice-model has "$VOICE_MODEL"; then
    echo "Voice model $VOICE_MODEL is already downloaded."
else
    echo "Downloading voice model $VOICE_MODEL..."
    kboard-voice-model download "$VOICE_MODEL"
fi

mkdir -p "$PLASMOID/contents/ui/lib"
cp "$REPO_DIR/shared/common/"*.qml "$REPO_DIR/shared/common/"*.js "$PLASMOID/contents/ui/lib/"
if kpackagetool6 -t Plasma/Applet -u "$PLASMOID" >/dev/null 2>&1; then
    echo "Upgraded KBoard Plasma applet"
else
    kpackagetool6 -t Plasma/Applet -i "$PLASMOID" >/dev/null
    echo "Installed KBoard Plasma applet"
fi

if $SYSTEM_UPDATE; then
    if $REBUILT; then
        notify_updated "KBoard is up to date. Restart Plasma or log out and back in to load the updated widget."
    else
        rm -f "$UPDATE_PENDING"
    fi
    exit 0
fi
register_system_updates "$REPO_DIR" "$AUR"

echo
echo "Installed. Tap a text field to type, or add 'KBoard' from Plasma's Add Widgets menu."
echo "Settings: kboard-settings"
echo "Logs: journalctl --user -u plasma-kwin_wayland.service -f"
echo "Restarting Plasma..."
systemctl --user stop plasma-plasmashell.service
python3 "$REPO_DIR/packaging/replace-keyboard-toggle" "${XDG_CONFIG_HOME:-$HOME/.config}/plasma-org.kde.plasma.desktop-appletsrc"
systemctl --user reset-failed plasma-plasmashell.service 2>/dev/null || true
systemctl --user start plasma-plasmashell.service
