#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../.."
CONTAINER="kboard-install-test"
IMAGE="${KBOARD_TEST_IMAGE:-archlinux:latest}"
CCACHE_HOST="${KBOARD_TEST_CCACHE:-}"
CCACHE_GUEST=/opt/ccache

as_root() {
    docker exec "$CONTAINER" "$@"
}

ccache_env=()
[[ -n $CCACHE_HOST ]] && ccache_env=(-e "CCACHE_DIR=$CCACHE_GUEST" -e CCACHE_MAXSIZE=2G -e CMAKE_C_COMPILER_LAUNCHER=ccache -e CMAKE_CXX_COMPILER_LAUNCHER=ccache)

as_tester() {
    docker exec -u tester -w /home/tester/KBoard "${ccache_env[@]}" -e USER=tester -e LOGNAME=tester -e XDG_RUNTIME_DIR=/run/user/1000 -e CMAKE_BUILD_PARALLEL_LEVEL \
        -e DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus -e WAYLAND_DISPLAY=wayland-0 "$CONTAINER" "$@"
}

step() {
    printf '\n==> %s\n' "$1"
}

step "Starting an Arch Linux system with Plasma"
docker rm -f "$CONTAINER" >/dev/null 2>&1 || true
docker run -d --name "$CONTAINER" --privileged --cgroupns=private --tmpfs /run --tmpfs /run/lock "$IMAGE" /usr/lib/systemd/systemd >/dev/null
finish() {
    if [[ -n $CCACHE_HOST ]]; then
        mkdir -p "$CCACHE_HOST"
        docker cp "$CONTAINER:$CCACHE_GUEST/." "$CCACHE_HOST" >/dev/null 2>&1 || true
    fi
    docker rm -f "$CONTAINER" >/dev/null 2>&1 || true
}
trap finish EXIT
packages=(plasma-desktop sudo git)
[[ -n $CCACHE_HOST ]] && packages+=(ccache)
as_root pacman -Syu --noconfirm --needed "${packages[@]}"
as_root bash -c 'useradd -m -u 1000 tester && echo "tester ALL=(ALL) NOPASSWD: ALL" >/etc/sudoers.d/tester'
as_root mkdir /home/tester/KBoard
tar --exclude='./build*' --exclude=./.ccache -cf - . | docker exec -i "$CONTAINER" tar -C /home/tester/KBoard -xf -
docker cp tests/install/. "$CONTAINER:/opt/install-test"
if [[ -n $CCACHE_HOST ]]; then
    as_root mkdir -p "$CCACHE_GUEST"
    [[ -d $CCACHE_HOST ]] && docker cp "$CCACHE_HOST/." "$CONTAINER:$CCACHE_GUEST"
    as_root chown -R tester: "$CCACHE_GUEST"
fi
as_root chown -R tester: /home/tester/KBoard
as_tester git config --global --add safe.directory '*'

step "Installing KBoard's dependencies"
as_tester packaging/dependencies.sh

step "Starting a Plasma session"
as_root bash -c "loginctl enable-linger tester; for _ in \$(seq 60); do [[ -S /run/user/1000/bus ]] && exit 0; sleep 1; done; exit 1"
as_tester /opt/install-test/session.sh
as_root /opt/install-test/snapshot.sh before

step "Installing KBoard"
as_tester ./install.sh --skip-deps
sleep 15

step "Checking that KBoard runs"
as_tester /opt/install-test/verify.sh

step "Uninstalling KBoard"
as_tester ./uninstall.sh
sleep 10

step "Checking that uninstalling left the system as it was"
as_root /opt/install-test/snapshot.sh after
as_root /opt/install-test/compare.sh
