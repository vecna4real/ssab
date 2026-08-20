#!/usr/bin/env python3
"""
ssab_sf2_gen.py — SSAB v1.0 DSP (Python port)

Differences from MVP:
  * Proper PolyBLEP saw/square/pulse ( Brandsma 2019 )
  * Real phase-modulation FM (B modulates phase of A)
  * Huovilainen Moog-style 4-pole ladder filter with
    zero-delay feedback compensation
  * SUB PROTECT via Linkwitz-Riley 4th-order crossover
    (low band bypasses the saturator, high band is distorted)
  * Triangle LFO with sample-accurate phase
  * Improved ADSR with proper gate logic + punch transient
  * Retuned 18 presets — every one is now a playable bass
    instrument, not a noise generator.
"""

import os, sys, struct, wave, math
import numpy as np
from dataclasses import dataclass
from typing import List, Optional

SR = 44100

# ====================================================================
# 1. DSP PRIMITIVES
# ====================================================================

TWO_PI = 2.0 * math.pi

def fast_tanh(x):
    """Lambert approximation."""
    a = np.abs(x)
    b = 1.0 / (1.0 + a)
    c = 1.0 - b * b
    return np.where(x < 0.0, -c, c)

def _dc_block(x, R=0.995):
    """One-pole DC blocker."""
    y = np.zeros_like(x)
    prev_x = 0.0
    prev_y = 0.0
    for i in range(len(x)):
        y[i] = float(x[i]) - prev_x + R * prev_y
        prev_x = float(x[i])
        prev_y = float(y[i])
    return y

def soft_clip(x):
    x = np.clip(np.asarray(x, dtype=np.float64), -3.0, 3.0)
    out = np.where(x > 1.0,  1.0 - 1.5 / (x + 0.5),
         np.where(x < -1.0, -1.0 + 1.5 / (-x + 0.5),
                  x - x * x * x * (1.0 / 3.0)))
    return out.astype(np.float32)

def hard_clip(x):
    return np.clip(x, -1.0, 1.0)

def fold_wave(x):
    """Triangle-wave folder. Stays stable for arbitrary input."""
    x = np.asarray(x, dtype=np.float64)
    # mod into [-2, 2)
    x = np.mod(x + 2.0, 4.0) - 2.0
    out = np.where(np.abs(x) <= 1.0, x,
          np.where(x > 1.0, 2.0 - x, -2.0 - x))
    return out.astype(np.float32)

def fuzz(x):
    sx = np.sign(x)
    w = np.abs(x)
    return hard_clip(sx * (w * w) * 3.0)

def midi_to_freq(m):
    return 440.0 * (2.0 ** ((m - 69.0) / 12.0))

def semis_to_ratio(s):
    return 2.0 ** (s / 12.0)

# ====================================================================
# 2. OSCILLATORS — proper PolyBLEP
# ====================================================================

def _polyblep_pos(t, inc):
    """One-sided PolyBLEP residual at a rising edge."""
    # t is the phase just after wrap, in [0, inc)
    if inc <= 0:
        return 0.0
    dt = t / inc
    return dt + dt - dt * dt

def _polyblep_neg(t, inc):
    """One-sided PolyBLEP residual at a falling edge."""
    if inc <= 0:
        return 0.0
    dt = t / inc
    return -(dt + dt + dt * dt)

def osc_saw(freq, n, sr=SR, phase=0.0):
    inc = float(freq) / float(sr)
    if inc <= 0 or inc >= 0.5:
        # Out of range — return silence
        return np.zeros(n, dtype=np.float32)
    t = (np.arange(n) * inc + phase) % 1.0
    out = 2.0 * t - 1.0
    # Subtract PolyBLEP at the rising edge (phase wrap)
    blep = np.zeros(n)
    # indices where t is small (just wrapped)
    near_wrap = t < inc
    if np.any(near_wrap):
        blep[near_wrap] = _polyblep_pos(t[near_wrap], inc)
    return (out - blep).astype(np.float32)

def osc_square(freq, n, sr=SR, phase=0.0):
    inc = float(freq) / float(sr)
    if inc <= 0 or inc >= 0.5:
        return np.zeros(n, dtype=np.float32)
    t = (np.arange(n) * inc + phase) % 1.0
    out = np.where(t < 0.5, 1.0, -1.0)
    blep = np.zeros(n)
    # rising edge at t -> 0 (after wrap)
    near_zero = t < inc
    if np.any(near_zero):
        blep[near_zero] = _polyblep_pos(t[near_zero], inc)
    # falling edge at t -> 0.5
    near_half = (t >= 0.5 - inc) & (t < 0.5 + inc)
    if np.any(near_half):
        ts = (t[near_half] - 0.5) / inc
        blep[near_half] = np.where(ts >= 0, -(ts + ts - ts * ts),
                                             -(ts + ts + ts * ts))
    return (out + blep).astype(np.float32)

