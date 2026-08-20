#pragma once

#include "Constants.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // Builds the AudioProcessorValueTreeState parameter layout.
    // All parameters are normalised 0..1 internally; UI values are
    // converted to/from real units via the helpers in DSPUtils.
    // ------------------------------------------------------------------
    juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();

    // Returns the human-readable value string for a given parameter ID.
    juce::String valueToText(const juce::String& paramID, float value);

    // Returns a normalised value (0..1) given a real-world text.
    float textToValue(const juce::String& paramID, const juce::String& text);
}
