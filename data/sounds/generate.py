#!/usr/bin/env python3
import json
import sys
from pathlib import Path

import numpy as np
from scipy import signal
from scipy.io import wavfile

SR = 48000
ROOT = Path(__file__).resolve().parent

LEVELS = {
    "key": -17.0,
    "space": -16.0,
    "backspace": -16.5,
    "enter": -15.5,
    "modifier": -19.0,
    "popup": -21.0,
    "open": -19.0,
    "close": -20.0,
}


def ms(value):
    return int(round(value * SR / 1000.0))


def timeline(n):
    return np.arange(n) / SR


def envelope(n, attack_ms, decay_ms):
    t = timeline(n)
    attack = max(attack_ms, 0.05) / 1000.0
    rise = np.where(t < attack, 0.5 - 0.5 * np.cos(np.pi * np.clip(t / attack, 0, 1)), 1.0)
    fall = np.exp(-np.maximum(t - attack, 0) / (decay_ms / 1000.0))
    return rise * fall


def sos_filter(x, kind, freqs, order=2):
    nyquist = SR / 2.0
    if isinstance(freqs, (tuple, list)):
        wn = [min(f / nyquist, 0.99) for f in freqs]
    else:
        wn = min(freqs / nyquist, 0.99)
    sos = signal.butter(order, wn, btype=kind, output="sos")
    return signal.sosfilt(sos, x)


def noise_burst(rng, n, attack_ms, decay_ms, kind, freqs, order=2):
    raw = rng.standard_normal(n)
    shaped = sos_filter(raw, kind, freqs, order)
    shaped /= max(np.max(np.abs(shaped)), 1e-9)
    return shaped * envelope(n, attack_ms, decay_ms)


def mode(n, freq, decay_ms, attack_ms=0.2, glide=0.0, glide_ms=20.0, phase=0.0):
    t = timeline(n)
    curve = freq * (1.0 + glide * np.exp(-t / (glide_ms / 1000.0)))
    phase_track = 2 * np.pi * np.cumsum(curve) / SR + phase
    return np.sin(phase_track) * envelope(n, attack_ms, decay_ms)


def chirp_mode(n, freq, rate, decay_ms, attack_ms=0.4):
    t = timeline(n)
    curve = freq * np.maximum(1.0 + rate * t, 0.2)
    phase_track = 2 * np.pi * np.cumsum(curve) / SR
    return np.sin(phase_track) * envelope(n, attack_ms, decay_ms)


def place(target, source, at_ms, gain=1.0):
    start = ms(at_ms)
    if start >= len(target):
        return target
    end = min(len(target), start + len(source))
    target[start:end] += source[: end - start] * gain
    return target


def modal(n, freqs, decays, amps, rng, jitter=0.0, attack_ms=0.2):
    out = np.zeros(n)
    for f, d, a in zip(freqs, decays, amps):
        shift = 1.0 + (rng.uniform(-jitter, jitter) if jitter else 0.0)
        out += a * mode(n, f * shift, d, attack_ms=attack_ms, phase=rng.uniform(0, 2 * np.pi))
    return out


def finish(x, kind):
    x = sos_filter(x, "highpass", 30.0, 2)
    peak = np.max(np.abs(x))
    if peak <= 0:
        raise RuntimeError("silent sound")
    x = x / peak
    head = x[: min(len(x), ms(40))]
    rms = np.sqrt(np.mean(head**2))
    rms_db = 20 * np.log10(max(rms, 1e-9))
    gain = 10 ** ((LEVELS[kind] - rms_db) / 20.0)
    gain = min(gain, 10 ** (-1.0 / 20.0))
    x = x * gain
    threshold = 10 ** (-66.0 / 20.0)
    loud = np.nonzero(np.abs(x) > threshold)[0]
    end = min(len(x), loud[-1] + ms(4)) if len(loud) else len(x)
    x = x[:end]
    fade_in = min(ms(0.3), len(x))
    x[:fade_in] *= 0.5 - 0.5 * np.cos(np.pi * np.arange(fade_in) / fade_in)
    fade_out = min(max(ms(6), int(len(x) * 0.3)), len(x))
    x[len(x) - fade_out :] *= (0.5 + 0.5 * np.cos(np.pi * np.arange(fade_out) / fade_out)) ** 2
    x[-1] = 0.0
    return x


