.pragma library
.import "Ease.js" as Ease

const Flat = 0;
const Bordered = 1;
const Raised = 2;
const Glass = 3;

function face(style, kind, palette) {
    const base = kind === "accent" ? palette.accent : (kind === "fn" ? Ease.mix(palette.key, palette.panel, 0.55) : palette.key);
    if (kind === "accent") {
        return style === Glass ? Qt.alpha(base, 0.7) : base;
    }
    switch (style) {
    case Flat:
        return Qt.alpha(base, kind === "fn" ? 0 : 0.55);
    case Bordered:
        return Qt.alpha(base, 0.28);
    case Glass:
        return Qt.alpha("white", palette.dark ? (kind === "fn" ? 0.05 : 0.11) : (kind === "fn" ? 0.25 : 0.5));
    }
    return base;
}

function borderWidth(style) {
    return style === Bordered || style === Glass ? 1 : 0;
}

function borderColor(style, palette) {
    return style === Glass ? Qt.alpha("white", palette.dark ? 0.2 : 0.7) : Qt.alpha(palette.text, 0.3);
}

function shadowColor(palette) {
    return Qt.alpha(Qt.darker(palette.panel, palette.dark ? 1.8 : 1.35), 0.9);
}
