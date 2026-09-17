.pragma library

const presets = [5, 15, 60, 240, 1440, 10080, 0];

function text(minutes) {
    if (minutes === 0) {
        return "forever";
    }
    if (minutes % 10080 === 0) {
        return plural(minutes / 10080, "week");
    }
    if (minutes % 1440 === 0) {
        return plural(minutes / 1440, "day");
    }
    if (minutes % 60 === 0) {
        return plural(minutes / 60, "hour");
    }
    return plural(minutes, "minute");
}

function plural(count, unit) {
    return count + " " + unit + (count === 1 ? "" : "s");
}

function options(current) {
    const values = presets.indexOf(current) >= 0 ? presets : presets.concat([current]).sort((a, b) => (a || 1e9) - (b || 1e9));
    return values.map(value => ({ value: value, label: value === 0 ? "Until removed" : text(value) }));
}
