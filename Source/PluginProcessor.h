#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Constants.h"
#include "Parameters.h"
#include "Presets.h"
#include "DSP/Voice.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // The main audio processor. Manages:
    //   - 16-voice polyphony (round-robin stealing)
    //   - parameter -> VoiceParams conversion (per block)
    //   - global LFO & doom stage
    // ------------------------------------------------------------------
    class SSABProcessor : public juce::AudioProcessor
    {
    public:
        SSABProcessor();
        ~SSABProcessor() override = default;

        // AudioProcessor
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return JucePlugin_Name; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 0.5; }

        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int index) override;
        const juce::String getProgramName(int index) override;
        void changeProgramName(int index, const juce::String& newName) override {}

        void getStateInformation(juce::MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        // Public so editor can attach widgets
        juce::AudioProcessorValueTreeState apvts;

        // Cached host context for editors
        // (none)

    private:
        // Build a VoiceParams from the current APVTS state.
        VoiceParams buildParamsFromState() const;

        // Handle a MIDI note on/off
        void handleMidi(const juce::MidiBuffer& midi, int numSamples);

        // Voice pool
        std::array<SSVoice, kNumVoices> voices;
        int voiceRoundRobin = 0;

        // Cached params (updated per block)
        VoiceParams cachedParams;

        // DOOM stereo width / limiter
        float lastLeftGain  = 0.0f;
        float lastRightGain = 0.0f;

        // brickwall limiter state
        float brickMax = 1.0f;

        // Master DC blockers (L/R) — used at the end of the signal chain
        DCBlocker masterDC_L, masterDC_R;

        // Current preset slot (for getNumPrograms / setCurrentProgram)
        int currentPresetSlot = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SSABProcessor)
    };
}
