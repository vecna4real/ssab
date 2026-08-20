#include "Parameters.h"

namespace ssab
{
using namespace juce;

static NormalisableRange<float> makeRange(float lo, float hi, float skew = 1.0f, float step = 0.0f)
{
    NormalisableRange<float> r(lo, hi);
    r.skew = skew;
    r.interval = step;
    return r;
}

AudioProcessorValueTreeState::ParameterLayout buildParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    // ============ THE CORE ============
    layout.add(std::make_unique<AudioParameterChoice>(
        ParameterID{ pid::waveA, 1 }, "WAVE A",
        StringArray{ "SAW", "SQUARE", "PULSE", "NOISE", "GRIT" }, 0));

    layout.add(std::make_unique<AudioParameterChoice>(
        ParameterID{ pid::waveB, 1 }, "WAVE B",
        StringArray{ "SAW", "SQUARE", "PULSE", "NOISE", "GRIT" }, 0));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::detune, 1 }, "DETUNE",
        makeRange(-100.0f, 100.0f, 1.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<AudioParameterChoice>(
        ParameterID{ pid::octave, 1 }, "OCTAVE",
        StringArray{ "-2", "-1", "0", "+1", "+2" }, 2));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::fmAmount, 1 }, "FM AMOUNT",
        makeRange(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::subLevel, 1 }, "SUB LEVEL",
        makeRange(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<AudioParameterBool>(
        ParameterID{ pid::sync, 1 }, "SYNC", false));

    // ============ ACID BATH ============
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::cutoff, 1 }, "CUTOFF",
        makeRange(20.0f, 18000.0f, 0.4f, 0.1f), 8000.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::resonance, 1 }, "RESONANCE",
        makeRange(0.0f, 1.0f), 0.4f));

    layout.add(std::make_unique<AudioParameterChoice>(
        ParameterID{ pid::filterType, 1 }, "FILTER TYPE",
        StringArray{ "LP", "BP", "HP" }, 0));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::envMod, 1 }, "ENV MOD",
        makeRange(-1.0f, 1.0f), 0.6f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::keytrack, 1 }, "KEYTRACK",
        makeRange(-1.0f, 1.0f), 0.0f));

    // ============ CARNAGE ============
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::drive, 1 }, "DRIVE",
        makeRange(0.0f, 1.0f), 0.3f));

    layout.add(std::make_unique<AudioParameterChoice>(
        ParameterID{ pid::distType, 1 }, "DIST TYPE",
        StringArray{ "CLIP", "FOLD", "FUZZ" }, 0));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::bitcrush, 1 }, "BITCRUSH",
        makeRange(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::subProtect, 1 }, "SUB PROTECT",
        makeRange(0.0f, 1.0f), 0.7f));

    // ============ IMPACT ============
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::attack, 1 }, "ATTACK",
        makeRange(0.1f, 200.0f, 0.6f, 0.1f), 1.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::decay, 1 }, "DECAY",
        makeRange(1.0f, 2000.0f, 0.5f, 1.0f), 200.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::sustain, 1 }, "SUSTAIN",
        makeRange(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::release, 1 }, "RELEASE",
        makeRange(1.0f, 4000.0f, 0.5f, 1.0f), 200.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::punch, 1 }, "PUNCH",
        makeRange(0.0f, 1.0f), 0.5f));

    // ============ LFO ============
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::lfoRate, 1 }, "RATE",
        makeRange(0.05f, 30.0f, 0.5f, 0.01f), 4.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::lfoDepth, 1 }, "DEPTH",
        makeRange(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<AudioParameterChoice>(
        ParameterID{ pid::lfoTarget, 1 }, "TARGET",
        StringArray{ "CUTOFF", "PITCH", "VOLUME", "DRIVE" }, 0));

    // ============ DOOM ============
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::width, 1 }, "WIDTH",
        makeRange(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::volume, 1 }, "VOLUME",
        makeRange(-60.0f, 6.0f, 2.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<AudioParameterBool>(
        ParameterID{ pid::brickwall, 1 }, "BRICKWALL", false));

    // ============ MASSACRE ============
    layout.add(std::make_unique<AudioParameterBool>(
        ParameterID{ pid::massacre1994, 1 }, "1994 SWITCH", false));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::ruiner, 1 }, "RUINER",
        makeRange(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{ pid::feedback, 1 }, "FEEDBACK",
        makeRange(0.0f, 0.99f), 0.0f));

    // ============ Preset slot ============
    layout.add(std::make_unique<AudioParameterInt>(
        ParameterID{ pid::presetSlot, 1 }, "PRESET",
        0, 17, 0));

    return layout;
}

String valueToText(const String& paramID, float value)
{
    if (paramID == pid::detune)       return String(value, 1) + " ct";
    if (paramID == pid::fmAmount)     return String(int(value * 100)) + "%";
    if (paramID == pid::subLevel)     return String(int(value * 100)) + "%";
    if (paramID == pid::cutoff)       return String(int(value)) + " Hz";
    if (paramID == pid::resonance)    return String(int(value * 100)) + "%";
    if (paramID == pid::envMod)       return String(int(value * 100)) + "%";
    if (paramID == pid::keytrack)     return String(int(value * 100)) + "%";
    if (paramID == pid::drive)        return String(int(value * 100)) + "%";
    if (paramID == pid::bitcrush)     return String(int(value * 100)) + "%";
    if (paramID == pid::subProtect)   return String(int(value * 100)) + "%";
    if (paramID == pid::attack)       return String(value, 1) + " ms";
    if (paramID == pid::decay)        return String(int(value)) + " ms";
    if (paramID == pid::sustain)      return String(int(value * 100)) + "%";
    if (paramID == pid::release)     return String(int(value)) + " ms";
    if (paramID == pid::punch)        return String(int(value * 100)) + "%";
    if (paramID == pid::lfoRate)      return String(value, 2) + " Hz";
    if (paramID == pid::lfoDepth)     return String(int(value * 100)) + "%";
    if (paramID == pid::width)        return String(int(value * 100)) + "%";
    if (paramID == pid::volume)       return String(value, 1) + " dB";
    if (paramID == pid::ruiner)       return String(int(value * 100)) + "%";
    if (paramID == pid::feedback)     return String(int(value * 100)) + "%";
    return String(value, 2);
}

float textToValue(const String& paramID, const String& text)
{
    float v = text.getFloatValue();
    if (paramID == pid::cutoff)        return v;
    if (paramID == pid::volume)        return v;
    if (paramID == pid::detune)       return v;
    if (paramID == pid::attack)        return v;
    if (paramID == pid::decay)         return v;
    if (paramID == pid::release)       return v;
    if (paramID == pid::lfoRate)       return v;
    return v / 100.0f;
}
}
