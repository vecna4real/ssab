#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "SSABKnob.h"
#include "SSABSwitch.h"
#include "SSABDisplay.h"
#include "SSABFonts.h"
#include "ColorPalette.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSABEditor: the main plugin window. Designed to be intentionally
    // "incredibly disgusting" - clashing neon, hazard stripes, horror
    // fonts, chunky borders, blinking bits.
    // ------------------------------------------------------------------
    class SSABEditor : public juce::AudioProcessorEditor
    {
    public:
        SSABEditor(SSABProcessor&);
        ~SSABEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        void paintHeader(juce::Graphics&);
        void paintSection(juce::Graphics&, juce::Rectangle<int> bounds,
                          const juce::String& title, juce::Colour bg, juce::Colour accent);
        void paintHazardStripes(juce::Graphics&, juce::Rectangle<int> bounds);

        SSABProcessor& processor;
        SSABLookAndFeel lnf;

        // Header
        SSABDisplay display;

        // Preset navigation
        juce::TextButton prevPreset, nextPreset;

        // THE CORE
        std::unique_ptr<SSABChoice> waveA, waveB, octave;
        std::unique_ptr<SSABKnob> detune, fmAmount, subLevel;
        std::unique_ptr<SSABToggle> sync;

        // ACID BATH
        std::unique_ptr<SSABKnob> cutoff, resonance, envMod, keytrack;
        std::unique_ptr<SSABChoice> filterType;

        // CARNAGE
        std::unique_ptr<SSABKnob> drive, bitcrush, subProtect;
        std::unique_ptr<SSABChoice> distType;

        // IMPACT
        std::unique_ptr<SSABKnob> attack, decay, sustain, release, punch;

        // LFO
        std::unique_ptr<SSABKnob> lfoRate, lfoDepth;
        std::unique_ptr<SSABChoice> lfoTarget;

        // DOOM
        std::unique_ptr<SSABKnob> width, volume;
        std::unique_ptr<SSABToggle> brickwall;

        // MASSACRE
        std::unique_ptr<SSABToggle> massacre1994;
        std::unique_ptr<SSABKnob> ruiner, feedback;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SSABEditor)
    };
}
