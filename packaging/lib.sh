#!/usr/bin/env bash

UPDATE_ID="kboard"
UPDATE_TITLE="KBoard"
UPDATE_UNIT="$UPDATE_ID-update.service"
UPDATE_DIR="/usr/lib/$UPDATE_ID"
UPDATE_STATE_DIR="/var/lib/$UPDATE_ID"
UPDATE_STATE_HOME="$HOME/.local/state/$UPDATE_ID"
UPDATE_PENDING="$UPDATE_STATE_HOME/update-pending"
UPDATE_LOGIN_UNIT="$UPDATE_ID-login-update.service"
UPDATE_SOURCE_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/$UPDATE_ID/source"
UPDATE_GIT_ENV=(GIT_TERMINAL_PROMPT=0 GIT_ASKPASS= "GIT_SSH_COMMAND=ssh -o BatchMode=yes -o ConnectTimeout=15")
UPDATE_LEGACY_HOOK="/etc/pacman.d/hooks/$UPDATE_ID-update.hook"
declare -A UPDATE_HOOKS=(
    [pacman]="$UPDATE_ID-update.hook /usr/share/libalpm/hooks/$UPDATE_ID-update.hook 644"
    [dnf]="$UPDATE_ID-update.actions /etc/dnf/libdnf5-plugins/actions.d/$UPDATE_ID-update.actions 644"
    [zypper]="$UPDATE_ID-update.zypp /usr/lib/zypp/plugins/commit/$UPDATE_ID-update 755"
    [apt-get]="$UPDATE_ID-update.apt /etc/apt/apt.conf.d/99$UPDATE_ID-update 644"
)

BUILD_BOX=""
BUILD_ENV=()
if [[ -e /run/ostree-booted ]]; then
    BUILD_BOX="$UPDATE_ID-fedora-$(. /etc/os-release && printf '%s' "$VERSION_ID")"
    BUILD_ENV=(toolbox run --container "$BUILD_BOX")
elif [[ $(. /etc/os-release && printf '%s' "$ID") == steamos ]]; then
    BUILD_BOX="$UPDATE_ID-steamos"
    BUILD_ENV=(distrobox enter "$BUILD_BOX" --)
fi

image_based_system() {
    [[ -n $BUILD_BOX ]]
}

in_build_env() {
    "${BUILD_ENV[@]}" "$@"
}

valve_repositories() {
    grep -qE '^\[(jupiter|holo)(-[0-9.]+)?\]' /etc/pacman.conf 2>/dev/null
}

library_version() {
    local library
    library=$(find /usr/lib /usr/lib64 -maxdepth 2 -name "$1.so.6" -print -quit 2>/dev/null)
    [[ -n $library ]] && basename "$(readlink -f "$library")" | sed "s/^$1\.so\.//"
}

system_fingerprint() {
    local module
    if image_based_system; then
        printf 'image=%s\n' "$(. /etc/os-release && printf '%s' "${OSTREE_VERSION:-${BUILD_ID:-$VERSION_ID}}")"
        printf 'qt=%s\n' "$(library_version libQt6Core)"
        printf 'kf=%s\n' "$(library_version libKF6CoreAddons)"
        return 0
    fi
    for module in Qt6Core Qt6WaylandClient whisper parakeet ggml; do
        printf '%s=%s\n' "$module" "$(pkg-config --modversion "$module" 2>/dev/null || echo none)"
    done
}

package_manager() {
    local manager
    for manager in pacman dnf zypper apt-get; do
        if command -v "$manager" >/dev/null; then
            printf '%s\n' "$manager"
            return 0
        fi
    done
    return 1
}

update_as_root() {
    if [[ $EUID -eq 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

package_owns() {
    case "$(package_manager)" in
    pacman) pacman -Qoq "$1" >/dev/null 2>&1 ;;
    dnf | zypper) rpm -qf "$1" >/dev/null 2>&1 ;;
    apt-get) dpkg -S "$1" >/dev/null 2>&1 ;;
    *) return 0 ;;
    esac
}

remove_system_file() {
    [[ -e $1 ]] || return 0
    update_as_root rm -rf "$1"
    local directory
    directory=$(dirname "$1")
    if [[ -d $directory && -z $(ls -A "$directory") ]] && ! package_owns "$directory"; then
        update_as_root rmdir "$directory"
    fi
}

install_update_hook() {
    local checkout="$1" manager source target mode
    manager=$(package_manager) || return 1
    read -r source target mode <<<"${UPDATE_HOOKS[$manager]}"
    update_as_root install -Dm755 "$checkout/packaging/system-update" "$UPDATE_DIR/system-update"
    update_as_root install -Dm644 "$checkout/packaging/lib.sh" "$UPDATE_DIR/lib.sh"
    update_as_root install -Dm"$mode" "$checkout/packaging/$source" "$target"
    if [[ $manager == pacman ]] && grep -qa NetworkAccess /usr/lib/libalpm.so.*; then
        update_as_root sed -i '/^Exec = /a NetworkAccess = allowed' "$target"
    fi
    remove_system_file "$UPDATE_LEGACY_HOOK"
}

update_source_url() {
    local url rest
    url=$(git -C "$1" remote get-url origin 2>/dev/null) || return 1
    case $url in
    git@*:*)
        rest=${url#git@}
        url="https://${rest/://}"
        ;;
    ssh://git@*) url="https://${url#ssh://git@}" ;;
    esac
    printf '%s\n' "$url"
}

