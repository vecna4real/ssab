#pragma once

#include "Constants.h"
#include "DSP/Voice.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace ssab
{
    // ------------------------------------------------------------------
    // Number of built-in presets (must match the ParameterInt range).
    // ------------------------------------------------------------------
    static constexpr int kNumPresets = 18;

    struct PresetInfo
    {
        const char* name;
        VoiceParams params;
    };

    // Returns the preset list (index = preset slot).
    const juce::Array<PresetInfo>& getPresetList();

    // Apply a preset slot's params to the APVTS.
    void applyPresetToState(juce::AudioProcessorValueTreeState& state, int slot);

    // Returns the name of a preset by slot index.
    juce::String presetName(int slot);
}
