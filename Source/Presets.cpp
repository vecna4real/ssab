#include "Presets.h"

namespace ssab
{
using namespace juce;

// ------------------------------------------------------------------
// 18 hand-crafted presets, v1.0. Each one is a playable bass
// instrument — no longer random noise generators. Tuned to match
// the Python DSP port in Scripts/ssab_sf2_gen.py so the .sf2 and
// .dll sound identical.
// ------------------------------------------------------------------
static const PresetInfo kPresets[kNumPresets] = {
    // 0 - RAGGA: classic reese, wide detune, big sub
    { "RAGGA", {
        WaveType::Saw, WaveType::Saw,
        28.0f, 2, 0.0f, 0.65f, false,
        1400.0f, 0.55f, FilterType::Lowpass, 0.4f, 0.0f,
        0.30f, DistType::Clip, 0.0f, 0.85f,
        4.0f, 220.0f, 0.85f, 300.0f, 0.55f,
        0.7f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, 0.0f
    }},
    // 1 - GUTCUTTER: massive sub + tight resonance, fuzz octave-up
    { "GUTCUTTER", {
        WaveType::Square, WaveType::Pulse,
        6.0f, 1, 0.0f, 0.90f, false,
        550.0f, 0.75f, FilterType::Lowpass, 0.55f, 0.0f,
        0.45f, DistType::Fuzz, 0.0f, 0.95f,
        2.0f, 120.0f, 0.90f, 500.0f, 0.85f,
        1.8f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, -1.0f
    }},
    // 2 - EXTERMINATION: midrange ripping wobble
    { "EXTERMINATION", {
        WaveType::Saw, WaveType::Square,
        12.0f, 2, 0.30f, 0.30f, false,
        2800.0f, 0.65f, FilterType::Bandpass, 0.55f, 0.3f,
        0.55f, DistType::Fuzz, 0.0f, 0.40f,
        1.5f, 220.0f, 0.65f, 200.0f, 0.70f,
        4.5f, LFOTarget::Cutoff, 0.45f,
        false, 0.0f, 0.15f, 0.0f
    }},
    // 3 - 1997: classic reese, MASSACRE 1994 ON
    { "1997", {
        WaveType::Saw, WaveType::Saw,
        22.0f, 2, 0.0f, 0.50f, false,
        900.0f, 0.50f, FilterType::Lowpass, 0.45f, 0.0f,
        0.30f, DistType::Clip, 0.0f, 0.80f,
        3.0f, 350.0f, 0.85f, 450.0f, 0.35f,
        0.6f, LFOTarget::Cutoff, 0.0f,
        true, 0.0f, 0.0f, 0.0f
    }},
    // 4 - ACABALLERO: FM lead with BP filter
    { "ACABALLERO", {
        WaveType::Square, WaveType::Square,
        8.0f, 2, 0.45f, 0.40f, false,
        2400.0f, 0.55f, FilterType::Bandpass, 0.60f, 0.35f,
        0.45f, DistType::Fold, 0.0f, 0.45f,
        1.0f, 180.0f, 0.75f, 240.0f, 0.70f,
        5.5f, LFOTarget::Pitch, 0.30f,
        false, 0.0f, 0.05f, 0.0f
    }},
    // 5 - TRASHCAN: industrial grit with hidden tonal body
    { "TRASHCAN", {
        WaveType::Grit, WaveType::Saw,
        35.0f, 2, 0.10f, 0.30f, false,
        3800.0f, 0.45f, FilterType::Bandpass, 0.30f, 0.0f,
        0.60f, DistType::Fuzz, 0.10f, 0.55f,
        1.0f, 120.0f, 0.40f, 100.0f, 0.85f,
        6.0f, LFOTarget::Volume, 0.20f,
        false, 0.25f, 0.0f, -3.0f
    }},
    // 6 - FUCKSERUM: modern wobble reese with phase FM and sync
    { "FUCKSERUM", {
        WaveType::Saw, WaveType::Saw,
        32.0f, 2, 0.55f, 0.55f, true,
        1600.0f, 0.70f, FilterType::Lowpass, 0.80f, 0.0f,
        0.40f, DistType::Clip, 0.0f, 0.75f,
        2.0f, 280.0f, 0.85f, 320.0f, 0.65f,
        1.2f, LFOTarget::Cutoff, 0.65f,
        false, 0.0f, 0.0f, 0.0f
    }},
    // 7 - SCUM: muddy sludgy low-end doom
    { "SCUM", {
        WaveType::Pulse, WaveType::Square,
        5.0f, 1, 0.0f, 0.85f, false,
        450.0f, 0.55f, FilterType::Lowpass, 0.50f, 0.0f,
        0.35f, DistType::Clip, 0.0f, 0.85f,
        1.0f, 500.0f, 0.90f, 800.0f, 0.30f,
        0.4f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, -2.0f
    }},
    // 8 - JESUSSAW: devotional saw pad with subtle fold and slow LFO
    { "JESUSSAW", {
        WaveType::Saw, WaveType::Saw,
        14.0f, 3, 0.05f, 0.35f, false,
        3200.0f, 0.30f, FilterType::Lowpass, 0.50f, 0.15f,
        0.40f, DistType::Fold, 0.0f, 0.50f,
        45.0f, 700.0f, 0.85f, 900.0f, 0.0f,
        0.4f, LFOTarget::Pitch, 0.20f,
        false, 0.0f, 0.0f, -2.0f
    }},
    // 9 - BUTTHURTED: sour detuned misery (intentionally dissonant)
    { "BUTTHURTED", {
        WaveType::Saw, WaveType::Saw,
        65.0f, 2, 0.0f, 0.50f, false,
        1900.0f, 0.55f, FilterType::Lowpass, 0.30f, 0.0f,
        0.40f, DistType::Clip, 0.0f, 0.60f,
        1.0f, 400.0f, 0.70f, 500.0f, 0.25f,
        0.7f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, -1.0f
    }},
    // 10 - ALIENFUCKER: weird FM space-synth with strong resonance
    { "ALIENFUCKER", {
        WaveType::Square, WaveType::Pulse,
        45.0f, 3, 0.70f, 0.25f, true,
        4200.0f, 0.70f, FilterType::Bandpass, 0.55f, 0.4f,
        0.50f, DistType::Fold, 0.0f, 0.35f,
        1.0f, 200.0f, 0.60f, 150.0f, 0.85f,
        8.5f, LFOTarget::Pitch, 0.55f,
        false, 0.10f, 0.25f, 0.0f
    }},
    // 11 - NOOTTNEEDED: pure tracker-style square, no filter, MASSACRE 1994 ON
    { "NOOTTNEEDED", {
        WaveType::Square, WaveType::Square,
        0.0f, 2, 0.0f, 0.0f, false,
        18000.0f, 0.0f, FilterType::Lowpass, 0.0f, 0.0f,
        0.0f, DistType::Clip, 0.0f, 0.0f,
        1.0f, 100.0f, 1.0f, 100.0f, 0.0f,
        4.0f, LFOTarget::Cutoff, 0.0f,
        true, 0.0f, 0.0f, 0.0f
    }},
    // 12 - THUNDERDOOM: massive doom riff, slow attack
    { "THUNDERDOOM", {
        WaveType::Saw, WaveType::Saw,
        18.0f, 0, 0.0f, 0.80f, false,
        750.0f, 0.60f, FilterType::Lowpass, 0.40f, 0.0f,
        0.55f, DistType::Clip, 0.0f, 0.90f,
        25.0f, 700.0f, 0.90f, 1100.0f, 0.45f,
        0.3f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, 0.0f
    }},
    // 13 - TRACKERSCUM: 8-bit crunch with bitcrush
    { "TRACKERSCUM", {
        WaveType::Square, WaveType::Pulse,
        0.0f, 2, 0.0f, 0.0f, false,
        6500.0f, 0.25f, FilterType::Lowpass, 0.0f, 0.0f,
        0.30f, DistType::Clip, 0.55f, 0.0f,
        1.0f, 80.0f, 0.80f, 90.0f, 0.30f,
        4.0f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, 0.0f
    }},
    // 14 - HAKKENBOOT: gabber kick-style distorted bass, no sustain
    { "HAKKENBOOT", {
        WaveType::Square, WaveType::Square,
        0.0f, 1, 0.0f, 0.95f, false,
        480.0f, 0.65f, FilterType::Lowpass, 0.65f, 0.0f,
        0.85f, DistType::Fuzz, 0.0f, 0.85f,
        1.0f, 70.0f, 0.0f, 90.0f, 1.0f,
        4.0f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, 0.0f
    }},
    // 15 - CHAINSAWLOBOTOMY: aggressive saw lead with feedback ruin
    { "CHAINSAWLOBOTOMY", {
        WaveType::Saw, WaveType::Saw,
        32.0f, 2, 0.15f, 0.25f, false,
        3800.0f, 0.55f, FilterType::Bandpass, 0.50f, 0.30f,
        0.60f, DistType::Fuzz, 0.0f, 0.40f,
        1.0f, 220.0f, 0.65f, 180.0f, 0.80f,
        5.5f, LFOTarget::Drive, 0.25f,
        false, 0.35f, 0.45f, 0.0f
    }},
    // 16 - PIGSQUEAL: high-resonance squeal, pitch-responsive
    { "PIGSQUEAL", {
        WaveType::Saw, WaveType::Square,
        25.0f, 2, 0.35f, 0.20f, true,
        3200.0f, 0.85f, FilterType::Bandpass, 0.85f, 0.55f,
        0.50f, DistType::Fold, 0.0f, 0.30f,
        1.0f, 180.0f, 0.55f, 220.0f, 0.90f,
        8.0f, LFOTarget::Pitch, 0.40f,
        false, 0.0f, 0.20f, 0.0f
    }},
    // 17 - SLUDGEPIT: massive sub with resonant tail, slow everything
    { "SLUDGEPIT", {
        WaveType::Pulse, WaveType::Square,
        7.0f, 1, 0.0f, 0.90f, false,
        280.0f, 0.78f, FilterType::Lowpass, 0.65f, 0.0f,
        0.40f, DistType::Clip, 0.0f, 0.95f,
        6.0f, 1100.0f, 0.92f, 1800.0f, 0.20f,
        0.25f, LFOTarget::Cutoff, 0.0f,
        false, 0.0f, 0.0f, -2.0f
    }},
};

const Array<PresetInfo>& getPresetList()
{
    static Array<PresetInfo> list;
    if (list.isEmpty())
        for (int i = 0; i < kNumPresets; ++i) list.add(kPresets[i]);
    return list;
}

template <typename T>
static void setParamSafe(AudioProcessorValueTreeState& s, const String& id, T v)
{
    if (auto* p = s.getParameter(id))
    {
        const float nv = p->convertTo0to1(static_cast<float>(v));
        p->setValueNotifyingHost(nv);
    }
}

static void setChoice(AudioProcessorValueTreeState& s, const String& id, int v)
{
    if (auto* p = s.getParameter(id))
    {
        const float range = p->getNumSteps() > 1 ? 1.0f / float(p->getNumSteps() - 1) : 0.0f;
        const float nv = juce::jlimit(0.0f, 1.0f, v * range);
        p->setValueNotifyingHost(nv);
    }
}

void applyPresetToState(AudioProcessorValueTreeState& state, int slot)
{
    if (slot < 0 || slot >= kNumPresets) return;
    const PresetInfo& info = kPresets[slot];
    const VoiceParams& p = info.params;

    // THE CORE
    setChoice(state, pid::waveA,        (int)p.waveA);
    setChoice(state, pid::waveB,        (int)p.waveB);
    setParamSafe(state, pid::detune,    p.detune);
    setChoice(state, pid::octave,       p.octave);
    setParamSafe(state, pid::fmAmount,  p.fmAmount);
    setParamSafe(state, pid::subLevel,  p.subLevel);
    setParamSafe(state, pid::sync,      p.sync ? 1.0f : 0.0f);

    // ACID BATH
    setParamSafe(state, pid::cutoff,    p.cutoff);
    setParamSafe(state, pid::resonance, p.resonance);
    setChoice(state, pid::filterType,   (int)p.filterType);
    setParamSafe(state, pid::envMod,    p.envMod);
    setParamSafe(state, pid::keytrack,  p.keytrack);

    // CARNAGE
    setParamSafe(state, pid::drive,         p.drive);
    setChoice(state, pid::distType,         (int)p.distType);
    setParamSafe(state, pid::bitcrush,      p.bitcrush);
    setParamSafe(state, pid::subProtect,   p.subProtect);

    // IMPACT
    setParamSafe(state, pid::attack,    p.attack);
    setParamSafe(state, pid::decay,     p.decay);
    setParamSafe(state, pid::sustain,   p.sustain);
    setParamSafe(state, pid::release,   p.release);
    setParamSafe(state, pid::punch,     p.punch);

    // LFO
    setParamSafe(state, pid::lfoRate,   p.lfoRate);
    setParamSafe(state, pid::lfoDepth, p.lfoDepth);
    setChoice(state, pid::lfoTarget,    (int)p.lfoTarget);

    // MASSACRE
    setParamSafe(state, pid::massacre1994, p.massacre1994 ? 1.0f : 0.0f);
    setParamSafe(state, pid::ruiner,        p.ruiner);
    setParamSafe(state, pid::feedback,      p.feedback);

    // DOOM
    setParamSafe(state, pid::volume, p.volumeDb);
}

String presetName(int slot)
{
    if (slot < 0 || slot >= kNumPresets) return "UNKNOWN";
    return kPresets[slot].name;
}
}
