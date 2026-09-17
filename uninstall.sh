#!/bin/bash
set -euo pipefail

REPO_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
source "$REPO_DIR/packaging/lib.sh"
PREFIX="$HOME/.local"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/kboard"
MANIFEST="$CONFIG_DIR/install-manifest"

echo "Removing KBoard..."
"$REPO_DIR/packaging/kboard-input-method" restore

unregister_system_updates
kpackagetool6 -t Plasma/Applet -r org.devl0rd.kboard >/dev/null 2>&1 || true

if [[ -f "$MANIFEST" ]]; then
    while IFS= read -r installed || [[ -n $installed ]]; do
        rm -f "$installed"
        directory=$(dirname "$installed")
        while [[ $directory == "$PREFIX"/*/* && $directory != "$PREFIX"/share/applications ]] && rmdir "$directory" 2>/dev/null; do
            directory=$(dirname "$directory")
        done
    done <"$MANIFEST"
    rm -f "$MANIFEST"
    echo "Removed the keyboard, the settings app and their libraries from $PREFIX."
else
    echo "No install manifest at $MANIFEST, so no KBoard files were removed from $PREFIX."
fi
rm -f "$CONFIG_DIR/installed-revision"
rmdir "$CONFIG_DIR" 2>/dev/null || true

echo "Removed the widget and the update hook."
echo "Kept ~/.config/kboardrc, learned words, clipboard pins and downloaded voice models in ~/.local/share/kboard."
