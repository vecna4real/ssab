#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ssab
{
    // ------------------------------------------------------------------
    // The "incredibly disgusting early 00s" palette.
    // Inspired by old Nuendo/Sonic Syndicate/early FL Studio skins:
    // aggressive neon green, hazard yellow, puke orange, blood red,
    // bruised purple, with a metallic background and sickly glow.
    // ------------------------------------------------------------------
    namespace palette
    {
        // Backgrounds (deliberately clashing)
        const juce::Colour panelBg       = juce::Colour(0xff1a1814);  // almost-black brown
        const juce::Colour panelBg2      = juce::Colour(0xff261f17);  // slightly lighter
        const juce::Colour panelEdge     = juce::Colour(0xff4a3a22);  // brown edge
        const juce::Colour panelEdgeHi   = juce::Colour(0xff8c6f3e);  // tan highlight

        // Section background colours (one sickly colour per block)
        const juce::Colour coreBg        = juce::Colour(0xff3a0f0f);  // dark red - "THE CORE"
        const juce::Colour acidBg        = juce::Colour(0xff0f2a1a);  // sick green - "ACID BATH"
        const juce::Colour carnageBg     = juce::Colour(0xff2a1f0a);  // brown-yellow - "CARNAGE"
        const juce::Colour impactBg      = juce::Colour(0xff0f0f3a);  // bruised blue - "IMPACT"
        const juce::Colour lfoBg         = juce::Colour(0xff2a0a2a);  // dark purple - "LFO"
        const juce::Colour doomBg        = juce::Colour(0xff000000);  // black - "DOOM"
        const juce::Colour massacreBg   = juce::Colour(0xff3a0a0a);  // blood - "MASSACRE"

        // Accents
        const juce::Colour acid         = juce::Colour(0xff7fff00);  // vomit-neon green
        const juce::Colour hazard       = juce::Colour(0xffffcc00);  // hazard yellow
        const juce::Colour hazardDark   = juce::Colour(0xffaa8800);
        const juce::Colour puke         = juce::Colour(0xffc8a000);  // puke gold
        const juce::Colour blood        = juce::Colour(0xffaa0000);  // blood red
        const juce::Colour bloodBright  = juce::Colour(0xffff3030);
        const juce::Colour bruise       = juce::Colour(0xff7a3aa0);  // bruised purple
        const juce::Colour sick         = juce::Colour(0xff9fc44a);  // sick moss green

        // Text
        const juce::Colour text         = juce::Colour(0xffd0d0a0);  // sickly pale yellow-green
        const juce::Colour textDark     = juce::Colour(0xff1a1a10);
        const juce::Colour textBright   = juce::Colour(0xffffffff);
        const juce::Colour textShadow   = juce::Colour(0xff000000);

        // Knob disc colours (rotate per section for max chaos)
        const juce::Colour knobBody    = juce::Colour(0xff1a1a1a);
        const juce::Colour knobRing    = juce::Colour(0xff7fff00);
        const juce::Colour knobPointer  = juce::Colour(0xffff3030);
    }
}
