# SSAB // LMMS Quick Start

Three ways to use SSAB in LMMS on Windows 10. Pick the one that matches your patience.

---

## Method 1 — SoundFont (.sf2) [works right now]

The fastest path. A ready-made `.sf2` file is included alongside this archive.

**File:** `SSAB.sf2` (~19 MB, 18 presets, 5 sampled notes per preset)

If `SSAB.sf2` is missing (e.g. you cloned the source repo without artefacts),
regenerate it:
```
python3 Scripts/ssab_sf2_gen.py /path/to/output/dir
```
Requires Python 3.9+ with `numpy` and `scipy` installed (`pip install numpy scipy soundfile`).

### How to use in LMMS
1. Open LMMS
2. Add a new instrument track → choose **Sf2 Player**
3. In the Sf2 Player panel, click the folder icon and load `SSAB.sf2`
4. Click the preset dropdown → pick any of the 18 names (RAGGA, GUTCUTTER, EXTERMINATION, 1997, ACABALLERO, TRASHCAN, FUCKSERUM, SCUM, JESUSSAW, BUTTHURTED, ALIENFUCKER, NOOTTNEEDED, THUNDERDOOM, TRACKERSCUM, HAKKENBOOT, CHAINSAWLOBOTOMY, PIGSQUEAL, SLUDGEPIT)
5. Play MIDI notes as usual — Sf2 Player will pitch-shift the nearest sampled note

### What you get
- All 18 presets with their exact DSP settings baked in (oscillator types, detune, octave, FM, filter cutoff/resonance/type, ENV MOD, keytrack, drive, dist type, bitcrush, sub protect, ADSR, punch, MASSACRE 1994, ruiner, feedback, volume)
- Sampled notes: C2 (36), C3 (48), C4 (60), C5 (72), C6 (84) — LMMS pitch-shifts between them
- Looping sustain region for each sample (so held notes don't go silent)

### What you DON'T get (compared to the .dll)
- Live parameter tweaking — once a sample is baked, you can't change CUTOFF or DRIVE on the fly
- LFO modulation (it's baked into the sample at render time)
- Full envelope retrigger behaviour (SF2 has its own simple env)
- Stereo widening (SF2 is mono here)
- 16-voice polyphony (depends on LMMS Sf2 Player's voice limit)

### Also included: per-preset WAV previews
`download/ssab_sf2/00_RAGGA_C3.wav` ... `17_SLUDGEPIT_C3.wav` — one short C3 preview per preset, useful for quickly auditioning.

---

## Method 2 — GitHub Actions (cloud .dll build, free)

Build the actual VST2 `.dll` in the cloud — no local compiler install needed.

### Steps
1. Go to <https://github.com> and create a free account if you don't have one
2. Create a new empty repo (e.g. `ssab`), **public** (GitHub Actions is free for public repos)
3. Unzip `SSAB_v1.0_source.zip` and push all files to that repo
4. On GitHub, go to the **Actions** tab of your repo
5. The workflow `Build SSAB VST2/VST3` will run automatically on push. You can also trigger it manually via "Run workflow" button
6. Wait ~5-10 min for the build to finish
7. Click the completed run → scroll down to **Artifacts** → download `SSAB_VST2_dll`
8. Unzip the artifact → you get `SSAB.dll`

### Install in LMMS
1. Copy `SSAB.dll` to `C:\Program Files\VstPlugins\` (or wherever LMMS scans for VSTs)
   - Default LMMS VST path is configured in: **Edit → Settings → Paths → VST plugins**
2. In LMMS, add a new instrument track → choose **Vestige**
3. In the Vestige panel, click the folder icon → select `SSAB.dll`
4. SSAB GUI opens with all 7 blocks (THE CORE, ACID BATH, CARNAGE, IMPACT, LFO, DOOM, MASSACRE)
5. All 18 presets are accessible from the built-in preset browser (PREV/NEXT buttons + display)

### Why GitHub Actions works
- `windows-latest` runner has VS 2022 + MSBuild preinstalled
- CMake 3.27 is installed by the action `jwlawson/actions-setup-cmake`
- JUCE 7.0.12 is auto-downloaded via CMake `FetchContent`
- VST2 SDK headers come from `robbert-vdh/juce6-vst2-headers` (open-source stub)
- The full workflow is in `.github/workflows/build.yml` — you can read and audit it

### Optional: release on tag
Push a git tag like `v1.0.0` and the workflow will also attach the `.dll` to a GitHub Release, so you have a permanent download URL.

---

## Method 3 — Local build (need VS 2022 + CMake)

If you have Visual Studio 2022 installed locally on Windows.

### Steps
```powershell
cd path\to\SSAB
.\Scripts\build.ps1
```

The script does everything: downloads JUCE via FetchContent, downloads VST2 stub headers, configures VS 2022 build, produces `SSAB.dll`, `SSAB.vst3`, `SSAB.exe` in `build\SSAB_artefacts\Release\`.

Install in LMMS as described in Method 2.

### Requirements
- Visual Studio 2022 with the "Desktop development with C++" workload
- CMake 3.22+ (download from <https://cmake.org/download/>)
- Internet access (for JUCE + VST2 header stub download on first build)

---

## Comparison

| Method | Time to working sound | Live tweaking? | All 18 presets? | Custom GUI? |
|--------|----------------------|----------------|------------------|-------------|
| .sf2 in Sf2 Player | 1 min | No (baked) | Yes | No |
| .dll from GitHub Actions | ~10 min | Yes | Yes | Yes |
| .dll local build | ~30 min (install VS) | Yes | Yes | Yes |

---

## Which should I use?

- **If you just want to hear the sounds now** → Method 1 (.sf2)
- **If you want a real VST2 plugin with full control** → Method 2 (GitHub Actions)
- **If you're going to develop / modify the synth** → Method 3 (local build)

Method 1 is your zero-friction starting point. Once you confirm the sounds are right for your track, switch to Method 2 to get the .dll with full tweakability.
