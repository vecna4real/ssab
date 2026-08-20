#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"
#include "SSABFonts.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSABSwitch: a 5-position rotary switch used for things like
    // WAVE A / WAVE B / OCTAVE. Drawn as a chunky hexagonal dial.
    // ------------------------------------------------------------------
    class SSABSwitch : public juce::Component
    {
    public:
        SSABSwitch(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts,
                    const juce::String& paramID, juce::Colour accent = palette::hazard)
            : attachment(vts, paramID, &slider)
        {
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
            slider.setColour(juce::Slider::textBoxTextColourId, palette::textBright);
            slider.setColour(juce::Slider::textBoxBackgroundColourId, palette::panelBg);
            slider.setColour(juce::Slider::textBoxOutlineColourId, accent);
            addAndMakeVisible(slider);

            label.setText(labelText, juce::dontSendNotification);
            label.setFont(SSABFonts::get().getLabelFont());
            label.setColour(juce::Label::textColourId, accent);
            label.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(label);
        }

        void resized() override
        {
            auto b = getLocalBounds();
            label.setBounds(b.removeFromTop(14));
            slider.setBounds(b);
        }

        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(1.0f);
            g.setColour(palette::panelBg2);
            g.fillRect(b);
            g.setColour(palette::blood);
            g.drawRect(b, 1.0f);
        }

    private:
        juce::Slider slider;
        juce::Label  label;
        juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    };
}
