.pragma library

const KeyMs = 170;
const HoldMs = 1100;

function expansionFor(expansions, abbreviation) {
    for (const entry of expansions) {
        const split = entry.indexOf("=");
        if (split > 0 && entry.substring(0, split) === abbreviation) {
            return entry.substring(split + 1);
        }
    }
    return "";
}

function strengthName(level) {
    return ["off", "mild", "normal", "aggressive"][level] || "normal";
}

function builder(options) {
    const frames = [];
    let text = "";
    const push = (extra) => frames.push(Object.assign({ ms: KeyMs, text: text, key: "", suggestions: ["", "", ""], markStart: -1, markLength: 0, caption: "", shifted: false }, extra));
    const suggest = (list) => options.suggestions ? list : ["", "", ""];
    const typeWord = (word, list, caption) => {
        for (let i = 0; i < word.length; ++i) {
            const capital = text.length === 0 || /[.!?] $/.test(text);
            const shifted = capital && options.autoCapitalize;
            const character = shifted ? word[i].toUpperCase() : word[i];
            text += character;
            push({ key: word[i].toLowerCase(), shifted: shifted, suggestions: suggest(i === word.length - 1 ? list : ["", text.split(" ").pop(), ""]), caption: caption });
        }
    };
    return { frames: frames, push: push, suggest: suggest, typeWord: typeWord, text: () => text, setText: value => { text = value; } };
}

function build(options) {
    const b = builder(options);
    const capsCaption = options.autoCapitalize ? "Sentences start with a capital" : "Automatic capitals are off";
    b.typeWord("see", ["see", "See", "seen"], capsCaption);
    b.setText(b.text() + " ");
    b.push({ key: "space", caption: capsCaption });
    b.typeWord("you", ["you", "you", "your"], capsCaption);
    b.setText(b.text() + " ");
    b.push({ key: "space" });
    const intended = "tomorrow";
    const typo = intended.slice(0, 5) + intended.slice(6);
    b.typeWord(typo, ["\"" + typo + "\"", intended, intended + "s"], "");
    const corrected = options.autocorrect > 0;
    const before = b.text();
    const start = before.length - typo.length;
    b.setText(before.substring(0, start) + (corrected ? intended : typo) + " ");
    b.push({ key: "space", ms: HoldMs, markStart: start, markLength: corrected ? intended.length : typo.length,
             suggestions: b.suggest(options.nextWordPrediction ? ["at", "then", "morning"] : ["", "", ""]),
             caption: corrected ? "Autocorrect (" + strengthName(options.autocorrect) + ") fixed “" + typo + "”" : "Autocorrect is off, so “" + typo + "” stays" });
    if (options.doubleSpacePeriod) {
        b.setText(b.text().replace(/ $/, ". "));
        b.push({ key: "space", ms: HoldMs, markStart: b.text().length - 2, markLength: 1, caption: "Double space typed a full stop" });
    } else {
        b.setText(b.text().replace(/ $/, ". "));
        b.push({ key: ".", ms: 500, caption: "Full stop typed by hand" });
    }
    const expansion = expansionFor(options.expansions, "omw");
    b.typeWord("omw", ["omw", expansion || "OMW", options.emojiSuggestions ? "🏃" : "own"], capsCaption);
    if (expansion.length > 0) {
        const beforeExpansion = b.text();
        b.setText(beforeExpansion.replace(/omw$/i, expansion) + " ");
        b.push({ key: "space", ms: HoldMs * 1.4, markStart: beforeExpansion.length - 3, markLength: expansion.length, caption: "Text shortcut “omw” expanded" });
    } else {
        b.setText(b.text() + " ");
        b.push({ key: "space", ms: HoldMs, caption: "Add “omw” as a text shortcut to expand it" });
    }
    b.push({ ms: 900, caption: "" });
    return b.frames;
}