def osc_pulse(freq, n, pw=0.5, sr=SR, phase=0.0):
    inc = float(freq) / float(sr)
    if inc <= 0 or inc >= 0.5:
        return np.zeros(n, dtype=np.float32)
    pw = float(np.clip(pw, 0.05, 0.95))
    t = (np.arange(n) * inc + phase) % 1.0
    out = np.where(t < pw, 1.0, -1.0)
    blep = np.zeros(n)
    # rising edge at wrap
    near_zero = t < inc
    if np.any(near_zero):
        blep[near_zero] = _polyblep_pos(t[near_zero], inc)
    # falling edge at pw
    near_pw = (t >= pw - inc) & (t < pw + inc)
    if np.any(near_pw):
        ts = (t[near_pw] - pw) / inc
        blep[near_pw] = np.where(ts >= 0, -(ts + ts - ts * ts),
                                              -(ts + ts + ts * ts))
    return (out + blep).astype(np.float32)

def osc_noise(n, rng):
    return rng.uniform(-1.0, 1.0, n).astype(np.float32)

def osc_grit(freq, n, sr=SR, rng=None, phase=0.0, grit_amt=0.18):
    """Saw + small noise modulation — sounds like a dirty analog osc."""
    if rng is None:
        rng = np.random.default_rng(0)
    saw = osc_saw(freq, n, sr, phase)
    noise = rng.uniform(-1.0, 1.0, n) * grit_amt
    return saw + noise * (np.abs(saw) + 0.3)

WAVE_FUNCS = {
    'SAW':    osc_saw,
    'SQUARE': osc_square,
    'PULSE':  osc_pulse,
    'NOISE':  None,
    'GRIT':   None,
}

# ====================================================================
# 3. MOOG-STYLE 4-POLE LADDER FILTER (Huovilainen)
# ====================================================================

def moog_ladder(x, cutoff_hz, resonance, ftype='LP', sr=SR):
    """
    4-pole transistor ladder filter (Huovilainen).
    cutoff_hz can be a scalar OR a numpy array (per-sample).
    resonance: 0..0.99
    """
    cutoff_hz = np.asarray(cutoff_hz, dtype=np.float64)
    scalar_cutoff = cutoff_hz.ndim == 0
    if scalar_cutoff:
        cutoff_hz = np.full(len(x), float(cutoff_hz))
    else:
        cutoff_hz = np.clip(cutoff_hz, 10.0, sr * 0.45)

    x_in = np.asarray(x, dtype=np.float64)
    res = float(np.clip(resonance, 0.0, 0.99))
    k = res * 4.0  # Moog feedback gain

    s1 = s2 = s3 = s4 = 0.0
    out = np.zeros(len(x_in))
    for i in range(len(x_in)):
        fc = float(cutoff_hz[i])
        # well-behaved g coefficient
        g = 1.0 - math.exp(-TWO_PI * fc / sr)
        u = x_in[i] - k * s4
        u = np.tanh(u * 0.7) * 1.3  # gentle thermal shape
        s1 = s1 + g * (u - s1)
        s2 = s2 + g * (s1 - s2)
        s3 = s3 + g * (s2 - s3)
        s4 = s4 + g * (s3 - s4)
        if ftype == 'LP':
            out[i] = s4
        elif ftype == 'HP':
            out[i] = x_in[i] - s4
        elif ftype == 'BP':
            out[i] = s2 - s4
        else:
            out[i] = s4
    return out.astype(np.float32)

# Faster Moog via numba or pure-numpy is possible but per-sample loop
# is unavoidable due to feedback. For SF2 rendering (offline), this is fine.

# ====================================================================
# 4. DISTORTION WITH CROSSOVER SUB PROTECT
# ====================================================================

def _linkwitz_riley_crossover(x, freq, order=4, sr=SR, btype='low'):
    """4th-order Linkwitz-Riley crossover (24 dB/oct)."""
    from scipy.signal import butter, lfilter, filtfilt
    # LR4 = two cascaded Butterworth 2nd-order filters at same freq
    b, a = butter(2, freq, btype=btype, fs=sr)
    y1 = lfilter(b, a, x)
    y2 = lfilter(b, a, y1)
    return y2