def soft_tap(rng, body, bright, decay=22.0, length=110.0, tone=0.0, tone_decay=30.0):
    n = ms(length)
    x = np.zeros(n)
    x += 0.55 * bright * noise_burst(rng, n, 0.25, 3.5, "bandpass", (1800, 6500))
    x += 0.7 * noise_burst(rng, n, 0.6, decay * 0.55, "bandpass", (140, 900))
    x += 0.9 * mode(n, body, decay, attack_ms=0.8, glide=0.12, glide_ms=8.0)
    x += 0.22 * mode(n, body * 2.71, decay * 0.35, attack_ms=0.5)
    x += 0.12 * mode(n, body * 4.6, decay * 0.18, attack_ms=0.4)
    if tone:
        x += 0.35 * mode(n, tone, tone_decay, attack_ms=2.0)
    return x


def soft_note(n, freq, decay_ms, rng):
    x = mode(n, freq, decay_ms, attack_ms=3.0)
    x += 0.18 * mode(n, freq * 3.99, decay_ms * 0.22, attack_ms=1.0)
    x += 0.08 * mode(n, freq * 2.0, decay_ms * 0.5, attack_ms=2.0)
    x += 0.05 * noise_burst(rng, n, 0.3, 2.0, "bandpass", (1500, 5000))
    return x


def pack_soft(rng):
    sounds = {}
    for i, (body, bright) in enumerate([(248, 1.0), (266, 0.85), (284, 1.1), (257, 0.95)], start=1):
        sounds[f"key{i}"] = ("key", soft_tap(rng, body, bright))
    sounds["space"] = ("space", soft_tap(rng, 172, 0.7, decay=34.0, length=118.0))
    sounds["backspace"] = ("backspace", soft_tap(rng, 214, 0.6, decay=26.0))
    sounds["enter"] = ("enter", soft_tap(rng, 196, 0.8, decay=30.0, length=118.0, tone=587.3, tone_decay=36.0))
    sounds["modifier"] = ("modifier", soft_tap(rng, 330, 1.2, decay=16.0, length=90.0))
    n = ms(110)
    popup = chirp_mode(n, 740, 9.0, 32.0, attack_ms=2.5) + 0.15 * chirp_mode(n, 1480, 9.0, 18.0, attack_ms=2.0)
    sounds["popup"] = ("popup", popup)
    n = ms(420)
    opening = np.zeros(n)
    place(opening, soft_note(ms(330), 659.25, 120.0, rng), 0)
    place(opening, soft_note(ms(330), 987.77, 140.0, rng), 72, 0.85)
    sounds["open"] = ("open", opening)
    closing = np.zeros(n)
    place(closing, soft_note(ms(330), 987.77, 100.0, rng), 0, 0.8)
    place(closing, soft_note(ms(330), 659.25, 130.0, rng), 72)
    sounds["close"] = ("close", closing)
    return sounds


def thock(rng, scale, click, length=118.0, stab=False, heavy=1.0, ping=1.0):
    n = ms(length)
    x = np.zeros(n)
    x += 0.8 * click * noise_burst(rng, n, 0.1, 1.6, "bandpass", (2500, 9000))
    x += 0.55 * noise_burst(rng, n, 0.2, 3.2, "bandpass", (900, 4500))
    body_freqs = np.array([210, 395, 610, 1080, 1720]) * scale
    x += heavy * modal(n, body_freqs, [42, 28, 17, 9, 5], [1.0, 0.62, 0.38, 0.2, 0.12], rng, jitter=0.01, attack_ms=0.35)
    x += 0.5 * heavy * noise_burst(rng, n, 0.3, 11.0, "bandpass", (90, 1100))
    spring = modal(n, [4310 * scale ** 0.3, 6870 * scale ** 0.3], [55, 38], [0.035, 0.018], rng, attack_ms=0.4)
    place(x, spring, 1.5, ping)
    if stab:
        for at, gain in [(3.5, 0.28), (8.5, 0.18), (14.0, 0.09)]:
            place(x, noise_burst(rng, ms(20), 0.1, 1.2, "bandpass", (1800, 7000)), at, gain)
            place(x, modal(ms(30), [1320 * scale, 2310 * scale], [6, 4], [0.12, 0.06], rng), at, gain)
    return x


