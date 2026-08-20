#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ssab
{
using namespace juce;

// ------------------------------------------------------------------
// Helper: read APVTS param as float (raw 0..1 -> converted)
// ------------------------------------------------------------------
static float getRaw(APVTS& s, const String& id)
{
    if (auto* p = s.getRawParameterValue(id)) return p->load();
    return 0.0f;
}
static int getChoiceIndex(APVTS& s, const String& id)
{
    if (auto* p = s.getParameter(id))
    {
        const int steps = p->getNumSteps();
        if (steps <= 1) return 0;
        return int(std::round(getRaw(s, id) * float(steps - 1)));
    }
    return 0;
}

SSABProcessor::SSABProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input",  AudioChannelSet::stereo(), false)
        .withOutput("Output", AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, JucePlugin_Name, buildParameterLayout)
{
    // Apply initial preset
    int slot = int(std::round(getRaw(apvts, pid::presetSlot) * float(kNumPresets - 1)));
    applyPresetToState(apvts, slot);
}

void SSABProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (auto& v : voices) v.prepare(sampleRate);
    cachedParams = buildParamsFromState();
    brickMax = 1.0f;
    lastLeftGain = 0.0f;
    lastRightGain = 0.0f;
    masterDC_L.reset();
    masterDC_R.reset();
}

void SSABProcessor::releaseResources()
{
    for (auto& v : voices) v.reset();
}

bool SSABProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& main = layouts.getMainOutputChannelSet();
    if (main != AudioChannelSet::stereo() && main != AudioChannelSet::mono())
        return false;
    return true;
}

VoiceParams SSABProcessor::buildParamsFromState() const
{
    VoiceParams p{};

    // THE CORE
    p.waveA     = (WaveType)getChoiceIndex(apvts, pid::waveA);
    p.waveB     = (WaveType)getChoiceIndex(apvts, pid::waveB);
    p.detune    = getRaw(apvts, pid::detune) * 200.0f - 100.0f;
    p.octave    = getChoiceIndex(apvts, pid::octave);
    p.fmAmount  = getRaw(apvts, pid::fmAmount);
    p.subLevel  = getRaw(apvts, pid::subLevel);
    p.sync      = getRaw(apvts, pid::sync) > 0.5f;

    // ACID BATH
    p.cutoff     = apvts.getRawParameterValue(pid::cutoff)->load() * (18000.0f - 20.0f) + 20.0f;
    p.resonance  = getRaw(apvts, pid::resonance);
    p.filterType = (FilterType)getChoiceIndex(apvts, pid::filterType);
    p.envMod     = getRaw(apvts, pid::envMod) * 2.0f - 1.0f;
    p.keytrack   = getRaw(apvts, pid::keytrack) * 2.0f - 1.0f;

    // CARNAGE
    p.drive       = getRaw(apvts, pid::drive);
    p.distType    = (DistType)getChoiceIndex(apvts, pid::distType);
    p.bitcrush    = getRaw(apvts, pid::bitcrush);
    p.subProtect  = getRaw(apvts, pid::subProtect);

    // IMPACT
    p.attack   = apvts.getRawParameterValue(pid::attack)->load()  * (200.0f - 0.1f) + 0.1f;
    p.decay    = apvts.getRawParameterValue(pid::decay)->load()   * (2000.0f - 1.0f) + 1.0f;
    p.sustain  = getRaw(apvts, pid::sustain);
    p.release  = apvts.getRawParameterValue(pid::release)->load() * (4000.0f - 1.0f) + 1.0f;
    p.punch    = getRaw(apvts, pid::punch);

    // LFO
    p.lfoRate   = apvts.getRawParameterValue(pid::lfoRate)->load() * (30.0f - 0.05f) + 0.05f;
    p.lfoDepth  = getRaw(apvts, pid::lfoDepth);
    p.lfoTarget = (LFOTarget)getChoiceIndex(apvts, pid::lfoTarget);

    // MASSACRE
    p.massacre1994 = getRaw(apvts, pid::massacre1994) > 0.5f;
    p.ruiner       = getRaw(apvts, pid::ruiner);
    p.feedback     = getRaw(apvts, pid::feedback) * 0.99f;

    // DOOM
    p.volumeDb = apvts.getRawParameterValue(pid::volume)->load() * (6.0f - (-60.0f)) + (-60.0f);

    return p;
}