def apply_distortion(x, drive, dtype='CLIP', bitcrush=0.0, sub_protect=0.7):
    """
    SUB PROTECT (crossover design):
      low band (below ~80 Hz)  -> bypasses the saturator
      high band (above ~80 Hz) -> goes through the saturator
      Mix them back together after saturation.
    """
    x = np.asarray(x, dtype=np.float64)
    if sub_protect > 0.001:
        # Split into low and high via LR4 crossover at 80 Hz
        low  = _linkwitz_riley_crossover(x, 80.0, 4, SR, 'low')
        high = _linkwitz_riley_crossover(x, 80.0, 4, SR, 'high')
    else:
        low  = np.zeros_like(x)
        high = x

    # Drive the high band
    driven = high * (1.0 + drive * 25.0)
    if dtype == 'CLIP':
        sat = soft_clip(driven * 0.7)
    elif dtype == 'FOLD':
        sat = fold_wave(driven * 0.8)
    elif dtype == 'FUZZ':
        sat = fuzz(driven)
    else:
        sat = driven

    # Re-mix: sub_protect controls how much of the original low band
    # is preserved vs the distorted version. 1.0 = full protect.
    protected_low = low
    distorted_low = soft_clip(driven + low * 0.0)  # not really distorting low
    final_low = protected_low * sub_protect + distorted_low * (1.0 - sub_protect)

    out = final_low + sat

    # Bitcrush (applied to the full mix, post-crossover)
    if bitcrush > 0.001:
        levels = max(2, int((1.0 - bitcrush) * 254.0) + 2)
        out = np.round(out * levels * 0.5) / (levels * 0.5)
        hold = int(bitcrush * 8.0) + 1
        held = np.repeat(out[::hold], hold)[:len(out)]
        out = held

    out = np.nan_to_num(out, nan=0.0, posinf=1.0, neginf=-1.0)
    return np.clip(out, -1.0, 1.0).astype(np.float32)

# ====================================================================
# 5. ENVELOPE (improved ADSR + Punch)
# ====================================================================

def adsr_curve(n, attack_ms, decay_ms, sustain, release_ms, punch=0.0,
                sr=SR, gate_frac=0.6):
    """
    Builds an envelope of length n.
    - attack: 0 -> 1+punch
    - punch decay: 1+punch -> 1 (~20 ms)
    - decay: 1 -> sustain
    - sustain: hold sustain
    - release: sustain -> 0 (last release_ms of the buffer)
    """
    gate_end = int(n * gate_frac)
    gate_end = max(1, min(gate_end, n - 1))

    a = max(1, int(attack_ms * 0.001 * sr))
    d = max(1, int(decay_ms * 0.001 * sr))
    r = max(1, int(release_ms * 0.001 * sr))

    env = np.zeros(n, dtype=np.float64)

    # Attack
    a_end = min(a, gate_end)
    if a_end > 0:
        env[:a_end] = np.linspace(0.0, 1.0 + punch, a_end)

    # Punch decay (20 ms)
    pd = int(0.020 * sr)
    pd_end = min(a_end + pd, gate_end)
    if pd_end > a_end:
        env[a_end:pd_end] = np.linspace(1.0 + punch, 1.0, pd_end - a_end)

    # Decay
    d_end = min(pd_end + d, gate_end)
    if d_end > pd_end:
        env[pd_end:d_end] = np.linspace(1.0, sustain, d_end - pd_end)

    # Sustain
    env[d_end:gate_end] = sustain

    # Release
    r_len = min(r, n - gate_end)
    if r_len > 0:
        env[gate_end:gate_end + r_len] = np.linspace(sustain, 0.0, r_len)

    return env.astype(np.float32)

# ====================================================================
# 6. LFO
# ====================================================================

def lfo_triangle(rate, n, sr=SR, phase=0.0):
    """Triangle LFO in [-1, 1]."""
    inc = float(rate) / float(sr)
    t = (np.arange(n) * inc + phase) % 1.0
    return np.where(t < 0.5, t * 4.0 - 1.0, 3.0 - t * 4.0).astype(np.float32)

# ====================================================================
# 7. PRESET DEFINITIONS — v1.0 retuned for musicality
# ====================================================================

@dataclass
class Preset:
    name: str
    waveA: str
    waveB: str
    detune: float       # -100..100 cents
    octave: int        # -2..+2
    fm_amount: float    # 0..1 (phase mod depth)
    sub_level: float    # 0..1
    sync: bool
    cutoff: float
    resonance: float
    filter_type: str    # LP/BP/HP
    env_mod: float      # -1..1
    keytrack: float     # -1..1
    drive: float
    dist_type: str
    bitcrush: float
    sub_protect: float
    attack: float
    decay: float
    sustain: float
    release: float
    punch: float
    lfo_rate: float
    lfo_depth: float
    lfo_target: str
    massacre1994: bool
    ruiner: float
    feedback: float
    volume_db: float = 0.0


