#include "PluginEditor.h"
#include "Presets.h"

namespace ssab
{
using namespace juce;

SSABEditor::SSABEditor(SSABProcessor& p)
    : AudioProcessorEditor(&p), processor(p), display(p.apvts)
{
    setLookAndFeel(&lnf);

    setSize(1100, 720);
    setResizable(false, false);

    // ----- HEADER: display + prev/next buttons -----
    addAndMakeVisible(display);
    prevPreset.setButtonText("< PREV");
    nextPreset.setButtonText("NEXT >");
    addAndMakeVisible(prevPreset);
    addAndMakeVisible(nextPreset);

    prevPreset.onClick = [this]()
    {
        int slot = int(std::round(
            processor.apvts.getRawParameterValue(pid::presetSlot)->load()
            * float(kNumPresets - 1)));
        slot = juce::jlimit(0, kNumPresets - 1, slot - 1);
        if (auto* p = processor.apvts.getParameter(pid::presetSlot))
        {
            const float nv = float(slot) / float(kNumPresets - 1);
            p->setValueNotifyingHost(nv);
        }
        applyPresetToState(processor.apvts, slot);
    };
    nextPreset.onClick = [this]()
    {
        int slot = int(std::round(
            processor.apvts.getRawParameterValue(pid::presetSlot)->load()
            * float(kNumPresets - 1)));
        slot = juce::jlimit(0, kNumPresets - 1, slot + 1);
        if (auto* p = processor.apvts.getParameter(pid::presetSlot))
        {
            const float nv = float(slot) / float(kNumPresets - 1);
            p->setValueNotifyingHost(nv);
        }
        applyPresetToState(processor.apvts, slot);
    };

    // ----- THE CORE -----
    waveA    = std::make_unique<SSABChoice>("WAVE A",  processor.apvts, pid::waveA,    palette::bloodBright);
    waveB    = std::make_unique<SSABChoice>("WAVE B",  processor.apvts, pid::waveB,    palette::bloodBright);
    octave   = std::make_unique<SSABChoice>("OCTAVE",   processor.apvts, pid::octave,   palette::bloodBright);
    detune   = std::make_unique<SSABKnob>("DETUNE",     processor.apvts, pid::detune,    palette::bloodBright);
    fmAmount = std::make_unique<SSABKnob>("FM AMOUNT", processor.apvts, pid::fmAmount, palette::bloodBright);
    subLevel = std::make_unique<SSABKnob>("SUB LEVEL", processor.apvts, pid::subLevel, palette::bloodBright);
    sync     = std::make_unique<SSABToggle>("SYNC",    processor.apvts, pid::sync,      palette::bloodBright);
    for (auto* c : { (Component*)waveA.get(), waveB.get(), octave.get(),
                      detune.get(), fmAmount.get(), subLevel.get(), sync.get() })
        addAndMakeVisible(c);

    // ----- ACID BATH -----
    cutoff     = std::make_unique<SSABKnob>("CUTOFF",    processor.apvts, pid::cutoff,     palette::acid);
    resonance = std::make_unique<SSABKnob>("RESONANCE",processor.apvts, pid::resonance, palette::acid);
    envMod    = std::make_unique<SSABKnob>("ENV MOD",   processor.apvts, pid::envMod,     palette::acid);
    keytrack   = std::make_unique<SSABKnob>("KEYTRACK", processor.apvts, pid::keytrack, palette::acid);
    filterType = std::make_unique<SSABChoice>("FILT TYPE", processor.apvts, pid::filterType, palette::acid);
    for (auto* c : { (Component*)cutoff.get(), resonance.get(), envMod.get(),
                      keytrack.get(), filterType.get() })
        addAndMakeVisible(c);

    // ----- CARNAGE -----
    drive      = std::make_unique<SSABKnob>("DRIVE",      processor.apvts, pid::drive,      palette::hazard);
    bitcrush   = std::make_unique<SSABKnob>("BITCRUSH",   processor.apvts, pid::bitcrush,   palette::hazard);
    subProtect = std::make_unique<SSABKnob>("SUB PROTECT",processor.apvts, pid::subProtect, palette::hazard);
    distType   = std::make_unique<SSABChoice>("DIST TYPE", processor.apvts, pid::distType, palette::hazard);
    for (auto* c : { (Component*)drive.get(), bitcrush.get(), subProtect.get(), distType.get() })
        addAndMakeVisible(c);

    // ----- IMPACT -----
    attack  = std::make_unique<SSABKnob>("ATTACK",  processor.apvts, pid::attack,  palette::bruise);
    decay   = std::make_unique<SSABKnob>("DECAY",   processor.apvts, pid::decay,   palette::bruise);
    sustain = std::make_unique<SSABKnob>("SUSTAIN", processor.apvts, pid::sustain, palette::bruise);
    release = std::make_unique<SSABKnob>("RELEASE", processor.apvts, pid::release, palette::bruise);
    punch   = std::make_unique<SSABKnob>("PUNCH",   processor.apvts, pid::punch,   palette::bruise);
    for (auto* c : { (Component*)attack.get(), decay.get(), sustain.get(), release.get(), punch.get() })
        addAndMakeVisible(c);

    // ----- LFO -----
    lfoRate   = std::make_unique<SSABKnob>("RATE",    processor.apvts, pid::lfoRate, palette::sick);
    lfoDepth  = std::make_unique<SSABKnob>("DEPTH",   processor.apvts, pid::lfoDepth, palette::sick);
    lfoTarget = std::make_unique<SSABChoice>("TARGET", processor.apvts, pid::lfoTarget, palette::sick);
    for (auto* c : { (Component*)lfoRate.get(), lfoDepth.get(), lfoTarget.get() })
        addAndMakeVisible(c);

    // ----- DOOM -----
    width     = std::make_unique<SSABKnob>("WIDTH",    processor.apvts, pid::width,     palette::hazard);
    volume    = std::make_unique<SSABKnob>("VOLUME",   processor.apvts, pid::volume,    palette::hazard);
    brickwall = std::make_unique<SSABToggle>("BRICKWALL", processor.apvts, pid::brickwall, palette::hazard);
    for (auto* c : { (Component*)width.get(), volume.get(), brickwall.get() })
        addAndMakeVisible(c);

    // ----- MASSACRE -----
    massacre1994 = std::make_unique<SSABToggle>("1994 SWITCH", processor.apvts, pid::massacre1994, palette::bloodBright);
    ruiner      = std::make_unique<SSABKnob>("RUINER",   processor.apvts, pid::ruiner, palette::bloodBright);
    feedback    = std::make_unique<SSABKnob>("FEEDBACK", processor.apvts, pid::feedback, palette::bloodBright);
    for (auto* c : { (Component*)massacre1994.get(), ruiner.get(), feedback.get() })
        addAndMakeVisible(c);
}

SSABEditor::~SSABEditor()
{
    setLookAndFeel(nullptr);
}

void SSABEditor::paintHazardStripes(Graphics& g, juce::Rectangle<int> bounds)
{
    const int stripeW = 12;
    g.setColour(palette::hazard);
    for (int x = bounds.getX(); x < bounds.getRight(); x += stripeW * 2)
    {
        juce::Path p;
        p.startNewSubPath(x, bounds.getY());
        p.lineTo(x + stripeW, bounds.getY());
        p.lineTo(x + stripeW - bounds.getHeight(), bounds.getBottom());
        p.lineTo(x - bounds.getHeight(), bounds.getBottom());
        p.closeSubPath();
        g.setColour(palette::hazard);
        g.fillPath(p);
    }
}

void SSABEditor::paintSection(Graphics& g, juce::Rectangle<int> bounds,
                              const String& title, Colour bg, Colour accent)
{
    auto f = bounds.toFloat();
    // background
    g.setColour(bg);
    g.fillRect(f);
    // header strip
    auto header = f.removeFromTop(22.0f);
    g.setColour(accent);
    g.fillRect(header);
    // title
    g.setColour(palette::textDark);
    g.setFont(SSABFonts::get().getDisplayFont().withHeight(18.0f));
    g.drawText(title, header.reduced(4.0f, 0.0f), Justification::centredLeft);
    // outer border
    g.setColour(accent);
    g.drawRect(f.expanded(1.0f), 1.5f);
    // inner border
    g.setColour(palette::panelEdge);
    g.drawRect(f, 1.0f);
}

void SSABEditor::paintHeader(Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // main bg gradient (dark brown to almost-black)
    juce::ColourGradient grad(b.getTopLeft(), palette::panelBg,
                              b.getBottomLeft(), palette::panelBg2, false);
    g.setGradientFill(grad);
    g.fillRect(b);

    // hazard stripes across the very top
    paintHazardStripes(g, getLocalBounds().removeFromTop(8));

    // hazard stripes across the very bottom
    paintHazardStripes(g, getLocalBounds().removeFromBottom(8));
}

void SSABEditor::paint(Graphics& g)
{
    paintHeader(g);

    // Background "noise" texture: random small dots for griminess
    juce::Random rng(0x5EED);
    g.setColour(palette::panelEdge.withAlpha(0.15f));
    for (int i = 0; i < 400; ++i)
    {
        const int x = rng.nextInt(getWidth());
        const int y = rng.nextInt(getHeight());
        g.setPixel(x, y);
    }

    // Draw the section blocks behind the components
    paintSection(g, juce::Rectangle<int>(12, 50, 280, 280),
                 "THE CORE", palette::coreBg, palette::bloodBright);
    paintSection(g, juce::Rectangle<int>(300, 50, 280, 280),
                 "ACID BATH", palette::acidBg, palette::acid);
    paintSection(g, juce::Rectangle<int>(588, 50, 250, 280),
                 "CARNAGE", palette::carnageBg, palette::hazard);
    paintSection(g, juce::Rectangle<int>(846, 50, 240, 280),
                 "IMPACT", palette::impactBg, palette::bruise);

    paintSection(g, juce::Rectangle<int>(12, 340, 200, 280),
                 "LFO", palette::lfoBg, palette::sick);
    paintSection(g, juce::Rectangle<int>(220, 340, 200, 280),
                 "DOOM", palette::doomBg, palette::hazard);
    paintSection(g, juce::Rectangle<int>(428, 340, 250, 280),
                 "MASSACRE", palette::massacreBg, palette::bloodBright);

    // Big "SSAB" graffiti logo in the bottom-right empty area
    auto logoArea = juce::Rectangle<int>(700, 360, 380, 240);
    g.setColour(palette::blood.withAlpha(0.18f));
    g.fillRect(logoArea.toFloat());
    g.setColour(palette::bloodBright);
    g.setFont(SSABFonts::get().getDisplayFont().withHeight(120.0f));
    g.drawText("SSAB", logoArea.toFloat().reduced(8.0f),
               Justification::centred);

    // Tagline
    g.setColour(palette::text);
    g.setFont(SSABFonts::get().getSmallFont().withHeight(14.0f));
    g.drawText("EXTREME BASS SYNTHESIZER",
               juce::Rectangle<int>(700, 590, 380, 20).toFloat(),
               Justification::centred);

    // Disclaimer text (very 00s-plugin style)
    g.setColour(palette::hazard.withAlpha(0.6f));
    g.setFont(SSABFonts::get().getSmallFont().withHeight(9.0f));
    g.drawText("NO LICENSE. NO REFUND. NO FUTURE. - SSAB NOISE LAB 2026",
               juce::Rectangle<int>(700, 612, 380, 14).toFloat(),
               Justification::centred);
}

// Helper: lay out — handled inline in resized() for max control.

void SSABEditor::resized()
{
    auto bounds = getLocalBounds();
    // Header area
    auto header = bounds.removeFromTop(36);
    header.removeFromLeft(8);
    prevPreset.setBounds(header.removeFromLeft(80).reduced(4));
    nextPreset.setBounds(header.removeFromLeft(80).reduced(4));
    display.setBounds(header.reduced(4));

    // THE CORE section content
    {
        auto core = juce::Rectangle<int>(12, 50, 280, 280).reduced(8, 28);
        // Two rows of three controls, plus a toggle on the bottom
        auto row1 = core.removeFromTop(110);
        auto row2 = core.removeFromTop(110);
        auto row3 = core;

        auto row1L = row1.removeFromLeft(core.getWidth() / 3);
        auto row1M = row1.removeFromLeft(core.getWidth() / 3);
        auto row1R = row1;
        waveA->setBounds(row1L.reduced(4));
        waveB->setBounds(row1M.reduced(4));
        octave->setBounds(row1R.reduced(4));

        auto row2L = row2.removeFromLeft(core.getWidth() / 3);
        auto row2M = row2.removeFromLeft(core.getWidth() / 3);
        auto row2R = row2;
        detune->setBounds(row2L.reduced(4));
        fmAmount->setBounds(row2M.reduced(4));
        subLevel->setBounds(row2R.reduced(4));

        sync->setBounds(row3.reduced(4, 4));
    }

    // ACID BATH section content
    {
        auto acid = juce::Rectangle<int>(300, 50, 280, 280).reduced(8, 28);
        auto row1 = acid.removeFromTop(120);
        auto row2 = acid;

        auto r1L = row1.removeFromLeft(acid.getWidth() / 2);
        auto r1R = row1;
        cutoff->setBounds(r1L.reduced(4));
        resonance->setBounds(r1R.reduced(4));

        auto r2L = row2.removeFromLeft(acid.getWidth() / 3);
        auto r2M = row2.removeFromLeft(acid.getWidth() / 3);
        auto r2R = row2;
        envMod->setBounds(r2L.reduced(4));
        keytrack->setBounds(r2M.reduced(4));
        filterType->setBounds(r2R.reduced(4));
    }

    // CARNAGE section
    {
        auto carn = juce::Rectangle<int>(588, 50, 250, 280).reduced(8, 28);
        auto row1 = carn.removeFromTop(120);
        auto row2 = carn;
        auto r1L = row1.removeFromLeft(carn.getWidth() / 2);
        auto r1R = row1;
        drive->setBounds(r1L.reduced(4));
        distType->setBounds(r1R.reduced(4));

        auto r2L = row2.removeFromLeft(carn.getWidth() / 2);
        auto r2R = row2;
        bitcrush->setBounds(r2L.reduced(4));
        subProtect->setBounds(r2R.reduced(4));
    }

    // IMPACT section
    {
        auto imp = juce::Rectangle<int>(846, 50, 240, 280).reduced(8, 28);
        auto r1 = imp.removeFromTop(120);
        auto r2 = imp;
        auto r1L = r1.removeFromLeft(r1.getWidth() / 2);
        auto r1R = r1;
        attack->setBounds(r1L.reduced(4));
        decay->setBounds(r1R.reduced(4));

        auto r2L = r2.removeFromLeft(r2.getWidth() / 3);
        auto r2M = r2.removeFromLeft(r2.getWidth() / 3);
        auto r2R = r2;
        sustain->setBounds(r2L.reduced(4));
        release->setBounds(r2M.reduced(4));
        punch->setBounds(r2R.reduced(4));
    }

    // LFO section
    {
        auto l = juce::Rectangle<int>(12, 340, 200, 280).reduced(8, 28);
        auto r1 = l.removeFromTop(120);
        auto r2 = l;
        lfoRate->setBounds(r1.reduced(4));
        lfoDepth->setBounds(r2.removeFromLeft(l.getWidth() / 2).reduced(4));
        lfoTarget->setBounds(r2.reduced(4));
    }

    // DOOM section
    {
        auto d = juce::Rectangle<int>(220, 340, 200, 280).reduced(8, 28);
        auto r1 = d.removeFromTop(120);
        auto r2 = d;
        width->setBounds(r1.reduced(4));
        volume->setBounds(r2.removeFromLeft(d.getWidth() / 2).reduced(4));
        brickwall->setBounds(r2.reduced(4, 4));
    }

    // MASSACRE section
    {
        auto m = juce::Rectangle<int>(428, 340, 250, 280).reduced(8, 28);
        auto r1 = m.removeFromTop(120);
        auto r2 = m;
        massacre1994->setBounds(r1.reduced(4));
        auto r2L = r2.removeFromLeft(r2.getWidth() / 2);
        auto r2R = r2;
        ruiner->setBounds(r2L.reduced(4));
        feedback->setBounds(r2R.reduced(4));
    }
}
}
