# Sound sources

All KBoard key sounds are original and synthesised procedurally by
`generate.py` in this directory. No recordings, sample libraries or
third-party audio were used. The sounds are released under CC0 1.0 (see
`LICENSE`).

The script shapes filtered noise bursts, damped modal resonators and pitch
glides with smooth envelopes, then high-pass filters, loudness-normalises,
fades and writes each sound as a 48 kHz mono 16-bit WAV.

| Pack | Character |
| --- | --- |
| soft | Gentle muted taps (default) |
| mechanical | Thocky switch with a bottom-out and a faint spring ping |
| typewriter | Type bar striking the platen, carriage bell on enter |
| bubble | Water-drop pops |
| glass | Crisp glass ticks |

Every pack contains `key1.wav` to `key4.wav`, `space.wav`, `backspace.wav`,
`enter.wav`, `modifier.wav`, `popup.wav`, `open.wav`, `close.wav` and
`pack.json`.

Regenerate with:

```sh
python3 data/sounds/generate.py
```

Requirements: Python 3, NumPy, SciPy. Output is deterministic.
