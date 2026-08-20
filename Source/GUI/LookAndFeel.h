#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ColorPalette.h"
#include "SSABFonts.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // The LookAndFeel for SSAB - aggressive early-00s style.
    // Knobs are chunky with an inner acid-green ring and a red pointer.
    // ------------------------------------------------------------------
    class SSABLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        SSABLookAndFeel()
        {
            setColour(juce::Slider::textBoxTextColourId,  palette::textBright);
            setColour(juce::Slider::textBoxBackgroundColourId, palette::panelBg);
            setColour(juce::Slider::textBoxOutlineColourId, palette::blood);
            setColour(juce::Slider::textBoxHighlightColourId, palette::acid);
            setColour(juce::Label::textColourId, palette::text);
            setColour(juce::Label::textWhenEditingColourId, palette::textBright);
            setColour(juce::ComboBox::backgroundColourId, palette::panelBg2);
            setColour(juce::ComboBox::outlineColourId, palette::blood);
            setColour(juce::ComboBox::textColourId, palette::text);
            setColour(juce::PopupMenu::backgroundColourId, palette::panelBg);
            setColour(juce::PopupMenu::textColourId, palette::text);
            setColour(juce::PopupMenu::highlightedBackgroundColourId, palette::blood);
            setColour(juce::TextButton::buttonColourId, palette::panelBg2);
            setColour(juce::TextButton::buttonOnColourId, palette::blood);
            setColour(juce::TextButton::textColourOffId, palette::text);
            setColour(juce::TextButton::textColourOnId, palette::textBright);
        }

        void drawSliderThumb(juce::Graphics& g, const juce::Slider& s, float radius) override
        {
            // intentionally empty - we override drawLinearSlider / drawRotarySlider
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float pos, float startAngle, float endAngle,
                              const juce::Slider& slider) override
        {
            const auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2.0f);
            const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
            const float centreX = bounds.getCentreX();
            const float centreY = bounds.getCentreY();
            const float angle = startAngle + pos * (endAngle - startAngle);

            // outer ring background
            juce::Path outerArc;
            outerArc.addCentredArc(centreX, centreY, radius, radius,
                0.0f, startAngle, endAngle, true);
            g.setColour(palette::panelBg2);
            g.strokePath(outerArc, juce::PathStrokeType(radius * 0.18f));

            // value arc (acid green)
            juce::Path valueArc;
            valueArc.addCentredArc(centreX, centreY, radius, radius,
                0.0f, startAngle, angle, true);
            g.setColour(palette::acid);
            g.strokePath(valueArc, juce::PathStrokeType(radius * 0.16f));

            // knob body (chunky disc)
            const float innerR = radius * 0.7f;
            g.setColour(palette::knobBody);
            g.fillEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f);

            // knob edge (dark brown)
            g.setColour(palette::panelEdge);
            g.drawEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f, 1.5f);

            // pointer (red line)
            const float pointerLen = innerR * 0.95f;
            const float px = centreX + std::sin(angle) * pointerLen;
            const float py = centreY - std::cos(angle) * pointerLen;
            g.setColour(palette::knobPointer);
            g.drawLine(centreX, centreY, px, py, 3.0f);

            // glow on pointer end
            g.setColour(palette::bloodBright.withAlpha(0.6f));
            g.fillEllipse(px - 2.5f, py - 2.5f, 5.0f, 5.0f);

            // small center dot
            g.setColour(palette::hazard);
            g.fillEllipse(centreX - 1.5f, centreY - 1.5f, 3.0f, 3.0f);
        }

        void drawLabel(juce::Graphics& g, juce::Label& l) override
        {
            g.fillAll(juce::Colours::transparentBlack);
            auto bounds = l.getLocalBounds().toFloat().reduced(1.0f);
            g.setColour(palette::textShadow);
            auto font = l.getFont();
            g.setFont(font);
            g.drawText(l.getText(), bounds.translated(1.0f, 1.0f), l.getJustificationType());
            g.setColour(l.findColour(juce::Label::textColourId));
            g.drawText(l.getText(), bounds, l.getJustificationType());
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool /*isDown*/,
                          int /*x*/, int /*y*/, int /*w*/, int /*h*/,
                          const juce::ComboBox& cb) override
        {
            auto bounds = cb.getLocalBounds().toFloat().reduced(1.0f);
            g.setColour(palette::panelBg2);
            g.fillRect(bounds);
            g.setColour(palette::blood);
            g.drawRect(bounds, 1.5f);

            g.setColour(palette::text);
            g.setFont(SSABFonts::get().getLabelFont());
            g.drawText(cb.getText(), bounds.reduced(4.0f, 0.0f),
                juce::Justification::centredLeft);

            // arrow
            auto ar = bounds.removeFromRight(20.0f).reduced(4.0f);
            juce::Path p;
            p.addTriangle(ar.getX(), ar.getY(),
                          ar.getRight(), ar.getY(),
                          ar.getCentreX(), ar.getBottom());
            g.setColour(palette::acid);
            g.fillPath(p);
        }

        juce::Font getLabelFont(juce::Label& /*l*/) override
        {
            return SSABFonts::get().getLabelFont();
        }

        juce::Font getComboBoxFont(juce::ComboBox& /*c*/) override
        {
            return SSABFonts::get().getLabelFont();
        }

        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
            const juce::Colour& /*bg*/, bool /*over*/, bool down) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
            bool on = button.getToggleState();
            g.setColour(on ? palette::blood : palette::panelBg2);
            g.fillRect(bounds);
            g.setColour(on ? palette::bloodBright : palette::panelEdge);
            g.drawRect(bounds, 2.0f);

            if (down)
            {
                g.setColour(palette::textShadow.withAlpha(0.5f));
                g.fillRect(bounds);
            }
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& button,
            bool /*over*/, bool /*down*/) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
            g.setColour(button.getToggleState() ? palette::textBright : palette::text);
            g.setFont(SSABFonts::get().getLabelFont());
            g.drawText(button.getButtonText(), bounds,
                juce::Justification::centred);
        }
    };
}
