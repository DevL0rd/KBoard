# Dictionary data sources

## Files

| File | Contents | Licence |
| --- | --- | --- |
| `en.freq` | Top ~94k English words with corpus counts, spelled/cased as accepted by Hunspell `en_US` | CC-BY-SA-4.0 |
| `en.ngram` | English bigram and trigram continuation counts (lowercase keys, `<s>` = sentence start) | CC-BY 2.0 FR |

## Sources

### FrequencyWords (OpenSubtitles 2018)

- Repository: https://github.com/hermitdave/FrequencyWords (file `content/2018/en/en_full.txt`)
- Author: Hermit Dave, derived from the OpenSubtitles 2018 corpus (http://opus.nlpl.eu/OpenSubtitles2018.php)
- Licence: content CC-BY-SA-4.0 (https://creativecommons.org/licenses/by-sa/4.0/), generator code MIT
- Use: unigram counts for `en.freq`. `en.freq` is an adapted database and is redistributed under CC-BY-SA-4.0.

### Tatoeba

- Export: https://downloads.tatoeba.org/exports/per_language/eng/eng_sentences.tsv.bz2
- Author: Tatoeba contributors (https://tatoeba.org)
- Licence: CC-BY 2.0 FR (https://creativecommons.org/licenses/by/2.0/fr/)
- Use: bigram/trigram counts in `en.ngram`, preferred casing of words (for example `I`, `TV`, `English`), and counts for contractions (`don't`, `I'm`) which the OpenSubtitles tokenizer splits apart. Sentences using the corpus' stock character names (Tom, Mary, Sami, Layla, ...) are excluded from the n-grams.

### Authored word lists

- `CHAT_WORDS` and `TECH_WORDS` in `tools/build_en.py` (informal chat words such as `lol`, `idk`, `y'all` and tech terms such as `KDE`, `GitHub`, `wifi`) were written for KBoard and are given a minimum count so they are not autocorrected away. Same licence as the project.

### Hunspell en_US

- Package: Arch `hunspell-en_us` (SCOWL-derived, `/usr/share/hunspell/en_US.dic`)
- Use at build time only, to keep valid words and pick their spelling. Not redistributed.

## Rebuilding

```
python3 tools/build_en.py en_full.txt eng_sentences.tsv.bz2 .
```

## Formats

- `<lang>.freq`: first line `#kboard-freq 1 <lang>`, then `word<TAB>count`, sorted by count descending.
- `<lang>.ngram`: first line `#kboard-ngram 1 <lang>`, then `context<TAB>next count<TAB>next count...` where context is one word (bigram) or two words separated by a space (trigram).

Adding a language: drop `<lang>.freq` (and optionally `<lang>.ngram`) here and install the matching Hunspell dictionary.
