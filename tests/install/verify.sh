#!/usr/bin/env bash
set -uo pipefail

failed=0
check() {
    if eval "$2" >/dev/null 2>&1; then
        echo "ok: $1"
    else
        echo "FAILED: $1"
        failed=1
    fi
}

desktop="$HOME/.local/share/applications/org.devl0rd.kboard.desktop"
check "KWin uses KBoard as the on-screen keyboard" "[[ \$(kreadconfig6 --file kwinrc --group Wayland --key InputMethod) == '$desktop' ]]"
check "KWin started KBoard" "pgrep -u \"\$USER\" -x kboard"
check "org.devl0rd.KBoard answers" "gdbus call --session --dest org.devl0rd.KBoard --object-path /KBoard --method org.devl0rd.KBoard.IsVisible"
check "the settings app is installed" "test -x \$HOME/.local/bin/kboard-settings"
check "the voice model is downloaded" "find \$HOME/.local/share/kboard/models -type f | grep -q ."
check "the widget is installed" "test -f \$HOME/.local/share/plasma/plasmoids/org.devl0rd.kboard/metadata.json"
check "the system update hook is registered" "test -f /usr/share/libalpm/hooks/kboard-update.hook"
check "kboard-update.service is enabled" "systemctl --user is-enabled kboard-update.service"
check "Plasma runs" "systemctl --user is-active plasma-plasmashell.service"
errors=$(journalctl --user --no-pager -o cat | grep -E 'org\.devl0rd\.kboard|org/devl0rd/kboard' | grep -iE 'error|not installed|not a type|unavailable' | sort -u)
check "no KBoard QML errors were logged" "[[ -z \"\$errors\" ]]"
[[ -n $errors ]] && echo "$errors"
exit "$failed"