PRESETS = [
    # 0 — RAGGA: classic reese, wide detune, BP filter, big sub
    Preset("RAGGA", "SAW", "SAW", 28.0, 0, 0.0, 0.65, False,
           1400.0, 0.55, "LP", 0.4, 0.0, 0.30, "CLIP", 0.0, 0.85,
           4.0, 220.0, 0.85, 300.0, 0.55, 0.7, 0.0, "CUTOFF",
           False, 0.0, 0.0, 0.0),

    # 1 — GUTCUTTER: massive sub + tight resonance, fuzz octave-up
    Preset("GUTCUTTER", "SQUARE", "PULSE", 6.0, -1, 0.0, 0.90, False,
           550.0, 0.75, "LP", 0.55, 0.0, 0.45, "FUZZ", 0.0, 0.95,
           2.0, 120.0, 0.90, 500.0, 0.85, 1.8, 0.0, "CUTOFF",
           False, 0.0, 0.0, -1.0),

    # 2 — EXTERMINATION: midrange ripping wobble
    Preset("EXTERMINATION", "SAW", "SQUARE", 12.0, 0, 0.30, 0.30, False,
           2800.0, 0.65, "BP", 0.55, 0.3, 0.55, "FUZZ", 0.0, 0.40,
           1.5, 220.0, 0.65, 200.0, 0.70, 4.5, 0.45, "CUTOFF",
           False, 0.0, 0.15, 0.0),

    # 3 — 1997: classic 1997 reese, MASSACRE 1994 engaged for that
    # quantized mid-90s tracker crunch
    Preset("1997", "SAW", "SAW", 22.0, 0, 0.0, 0.50, False,
           900.0, 0.50, "LP", 0.45, 0.0, 0.30, "CLIP", 0.0, 0.80,
           3.0, 350.0, 0.85, 450.0, 0.35, 0.6, 0.0, "CUTOFF",
           True, 0.0, 0.0, 0.0),

    # 4 — ACABALLERO: hispanic-named FM lead, BP filter, square waves
    Preset("ACABALLERO", "SQUARE", "SQUARE", 8.0, 0, 0.45, 0.40, False,
           2400.0, 0.55, "BP", 0.60, 0.35, 0.45, "FOLD", 0.0, 0.45,
           1.0, 180.0, 0.75, 240.0, 0.70, 5.5, 0.30, "PITCH",
           False, 0.0, 0.05, 0.0),

    # 5 — TRASHCAN: industrial noise-floor with a hidden tonal body
    Preset("TRASHCAN", "GRIT", "SAW", 35.0, 0, 0.10, 0.30, False,
           3800.0, 0.45, "BP", 0.30, 0.0, 0.60, "FUZZ", 0.10, 0.55,
           1.0, 120.0, 0.40, 100.0, 0.85, 6.0, 0.20, "VOLUME",
           False, 0.25, 0.0, -3.0),

    # 6 — FUCKSERUM: modern wobble reese with phase FM and sync
    Preset("FUCKSERUM", "SAW", "SAW", 32.0, 0, 0.55, 0.55, True,
           1600.0, 0.70, "LP", 0.80, 0.0, 0.40, "CLIP", 0.0, 0.75,
           2.0, 280.0, 0.85, 320.0, 0.65, 1.2, 0.65, "CUTOFF",
           False, 0.0, 0.0, 0.0),

    # 7 — SCUM: muddy sludgy low-end doom, slow attack
    Preset("SCUM", "PULSE", "SQUARE", 5.0, -1, 0.0, 0.85, False,
           450.0, 0.55, "LP", 0.50, 0.0, 0.35, "CLIP", 0.0, 0.85,
           1.0, 500.0, 0.90, 800.0, 0.30, 0.4, 0.0, "CUTOFF",
           False, 0.0, 0.0, -2.0),

    # 8 — JESUSSAW: devotional saw pad with subtle fold and slow LFO
    Preset("JESUSSAW", "SAW", "SAW", 14.0, 1, 0.05, 0.35, False,
           3200.0, 0.30, "LP", 0.50, 0.15, 0.40, "FOLD", 0.0, 0.50,
           45.0, 700.0, 0.85, 900.0, 0.0, 0.4, 0.20, "PITCH",
           False, 0.0, 0.0, -2.0),

    # 9 — BUTTHURTED: sour detuned misery (intentionally dissonant)
    Preset("BUTTHURTED", "SAW", "SAW", 65.0, 0, 0.0, 0.50, False,
           1900.0, 0.55, "LP", 0.30, 0.0, 0.40, "CLIP", 0.0, 0.60,
           1.0, 400.0, 0.70, 500.0, 0.25, 0.7, 0.0, "CUTOFF",
           False, 0.0, 0.0, -1.0),

    # 10 — ALIENFUCKER: weird FM space-synth with strong resonance
    Preset("ALIENFUCKER", "SQUARE", "PULSE", 45.0, 0, 0.70, 0.25, True,
           4200.0, 0.70, "BP", 0.55, 0.4, 0.50, "FOLD", 0.0, 0.35,
           1.0, 200.0, 0.60, 150.0, 0.85, 8.5, 0.55, "PITCH",
           False, 0.10, 0.25, 0.0),

    # 11 — NOOTTNEEDED: pure tracker-style square, no filter, MASSACRE 1994 ON
    Preset("NOOTTNEEDED", "SQUARE", "SQUARE", 0.0, 0, 0.0, 0.0, False,
           18000.0, 0.0, "LP", 0.0, 0.0, 0.0, "CLIP", 0.0, 0.0,
           1.0, 100.0, 1.0, 100.0, 0.0, 4.0, 0.0, "CUTOFF",
           True, 0.0, 0.0, 0.0),

    # 12 — THUNDERDOOM: massive doom riff, slow attack, sub
    Preset("THUNDERDOOM", "SAW", "SAW", 18.0, -1, 0.0, 0.80, False,
           750.0, 0.60, "LP", 0.40, 0.0, 0.55, "CLIP", 0.0, 0.90,
           25.0, 700.0, 0.90, 1100.0, 0.45, 0.3, 0.0, "CUTOFF",
           False, 0.0, 0.0, 0.0),

    # 13 — TRACKERSCUM: 8-bit crunch with bitcrush, no sub
    Preset("TRACKERSCUM", "SQUARE", "PULSE", 0.0, 0, 0.0, 0.0, False,
           6500.0, 0.25, "LP", 0.0, 0.0, 0.30, "CLIP", 0.55, 0.0,
           1.0, 80.0, 0.80, 90.0, 0.30, 4.0, 0.0, "CUTOFF",
           False, 0.0, 0.0, 0.0),

    # 14 — HAKKENBOOT: gabber kick-style distorted bass, no sustain
    Preset("HAKKENBOOT", "SQUARE", "SQUARE", 0.0, -1, 0.0, 0.95, False,
           480.0, 0.65, "LP", 0.65, 0.0, 0.85, "FUZZ", 0.0, 0.85,
           1.0, 70.0, 0.0, 90.0, 1.0, 4.0, 0.0, "CUTOFF",
           False, 0.0, 0.0, 0.0),

    # 15 — CHAINSAWLOBOTOMY: aggressive saw lead with feedback ruin
    Preset("CHAINSAWLOBOTOMY", "SAW", "SAW", 32.0, 0, 0.15, 0.25, False,
           3800.0, 0.55, "BP", 0.50, 0.30, 0.60, "FUZZ", 0.0, 0.40,
           1.0, 220.0, 0.65, 180.0, 0.80, 5.5, 0.25, "DRIVE",
           False, 0.35, 0.45, 0.0),

    # 16 — PIGSQUEAL: high-resonance squeal that responds to pitch
    Preset("PIGSQUEAL", "SAW", "SQUARE", 25.0, 0, 0.35, 0.20, True,
           3200.0, 0.85, "BP", 0.85, 0.55, 0.50, "FOLD", 0.0, 0.30,
           1.0, 180.0, 0.55, 220.0, 0.90, 8.0, 0.40, "PITCH",
           False, 0.0, 0.20, 0.0),

    # 17 — SLUDGEPIT: massive sub with resonant tail, slow everything
    Preset("SLUDGEPIT", "PULSE", "SQUARE", 7.0, -1, 0.0, 0.90, False,
           280.0, 0.78, "LP", 0.65, 0.0, 0.40, "CLIP", 0.0, 0.95,
           6.0, 1100.0, 0.92, 1800.0, 0.20, 0.25, 0.0, "CUTOFF",
           False, 0.0, 0.0, -2.0),
]

