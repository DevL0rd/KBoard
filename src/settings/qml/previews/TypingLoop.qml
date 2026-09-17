pragma ComponentBehavior: Bound

import QtQuick
import "Ease.js" as Ease

SceneClock {
    id: loop

    property var keys: []
    property int keyMs: 340
    property int restMs: 1000
    readonly property int keyIndex: step.startsWith("k") ? Number(step.substring(1)) : -1
    readonly property string pressedKey: keyIndex >= 0 ? keys[keyIndex] : ""
    readonly property real local: keyIndex >= 0 ? raw(step) : 0
    readonly property real press: keyIndex >= 0 ? Ease.pulse(Ease.segment(local, 0, 0.5)) : 0
    readonly property real popup: keyIndex >= 0 ? Ease.pulse(Ease.segment(local, 0, 0.85)) : 0
    readonly property real ripple: keyIndex >= 0 ? Ease.segment(local, 0.05, 0.75) : 0
    readonly property int typedCount: keyIndex >= 0 ? keyIndex + (local > 0.3 ? 1 : 0) : (step === "rest" ? keys.length : 0)
    readonly property string typed: keys.slice(0, typedCount).map(key => key === "space" ? " " : key).join("")

    steps: keys.map((key, index) => ({ id: "k" + index, ms: keyMs })).concat([{ id: "rest", ms: restMs }])
    stillTime: keys.length * keyMs + restMs / 2
}