def pack_mechanical(rng):
    sounds = {}
    for i, (scale, click) in enumerate([(1.0, 1.0), (1.045, 0.85), (0.96, 1.1), (1.02, 0.95)], start=1):
        sounds[f"key{i}"] = ("key", thock(rng, scale, click))
    sounds["space"] = ("space", thock(rng, 0.68, 0.7, stab=True, heavy=1.2, ping=0.6))
    sounds["backspace"] = ("backspace", thock(rng, 0.86, 0.9, heavy=1.1))
    sounds["enter"] = ("enter", thock(rng, 0.76, 1.0, stab=True, heavy=1.25))
    sounds["modifier"] = ("modifier", thock(rng, 1.18, 0.8, length=100.0, heavy=0.85, ping=0.7))
    n = ms(100)
    popup = modal(n, [1760, 3050, 4700], [16, 9, 5], [1.0, 0.45, 0.2], rng)
    popup += 0.4 * noise_burst(rng, n, 0.1, 1.2, "bandpass", (3000, 9000))
    popup += 0.35 * modal(n, [520, 910], [14, 8], [1.0, 0.5], rng)
    sounds["popup"] = ("popup", popup)
    n = ms(330)
    opening = np.zeros(n)
    place(opening, thock(rng, 0.9, 0.8, length=160.0, ping=1.8), 0)
    place(opening, thock(rng, 1.12, 0.9, length=170.0, ping=2.4), 70, 0.9)
    sounds["open"] = ("open", opening)
    closing = np.zeros(n)
    place(closing, thock(rng, 1.12, 0.8, length=160.0, ping=1.2), 0, 0.8)
    place(closing, thock(rng, 0.84, 0.9, length=170.0, ping=1.6), 70)
    sounds["close"] = ("close", closing)
    return sounds


def typebar(rng, metal, thud=1.0, length=118.0):
    n = ms(length)
    x = np.zeros(n)
    x += 1.0 * noise_burst(rng, n, 0.05, 1.8, "highpass", 700)
    x += modal(n, metal, [26, 19, 13, 8], [0.34, 0.26, 0.18, 0.1], rng, jitter=0.004, attack_ms=0.1)
    x += thud * modal(n, [108, 186, 290], [30, 20, 12], [0.7, 0.45, 0.25], rng, attack_ms=0.5)
    x += 0.45 * thud * noise_burst(rng, n, 0.2, 16.0, "lowpass", 900)
    for _ in range(3):
        at = rng.uniform(16.0, 34.0)
        place(x, noise_burst(rng, ms(12), 0.05, 0.9, "bandpass", (2000, 8000)), at, rng.uniform(0.05, 0.12))
    return x


def bell(rng, n, freq, decay_ms):
    ratios = [1.0, 2.756, 5.404, 8.933]
    decays = [decay_ms, decay_ms * 0.42, decay_ms * 0.2, decay_ms * 0.1]
    amps = [1.0, 0.5, 0.26, 0.12]
    x = modal(n, [freq * r for r in ratios], decays, amps, rng, attack_ms=0.3)
    x += 0.35 * mode(n, freq * 1.003, decay_ms * 0.9, attack_ms=0.3)
    return x


def ratchet(rng, n, clicks):
    x = np.zeros(n)
    for at, gain in clicks:
        place(x, noise_burst(rng, ms(15), 0.05, 1.0, "bandpass", (2200, 9000)), at, gain)
        place(x, modal(ms(30), [3180, 5230], [7, 4], [0.3, 0.14], rng), at, gain)
    return x