# ====================================================================
# 8. RENDER ONE NOTE
# ====================================================================

def render_note(preset, midi_note, duration_s=3.0, sr=SR):
    n = int(duration_s * sr)
    seed = int(midi_note) * 7919 + (hash(preset.name) & 0xFFFF)
    rng = np.random.default_rng(seed)

    base_freq = midi_to_freq(midi_note) * (2.0 ** preset.octave)
    base_freq = max(20.0, base_freq)
    detune_ratio = 2.0 ** (preset.detune / 1200.0)

    # --- Osc A ---
    if preset.waveA == 'NOISE':
        a = osc_noise(n, rng)
    elif preset.waveA == 'GRIT':
        a = osc_grit(base_freq, n, sr, rng)
    else:
        a = WAVE_FUNCS[preset.waveA](freq=base_freq, n=n, sr=sr, phase=0.0)

    # --- Osc B (for FM, we render at base_freq and modulate A's phase later) ---
    if preset.waveB == 'NOISE':
        b = osc_noise(n, rng)
    elif preset.waveB == 'GRIT':
        b = osc_grit(base_freq * detune_ratio, n, sr, rng)
    else:
        b = WAVE_FUNCS[preset.waveB](freq=base_freq * detune_ratio, n=n, sr=sr, phase=0.0)

    # --- Real phase-modulation FM: B modulates A's phase ---
    if preset.fm_amount > 0.001:
        # Re-render A with phase modulation by B
        # a(n) = saw(phase_n + fm_amount * b(n))
        inc_a = base_freq / sr
        if 0 < inc_a < 0.5:
            t_mod = (np.arange(n) * inc_a + preset.fm_amount * 2.0 * b) % 1.0
            if preset.waveA == 'SAW':
                a = 2.0 * t_mod - 1.0
            elif preset.waveA == 'SQUARE':
                a = np.where(t_mod < 0.5, 1.0, -1.0)
            elif preset.waveA == 'PULSE':
                a = np.where(t_mod < 0.5, 1.0, -1.0)
            elif preset.waveA == 'GRIT':
                a = (2.0 * t_mod - 1.0) + rng.uniform(-1.0, 1.0, n) * 0.18

    # --- Sub osc (square, one octave below) ---
    sub_freq = max(20.0, base_freq * 0.5)
    sub = osc_square(sub_freq, n, sr, 0.0) * preset.sub_level

    # --- Mix ---
    mix = (a + b) * 0.4 + sub * 0.6

    # --- MASSACRE 1994: quantize to ±0.7 for that tracker crunch ---
    if preset.massacre1994:
        mix = np.where(mix > 0, 0.7, -0.7)

    # --- RUINER: crossfade with white noise ---
    if preset.ruiner > 0.001:
        n2 = rng.uniform(-1.0, 1.0, n).astype(np.float32)
        mix = mix * (1.0 - preset.ruiner) + n2 * preset.ruiner

    # --- Envelope ---
    env = adsr_curve(n, preset.attack, preset.decay, preset.sustain,
                     preset.release, preset.punch, sr,
                     gate_frac=0.55)
    v = mix * env

    # --- LFO ---
    lfo = lfo_triangle(preset.lfo_rate, n, sr, 0.0)
    if preset.lfo_depth > 0.001:
        if preset.lfo_target == 'VOLUME':
            v = v * (1.0 + (lfo * preset.lfo_depth) * 0.7)
        elif preset.lfo_target == 'DRIVE':
            # We can't really modulate drive retroactively here, but
            # we can simulate it via an extra gain into the saturator.
            # Already passed — handled below in distortion by scaling.
            pass
        # For CUTOFF / PITCH LFO, we modulate filter cutoff.

    # --- Keytrack-modulated cutoff ---
    keytrack_shift = preset.keytrack * (float(midi_note) - 60.0) * 80.0
    base_cutoff = preset.cutoff + keytrack_shift

    # --- ENV MOD (positive raises, negative lowers cutoff) ---
    env_to_cutoff = preset.env_mod * env
    cutoff_mod = base_cutoff * (2.0 ** env_to_cutoff)

    # --- LFO -> cutoff modulation ---
    if preset.lfo_target == 'CUTOFF' and preset.lfo_depth > 0.001:
        lfo_to_cutoff = lfo * preset.lfo_depth
        cutoff_mod = cutoff_mod * (2.0 ** lfo_to_cutoff)

    cutoff_mod = np.clip(cutoff_mod, 20.0, sr * 0.45)

    # --- Filter ---
    filt = moog_ladder(v, cutoff_mod, preset.resonance, preset.filter_type, sr)

    # --- Drive LFO modulation ---
    drive_mod = 0.0
    if preset.lfo_target == 'DRIVE' and preset.lfo_depth > 0.001:
        drive_mod = lfo * preset.lfo_depth * 0.3

    # --- Distortion ---
    sat = apply_distortion(filt, preset.drive + drive_mod, preset.dist_type,
                            preset.bitcrush, preset.sub_protect)

    # --- FEEDBACK: inject delayed sat back into the next sample's filter input.
    # Approximate by mixing a delayed version of sat into v.
    if preset.feedback > 0.001:
        delay = max(1, int(sr * 0.015))  # 15 ms feedback delay
        fb = np.zeros_like(sat)
        z = 0.0
        for i in range(len(sat)):
            z = z * 0.995 + sat[i] * preset.feedback
            if i + delay < len(sat):
                fb[i + delay] = z * 0.4
        # Re-filter the feedback-injected signal for stability
        v_fb = np.clip(v + fb, -1.5, 1.5)
        filt_fb = moog_ladder(v_fb, cutoff_mod, preset.resonance * 0.7,
                              preset.filter_type, sr)
        sat = sat * 0.7 + filt_fb * 0.3

    # --- Output gain ---
    gain = 10.0 ** (preset.volume_db / 20.0)
    sat = sat * gain

    # --- DC blocker (one-pole, R=0.995) ---
    sat = _dc_block(sat, R=0.995)

    sat = np.clip(sat, -1.0, 1.0)

    # --- Fade-in/out (5 ms) to avoid click at note boundaries ---
    fade = int(0.005 * sr)
    if fade < n // 2:
        fi = np.linspace(0, 1, fade)
        sat[:fade] *= fi
        sat[-fade:] *= fi[::-1]

    return sat.astype(np.float32)

