#pragma once

#include "DSPUtils.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSLFO: free-running sine/triangle LFO with optional random S&H.
    // ------------------------------------------------------------------
    class SSLFO
    {
    public:
        void prepare(double sampleRate)
        {
            sr = sampleRate;
            phase = 0.0;
            hold = 0.0f;
            counter = 0;
            rng.setSeedRandomly();
        }

        void reset() noexcept
        {
            phase = 0.0;
            hold = 0.0f;
            counter = 0;
        }

        void setRate(float hz) noexcept
        {
            rate = juce::jlimit(0.05f, 30.0f, hz);
        }

        inline float process() noexcept
        {
            phase += double(rate) / sr;
            while (phase >= 1.0) phase -= 1.0;

            // Triangle wave -1..1
            float tri = (phase < 0.5)
                ? float(phase * 4.0 - 1.0)
                : float(3.0 - phase * 4.0);

            return tri;
        }

    private:
        double sr = 44100.0;
        double phase = 0.0;
        float  rate = 4.0f;
        float  hold = 0.0f;
        int    counter = 0;
        juce::Random rng;
    };
}