def pack_typewriter(rng):
    sounds = {}
    metals = [
        [1240, 2880, 4620, 6110],
        [1310, 2970, 4480, 6320],
        [1190, 2760, 4750, 5980],
        [1275, 3040, 4560, 6240],
    ]
    for i, metal in enumerate(metals, start=1):
        sounds[f"key{i}"] = ("key", typebar(rng, metal))
    n = ms(115)
    space = ratchet(rng, n, [(0.0, 0.9), (13.0, 0.55)])
    space += modal(n, [92, 165], [26, 16], [0.8, 0.4], rng, attack_ms=0.6)
    space += 0.4 * noise_burst(rng, n, 0.3, 14.0, "lowpass", 700)
    sounds["space"] = ("space", space)
    n = ms(115)
    back = ratchet(rng, n, [(0.0, 0.7), (9.0, 0.9), (21.0, 0.4)])
    back += modal(n, [128, 230], [20, 12], [0.6, 0.3], rng, attack_ms=0.6)
    sounds["backspace"] = ("backspace", back)
    n = ms(560)
    enter = np.zeros(n)
    place(enter, typebar(rng, [1160, 2710, 4380, 5900], thud=1.3, length=118.0), 0)
    place(enter, bell(rng, ms(530), 2093.0, 230.0), 24, 0.55)
    sounds["enter"] = ("enter", enter)
    n = ms(100)
    shift = modal(n, [132, 240, 910], [26, 14, 9], [0.8, 0.4, 0.2], rng, attack_ms=1.2)
    shift += 0.35 * noise_burst(rng, n, 0.6, 6.0, "bandpass", (400, 2500))
    sounds["modifier"] = ("modifier", shift)
    sounds["popup"] = ("popup", bell(rng, ms(115), 3135.96, 34.0))
    n = ms(520)
    opening = np.zeros(n)
    place(opening, bell(rng, ms(500), 2637.02, 170.0), 0)
    sounds["open"] = ("open", opening)
    n = ms(380)
    closing = ratchet(rng, n, [(0.0, 0.5), (22.0, 0.6), (40.0, 0.7), (55.0, 0.8), (68.0, 0.85), (79.0, 0.9), (88.0, 0.95)])
    closing += 0.2 * noise_burst(rng, n, 1.0, 60.0, "bandpass", (500, 3000))
    place(closing, modal(ms(160), [96, 170, 310], [40, 26, 14], [0.9, 0.5, 0.25], rng, attack_ms=0.8), 100)
    sounds["close"] = ("close", closing)
    return sounds


def pop(rng, f0, rise, decay, length=110.0, down=False):
    n = ms(length)
    rate = -rise if down else rise
    x = chirp_mode(n, f0, rate, decay, attack_ms=0.6)
    x += 0.12 * chirp_mode(n, f0 * 2.0, rate, decay * 0.5, attack_ms=0.6)
    x += 0.18 * noise_burst(rng, n, 0.05, 0.8, "bandpass", (1500, 6000))
    return x


def pack_bubble(rng):
    sounds = {}
    for i, (f0, rise) in enumerate([(640, 14.0), (715, 16.0), (790, 13.0), (680, 17.0)], start=1):
        sounds[f"key{i}"] = ("key", pop(rng, f0, rise, 24.0))
    sounds["space"] = ("space", pop(rng, 360, 10.0, 30.0, length=118.0))
    sounds["backspace"] = ("backspace", pop(rng, 560, 6.0, 30.0, down=True))
    n = ms(118)
    enter = np.zeros(n)
    place(enter, pop(rng, 460, 12.0, 22.0, length=60.0), 0)
    place(enter, pop(rng, 860, 15.0, 24.0, length=80.0), 38, 0.9)
    sounds["enter"] = ("enter", enter)
    sounds["modifier"] = ("modifier", pop(rng, 1020, 18.0, 16.0, length=80.0))
    sounds["popup"] = ("popup", pop(rng, 920, 26.0, 20.0, length=90.0))
    n = ms(340)
    opening = np.zeros(n)
    for at, f0 in [(0, 420), (48, 560), (96, 740), (144, 980)]:
        place(opening, pop(rng, f0, 16.0, 26.0, length=120.0), at, 0.85 + at / 700.0)
    sounds["open"] = ("open", opening)
    closing = np.zeros(n)
    for at, f0 in [(0, 900), (52, 700), (104, 540), (156, 420)]:
        place(closing, pop(rng, f0, 8.0, 26.0, length=120.0, down=True), at, 1.0 - at / 500.0)
    sounds["close"] = ("close", closing)
    return sounds


GLASS_RATIOS = [1.0, 2.32, 4.25, 6.63]


