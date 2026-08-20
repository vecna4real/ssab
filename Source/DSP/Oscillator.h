#pragma once

#include "DSPUtils.h"
#include "../Constants.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSOsc: a single oscillator with a selectable waveform.
    // Uses one-sided PolyBLEP residual for band-limiting saw / square /
    // pulse. NOISE = white RNG; GRIT = saw + small noise modulation.
    // ------------------------------------------------------------------
    class SSOsc
    {
    public:
        SSOsc() = default;

        void prepare(double sampleRate)
        {
            sr = sampleRate;
            phase = 0.0;
            phaseInc = 0.0;
            lastOut = 0.0f;
            noiseGen.reset();
        }

        void reset()
        {
            phase = 0.0;
            lastOut = 0.0f;
            noiseGen.reset();
        }

        void setFrequency(double freq)
        {
            freq = juce::jlimit(0.5, sr * 0.48, freq);
            phaseInc = freq / sr;
        }

        void setWaveform(WaveType t) noexcept { wave = t; }
        void setPulseWidth(float pw) noexcept { pulseWidth = juce::jlimit(0.05f, 0.95f, pw); }

        // One-sided PolyBLEP residual at a rising edge.
        // dt is the time-since-edge in phase increments.
        static inline float polyblepRising(float dt) noexcept
        {
            return dt + dt - dt * dt;
        }

        // One-sided PolyBLEP residual at a falling edge.
        static inline float polyblepFalling(float dt) noexcept
        {
            return -(dt + dt + dt * dt);
        }

        // ---- Master render: returns next mono sample ----
        inline float process() noexcept
        {
            float out = 0.0f;
            const double inc = phaseInc;
            const float inc_f = float(inc);

            switch (wave)
            {
            case WaveType::Saw:
            {
                out = float(2.0 * phase - 1.0);
                // PolyBLEP at the rising edge (phase wraps from ~1 to ~0)
                if (phase < inc)
                {
                    float dt = float(phase) / inc_f;
                    out -= polyblepRising(dt);
                }
                break;
            }
            case WaveType::Square:
            {
                out = (phase < 0.5) ? 1.0f : -1.0f;
                // Rising edge at the wrap
                if (phase < inc)
                {
                    float dt = float(phase) / inc_f;
                    out += polyblepRising(dt);
                }
                // Falling edge at phase == 0.5
                if (phase >= 0.5 - inc && phase < 0.5)
                {
                    float dt = float(phase - (0.5 - inc)) / inc_f;
                    out += polyblepFalling(dt);
                }
                else if (phase >= 0.5 && phase < 0.5 + inc)
                {
                    float dt = float(phase - 0.5) / inc_f;
                    out += polyblepRising(dt);
                }
                break;
            }
            case WaveType::Pulse:
            {
                const float pw = pulseWidth;
                out = (phase < pw) ? 1.0f : -1.0f;
                // Rising edge at wrap
                if (phase < inc)
                {
                    float dt = float(phase) / inc_f;
                    out += polyblepRising(dt);
                }
                // Falling edge at pw
                if (phase >= pw - inc && phase < pw)
                {
                    float dt = float(phase - (pw - inc)) / inc_f;
                    out += polyblepFalling(dt);
                }
                else if (phase >= pw && phase < pw + inc)
                {
                    float dt = float(phase - pw) / inc_f;
                    out += polyblepRising(dt);
                }
                break;
            }
            case WaveType::Noise:
                out = noiseGen.nextFloat() * 2.0f - 1.0f;
                break;
            case WaveType::Grit:
            {
                float saw = float(2.0 * phase - 1.0);
                if (phase < inc)
                {
                    float dt = float(phase) / inc_f;
                    saw -= polyblepRising(dt);
                }
                const float n = (noiseGen.nextFloat() * 2.0f - 1.0f) * 0.18f;
                out = saw + n * (std::fabs(saw) + 0.3f);
                break;
            }
            default:
                out = 0.0f;
            }

            // Advance phase
            phase += inc;
            while (phase >= 1.0) phase -= 1.0;

            lastOut = out;
            return out;
        }

        // Hard-sync to a master phase position.
        void snapToPhase(double p) noexcept { phase = p; }

        float lastOutput() const noexcept { return lastOut; }
        double currentPhase() const noexcept { return phase; }
        double phaseIncrement() const noexcept { return phaseInc; }

    private:
        double sr = 44100.0;
        double phase = 0.0;
        double phaseInc = 0.0;
        float  lastOut = 0.0f;
        WaveType wave = WaveType::Saw;
        float  pulseWidth = 0.5f;
        juce::Random noiseGen;
    };
}
