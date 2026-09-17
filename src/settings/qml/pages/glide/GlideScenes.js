.pragma library

function steps(options) {
    let list = [];
    if (options.glide) {
        list = list.concat([{ id: "glideIn", ms: 400 }, { id: "glide", ms: 1700, ease: "inOutQuad" }, { id: "glideHold", ms: 1000 }]);
    }
    if (options.cursor) {
        list = list.concat([{ id: "cursorIn", ms: 400 }, { id: "cursor", ms: 1900, ease: "inOutCubic" }, { id: "cursorHold", ms: 500 }]);
    }
    if (options.backspace) {
        list = list.concat([{ id: "deleteIn", ms: 400 }, { id: "delete", ms: 1400, ease: "inOutCubic" }, { id: "deleteHold", ms: 900 }]);
    }
    return list.length > 0 ? list : [{ id: "off", ms: 1000 }];
}

function scene(step) {
    if (step.startsWith("glide")) {
        return "glide";
    }
    if (step.startsWith("cursor")) {
        return "cursor";
    }
    if (step.startsWith("delete")) {
        return "delete";
    }
    return "off";
}

const captions = {
    glide: "Slide through the letters and lift to type the word",
    cursor: "Slide along the space bar to move the cursor",
    delete: "Slide left from backspace to select words, lift to delete them",
    off: "Glide typing and swipe gestures are turned off"
};
