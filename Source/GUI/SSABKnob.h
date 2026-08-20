#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"
#include "SSABFonts.h"
#include "../Constants.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSABKnob: a slider with a label above and value text below,
    // drawn with the SSAB look.
    // ------------------------------------------------------------------
    class SSABKnob : public juce::Component
    {
    public:
        SSABKnob(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts,
                  const juce::String& paramID, juce::Colour accent = palette::acid)
            : paramID(paramID), accentColour(accent), attachment(vts, paramID, &slider)
        {
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
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
            auto bounds = getLocalBounds();
            label.setBounds(bounds.removeFromTop(14));
            slider.setBounds(bounds);
        }

        void paint(juce::Graphics& g) override
        {
            // Draw an outer "frame" - chunky early-00s vibe.
            auto b = getLocalBounds().toFloat().reduced(1.0f);
            g.setColour(palette::panelBg2);
            g.fillRect(b);
            g.setColour(accentColour);
            g.drawRect(b, 1.0f);
        }

        juce::Slider& getSlider() { return slider; }

    private:
        juce::String paramID;
        juce::Colour accentColour;
        juce::Slider slider;
        juce::Label  label;
        juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    };

    // ------------------------------------------------------------------
    // SSABChoice: a ComboBox with attached label, styled.
    // ------------------------------------------------------------------
    class SSABChoice : public juce::Component
    {
    public:
        SSABChoice(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts,
                    const juce::String& paramID, juce::Colour accent = palette::hazard)
            : attachment(vts, paramID, &combo)
        {
            // Set items based on paramID
            if (paramID == pid::waveA || paramID == pid::waveB)
            {
                for (int i = 0; i < (int)WaveType::NumTypes; ++i)
                    combo.addItem(waveName((WaveType)i), i + 1);
            }
            else if (paramID == pid::filterType)
            {
                combo.addItem("LP", 1);
                combo.addItem("BP", 2);
                combo.addItem("HP", 3);
            }
            else if (paramID == pid::distType)
            {
                combo.addItem("CLIP", 1);
                combo.addItem("FOLD", 2);
                combo.addItem("FUZZ", 3);
            }
            else if (paramID == pid::lfoTarget)
            {
                combo.addItem("CUTOFF", 1);
                combo.addItem("PITCH", 2);
                combo.addItem("VOLUME", 3);
                combo.addItem("DRIVE", 4);
            }
            else if (paramID == pid::octave)
            {
                combo.addItem("-2", 1);
                combo.addItem("-1", 2);
                combo.addItem("0", 3);
                combo.addItem("+1", 4);
                combo.addItem("+2", 5);
            }

            addAndMakeVisible(combo);

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
            combo.setBounds(b.reduced(2, 1));
        }

    private:
        juce::ComboBox combo;
        juce::Label    label;
        juce::AudioProcessorValueTreeState::ComboBoxAttachment attachment;
    };

    // ------------------------------------------------------------------
    // SSABToggle: a chunky toggle button styled like a hazard switch.
    // ------------------------------------------------------------------
    class SSABToggle : public juce::Component
    {
    public:
        SSABToggle(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts,
                    const juce::String& paramID, juce::Colour accent = palette::bloodBright)
            : button(labelText), attachment(vts, paramID, &button)
        {
            button.setClickingTogglesState(true);
            button.setColour(juce::TextButton::buttonColourId, palette::panelBg2);
            button.setColour(juce::TextButton::buttonOnColourId, palette::blood);
            button.setColour(juce::TextButton::textColourOffId, palette::text);
            button.setColour(juce::TextButton::textColourOnId, palette::textBright);
            addAndMakeVisible(button);
        }

        void resized() override
        {
            button.setBounds(getLocalBounds().reduced(1, 1));
        }

    private:
        juce::TextButton button;
        juce::AudioProcessorValueTreeState::ButtonAttachment attachment;
    };
}
