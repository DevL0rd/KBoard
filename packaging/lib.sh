#!/usr/bin/env bash

UPDATE_ID="kboard"
UPDATE_TITLE="KBoard"
UPDATE_UNIT="$UPDATE_ID-update.service"
UPDATE_DIR="/usr/lib/$UPDATE_ID"
UPDATE_STATE_DIR="/var/lib/$UPDATE_ID"
UPDATE_STATE_HOME="$HOME/.local/state/$UPDATE_ID"
UPDATE_PENDING="$UPDATE_STATE_HOME/update-pending"
UPDATE_LEGACY_HOOK="/etc/pacman.d/hooks/$UPDATE_ID-update.hook"
declare -A UPDATE_HOOKS=(
    [pacman]="$UPDATE_ID-update.hook /usr/share/libalpm/hooks/$UPDATE_ID-update.hook 644"
    [dnf]="$UPDATE_ID-update.actions /etc/dnf/libdnf5-plugins/actions.d/$UPDATE_ID-update.actions 644"
    [zypper]="$UPDATE_ID-update.zypp /usr/lib/zypp/plugins/commit/$UPDATE_ID-update 755"
    [apt-get]="$UPDATE_ID-update.apt /etc/apt/apt.conf.d/99$UPDATE_ID-update 644"
)

image_based_system() {
    [[ -e /run/ostree-booted || $(. /etc/os-release && printf '%s' "$ID") == steamos ]]
}

refuse_image_based_system() {
    echo "KBoard can't be installed on image-based systems like Fedora Atomic desktops and SteamOS yet."
    echo "Voice typing needs whisper.cpp with Parakeet, and their read-only system images don't include it."
    exit 1
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

register_system_updates() {
    local checkout="$1" aur="$2"
    if [[ $aur == true ]] || ! package_manager >/dev/null || ! git -C "$checkout" rev-parse --git-dir >/dev/null 2>&1; then
        unregister_system_updates
        return 0
    fi
    echo "Registering $UPDATE_TITLE with system updates..."
    install_update_hook "$checkout"
    printf '%s\n%s\n' "$checkout" "$(id -un)" | update_as_root install -Dm644 /dev/stdin "$UPDATE_STATE_DIR/source"
    local units="${XDG_CONFIG_HOME:-$HOME/.config}/systemd/user"
    mkdir -p "$units"
    sed "s|@CHECKOUT@|$checkout|g" "$checkout/packaging/$UPDATE_ID-update.service.in" >"$units/$UPDATE_UNIT"
    systemctl --user daemon-reload
    systemctl --user enable "$UPDATE_UNIT" >/dev/null 2>&1
}

unregister_system_updates() {
    local entry source target mode
    for entry in "${UPDATE_HOOKS[@]}" "- $UPDATE_LEGACY_HOOK -"; do
        read -r source target mode <<<"$entry"
        remove_system_file "$target"
    done
    remove_system_file "$UPDATE_STATE_DIR"
    remove_system_file "$UPDATE_DIR"
    local unit="${XDG_CONFIG_HOME:-$HOME/.config}/systemd/user/$UPDATE_UNIT"
    if [[ -e $unit ]]; then
        systemctl --user disable "$UPDATE_UNIT" >/dev/null 2>&1 || true
        rm -f "$unit"
        systemctl --user daemon-reload
    fi
    rm -f "$UPDATE_PENDING"
}

notify_updated() {
    rm -f "$UPDATE_PENDING"
    gdbus call --session --dest org.freedesktop.Notifications --object-path /org/freedesktop/Notifications \
        --method org.freedesktop.Notifications.Notify "$UPDATE_TITLE" 0 system-software-update "$UPDATE_TITLE updated" "$1" '[]' '{}' 10000 >/dev/null 2>&1 || true
}
