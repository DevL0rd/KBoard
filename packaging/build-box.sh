#!/usr/bin/env bash

BOX_MATCHED_PACKAGES='^(qt6-|kf6-)'
STEAMOS_MIRROR="https://steamdeck-packages.steamos.cloud/archlinux-mirror"
PULLED_IMAGES="$UPDATE_STATE_HOME/pulled-images"

remember_pulled_image() {
    podman image exists "$1" && return 0
    mkdir -p "$(dirname "$PULLED_IMAGES")"
    printf '%s\n' "$1" >>"$PULLED_IMAGES"
}

remove_unused_pulled_images() {
    local image kept=()
    [[ -f $PULLED_IMAGES ]] || return 0
    while IFS= read -r image; do
        if [[ -n $(podman ps --all --quiet --filter "ancestor=$image") ]]; then
            kept+=("$image")
        elif podman image exists "$image"; then
            echo "Removing the $image image..."
            podman image rm "$image" >/dev/null
        fi
    done <"$PULLED_IMAGES"
    if ((${#kept[@]})); then
        printf '%s\n' "${kept[@]}" >"$PULLED_IMAGES"
    else
        rm -f "$PULLED_IMAGES"
    fi
}

build_boxes() {
    command -v podman >/dev/null || return 0
    podman ps --all --format '{{.Names}}' | grep -E "^$UPDATE_ID-(fedora-[0-9]+|steamos)\$" || true
}

remove_build_boxes() {
    local name
    for name in $(build_boxes); do
        [[ $name == "${1:-}" ]] && continue
        echo "Removing the $name build container..."
        podman rm --force --volumes "$name" >/dev/null
    done
    command -v podman >/dev/null && remove_unused_pulled_images
    return 0
}

fail() {
    echo "$1"
    exit 1
}

fedora_matched_packages() {
    "$@" rpm -qa --qf '%{NAME} %{VERSION}-%{RELEASE}\n' | tr -d '\r' | awk -v pattern="$BOX_MATCHED_PACKAGES" '$1 ~ pattern' | LC_ALL=C sort -u
}

prepare_fedora_box() {
    local release locks mismatched
    release=$(. /etc/os-release && [[ " $ID ${ID_LIKE:-} " == *" fedora "* ]] && printf '%s' "$VERSION_ID") \
        || fail "On image-based systems KBoard builds in a Fedora toolbox, and this system isn't based on Fedora."
    command -v toolbox >/dev/null || fail "toolbox is missing. Fedora Atomic desktops ship it; install it with: rpm-ostree install toolbox"
    remove_build_boxes "$BUILD_BOX"
    if ! podman container exists "$BUILD_BOX"; then
        echo "Creating the $BUILD_BOX toolbox to build KBoard in..."
        remember_pulled_image "registry.fedoraproject.org/fedora-toolbox:$release"
        toolbox --assumeyes create --distro fedora --release "$release" "$BUILD_BOX"
    fi
    echo "Matching the toolbox to this system's Qt and KDE Frameworks..."
    mapfile -t locks < <(fedora_matched_packages | tr ' ' '-')
    in_build_env sudo dnf install --assumeyes --quiet fedora-repos-archive
    in_build_env sudo dnf versionlock clear >/dev/null
    in_build_env sudo dnf versionlock add "${locks[@]}" >/dev/null
    in_build_env sudo dnf install --assumeyes "${FEDORA_PACKAGES[@]}"
    in_build_env sudo dnf distro-sync --assumeyes
    mismatched=$(LC_ALL=C join <(fedora_matched_packages) <(fedora_matched_packages in_build_env) | awk '$2 != $3 { printf "%s (system %s, toolbox %s) ", $1, $2, $3 }')
    [[ -z $mismatched ]] || fail "Fedora's repositories don't carry the exact versions this system runs: ${mismatched}Update the system, reboot and run ./install.sh again."
}

steamos_matched_packages() {
    local frameworks
    mapfile -t frameworks < <("$@" pacman -Qgq kf6 | tr -d '\r')
    {
        "$@" pacman -Q | awk '$1 ~ /^qt6-/'
        if ((${#frameworks[@]})); then
            "$@" pacman -Q "${frameworks[@]}"
        fi
    } | tr -d '\r' | LC_ALL=C sort -u
}

steamos_mismatches() {
    LC_ALL=C join <(steamos_matched_packages) <(steamos_matched_packages in_build_env) | awk '$2 != $3 { print $1, $2, $3 }'
}

steamos_pacman_conf() {
    local repo
    printf '[options]\nArchitecture = auto\nSigLevel = %s\nLocalFileSigLevel = Optional\n' "$1"
    for repo in "${@:2}"; do
        printf "[%s]\nServer = %s/\$repo/os/\$arch\n" "$repo" "$STEAMOS_MIRROR"
    done
}

steamos_package_url() {
    local info repo arch
    info=$(in_build_env env LC_ALL=C pacman -Si "$1" | tr -d '\r')
    repo=$(awk -F' *: *' '$1 == "Repository" { print $2; exit }' <<<"$info")
    arch=$(awk -F' *: *' '$1 == "Architecture" { print $2; exit }' <<<"$info")
    printf '%s/%s/os/x86_64/%s-%s-%s.pkg.tar.zst\n' "$STEAMOS_MIRROR" "$repo" "$1" "$2" "$arch"
}

prepare_steamos_box() {
    local qt repos pins name version rest mismatched
    qt=$(library_version libQt6Core)
    [[ $(printf '%s\n' "$QT_MIN_VERSION" "$qt" | sort -V | head -1) == "$QT_MIN_VERSION" ]] \
        || fail "This SteamOS build has Qt $qt, and KBoard needs Qt $QT_MIN_VERSION, which SteamOS 3.9 brings. Switch to a newer update channel in Settings > System, update, and run ./install.sh again."
    command -v distrobox >/dev/null || fail "distrobox is missing. SteamOS 3.5 and newer ship it."
    mapfile -t repos < <(sed -n 's/^\[\(.*\)\]$/\1/p' /etc/pacman.conf | grep -vx options)
    remove_build_boxes "$BUILD_BOX"
    if ! podman container exists "$BUILD_BOX"; then
        echo "Creating the $BUILD_BOX box to build KBoard in..."
        remember_pulled_image docker.io/library/archlinux:latest
        distrobox create --yes --no-entry --name "$BUILD_BOX" --image docker.io/library/archlinux:latest
    fi
    echo "Matching the box to this SteamOS build's Qt and KDE Frameworks..."
    steamos_pacman_conf "Required DatabaseOptional" "${repos[@]}" | in_build_env sudo tee /etc/pacman.conf >/dev/null
    steamos_pacman_conf Never "${repos[@]}" | in_build_env sudo tee /etc/pacman-keyring.conf >/dev/null
    in_build_env sudo pacman -Sy --noconfirm --config /etc/pacman-keyring.conf holo-keyring
    in_build_env sudo pacman-key --init
    in_build_env sudo pacman-key --populate holo
    in_build_env sudo pacman -Syuu --noconfirm --ignore filesystem
    in_build_env sudo pacman -S --needed --noconfirm "${ARCH_PACKAGES[@]}"
    pins=()
    while read -r name version rest; do
        pins+=("$(steamos_package_url "$name" "$version")")
    done < <(steamos_mismatches)
    if ((${#pins[@]})); then
        in_build_env sudo pacman -U --noconfirm "${pins[@]}"
    fi
    mismatched=$(steamos_mismatches | awk '{ printf "%s (system %s, box %s) ", $1, $2, $3 }')
    [[ -z $mismatched ]] || fail "Valve's repositories don't carry the exact versions this SteamOS build runs: ${mismatched}Update SteamOS and run ./install.sh again."
}

prepare_build_box() {
    QT_MIN_VERSION=$(sed -n 's/^set(QT_MIN_VERSION "\(.*\)")$/\1/p' "$(dirname "${BASH_SOURCE[0]}")/../CMakeLists.txt")
    if [[ $BUILD_BOX == "$UPDATE_ID-steamos" ]]; then
        prepare_steamos_box
    else
        prepare_fedora_box
    fi
}
