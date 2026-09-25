#!/bin/bash
set -euo pipefail

source "$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)/lib.sh"

ARCH_PACKAGES=(base-devel cmake ninja git pkgconf python extra-cmake-modules
    qt6-base qt6-declarative qt6-wayland qt6-multimedia
    kconfig kcoreaddons ki18n kguiaddons kwindowsystem kdbusaddons kcrash kirigami kirigami-addons kservice kpackage
    wayland wayland-protocols libxkbcommon systemd-libs sdl3 whisper-cpp ggml hunspell hunspell-en_us noto-fonts-emoji)
FEDORA_PACKAGES=(cmake ninja-build git gcc-c++ pkgconf-pkg-config python3 extra-cmake-modules
    qt6-qtbase-devel qt6-qtbase-private-devel qt6-qtdeclarative-devel qt6-qtwayland-devel qt6-qtmultimedia-devel
    kf6-kconfig kf6-kconfig-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-kguiaddons-devel kf6-kwindowsystem-devel
    kf6-kdbusaddons-devel kf6-kcrash-devel kf6-kirigami-devel kf6-kirigami-addons kf6-kservice-devel kf6-kpackage
    wayland-devel wayland-protocols-devel libxkbcommon-devel systemd-devel SDL3-devel whisper-cpp-devel
    hunspell-devel hunspell-en-US google-noto-color-emoji-fonts libdnf5-plugin-actions)
SUSE_PACKAGES=(cmake ninja git gcc-c++ pkgconf-pkg-config python3 kf6-extra-cmake-modules
    qt6-base-devel qt6-base-private-devel qt6-declarative-devel qt6-declarative-imports qt6-wayland-devel qt6-wayland-private-devel
    qt6-multimedia-devel qt6-multimedia-imports
    kf6-kconfig kf6-kconfig-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-kguiaddons-devel kf6-kwindowsystem-devel
    kf6-kdbusaddons-devel kf6-kcrash-devel kf6-kirigami-devel kf6-kirigami-imports kirigami-addons6 kf6-kservice-devel kf6-kpackage
    wayland-devel wayland-protocols-devel libxkbcommon-devel libudev-devel SDL3-devel ggml-devel
    hunspell-devel myspell-en_US google-noto-coloremoji-fonts)
DEBIAN_PACKAGES=(cmake ninja-build git g++ pkgconf python3 extra-cmake-modules
    qt6-base-dev qt6-base-private-dev qt6-declarative-dev qt6-declarative-dev-tools qt6-wayland-dev qt6-wayland-dev-tools
    qt6-wayland-private-dev qt6-multimedia-dev
    libkf6config-dev libkf6config-bin libkf6coreaddons-dev libkf6i18n-dev libkf6guiaddons-dev libkf6windowsystem-dev
    libkf6dbusaddons-dev libkf6crash-dev libkirigami-dev libkf6service-dev kpackagetool6
    qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-shapes qml6-module-qtquick-effects
    qml6-module-qtquick-dialogs qml6-module-qt-labs-folderlistmodel qml6-module-qtqml-models qml6-module-qtcore
    qml6-module-qtmultimedia qml6-module-org-kde-kirigami qml6-module-org-kde-kirigamiaddons-formcard
    libwayland-dev wayland-protocols libxkbcommon-dev libudev-dev libsdl3-dev libwhisper-dev libggml-dev
    libhunspell-dev hunspell-en-us fonts-noto-color-emoji)

if image_based_system; then
    refuse_image_based_system
fi
case "$(package_manager)" in
pacman) update_as_root pacman -S --needed --noconfirm "${ARCH_PACKAGES[@]}" ;;
dnf) update_as_root dnf install -y "${FEDORA_PACKAGES[@]}" ;;
zypper) update_as_root zypper --non-interactive install "${SUSE_PACKAGES[@]}" ;;
apt-get) update_as_root apt-get install -y "${DEBIAN_PACKAGES[@]}" ;;
*)
    echo "Unsupported package manager: install the Qt 6, KDE Frameworks 6, whisper.cpp, SDL3 and Hunspell development packages, then run ./install.sh --skip-deps"
    exit 1
    ;;
esac
