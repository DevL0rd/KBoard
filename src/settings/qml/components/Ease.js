.pragma library

function clamp(v, lo, hi) {
    return Math.max(lo, Math.min(hi, v));
}

function lerp(a, b, t) {
    return a + (b - a) * t;
}

function segment(t, from, to) {
    return to <= from ? (t >= to ? 1 : 0) : clamp((t - from) / (to - from), 0, 1);
}

function inOutCubic(t) {
    return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
}

function outCubic(t) {
    return 1 - Math.pow(1 - t, 3);
}

function inCubic(t) {
    return t * t * t;
}

function inOutQuad(t) {
    return t < 0.5 ? 2 * t * t : 1 - Math.pow(-2 * t + 2, 2) / 2;
}

function outBack(t) {
    const c1 = 1.70158;
    const c3 = c1 + 1;
    return 1 + c3 * Math.pow(t - 1, 3) + c1 * Math.pow(t - 1, 2);
}

function pulse(t) {
    return Math.sin(clamp(t, 0, 1) * Math.PI);
}

function apply(name, t) {
    switch (name) {
    case "inOutCubic":
        return inOutCubic(t);
    case "outCubic":
        return outCubic(t);
    case "inCubic":
        return inCubic(t);
    case "inOutQuad":
        return inOutQuad(t);
    case "outBack":
        return outBack(t);
    }
    return t;
}

function mix(a, b, t) {
    return Qt.rgba(lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t), lerp(a.a, b.a, t));
}