void SSABProcessor::handleMidi(const MidiBuffer& midi, int numSamples)
{
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            // steal oldest voice if needed
            int v = voiceRoundRobin;
            for (int i = 0; i < kNumVoices; ++i)
            {
                int idx = (voiceRoundRobin + i) % kNumVoices;
                if (!voices[idx].isActive()) { v = idx; break; }
            }
            voices[v].noteOn(msg.getNoteNumber(), msg.getVelocity() / 127.0f, cachedParams);
            voiceRoundRobin = (v + 1) % kNumVoices;
        }
        else if (msg.isNoteOff())
        {
            for (auto& voice : voices)
                if (voice.isActive() && voice.currentNote() == msg.getNoteNumber())
                    voice.noteOff();
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            for (auto& voice : voices) { voice.noteOff(); voice.markInactive(); }
        }
    }
}

void SSABProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Update cached params at the start of each block.
    cachedParams = buildParamsFromState();

    // Handle preset changes
    const int newSlot = int(std::round(getRaw(apvts, pid::presetSlot) * float(kNumPresets - 1)));
    if (newSlot != currentPresetSlot)
    {
        currentPresetSlot = newSlot;
    }

    // Update voice params continuously
    for (auto& v : voices)
        v.updateParams(cachedParams);

    handleMidi(midi, buffer.getNumSamples());

    // Render
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer(0);
    auto* right = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    // DOOM width (1 = mono->stereo widening; 0 = mono)
    const float width = getRaw(apvts, pid::width);
    const float volumeDb = cachedParams.volumeDb;
    const float gain = juce::Decibels::decibelsToGain(volumeDb);
    const bool brickwall = getRaw(apvts, pid::brickwall) > 0.5f;

    for (int s = 0; s < numSamples; ++s)
    {
        float L = 0.0f, R = 0.0f;
        for (auto& v : voices)
        {
            if (!v.isActive()) continue;
            auto out = v.process(cachedParams);
            L += out.first;
            R += out.second;
        }

        // Stereo width: simple mid/side widening.
        // (L == R from voice, so widening creates artificial stereo via small phase trick.)
        if (width > 0.0f)
        {
            const float mid  = (L + R) * 0.5f;
            const float side = (L - R) * 0.5f * (1.0f + width * 3.0f);
            L = mid + side;
            R = mid - side;
        }

        // Output gain
        L *= gain;
        R *= gain;

        // Master DC blocker (one-pole, R=0.995)
        L = masterDC_L.process(L, 0.995f);
        R = masterDC_R.process(R, 0.995f);

        // Brickwall limiter (very simple lookahead-free hard limiter)
        if (brickwall)
        {
            if (L > 1.0f) L = 1.0f;
            else if (L < -1.0f) L = -1.0f;
            if (R > 1.0f) R = 1.0f;
            else if (R < -1.0f) R = -1.0f;
        }

        left[s] = L;
        if (right) right[s] = R;
    }
}

int SSABProcessor::getNumPrograms()    { return kNumPresets; }
int SSABProcessor::getCurrentProgram() { return currentPresetSlot; }
void SSABProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= kNumPresets) return;
    currentPresetSlot = index;
    applyPresetToState(apvts, index);
}
const String SSABProcessor::getProgramName(int index)
{
    return presetName(index);
}

void SSABProcessor::getStateInformation(MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    copyXmlToBinary(*xml, destData);
}

void SSABProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(ValueTree::fromXml(*xml));
        cachedParams = buildParamsFromState();
        currentPresetSlot = int(std::round(getRaw(apvts, pid::presetSlot) * float(kNumPresets - 1)));
    }
}

AudioProcessorEditor* SSABProcessor::createEditor()
{
    return new SSABEditor(*this);
}

// ------------------------------------------------------------------
// JUCE plugin entry points
// ------------------------------------------------------------------
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SSABProcessor();
}
}
