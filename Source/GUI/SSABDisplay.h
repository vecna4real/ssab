#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"
#include "SSABFonts.h"
#include "../Constants.h"
#include "../Presets.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSABDisplay: the "LCD-like" display showing the current preset
    // name and version. Draws chunky border + blinking cursor.
    // ------------------------------------------------------------------
    class SSABDisplay : public juce::Component,
                       public juce::Timer
    {
    public:
        SSABDisplay(juce::AudioProcessorValueTreeState& vts)
            : apvts(vts)
        {
            startTimerHz(4);
        }

        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(1.0f);

            // background: sickly dark green like an old LCD
            g.setColour(juce::Colour(0xff0a1a0a));
            g.fillRect(b);

            // chunky border
            g.setColour(palette::acid);
            g.drawRect(b, 2.0f);

            // scanline effect
            g.setColour(juce::Colour(0xff142814).withAlpha(0.5f));
            for (int y = 0; y < getHeight(); y += 3)
                g.drawHorizontalLine(y, b.getX(), b.getRight());

            // preset name
            int slot = int(std::round(
                apvts.getRawParameterValue(pid::presetSlot)->load()
                * float(kNumPresets - 1)));
            const juce::String name = presetName(slot);

            g.setColour(palette::acid);
            g.setFont(SSABFonts::get().getDisplayFont().withHeight(22.0f));
            g.drawText(name, b.reduced(8.0f, 4.0f),
                       juce::Justification::centredLeft);

            // blinking cursor
            if (blinkOn)
            {
                g.setColour(palette::acid);
                g.fillRect(b.getRight() - 14.0f, b.getBottom() - 18.0f, 8.0f, 12.0f);
            }

            // small text in top-right
            g.setFont(SSABFonts::get().getSmallFont());
            g.setColour(palette::hazard);
            g.drawText("SSAB v1.0 // NOISE LAB", b.reduced(6.0f, 3.0f),
                       juce::Justification::topRight);
        }

        void timerCallback() override
        {
            blinkOn = !blinkOn;
            repaint();
        }

    private:
        juce::AudioProcessorValueTreeState& apvts;
        bool blinkOn = true;
    };
}