prepare_update_source() {
    local checkout="$1" url branch
    [[ $checkout -ef $UPDATE_SOURCE_DIR ]] && return 0
    if ! url=$(update_source_url "$checkout"); then
        echo "$checkout has no origin remote to keep an update copy of $UPDATE_TITLE from."
        exit 1
    fi
    echo "Keeping a copy of $UPDATE_TITLE in $UPDATE_SOURCE_DIR for updates..."
    if ! git -C "$UPDATE_SOURCE_DIR" rev-parse --git-dir >/dev/null 2>&1; then
        rm -rf "$UPDATE_SOURCE_DIR"
        mkdir -p "$(dirname "$UPDATE_SOURCE_DIR")"
        if ! env "${UPDATE_GIT_ENV[@]}" git clone --quiet --recurse-submodules "$url" "$UPDATE_SOURCE_DIR"; then
            echo "Could not clone $url into $UPDATE_SOURCE_DIR."
            exit 1
        fi
        return 0
    fi
    git -C "$UPDATE_SOURCE_DIR" remote set-url origin "$url"
    branch=$(git -C "$UPDATE_SOURCE_DIR" symbolic-ref --short HEAD)
    if ! env "${UPDATE_GIT_ENV[@]}" git -C "$UPDATE_SOURCE_DIR" fetch --quiet origin; then
        echo "Could not fetch $url into $UPDATE_SOURCE_DIR."
        exit 1
    fi
    git -C "$UPDATE_SOURCE_DIR" checkout --quiet --force -B "$branch" "origin/$branch"
    env "${UPDATE_GIT_ENV[@]}" git -C "$UPDATE_SOURCE_DIR" submodule update --init --recursive --quiet
}

remove_update_source() {
    [[ -d $UPDATE_SOURCE_DIR ]] || return 0
    rm -rf "$UPDATE_SOURCE_DIR"
    rmdir --ignore-fail-on-non-empty "$(dirname "$UPDATE_SOURCE_DIR")"
}

enable_user_unit() {
    local units="${XDG_CONFIG_HOME:-$HOME/.config}/systemd/user"
    mkdir -p "$units"
    sed "s|@CHECKOUT@|$UPDATE_SOURCE_DIR|g" "$1/packaging/$2.in" >"$units/$2"
    systemctl --user daemon-reload
    systemctl --user enable "$2" >/dev/null 2>&1
}

register_system_updates() {
    local checkout="$1" aur="$2"
    if [[ $aur == true ]] || ! git -C "$checkout" rev-parse --git-dir >/dev/null 2>&1 || { ! image_based_system && ! package_manager >/dev/null; }; then
        unregister_system_updates
        return 0
    fi
    prepare_update_source "$checkout"
    echo "Registering $UPDATE_TITLE with system updates..."
    if image_based_system; then
        enable_user_unit "$checkout" "$UPDATE_LOGIN_UNIT"
        return 0
    fi
    install_update_hook "$checkout"
    printf '%s\n%s\n' "$UPDATE_SOURCE_DIR" "$(id -un)" | update_as_root install -Dm644 /dev/stdin "$UPDATE_STATE_DIR/source"
    enable_user_unit "$checkout" "$UPDATE_UNIT"
}

unregister_system_updates() {
    local entry source target mode
    for entry in "${UPDATE_HOOKS[@]}" "- $UPDATE_LEGACY_HOOK -"; do
        read -r source target mode <<<"$entry"
        remove_system_file "$target"
    done
    remove_system_file "$UPDATE_STATE_DIR"
    remove_system_file "$UPDATE_DIR"
    local name unit
    for name in "$UPDATE_UNIT" "$UPDATE_LOGIN_UNIT"; do
        unit="${XDG_CONFIG_HOME:-$HOME/.config}/systemd/user/$name"
        if [[ -e $unit ]]; then
            systemctl --user disable "$name" >/dev/null 2>&1 || true
            rm -f "$unit"
            systemctl --user daemon-reload
        fi
    done
    rm -f "$UPDATE_PENDING"
    remove_update_source
}

pull_checkout() {
    "$@" rev-parse --abbrev-ref '@{upstream}' >/dev/null 2>&1 || return 1
    if ! "$@" fetch --quiet; then
        printf '%s: %s\n' "$UPDATE_ID" "could not fetch updates, rebuilding the current checkout only"
        return 1
    fi
    [[ $("$@" rev-list --count 'HEAD..@{upstream}') -gt 0 ]] || return 1
    if [[ -n $("$@" status --porcelain --untracked-files=no) ]]; then
        printf '%s: %s\n' "$UPDATE_ID" "the checkout has local changes, skipping the pull"
        return 1
    fi
    if [[ $("$@" rev-list --count '@{upstream}..HEAD') -gt 0 ]]; then
        printf '%s: %s\n' "$UPDATE_ID" "the checkout has local commits that aren't upstream, skipping the pull"
        return 1
    fi
    "$@" merge --ff-only --quiet '@{upstream}' || return 1
    "$@" submodule update --init --recursive --quiet
    printf '%s: updated to %s\n' "$UPDATE_ID" "$("$@" rev-parse --short HEAD)"
}

notify_updated() {
    rm -f "$UPDATE_PENDING"
    gdbus call --session --dest org.freedesktop.Notifications --object-path /org/freedesktop/Notifications \
        --method org.freedesktop.Notifications.Notify "$UPDATE_TITLE" 0 system-software-update "$UPDATE_TITLE updated" "$1" '[]' '{}' 10000 >/dev/null 2>&1 || true
}
