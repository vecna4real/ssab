#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace ssab
{
    // ------------------------------------------------------------------
    // Plugin metadata
    // ------------------------------------------------------------------
    static constexpr int     kVersion        = 0x010000;
    static constexpr int     kNumVoices      = 16;
    static constexpr double  kDefaultSampleRate = 44100.0;
    static constexpr int     kMaxBlockSize   = 512;

    // ------------------------------------------------------------------
    // Parameter IDs (kept short & stable for VST2 compatibility)
    // ------------------------------------------------------------------
    namespace pid
    {
        // THE CORE
        static const juce::String waveA          = "waveA";
        static const juce::String waveB          = "waveB";
        static const juce::String detune         = "detune";
        static const juce::String octave         = "octave";
        static const juce::String fmAmount       = "fmAmount";
        static const juce::String subLevel       = "subLevel";
        static const juce::String sync          = "sync";

        // ACID BATH
        static const juce::String cutoff         = "cutoff";
        static const juce::String resonance     = "resonance";
        static const juce::String filterType     = "filterType";
        static const juce::String envMod         = "envMod";
        static const juce::String keytrack      = "keytrack";

        // CARNAGE
        static const juce::String drive          = "drive";
        static const juce::String distType       = "distType";
        static const juce::String bitcrush       = "bitcrush";
        static const juce::String subProtect     = "subProtect";

        // IMPACT
        static const juce::String attack         = "attack";
        static const juce::String decay          = "decay";
        static const juce::String sustain        = "sustain";
        static const juce::String release        = "release";
        static const juce::String punch          = "punch";

        // LFO
        static const juce::String lfoRate        = "lfoRate";
        static const juce::String lfoDepth       = "lfoDepth";
        static const juce::String lfoTarget      = "lfoTarget";

        // DOOM
        static const juce::String width          = "width";
        static const juce::String volume          = "volume";
        static const juce::String brickwall      = "brickwall";

        // MASSACRE
        static const juce::String massacre1994   = "massacre1994";
        static const juce::String ruiner         = "ruiner";
        static const juce::String feedback       = "feedback";

        // Preset slot
        static const juce::String presetSlot     = "presetSlot";
    }

    // ------------------------------------------------------------------
    // Enums
    // ------------------------------------------------------------------
    enum class WaveType  : int { Saw = 0, Square, Pulse, Noise, Grit, NumTypes };
    enum class FilterType: int { Lowpass = 0, Bandpass, Highpass, NumTypes };
    enum class DistType  : int { Clip = 0, Fold, Fuzz, NumTypes };
    enum class LFOTarget : int { Cutoff = 0, Pitch, Volume, Drive, NumTargets };

    inline const char* waveName(WaveType w)
    {
        switch (w) {
        case WaveType::Saw:    return "SAW";
        case WaveType::Square: return "SQR";
        case WaveType::Pulse:  return "PLS";
        case WaveType::Noise:  return "NSE";
        case WaveType::Grit:   return "GRT";
        default:               return "???";
        }
    }
}
