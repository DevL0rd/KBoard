#!/usr/bin/env bash
set -euo pipefail

mode="${1:?usage: run.sh lint|unit|install|tidy|xunused}"

cd "$(dirname "$0")/../.."
owner=$(stat -c %u:%g .)
trap 'chown -R "$owner" .' EXIT

if [[ $mode == lint ]]; then
    pacman -Syu --noconfirm --needed git clang typos nodejs npm shellcheck
    git config --global --add safe.directory "$PWD"
    ./tools/check.sh --lint-only
    exit
fi

pacman -Syu --noconfirm --needed git cmake ninja ccache clang llvm openmp dbus
if [[ $mode == xunused ]]; then
    git clone https://github.com/mgehre/xunused.git /tmp/xunused
    git -C /tmp/xunused checkout b81e0ef
    if [[ ! -f /usr/include/clang/Driver/Options.h ]]; then
        sed -i 's|#include "clang/Driver/Options.h"|#include "clang/Options/Options.h"|' /tmp/xunused/main.cpp
    fi
    if [[ ! -f /usr/include/clang/Index/USRGeneration.h ]]; then
        sed -i 's|#include "clang/Index/USRGeneration.h"|#include "clang/UnifiedSymbolResolution/USRGeneration.h"|' /tmp/xunused/main.cpp
    fi
    cmake -S /tmp/xunused -B /tmp/xunused/build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build /tmp/xunused/build
    install -Dm755 /tmp/xunused/build/xunused /usr/local/bin/xunused
fi
git config --global --add safe.directory "$PWD"
packaging/dependencies.sh

export CCACHE_DIR="$PWD/.ccache" CCACHE_MAXSIZE=2G CMAKE_C_COMPILER_LAUNCHER=ccache CMAKE_CXX_COMPILER_LAUNCHER=ccache

case $mode in
unit) ./tools/check.sh --unit-only ;;
install) ./tools/check.sh --install-only ;;
tidy) ./tools/check.sh --tidy-only ;;
xunused) ./tools/check.sh --xunused-only ;;
*)
    echo "unknown mode $mode" >&2
    exit 2
    ;;
esac
ccache --show-stats
