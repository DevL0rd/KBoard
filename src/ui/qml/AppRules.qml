pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.config

QtObject {
    id: rules

    property string application

    readonly property var active: {
        const name = application.toLowerCase()
        if (name === "")
            return []
        return Settings.appRules.map(entry => {
            const split = entry.lastIndexOf("=")
            return split > 0 ? { appId: entry.slice(0, split).toLowerCase(), rule: entry.slice(split + 1) } : { appId: entry.toLowerCase(), rule: "" }
        }).filter(entry => entry.appId !== "" && name.includes(entry.appId)).map(entry => entry.rule)
    }

    readonly property bool alwaysShow: active.includes("always")
    readonly property bool neverShow: active.includes("never")
    readonly property bool desktopRow: active.includes("desktoprow")
    readonly property bool noLearn: active.includes("nolearn")
}
