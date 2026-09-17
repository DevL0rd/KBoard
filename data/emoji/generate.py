#!/usr/bin/env python3
import argparse
import json
import re
import sys
import urllib.request
from pathlib import Path

EMOJI_TEST_URL = "https://unicode.org/Public/{version}.0/emoji/emoji-test.txt"
CLDR_URL = "https://cdn.jsdelivr.net/npm/{package}@{version}/{folder}/en/annotations.json"

EMOJI_VERSION = "18.0"
CLDR_VERSION = "48.2.0"

GROUPS = [
    ("smileys", "Smileys & Emotion", "😀"),
    ("people", "People & Body", "👋"),
    ("animals", "Animals & Nature", "🐻"),
    ("food", "Food & Drink", "🍔"),
    ("travel", "Travel & Places", "🚗"),
    ("activities", "Activities", "⚽"),
    ("objects", "Objects", "💡"),
    ("symbols", "Symbols", "🔣"),
    ("flags", "Flags", "🏁"),
]

POPULAR = "😂 ❤️ 🤣 👍 😭 🙏 😍 🥰 😘 😊 🎉 😁 💕 🥺 😅 🔥 ☺️ 🤦 ♥️ 🤷 🙄 😆 🤗 😉 🎂 🤔 👏 🙂 😳 🥳 😎 👌 💜 😔 💪 ✨ 💖 👀 😋 😏 😢 👉 💗 😩 💯 🌹 💞 🎈 💙 😃 😡 💐 😜 🙈 🤞 😄 🤤 🙌 🤪 ❣️ 😀 💋 💀 👇 💔 😌 💓 🤩 🙃 😬 😱 😴 🤭 😐 🌞 😒 😇 🌸 😈 🎶 ✌️ 🎊 🥵 😞 💚 ☀️ 🖤 💰 😚 👑 🎁 💥 🙋 ☹️ 😑 🥴 👈 💩 ✅ 🍕 🍺 ☕ 🎄 🐶 🐱 🌈 ⭐ 🌙 🚀 🎮 ⚽ 🏆 📷 💻 📱 🎵 🍔 🍩 🍰 🥂 🍻 🤝 👋 🫶 🥹 🫠 😮‍💨 🤯 😤 🤬 🥶 🤮 🤓 😷 🤒 👻 👽 🤖 💫 💤 💦 🍑 🍆 🌮 🍿"

TONES = ["1F3FB", "1F3FC", "1F3FD", "1F3FE", "1F3FF"]
TONE_CHARS = {chr(int(t, 16)) for t in TONES}
LINE = re.compile(r"^([0-9A-F ]+?)\s*;\s*([a-z-]+)\s*#\s*(\S+)\s+E(\d+\.\d+)\s+(.+)$")


def fetch(url, cache):
    target = cache / url.split("//", 1)[1].replace("/", "_")
    if not target.exists():
        print(f"downloading {url}", file=sys.stderr)
        with urllib.request.urlopen(url) as response:
            target.write_bytes(response.read())
    return target.read_text(encoding="utf-8")


def strip_vs(text):
    return text.replace("\ufe0f", "")


def strip_tones(text):
    return "".join(c for c in text if c not in TONE_CHARS)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cache", default=str(Path.home() / ".cache/kboard-emoji-sources"))
    parser.add_argument("--output", default=str(Path(__file__).with_name("emoji.json")))
    args = parser.parse_args()

    cache = Path(args.cache)
    cache.mkdir(parents=True, exist_ok=True)

    test = fetch(EMOJI_TEST_URL.format(version=EMOJI_VERSION), cache)
    annotations = json.loads(fetch(CLDR_URL.format(package="cldr-annotations-full", version=CLDR_VERSION, folder="annotations"), cache))
    derived = json.loads(fetch(CLDR_URL.format(package="cldr-annotations-derived-full", version=CLDR_VERSION, folder="annotationsDerived"), cache))

    keywords = {}
    for source in (annotations["annotations"]["annotations"], derived["annotationsDerived"]["annotations"]):
        for key, value in source.items():
            keywords.setdefault(strip_vs(key), value.get("default", []))

    group_ids = {name: index for index, (_, name, _) in enumerate(GROUPS)}
    subgroups = []
    subgroup_index = {}
    entries = []
    by_skeleton = {}
    toned = []
    group = None
    subgroup = None

    for raw in test.splitlines():
        if raw.startswith("# group:"):
            group = raw.split(":", 1)[1].strip()
            continue
        if raw.startswith("# subgroup:"):
            subgroup = raw.split(":", 1)[1].strip()
            continue
        match = LINE.match(raw)
        if not match or match.group(2) != "fully-qualified" or group not in group_ids:
            continue
        codepoints = match.group(1).split()
        emoji = "".join(chr(int(c, 16)) for c in codepoints)
        name = match.group(5).strip()
        if any(c in TONES for c in codepoints):
            toned.append(emoji)
            continue
        if subgroup not in subgroup_index:
            subgroup_index[subgroup] = len(subgroups)
            subgroups.append(subgroup)
        words = []
        for word in keywords.get(strip_vs(emoji), []) + re.split(r"[\s:,]+", name.lower()):
            word = word.strip().lower()
            if word and word not in words and word != name.lower():
                words.append(word)
        entry = [emoji, group_ids[group], subgroup_index[subgroup], name, "|".join(words), match.group(4), 0]
        entries.append(entry)
        by_skeleton[strip_vs(emoji)] = entry

    for emoji in toned:
        tones = {c for c in emoji if c in TONE_CHARS}
        if len(tones) != 1:
            continue
        entry = by_skeleton.get(strip_vs(strip_tones(emoji)))
        if entry is None:
            continue
        index = sorted(TONE_CHARS).index(next(iter(tones)))
        if entry[6] == 0:
            entry[6] = [""] * 5
        entry[6][index] = emoji

    for entry in entries:
        if entry[6] != 0 and any(not variant for variant in entry[6]):
            entry[6] = 0

    document = {
        "emojiVersion": EMOJI_VERSION,
        "cldrVersion": CLDR_VERSION,
        "groups": [{"id": gid, "name": name, "icon": icon} for gid, name, icon in GROUPS],
        "subgroups": subgroups,
        "fields": ["emoji", "group", "subgroup", "name", "keywords", "version", "skinTones"],
        "emoji": entries,
        "popular": POPULAR.split(),
    }
    Path(args.output).write_text(json.dumps(document, ensure_ascii=False, separators=(",", ":")), encoding="utf-8")
    print(f"wrote {len(entries)} emoji to {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
