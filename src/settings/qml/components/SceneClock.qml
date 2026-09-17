pragma ComponentBehavior: Bound

import QtQuick
import "Ease.js" as Ease

Item {
    id: clock

    property var steps: [
        { id: "pause", ms: 400 },
        { id: "move", ms: 1400, ease: "inOutCubic" },
        { id: "hold", ms: 700 },
        { id: "return", ms: 500, ease: "inOutQuad" }
    ]
    property bool active: true
    property real stillTime: -1
    property real time: 0
    property int cycle: 0
    readonly property real total: steps.reduce((sum, step) => sum + step.ms, 0)
    readonly property bool animating: loop.running
    readonly property real now: Motion.enabled ? time : (stillTime >= 0 ? stillTime : defaultStill())
    readonly property var starts: {
        const result = {};
        let at = 0;
        for (const step of steps) {
            result[step.id] = at;
            at += step.ms;
        }
        return result;
    }
    readonly property string step: {
        let at = 0;
        for (const s of steps) {
            if (now < at + s.ms) {
                return s.id;
            }
            at += s.ms;
        }
        return steps.length ? steps[steps.length - 1].id : "";
    }

    signal looped

    function defaultStill() {
        const hold = steps.find(s => s.id === "hold");
        return hold ? starts["hold"] + hold.ms / 2 : total / 2;
    }

    function raw(id) {
        const s = steps.find(entry => entry.id === id);
        if (!s) {
            return 0;
        }
        return Ease.segment(now, starts[id], starts[id] + s.ms);
    }

    function progress(id) {
        const s = steps.find(entry => entry.id === id);
        return s ? Ease.apply(s.ease || "linear", raw(id)) : 0;
    }

    function within(id, from, to, ease) {
        return Ease.apply(ease || "linear", Ease.segment(raw(id), from, to));
    }

    width: 0
    height: 0

    onTimeChanged: {
        if (time < lastTime) {
            cycle += 1;
            looped();
        }
        lastTime = time;
    }
    property real lastTime: 0

    NumberAnimation on time {
        id: loop
        from: 0
        to: clock.total
        duration: Math.max(1, Math.round(clock.total / Motion.speed))
        loops: Animation.Infinite
        running: clock.active && Motion.enabled && clock.visible && clock.total > 0
                 && clock.Window.window !== null && clock.Window.window.visibility !== Window.Minimized && clock.Window.window.visibility !== Window.Hidden
    }
}
