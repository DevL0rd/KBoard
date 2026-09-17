.pragma library

function along(points, t) {
    if (points.length === 0) {
        return [];
    }
    const lengths = [0];
    for (let i = 1; i < points.length; ++i) {
        lengths.push(lengths[i - 1] + Math.hypot(points[i].x - points[i - 1].x, points[i].y - points[i - 1].y));
    }
    const target = lengths[lengths.length - 1] * Math.max(0, Math.min(1, t));
    const result = [points[0]];
    for (let i = 1; i < points.length; ++i) {
        if (lengths[i] <= target) {
            result.push(points[i]);
            continue;
        }
        const span = lengths[i] - lengths[i - 1];
        const f = span > 0 ? (target - lengths[i - 1]) / span : 0;
        result.push(Qt.point(points[i - 1].x + (points[i].x - points[i - 1].x) * f, points[i - 1].y + (points[i].y - points[i - 1].y) * f));
        break;
    }
    return result;
}

function smooth(points, steps) {
    if (points.length < 3) {
        return points;
    }
    const result = [];
    for (let i = 0; i < points.length - 1; ++i) {
        const p0 = points[Math.max(0, i - 1)];
        const p1 = points[i];
        const p2 = points[i + 1];
        const p3 = points[Math.min(points.length - 1, i + 2)];
        for (let s = 0; s < steps; ++s) {
            const t = s / steps;
            const t2 = t * t;
            const t3 = t2 * t;
            result.push(Qt.point(
                0.5 * (2 * p1.x + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3),
                0.5 * (2 * p1.y + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3)));
        }
    }
    result.push(points[points.length - 1]);
    return result;
}

function wordSelection(text, keep, amount) {
    const removable = text.length - keep;
    return Math.round(removable * amount);
}
