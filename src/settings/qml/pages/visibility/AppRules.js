.pragma library

const rules = [
    { value: "never", label: "Never show the keyboard" },
    { value: "always", label: "Always show the keyboard" },
    { value: "desktoprow", label: "Always show the desktop row" },
    { value: "nolearn", label: "Don't learn words" }
];

function parse(list) {
    return list.map(entry => {
        const split = entry.lastIndexOf("=");
        return split > 0 ? { appId: entry.substring(0, split), rule: entry.substring(split + 1) } : { appId: entry, rule: "" };
    });
}

function serialize(entries) {
    return entries.map(entry => entry.appId + "=" + entry.rule);
}

function labelFor(rule) {
    const found = rules.find(entry => entry.value === rule);
    return found ? found.label : "Unknown rule “" + rule + "”";
}
