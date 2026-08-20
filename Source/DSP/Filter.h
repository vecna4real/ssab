#pragma once

#include "DSPUtils.h"
#include "../Constants.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSSFilter: Huovilainen-style 4-pole Moog transistor ladder filter
    // with zero-delay feedback. Self-oscillates at resonance > 0.9.
    // cutoff_hz is clamped to (10, sr*0.45) for stability.
    // ------------------------------------------------------------------
    class SSSFilter
    {
    public:
        void prepare(double sampleRate)
        {
            sr = sampleRate;
            reset();
        }

        void reset() noexcept
        {
            s1 = s2 = s3 = s4 = 0.0f;
        }

        void setType(FilterType t) noexcept { type = t; }

        void setCutoff(float hz) noexcept
        {
            cutoff = juce::jlimit(10.0f, float(sr) * 0.45f, hz);
            // Pre-compute g (one-pole stage coefficient).
            // g = 1 - exp(-2*pi*fc/sr)
            g = 1.0f - std::exp(-2.0f * float(kPi) * cutoff / float(sr));
        }

        void setResonance(float r) noexcept
        {
            // r in 0..1 -> k in 0..3.96 (clamped to avoid runaway feedback)
            resonance = juce::jlimit(0.0f, 0.99f, r);
            k = resonance * 4.0f;
        }

        inline float process(float in) noexcept
        {
            // 4 cascaded one-pole stages with feedback from s4.
            // Thermal-style input shaping via tanh keeps it stable.
            const float u = fastTanh((in - k * s4) * 0.7f) * 1.3f;

            s1 = s1 + g * (u  - s1);
            s2 = s2 + g * (s1 - s2);
            s3 = s3 + g * (s2 - s3);
            s4 = s4 + g * (s3 - s4);

            switch (type)
            {
            case FilterType::Lowpass:  return s4;
            case FilterType::Bandpass: return s2 - s4;  // 12 dB/oct BP
            case FilterType::Highpass: return in - s4;
            default:                   return s4;
            }
        }

        float currentCutoff() const noexcept { return cutoff; }
        float currentResonance() const noexcept { return resonance; }

    private:
        double sr = 44100.0;
        float cutoff = 1000.0f;
        float resonance = 0.4f;
        float g = 0.1f;        // stage coefficient
        float k = 1.6f;        // feedback gain (4 * resonance)
        float s1 = 0.0f, s2 = 0.0f, s3 = 0.0f, s4 = 0.0f;
        FilterType type = FilterType::Lowpass;
    };
}
