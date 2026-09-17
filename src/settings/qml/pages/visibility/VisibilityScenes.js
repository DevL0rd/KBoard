.pragma library

function steps(swipe) {
    const list = [
        { id: "touchMove", ms: 700, ease: "inOutCubic" }, { id: "touchTap", ms: 300 }, { id: "touchShow", ms: 500, ease: "outCubic" }, { id: "touchHold", ms: 1000 }, { id: "touchHide", ms: 400, ease: "inCubic" },
        { id: "mouseMove", ms: 700, ease: "inOutCubic" }, { id: "mouseTap", ms: 300 }, { id: "mouseShow", ms: 500, ease: "outCubic" }, { id: "mouseHold", ms: 1000 }
    ];
    if (swipe) {
        return list.concat([{ id: "swipeTouch", ms: 300 }, { id: "swipe", ms: 600, ease: "inCubic" }, { id: "swipeRest", ms: 700 }]);
    }
    return list.concat([{ id: "mouseHide", ms: 400, ease: "inCubic" }, { id: "rest", ms: 500 }]);
}

function input(step) {
    return step.startsWith("mouse") ? "mouse" : "touch";
}

function opens(mode, input) {
    return mode === 2 || (mode === 1 && input === "touch");
}

function caption(step, mode) {
    if (step.startsWith("swipe")) {
        return "Swipe down on the keyboard to hide it";
    }
    const how = input(step) === "mouse" ? "Clicking a text field with the mouse" : "Tapping a text field";
    return how + (opens(mode, input(step)) ? " opens the keyboard" : " leaves the keyboard closed");
}