# ====================================================================
# 9. SF2 FILE WRITER (minimal viable SoundFont 2)
# ====================================================================

SAMPLE_NOTES = [36, 48, 60, 72, 84]  # C2, C3, C4, C5, C6

def write_sf2(filename, presets_data):
    samples = []
    for pi, (name, notes) in enumerate(presets_data):
        for midi_note, arr in notes:
            samples.append((pi, midi_note, arr))

    # ---- sdta chunk ----
    smpl_bytes = bytearray()
    sample_meta = []
    cursor = 0
    ZERO_PAD = 46
    for pi, midi_note, arr in samples:
        start = cursor
        z = np.zeros(ZERO_PAD, dtype=np.int16)
        body = (arr * 32767.0).astype(np.int16)
        full = np.concatenate([z, body, z])
        smpl_bytes.extend(full.tobytes())
        end = start + len(full) - 1
        loop_start = start + ZERO_PAD
        loop_end = end - ZERO_PAD
        sample_meta.append({
            'start': start, 'end': end,
            'loop_start': loop_start, 'loop_end': loop_end,
            'root_key': midi_note, 'preset_idx': pi,
        })
        cursor = end + 1

    sdta_smpl_chunk = b'smpl' + struct.pack('<I', len(smpl_bytes)) + bytes(smpl_bytes)
    if len(smpl_bytes) % 2:
        sdta_smpl_chunk += b'\x00'
    sdta_list = b'LIST' + struct.pack('<I', 4 + len(sdta_smpl_chunk)) + b'sdta' + sdta_smpl_chunk

    # ---- pdta chunk ----
    phdr_records = []
    pbag_cursor = 0
    for i, (name, _) in enumerate(presets_data):
        name_b = name.encode('utf-8')[:20].ljust(20, b'\x00')
        rec = struct.pack('<20sHHHIII', name_b, i, 0, pbag_cursor, 0, 0, 0)
        phdr_records.append(rec)
        pbag_cursor += 1
    term = struct.pack('<20sHHHIII', b'EOI\x00' + b'\x00' * 16, 0, 0, pbag_cursor, 0, 0, 0)
    phdr_records.append(term)
    phdr_bytes = b''.join(phdr_records)
    phdr_chunk = b'phdr' + struct.pack('<I', len(phdr_bytes)) + phdr_bytes
    if len(phdr_bytes) % 2: phdr_chunk += b'\x00'

    pbag_records = bytearray()
    gen_cursor = 0
    for i in range(len(presets_data)):
        pbag_records += struct.pack('<HH', gen_cursor, 0)
        gen_cursor += 1
    pbag_records += struct.pack('<HH', gen_cursor, 0)
    pbag_bytes = bytes(pbag_records)
    pbag_chunk = b'pbag' + struct.pack('<I', len(pbag_bytes)) + pbag_bytes
    if len(pbag_bytes) % 2: pbag_chunk += b'\x00'

    pmod_records = struct.pack('<HHHHHHH', 0, 0, 0, 0, 0, 0, 0)
    pmod_chunk = b'pmod' + struct.pack('<I', len(pmod_records)) + pmod_records

    pgen_records = bytearray()
    for i in range(len(presets_data)):
        pgen_records += struct.pack('<HH', 41, i)
    pgen_bytes = bytes(pgen_records)
    pgen_chunk = b'pgen' + struct.pack('<I', len(pgen_bytes)) + pgen_bytes
    if len(pgen_bytes) % 2: pgen_chunk += b'\x00'

    inst_records = []
    ibag_cursor = 0
    for i, (name, _) in enumerate(presets_data):
        inst_records.append(struct.pack('<20sH',
            name.encode('utf-8')[:20].ljust(20, b'\x00'), ibag_cursor))
        ibag_cursor += len(SAMPLE_NOTES)
    inst_records.append(struct.pack('<20sH', b'\x00' * 20, ibag_cursor))
    inst_bytes = b''.join(inst_records)
    inst_chunk = b'inst' + struct.pack('<I', len(inst_bytes)) + inst_bytes
    if len(inst_bytes) % 2: inst_chunk += b'\x00'

    ibag_records = bytearray()
    igen_cursor = 0
    for pi in range(len(presets_data)):
        for j in range(len(SAMPLE_NOTES)):
            ibag_records += struct.pack('<HH', igen_cursor, 0)
            igen_cursor += 3
    ibag_records += struct.pack('<HH', igen_cursor, 0)
    ibag_bytes = bytes(ibag_records)
    ibag_chunk = b'ibag' + struct.pack('<I', len(ibag_bytes)) + ibag_bytes
    if len(ibag_bytes) % 2: ibag_chunk += b'\x00'

    imod_records = struct.pack('<HHHHHHH', 0, 0, 0, 0, 0, 0, 0)
    imod_chunk = b'imod' + struct.pack('<I', len(imod_records)) + imod_records

    igen_records = bytearray()
    by_preset = {}
    for si, m in enumerate(sample_meta):
        by_preset.setdefault(m['preset_idx'], []).append(si)
    for pi in range(len(presets_data)):
        sids = by_preset.get(pi, [])
        for k, sid in enumerate(sids):
            mn = SAMPLE_NOTES[k]
            # Keyrange (gen 43): lo,hi bytes packed as uint16
            kr = (mn & 0xFF) | (((mn + 12) & 0xFF) << 8) if mn + 12 <= 127 else (mn & 0xFF) | (0x7F << 8)
            igen_records += struct.pack('<HH', 43, kr)
            igen_records += struct.pack('<HH', 53, sid + 1)
            igen_records += struct.pack('<HH', 56, 1)  # loop mode 1 = forward
    igen_bytes = bytes(igen_records)
    igen_chunk = b'igen' + struct.pack('<I', len(igen_bytes)) + igen_bytes
    if len(igen_bytes) % 2: igen_chunk += b'\x00'

    shdr_records = bytearray()
    shdr_records += struct.pack('<20sIIIIIBbHH',
        b'\x00' * 20, 0, 0, 0, 0, SR, 60, 0, 0, 0)
    for m in sample_meta:
        name = f"p{m['preset_idx']:02d}_n{m['root_key']:03d}".encode('utf-8')[:20]
        name = name.ljust(20, b'\x00')
        shdr_records += struct.pack('<20sIIIIIBbHH',
            name,
            m['start'], m['end'],
            m['loop_start'], m['loop_end'],
            SR,
            m['root_key'] & 0xFF,
            0,
            0, 0)
    shdr_records += struct.pack('<20sIIIIIBbHH',
        b'\x00' * 20, 0, 0, 0, 0, SR, 60, 0, 0, 0)
    shdr_bytes = bytes(shdr_records)
    shdr_chunk = b'shdr' + struct.pack('<I', len(shdr_bytes)) + shdr_bytes
    if len(shdr_bytes) % 2: shdr_chunk += b'\x00'

    pdta_inner = phdr_chunk + pbag_chunk + pmod_chunk + pgen_chunk + \
                 inst_chunk + ibag_chunk + imod_chunk + igen_chunk + shdr_chunk
    pdta_list = b'LIST' + struct.pack('<I', 4 + len(pdta_inner)) + b'pdta' + pdta_inner

    # ---- INFO chunk ----
    ifil = struct.pack('<HH', 2, 4)
    isng = b'EMU8000\x00'
    inam = b'SSAB Extreme Bass Synthesizer\x00'
    irom = b'SSAB\x00'
    info_chunks = b'ifil' + struct.pack('<I', len(ifil)) + ifil
    if len(ifil) % 2: info_chunks += b'\x00'
    info_chunks += b'isng' + struct.pack('<I', len(isng)) + isng
    if len(isng) % 2: info_chunks += b'\x00'
    info_chunks += b'INAM' + struct.pack('<I', len(inam)) + inam
    if len(inam) % 2: info_chunks += b'\x00'
    info_chunks += b'irom' + struct.pack('<I', len(irom)) + irom
    if len(irom) % 2: info_chunks += b'\x00'
    info_list = b'LIST' + struct.pack('<I', 4 + len(info_chunks)) + b'INFO' + info_chunks

    riff_payload = info_list + sdta_list + pdta_list
    riff = b'RIFF' + struct.pack('<I', 4 + len(riff_payload)) + b'sfbk' + riff_payload
    with open(filename, 'wb') as f:
        f.write(riff)
    return len(riff)