def glass_tick(rng, f0, decay=26.0, length=110.0, knock=1.0):
    n = ms(length)
    x = modal(n, [f0 * r for r in GLASS_RATIOS], [decay, decay * 0.45, decay * 0.22, decay * 0.13], [1.0, 0.42, 0.22, 0.1], rng, attack_ms=0.1)
    x += 0.6 * mode(n, f0 * 1.004, decay * 0.85, attack_ms=0.1)
    x += 0.5 * noise_burst(rng, n, 0.05, 0.7, "highpass", 5000)
    x += 0.25 * knock * modal(n, [880, 1530], [5, 3], [1.0, 0.5], rng, attack_ms=0.2)
    return x


def glass_note(rng, f0, decay, length):
    n = ms(length)
    x = modal(n, [f0 * r for r in GLASS_RATIOS], [decay, decay * 0.4, decay * 0.18, decay * 0.1], [1.0, 0.3, 0.14, 0.06], rng, attack_ms=0.6)
    x += 0.7 * mode(n, f0 * 1.0035, decay * 0.9, attack_ms=0.6)
    x += 0.2 * noise_burst(rng, n, 0.05, 0.6, "highpass", 6000)
    return x


def pack_glass(rng):
    sounds = {}
    for i, f0 in enumerate([2637.0, 2794.0, 2960.0, 3136.0], start=1):
        sounds[f"key{i}"] = ("key", glass_tick(rng, f0))
    sounds["space"] = ("space", glass_tick(rng, 1760.0, decay=36.0, length=118.0, knock=1.6))
    n = ms(110)
    back = np.zeros(n)
    place(back, glass_tick(rng, 2349.0, decay=24.0, length=60.0), 0)
    place(back, glass_tick(rng, 2093.0, decay=28.0, length=70.0, knock=0.5), 26, 0.55)
    sounds["backspace"] = ("backspace", back)
    enter = glass_tick(rng, 1975.5, decay=40.0, length=118.0, knock=1.3)
    enter += 0.6 * glass_tick(rng, 2489.0, decay=34.0, length=118.0, knock=0.0)
    sounds["enter"] = ("enter", enter)
    sounds["modifier"] = ("modifier", glass_tick(rng, 3520.0, decay=22.0, length=80.0, knock=0.6))
    sounds["popup"] = ("popup", glass_note(rng, 4186.0, 22.0, 100.0))
    n = ms(560)
    opening = np.zeros(n)
    for at, f0 in [(0, 1318.5), (62, 1661.2), (124, 1975.5)]:
        place(opening, glass_note(rng, f0, 150.0, 430.0), at, 0.8 + at / 600.0)
    sounds["open"] = ("open", opening)
    n = ms(480)
    closing = np.zeros(n)
    for at, f0 in [(0, 1975.5), (62, 1661.2), (124, 1318.5)]:
        place(closing, glass_note(rng, f0, 120.0, 350.0), at, 1.0 - at / 500.0)
    sounds["close"] = ("close", closing)
    return sounds


PACKS = [
    ("soft", "Soft", "Gentle, muted taps", pack_soft),
    ("mechanical", "Mechanical", "Thocky switches with a bottom-out and a faint spring ping", pack_mechanical),
    ("typewriter", "Typewriter", "Type bars striking the platen, with a carriage bell on enter", pack_typewriter),
    ("bubble", "Bubble", "Playful water-drop pops", pack_bubble),
    ("glass", "Glass", "Crisp, bright glass ticks", pack_glass),
]


def write_pack(order, pack_id, name, description, builder):
    rng = np.random.default_rng(sum(map(ord, pack_id)) * 7919)
    directory = ROOT / pack_id
    directory.mkdir(parents=True, exist_ok=True)
    for old in directory.glob("*.wav"):
        old.unlink()
    sounds = builder(rng)
    for file_name, (kind, samples) in sounds.items():
        data = finish(np.asarray(samples, dtype=np.float64), kind)
        pcm = np.clip(np.round(data * 32767.0), -32768, 32767).astype(np.int16)
        wavfile.write(directory / f"{file_name}.wav", SR, pcm)
    meta = {"id": pack_id, "name": name, "description": description, "order": order, "license": "CC0-1.0"}
    (directory / "pack.json").write_text(json.dumps(meta, indent=4) + "\n")
    return sorted(sounds)


def main():
    for order, (pack_id, name, description, builder) in enumerate(PACKS):
        files = write_pack(order, pack_id, name, description, builder)
        print(f"{pack_id}: {', '.join(files)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
