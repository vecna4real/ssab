#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace ssab
{
    constexpr double kPi = 3.14159265358979323846;

    // Fast tanh approximation (Lambert-style).
    inline float fastTanh(float x) noexcept
    {
        const float a = std::fabs(x);
        const float b = 1.0f / (1.0f + a);
        const float c = 1.0f - b * b;
        return (x < 0.0f) ? -c : c;
    }

    // Soft clipper (cubic).
    inline float softClip(float x) noexcept
    {
        if (x > 1.0f)  return 1.0f - (1.5f / (x + 0.5f));
        if (x < -1.0f) return -1.0f + (1.5f / (-x + 0.5f));
        return x - (x * x * x) * 0.3333333f;
    }

    // Hard clipper.
    inline float hardClip(float x) noexcept
    {
        return juce::jlimit(-1.0f, 1.0f, x);
    }

    // Wavefolder: folds the signal around the [-1, 1] boundary.
    inline float foldWave(float x) noexcept
    {
        // Classic triangle-folder implementation.
        x = x * 2.0f;
        while (x > 2.0f)  x -= 4.0f;
        while (x < -2.0f) x += 4.0f;
        return (x >= -1.0f && x <= 1.0f) ? x
             : (x > 1.0f) ? (2.0f - x)
             : (-2.0f - x);
    }

    // Fuzz: square-law + hard clip.
    inline float fuzz(float x) noexcept
    {
        const float sx = std::copysign(1.0f, x);
        const float w = std::fabs(x);
        const float shaped = sx * (w * w);
        return hardClip(shaped * 3.0f);
    }

    // Convert MIDI note -> frequency (A4 = 440 Hz).
    inline float midiToFreq(float midi) noexcept
    {
        return 440.0f * std::pow(2.0f, (midi - 69.0f) / 12.0f);
    }

    // Convert semitones + cents to a frequency ratio.
    inline float semisToRatio(float semis) noexcept
    {
        return std::pow(2.0f, semis / 12.0f);
    }

    // Convert milliseconds -> exponential time coefficient.
    inline float msToCoeff(float ms, float sampleRate) noexcept
    {
        const float samples = std::max(1.0f, ms * 0.001f * sampleRate);
        return std::exp(-1.0f / samples);
    }

    // Bitcrusher-style quantiser. depth = 0..1 (0 = bypass, 1 = 1-bit).
    inline float quantize(float x, float depth) noexcept
    {
        if (depth <= 0.001f) return x;
        const int levels = juce::jmax(2, int((1.0f - depth) * 254.0f) + 2);
        const float scale = float(levels);
        return std::round(x * scale * 0.5f) / (scale * 0.5f);
    }

    // Sample-rate hold for bitcrusher.
    inline float holdStep(float& last, float x, int& counter, int holdEvery) noexcept
    {
        if (counter >= holdEvery) { last = x; counter = 0; }
        else ++counter;
        return last;
    }

    // Linear map.
    inline float lmap(float v, float a, float b) noexcept { return a + v * (b - a); }

    // Two-pole one-zero DC blocker.
    struct DCBlocker
    {
        float x = 0.0f, y = 0.0f;
        float process(float in, float R = 0.995f) noexcept
        {
            y = in - x + R * y;
            x = in;
            return y;
        }
        void reset() noexcept { x = y = 0.0f; }
    };

    // One-pole lowpass (used for smoothing).
    struct OnePoleLP
    {
        float z = 0.0f;
        void setCoeff(float coeff) noexcept { c = coeff; }
        float process(float in) noexcept { z = in + c * (z - in); return z; }
        void reset() noexcept { z = 0.0f; }
    private:
        float c = 0.0f;
    };
}
