.pragma library

const hints = { q: "1", w: "2", e: "3", r: "4", t: "5", y: "6", u: "7", i: "8", o: "9", p: "0",
                a: "@", s: "#", d: "&", f: "*", g: "-", h: "+", j: "(", k: ")", l: "'",
                z: "_", x: "\"", c: ":", v: ";", b: "!", n: "?", m: "/" };

const accents = { e: ["è", "é", "ê", "ë", "ē"], a: ["à", "á", "â", "ä", "å"], o: ["ò", "ó", "ô", "ö", "ø"], u: ["ù", "ú", "û", "ü"], i: ["ì", "í", "î", "ï"], n: ["ñ"], c: ["ç"], s: ["ß", "ś", "š"] };

function rows(options) {
    const result = [];
    if (options.desktopRow) {
        result.push({ height: 0.8, keys: [
            { id: "esc", label: "Esc", w: 1.25, kind: "fn" }, { id: "tab", label: "Tab", w: 1.25, kind: "fn" },
            { id: "ctrl", label: "Ctrl", w: 1.25, kind: "fn" }, { id: "alt", label: "Alt", w: 1.25, kind: "fn" },
            { id: "left", icon: "arrow-left", w: 1.25, kind: "fn" }, { id: "down", icon: "arrow-down", w: 1.25, kind: "fn" },
            { id: "up", icon: "arrow-up", w: 1.25, kind: "fn" }, { id: "right", icon: "arrow-right", w: 1.25, kind: "fn" }
        ] });
    }
    if (options.numberRow) {
        result.push({ height: 0.85, keys: "1234567890".split("").map(c => ({ id: c, label: c, w: 1, kind: "char" })) });
    }
    result.push({ height: 1, keys: "qwertyuiop".split("").map(c => ({ id: c, label: c, w: 1, kind: "char" })) });
    result.push({ height: 1, indent: 0.5, keys: "asdfghjkl".split("").map(c => ({ id: c, label: c, w: 1, kind: "char" })) });
    result.push({ height: 1, keys: [{ id: "shift", icon: "arrow-up", w: 1.5, kind: "fn" }]
        .concat("zxcvbnm".split("").map(c => ({ id: c, label: c, w: 1, kind: "char" })))
        .concat([{ id: "backspace", icon: "edit-clear-symbolic", w: 1.5, kind: "fn" }]) });
    result.push({ height: 1, keys: [
        { id: "symbols", label: "?123", w: 1.5, kind: "fn" }, { id: "emoji", icon: "smiley-symbolic", w: 1, kind: "fn" },
        { id: ",", label: ",", w: 1, kind: "char" }, { id: "space", label: options.spaceLabel || "", w: 4, kind: "space" },
        { id: ".", label: ".", w: 1, kind: "char" }, { id: "enter", icon: "keyboard-enter-symbolic", w: 1.5, kind: "accent" }
    ] });
    return result;
}

function layout(options) {
    const list = rows(options);
    const totalHeight = list.reduce((sum, row) => sum + row.height, 0);
    const keys = [];
    let y = 0;
    for (const row of list) {
        let x = row.indent || 0;
        const width = row.keys.reduce((sum, key) => sum + key.w, 0) + 2 * (row.indent || 0);
        const factor = 10 / width;
        x *= factor;
        for (const key of row.keys) {
            const w = key.w * factor;
            if (key.kind === "space") {
                keys.push(Object.assign({}, key, { x: x, y: y, w: w, h: row.height, part: "whole" }));
                keys.push(Object.assign({}, key, { id: "space-left", x: x, y: y, w: 5 - x, h: row.height, part: "left" }));
                keys.push(Object.assign({}, key, { id: "space-right", x: 5, y: y, w: x + w - 5, h: row.height, part: "right" }));
            } else {
                keys.push(Object.assign({}, key, { x: x, y: y, w: w, h: row.height, part: "" }));
            }
            x += w;
        }
        y += row.height;
    }
    return { keys: keys, height: totalHeight };
}

function splitX(x, w, split, halfWidth) {
    const factor = 1 + (2 * halfWidth - 1) * split;
    const center = x + w / 2;
    if (center < 5) {
        return { x: x * factor, w: w * factor };
    }
    return { x: 10 - (10 - x) * factor, w: w * factor };
}
