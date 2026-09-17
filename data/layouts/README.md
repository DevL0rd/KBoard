# Keyboard layouts

KBoard reads every `*.json` file in this directory at startup and again whenever a file changes. Each file has a `kind`:

| kind | What it holds |
| --- | --- |
| `letters` | A language layout: the letters page plus metadata (`us.json`, `de.json`, ...) |
| `page` | A shared page reachable from any layout (`symbols`, `symbols-more`, `numpad`, `phone`) |
| `rows` | Named rows that pages include (`common.json`: number row, desktop row, F1–F12 row, bottom rows) |

## Letters layout

```json
{
    "id": "de",
    "kind": "letters",
    "name": "Deutsch",
    "shortName": "DE",
    "language": "de_DE",
    "locale": "de_DE",
    "currency": "€",
    "columns": 11,
    "letters": [ { "keys": [ ... ] }, { "include": "bottom" } ],
    "rows": { "numbers": { "keys": [ ... ] } },
    "pages": { "symbols": { "columns": 10, "rows": [ ... ] } }
}
```

- `columns` is the number of 1-wide keys that fill the width. Rows with fewer units are centred, rows with more are scaled to fit.
- `rows` (optional) overrides a named row from `common.json` for this layout only.
- `pages` (optional) overrides a shared page for this layout only.
- `{currency}` in any label or output is replaced with the layout's `currency`.

## Rows

A row is either `{ "include": "name" }` or an object with:

| field | default | meaning |
| --- | --- | --- |
| `keys` | | the keys, left to right |
| `height` | `1` | relative height (the number and desktop rows use `0.72`–`0.8`) |
| `split` | automatic | index of the first key of the right half in split mode. Automatic splitting picks the key boundary nearest the middle and cuts the space bar in two |
| `glide` | `true` | whether single-letter keys in this row take part in glide typing |
| `special` | `false` | draw every key in the row with the darker special-key colour |

`include` looks for `name-variant` first when the field asks for a variant (`bottom-email`, `bottom-url`), then `name`. The keyboard prepends `desktop` (or `function` while Fn is on) when the desktop row is shown and `numbers` when the number row is on.

## Keys

| field | default | meaning |
| --- | --- | --- |
| `type` | `char` | `char`, `space`, `shift`, `backspace`, `enter`, `symbols`, `letters`, `emoji`, `globe`, `modifier`, `key`, `arrow`, `fn`, `hide` |
| `label` | `""` (`" "` for space) | text on the key |
| `shiftLabel` | upper-case `label` for one character | label while shift is on |
| `output` | `label` | text committed for `char` keys |
| `shiftOutput` | `shiftLabel` or upper-case `output` | text committed while shift is on |
| `sublabel` | `""` | small second line (phone pad letters) |
| `width` | `1` | width in key units |
| `grow` | `false` | take the remaining units of the row (space bar) |
| `longPress` | `[]` | popup items, strings or `{ "label", "output", "action" }`. The first item is preselected |
| `hint` | `""` | small corner hint, usually the first long-press item |
| `icon` | the type | icon for `arrow` (`left`, `right`, `up`, `down`), `tab`, `super` |
| `key` | `""` | key name sent by `key` and `arrow` keys (`esc`, `tab`, `home`, `pageup`, `f5`, ...) |
| `modifier` | `""` | `ctrl`, `alt` or `super` for `modifier` keys. Tap latches for one key, double tap locks |
| `page` | `""` | page opened by a `symbols` key |
| `action` | `""` | action for custom keys and popup items: `panel:emoji`, `newline`, `enter`, `layout:<id>` |
| `repeat` | `true` for backspace and arrows | repeat while held |
| `special` | `true` for everything except `char` and `space` | use the special-key colour |
| `when` | always | `multipleLayouts`, `singleLayout` or `variant:<name>` |
