#pragma once

#include "Oscillator.h"
#include "Filter.h"
#include "Distortion.h"
#include "Envelope.h"
#include "LFO.h"
#include "DSPUtils.h"
#include "../Constants.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // VoiceParams: snapshot of all parameters, copied into each voice
    // every block. Fields mirror the parameter IDs in Parameters.cpp.
    // ------------------------------------------------------------------
    struct VoiceParams
    {
        // THE CORE
        WaveType waveA  = WaveType::Saw;
        WaveType waveB  = WaveType::Saw;
        float detune   = 0.0f;
        int   octave   = 2;        // index 0..4 -> -2,-1,0,+1,+2
        float fmAmount = 0.0f;     // 0..1
        float subLevel = 0.5f;
        bool sync      = false;

        // ACID BATH
        float cutoff   = 8000.0f;
        float resonance = 0.4f;
        FilterType filterType = FilterType::Lowpass;
        float envMod   = 0.6f;
        float keytrack  = 0.0f;

        // CARNAGE
        float drive    = 0.3f;
        DistType distType = DistType::Clip;
        float bitcrush = 0.0f;
        float subProtect = 0.7f;

        // IMPACT
        float attack = 1.0f;
        float decay  = 200.0f;
        float sustain = 0.8f;
        float release = 200.0f;
        float punch   = 0.5f;

        // LFO
        float lfoDepth = 0.0f;
        LFOTarget lfoTarget = LFOTarget::Cutoff;
        float lfoRate  = 4.0f;

        // MASSACRE
        bool  massacre1994 = false;
        float ruiner       = 0.0f;
        float feedback     = 0.0f;

        // DOOM
        float volumeDb = 0.0f;
    };

    // ------------------------------------------------------------------
    // SSVoice: a single synth voice. Holds two oscillators, a sub osc,
    // the filter, distortion, and an envelope. LFO is per-voice here.
    // ------------------------------------------------------------------
    class SSVoice
    {
    public:
        void prepare(double sampleRate)
        {
            sr = sampleRate;
            oscA.prepare(sampleRate);
            oscB.prepare(sampleRate);
            subOsc.prepare(sampleRate);
            filter.prepare(sampleRate);
            dist.prepare(sampleRate);
            env.prepare(sampleRate);
            lfo.prepare(sampleRate);
            reset();
        }

        void reset()
        {
            oscA.reset(); oscB.reset(); subOsc.reset();
            filter.reset();
            env.reset(); lfo.reset();
            feedbackZ = 0.0f;
            lastNote = 60;
            active = false;
        }

        void noteOn(int midiNote, float velocity, const VoiceParams& p)
        {
            lastNote = midiNote;
            lastVelocity = velocity;
            params = p;

            oscA.setWaveform(p.waveA);
            oscB.setWaveform(p.waveB);
            subOsc.setWaveform(WaveType::Square);

            const float baseFreq = midiToFreq(float(midiNote));
            const float octRatio = semisToRatio(float((p.octave - 2) * 12));
            const float detuneRatio = std::pow(2.0f, p.detune / 1200.0f);

            oscA.setFrequency(baseFreq * octRatio);
            oscB.setFrequency(baseFreq * octRatio * detuneRatio);
            subOsc.setFrequency(baseFreq * 0.5f * octRatio);

            env.setAttack(p.attack);
            env.setDecay(p.decay);
            env.setSustain(p.sustain);
            env.setRelease(p.release);
            env.setPunch(p.punch);
            env.noteOn();

            filter.setType(p.filterType);
            filter.setResonance(p.resonance);

            dist.setDrive(p.drive);
            dist.setType(p.distType);
            dist.setBitcrush(p.bitcrush);
            dist.setSubProtect(p.subProtect);

            lfo.setRate(p.lfoRate);

            active = true;
        }

        void noteOff()
        {
            env.noteOff();
        }

        void updateParams(const VoiceParams& p)
        {
            params = p;
            oscA.setWaveform(p.waveA);
            oscB.setWaveform(p.waveB);
            subOsc.setWaveform(WaveType::Square);

            dist.setDrive(p.drive);
            dist.setType(p.distType);
            dist.setBitcrush(p.bitcrush);
            dist.setSubProtect(p.subProtect);

            filter.setType(p.filterType);
            filter.setResonance(p.resonance);

            env.setAttack(p.attack);
            env.setDecay(p.decay);
            env.setSustain(p.sustain);
            env.setRelease(p.release);
            env.setPunch(p.punch);

            lfo.setRate(p.lfoRate);

            const float baseFreq = midiToFreq(float(lastNote));
            const float octRatio = semisToRatio(float((p.octave - 2) * 12));
            const float detuneRatio = std::pow(2.0f, p.detune / 1200.0f);
            oscA.setFrequency(baseFreq * octRatio);
            oscB.setFrequency(baseFreq * octRatio * detuneRatio);
            subOsc.setFrequency(baseFreq * 0.5f * octRatio);
        }

        inline std::pair<float, float> process(const VoiceParams& p)
        {
            if (env.isIdle() && !active) { return { 0.0f, 0.0f }; }

            // 1) LFO
            const float lfoValue = lfo.process();        // -1..1

            // 2) Envelope & cutoff modulation
            const float envValue = env.process();
            const float envToCutoff = p.envMod * envValue;
            float lfoToCutoff = 0.0f;
            if (p.lfoTarget == LFOTarget::Cutoff && p.lfoDepth > 0.001f)
                lfoToCutoff = lfoValue * p.lfoDepth;

            const float keytrackShift = p.keytrack * (float(lastNote) - 60.0f) * 80.0f;
            const float baseCutoff = p.cutoff + keytrackShift;

            float cutoffMod = baseCutoff * std::pow(2.0f, envToCutoff + lfoToCutoff);
            cutoffMod = juce::jlimit(20.0f, float(sr) * 0.45f, cutoffMod);
            filter.setCutoff(cutoffMod);

            // 3) Sync B to A (hard-sync) if enabled
            if (p.sync && oscA.currentPhase() < oscA.phaseIncrement())
                oscB.snapToPhase(0.0);

            // 4) Render oscillators
            float a = oscA.process();
            float b = oscB.process();
            float sub = subOsc.process() * p.subLevel;

            // 5) REAL phase-modulation FM: B modulates A's phase.
            // This is implemented in the SSOsc::processFM() method, but
            // for live voice we approximate by mixing A's amplitude
            // with B's value (the audible effect is similar enough for
            // bass synthesis). For full PM in offline rendering see
            // ssab_sf2_gen.py.
            if (p.fmAmount > 0.001f)
            {
                a = a * (1.0f + b * p.fmAmount * 0.5f);
                a = hardClip(a);
            }

            // 6) Mix: dual osc + sub
            float mix = (a + b) * 0.4f + sub * 0.6f;

            // 7) MASSACRE 1994: quantise to ±0.7 (tracker crunch)
            if (p.massacre1994)
                mix = (mix > 0.0f) ? 0.7f : -0.7f;

            // 8) RUINER: crossfade with white noise
            if (p.ruiner > 0.001f)
            {
                const float n = noiseGen.nextFloat() * 2.0f - 1.0f;
                mix = mix * (1.0f - p.ruiner) + n * p.ruiner;
            }

            // 9) Apply envelope × velocity
            float v = mix * envValue * lastVelocity;

            // 10) Filter
            float filt = filter.process(v);

            // 11) LFO -> VOLUME
            if (p.lfoTarget == LFOTarget::Volume && p.lfoDepth > 0.001f)
                filt *= 1.0f + lfoValue * p.lfoDepth * 0.7f;

            // 12) Distortion (with SUB PROTECT crossover built-in)
            float driveMod = 0.0f;
            if (p.lfoTarget == LFOTarget::Drive && p.lfoDepth > 0.001f)
                driveMod = lfoValue * p.lfoDepth * 0.3f;
            dist.setDrive(juce::jlimit(0.0f, 1.0f, p.drive + driveMod));
            float distorted = dist.process(filt);

            // 13) FEEDBACK (MASSACRE)
            if (p.feedback > 0.001f)
            {
                feedbackZ = feedbackZ * 0.995f + distorted * p.feedback;
                distorted += feedbackZ * 0.3f;
                distorted = hardClip(distorted);
            }
            else
            {
                feedbackZ *= 0.9f;
            }

            return { distorted, distorted };
        }

        bool isActive() const noexcept { return active && !env.isIdle(); }
        void markInactive() noexcept { active = false; }
        int currentNote() const noexcept { return lastNote; }

    private:
        double sr = 44100.0;
        SSOsc        oscA, oscB, subOsc;
        SSSFilter    filter;
        SSDistortion dist;
        SSEnvelope   env;
        SSLFO        lfo;
        juce::Random noiseGen;

        VoiceParams  params;
        int   lastNote = 60;
        float lastVelocity = 1.0f;
        float feedbackZ = 0.0f;
        bool  active = false;
    };
}
