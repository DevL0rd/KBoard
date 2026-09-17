#!/usr/bin/env python3
import bz2
import collections
import re
import subprocess
import sys
from pathlib import Path

LEXICON_SIZE = 100000
FRAGMENTS = {"don", "didn", "doesn", "isn", "wasn", "aren", "weren", "couldn", "wouldn", "shouldn", "haven", "hasn", "hadn", "won", "ain", "mustn", "needn", "t", "s", "m", "ll", "re", "ve", "d"}
SKIP_NGRAM_TOKENS = {"tom", "mary", "tom's", "mary's", "sami", "layla", "sami's", "layla's", "fadil", "dan", "linda"}
CHAT_WORDS = "haha hahaha hehe lol lmao rofl omg btw tbh idk imo imho thx pls plz np cya lmk ttyl brb gtg afaik yep yup nope ok okay gonna wanna gotta kinda sorta dunno lemme gimme y'all yeah nah hmm ugh wow yay oops"
TECH_WORDS = "KDE Linux GitHub GitLab YouTube Google Wayland Ubuntu Fedora Debian Firefox Chromium Plasma API UI UX FAQ DIY app apps email emails emoji emojis online offline wifi website websites blog podcast selfie hashtag smartphone laptop desktop login logout username screenshot"
CHAT_FLOOR = 20000
TECH_FLOOR = 5000
TOKEN = re.compile(r"[A-Za-z]+(?:'[A-Za-z]+)*")
SENTENCE_SPLIT = re.compile(r"[.!?;:]+|\s[-–—]\s|\"")


def hunspell_accepts(words, dictionary):
    proc = subprocess.run(["hunspell", "-d", dictionary, "-G"], input="\n".join(words), capture_output=True, text=True, check=True)
    return set(proc.stdout.split("\n"))


def add_authored(entries, os_counts):
    by_key = {form.lower(): (form, count) for form, count in entries}
    for words, floor, override in ((CHAT_WORDS, CHAT_FLOOR, True), (TECH_WORDS, TECH_FLOOR, False)):
        for form in words.split():
            key = form.lower()
            existing = by_key.get(key)
            count = max(floor, os_counts.get(key, 0), existing[1] if existing else 0)
            by_key[key] = (existing[0] if existing and not override else form, count)
    return list(by_key.values())


def main():
    frequency_path = Path(sys.argv[1])
    tatoeba_path = Path(sys.argv[2])
    out_dir = Path(sys.argv[3])
    dictionary = "/usr/share/hunspell/en_US"

    stems = collections.defaultdict(set)
    with open(dictionary + ".dic", encoding="utf-8") as dic:
        next(dic)
        for line in dic:
            stem = line.strip().split("/")[0]
            if stem and TOKEN.fullmatch(stem):
                stems[stem.lower()].add(stem)

    casing = collections.defaultdict(collections.Counter)
    tat_unigrams = collections.Counter()
    bigrams = collections.defaultdict(collections.Counter)
    trigrams = collections.defaultdict(collections.Counter)
    with bz2.open(tatoeba_path, "rt", encoding="utf-8") as source:
        for row in source:
            parts = row.rstrip("\n").split("\t")
            if len(parts) < 3:
                continue
            text = parts[2].replace("’", "'").replace("‘", "'")
            for sentence in SENTENCE_SPLIT.split(text):
                tokens = TOKEN.findall(sentence)
                if not tokens:
                    continue
                for index, token in enumerate(tokens):
                    lower = token.lower()
                    tat_unigrams[lower] += 1
                    if index > 0 or lower == "i" or lower.startswith("i'"):
                        casing[lower][token] += 1
                keys = ["<s>"] + [t.lower() for t in tokens]
                for a, b in zip(keys, keys[1:]):
                    if a in SKIP_NGRAM_TOKENS or b in SKIP_NGRAM_TOKENS:
                        continue
                    bigrams[a][b] += 1
                for a, b, c in zip(keys, keys[1:], keys[2:]):
                    if a in SKIP_NGRAM_TOKENS or b in SKIP_NGRAM_TOKENS or c in SKIP_NGRAM_TOKENS:
                        continue
                    trigrams[a + " " + b][c] += 1

    os_counts = {}
    with open(frequency_path, encoding="utf-8") as source:
        for line in source:
            word, _, count = line.rstrip("\n").rpartition(" ")
            if not word or not re.fullmatch(r"[a-z]+", word):
                continue
            os_counts[word] = int(count)
            if len(os_counts) >= 400000:
                break

    ratios = sorted(os_counts[w] / tat_unigrams[w] for w in list(os_counts)[:600] if w not in FRAGMENTS and tat_unigrams[w] >= 50)
    scale = ratios[len(ratios) // 2]

    counts = {}
    for word, count in os_counts.items():
        counts[word] = int(tat_unigrams[word] * scale) if word in FRAGMENTS else count
    for word, count in tat_unigrams.items():
        if "'" in word and count >= 3:
            counts[word] = max(counts.get(word, 0), int(count * scale))

    lowers = [w for w in counts]
    accepted_lower = hunspell_accepts(lowers, dictionary)
    capitalised = {w: w[0].upper() + w[1:] for w in lowers}
    accepted_cap = hunspell_accepts(list(capitalised.values()), dictionary)

    entries = []
    for word in lowers:
        if len(word) == 1 and word not in ("a", "i"):
            continue
        forms = set(stems.get(word, ()))
        if word in accepted_lower:
            forms.add(word)
        if capitalised[word] in accepted_cap and word not in accepted_lower:
            forms.add(capitalised[word])
        if not forms:
            continue
        seen = casing.get(word)
        if seen:
            ranked = [(f, c) for f, c in seen.most_common() if f in forms]
            total = sum(c for _, c in ranked)
            form = None
            if ranked:
                form = ranked[0][0]
                if word in forms and form != word and ranked[0][1] < 0.9 * total:
                    form = word
        else:
            form = None
        if form is None:
            form = word if word in forms else sorted(forms, key=lambda f: (f != f.capitalize(), f))[0]
        entries.append((form, counts[word]))

    entries = add_authored(entries, os_counts)
    entries.sort(key=lambda e: (-e[1], e[0]))
    entries = entries[:LEXICON_SIZE]
    keys = {form.lower() for form, _ in entries}

    out_dir.mkdir(parents=True, exist_ok=True)
    with open(out_dir / "en.freq", "w", encoding="utf-8") as out:
        out.write("#kboard-freq 1 en\n")
        for form, count in entries:
            out.write(f"{form}\t{count}\n")

    with open(out_dir / "en.ngram", "w", encoding="utf-8") as out:
        out.write("#kboard-ngram 1 en\n")
        for first in sorted(bigrams):
            if first != "<s>" and first not in keys:
                continue
            followers = [(w, c) for w, c in bigrams[first].most_common() if w in keys and c >= 3][:12]
            if followers:
                out.write(first + "\t" + "\t".join(f"{w} {c}" for w, c in followers) + "\n")
        for pair in sorted(trigrams):
            a, b = pair.split(" ")
            if (a != "<s>" and a not in keys) or b not in keys:
                continue
            followers = [(w, c) for w, c in trigrams[pair].most_common() if w in keys and c >= 4][:6]
            if followers:
                out.write(pair + "\t" + "\t".join(f"{w} {c}" for w, c in followers) + "\n")

    print(f"scale={scale:.1f} lexicon={len(entries)}")


if __name__ == "__main__":
    main()
