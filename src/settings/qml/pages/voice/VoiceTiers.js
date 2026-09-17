.pragma library

const accuracyByTier = { light: 0.6, default: 0.84, quality: 0.93, languages: 0.9 };

function accuracy(model) {
    const base = accuracyByTier[model.tier] || 0.75;
    return Math.min(1, base + (model.tier === "light" ? Math.log2(Math.max(1, model.sizeMb / 60)) * 0.08 : 0));
}

function speed(model) {
    return Math.max(0.15, Math.min(1, 1.25 - Math.log10(Math.max(10, model.sizeMb)) / 3.2));
}

function details(model) {
    const count = (model.languages || []).length;
    const parts = [sizeText(model.sizeMb)];
    if (count > 0) {
        parts.push(count === 1 ? "1 language" : count + " languages");
    }
    parts.push(model.license);
    return parts.join(" · ");
}

function sizeText(megabytes) {
    return megabytes >= 1024 ? (megabytes / 1024).toFixed(1) + " GB" : megabytes + " MB";
}