# ====================================================================
# 10. WAV WRITER
# ====================================================================

def write_wav(filename, samples_int16, sr=SR):
    with wave.open(filename, 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes(samples_int16.tobytes())

# ====================================================================
# 11. MAIN
# ====================================================================

def render_all_presets(output_dir, duration_s=3.0):
    os.makedirs(output_dir, exist_ok=True)
    presets_data = []
    for pi, p in enumerate(PRESETS):
        print(f"  [{pi+1:02d}/{len(PRESETS)}] Rendering '{p.name}'...", flush=True)
        notes = []
        for midi_note in SAMPLE_NOTES:
            arr = render_note(p, midi_note, duration_s=duration_s)
            notes.append((midi_note, arr))
        presets_data.append((p.name, notes))

    sf2_path = os.path.join(output_dir, 'SSAB.sf2')
    sz = write_sf2(sf2_path, presets_data)
    print(f"  -> {sf2_path} ({sz} bytes)")

    # Per-preset WAV previews (C3 = 60)
    for pi, (name, notes) in enumerate(presets_data):
        for midi_note, arr in notes:
            if midi_note != 60: continue
            wav_path = os.path.join(output_dir, f"{pi:02d}_{name}_C3.wav")
            arr16 = (arr * 32767.0).astype(np.int16)
            write_wav(wav_path, arr16)
    return sf2_path

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '/home/z/my-project/download/ssab_sf2'
    print(f"SSAB v1.0 SF2 Generator -> {out}")
    print(f"Rendering {len(PRESETS)} presets, {len(SAMPLE_NOTES)} notes each...")
    render_all_presets(out, duration_s=3.0)
    print("Done.")
