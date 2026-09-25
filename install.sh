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
SKIP_DEPS=false
[[ ${KBOARD_AUR:-} == @(1|true|yes) ]] && AUR=true
for argument in "$@"; do
    case "$argument" in
    --aur) AUR=true ;;
    --system-update) SYSTEM_UPDATE=true ;;
    --skip-deps) SKIP_DEPS=true ;;
    -h | --help)
        echo "Usage: ./install.sh [--skip-deps] [--aur]"
        echo "Installs the build dependencies, builds and installs KBoard, makes it KWin's on-screen keyboard on the first install and, for a git checkout, updates it with every system update."
        echo "  --skip-deps  Don't install the build dependencies with the system package manager."
        echo "  --aur        Installed by a package (also KBOARD_AUR=true); the package manager handles dependencies and updates, so no update hook is registered."
        exit 0
        ;;
    *) echo "Unknown option: $argument"; exit 1 ;;
    esac
done

if image_based_system; then
    refuse_image_based_system
fi
mkdir -p "$UPDATE_STATE_HOME"
exec 9>"$UPDATE_STATE_HOME/install.lock"
if ! flock -n 9; then
    echo "Another KBoard install or update is running."
    $SYSTEM_UPDATE && exit 0
    exit 1
fi
if ! $SKIP_DEPS && ! $AUR && ! $SYSTEM_UPDATE; then
    echo "Installing build dependencies..."
    "$REPO_DIR/packaging/dependencies.sh"
fi

QT_MIN_VERSION=$(sed -n 's/^set(QT_MIN_VERSION "\(.*\)")$/\1/p' "$REPO_DIR/CMakeLists.txt")
missing=0
for command_name in cmake ninja c++ pkg-config python3 kreadconfig6 kwriteconfig6 kpackagetool6 busctl; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "Missing required command: $command_name"
        missing=1
    fi
done
for module in "Qt6Core >= $QT_MIN_VERSION" Qt6Quick Qt6WaylandClient Qt6Multimedia wayland-client wayland-protocols xkbcommon whisper parakeet ggml sdl3 hunspell libudev; do
    if ! pkg-config --exists "$module" 2>/dev/null; then
        echo "Missing required library: $module"
        missing=1
    fi
done
for package in ECM KF6Config KF6CoreAddons KF6I18n KF6GuiAddons KF6WindowSystem KF6DBusAddons KF6Kirigami KF6Crash KF6Service; do
    found=0
    for directory in /usr/lib/cmake /usr/lib64/cmake /usr/lib/*-linux-gnu/cmake /usr/local/lib/cmake /usr/share/"$package"/cmake /usr/share/cmake; do
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
BUILT_FOR="$BUILD_DIR/kboard-built-for"
MANIFEST="$CONFIG_DIR/install-manifest"
SET_UP="$CONFIG_DIR/set-up"
VOICE_MODEL="parakeet-tdt-0.6b-v3-q4_0"

mkdir -p "$CONFIG_DIR"

cmake_options=(-DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="$PREFIX" -DKDE_INSTALL_USE_QT_SYS_PATHS=OFF -DBUILD_TESTING=OFF -DKBOARD_WERROR=OFF)
[[ -n ${KBOARD_KLIPY_API_KEY:-} ]] && cmake_options+=(-DKBOARD_KLIPY_API_KEY="$KBOARD_KLIPY_API_KEY")

SYSTEM=$(pkg-config --modversion Qt6Core Qt6WaylandClient whisper parakeet ggml)
if $SYSTEM_UPDATE; then
    qt_library=$(find /usr/lib /usr/lib64 -maxdepth 2 -name libQt6Core.so.6 -print -quit 2>/dev/null)
    if [[ -n $qt_library ]]; then
        qt_runtime=$(basename "$(readlink -f "$qt_library")")
        qt_runtime=${qt_runtime#libQt6Core.so.}
        if [[ $qt_runtime != "$(pkg-config --modversion Qt6Core)" ]]; then
            echo "Qt $qt_runtime and its development files $(pkg-config --modversion Qt6Core) don't match yet, waiting for the update to finish."
            exit 0
        fi
    fi
fi

REVISION=$( {
    printf '%s\n' "${cmake_options[@]}" "$PREFIX" "$SYSTEM"
    find "$REPO_DIR/CMakeLists.txt" "$REPO_DIR/src" "$REPO_DIR/data" "$REPO_DIR/packaging/kboard-input-method" -type f -not -path '*/__pycache__/*' -print0 | LC_ALL=C sort -z | xargs -0 sha256sum
} | sha256sum | cut -d' ' -f1)

REBUILT=false
if [[ -f "$STAMP" && "$(<"$STAMP")" == "$REVISION" && -f "$MANIFEST" && -x "$BIN_DIR/kboard" ]]; then
    echo "KBoard is up to date."
else
    echo "Building KBoard..."
    cmake -S "$REPO_DIR" -B "$BUILD_DIR" -G Ninja "${cmake_options[@]}" >/dev/null
    clean=()
    [[ -f $BUILT_FOR && "$(<"$BUILT_FOR")" == "$SYSTEM" ]] || clean=(--clean-first)
    cmake --build "$BUILD_DIR" "${clean[@]}"
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
    printf '%s\n' "$SYSTEM" > "$BUILT_FOR"
    REBUILT=true
    echo "Installed KBoard to $PREFIX"
fi

FIRST_INSTALL=false
[[ -f $SET_UP ]] || $SYSTEM_UPDATE || FIRST_INSTALL=true
if $FIRST_INSTALL && $REBUILT; then
    "$REPO_DIR/packaging/kboard-input-method" restart
elif $FIRST_INSTALL; then
    "$REPO_DIR/packaging/kboard-input-method" enable
elif $REBUILT; then
    "$REPO_DIR/packaging/kboard-input-method" refresh
fi

export PATH="$PATH:$BIN_DIR"
if ! command -v kboard-voice-model >/dev/null 2>&1; then
    echo "The build did not install kboard-voice-model into $BIN_DIR."
    exit 1
fi
kboard-voice-model "$VOICE_MODEL"

rm -rf "$REPO_DIR/plasmoids/org.devl0rd.kboard/contents/ui/lib"
PLASMOID_STAGE=$(mktemp -d)
trap 'rm -rf "$PLASMOID_STAGE"' EXIT
PLASMOID="$PLASMOID_STAGE/org.devl0rd.kboard"
cp -r "$REPO_DIR/plasmoids/org.devl0rd.kboard" "$PLASMOID"
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
        notify_updated "KBoard is up to date."
    else
        rm -f "$UPDATE_PENDING"
    fi
    exit 0
fi
register_system_updates "$REPO_DIR" "$AUR"

echo
echo "Installed. Touch a text field to type, or add 'KBoard' from Add Widgets to turn it on and off."
echo "Settings: kboard-settings"
echo "Logs: journalctl --user -u plasma-kwin_wayland.service -f"
echo "Restarting Plasma..."
systemctl --user stop plasma-plasmashell.service
if $FIRST_INSTALL; then
    python3 "$REPO_DIR/packaging/replace-keyboard-toggle" "${XDG_CONFIG_HOME:-$HOME/.config}/plasma-org.kde.plasma.desktop-appletsrc"
    touch "$SET_UP"
fi
systemctl --user reset-failed plasma-plasmashell.service 2>/dev/null || true
systemctl --user start plasma-plasmashell.service
