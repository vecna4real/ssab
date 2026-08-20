#pragma once

#include "DSPUtils.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSEnvelope: standard ADSR with an extra "PUNCH" parameter that
    // adds a fast initial transient on note-on (extra attack peak).
    // ------------------------------------------------------------------
    class SSEnvelope
    {
    public:
        void prepare(double sampleRate)
        {
            sr = sampleRate;
            calculateCoeffs();
            reset();
        }

        void reset() noexcept
        {
            value = 0.0f;
            state = State::Idle;
            punchHold = 0;
            target = 0.0f;
        }

        void setAttack(float ms)  noexcept { attackMs = juce::jmax(0.1f, ms); calcAttack(); }
        void setDecay(float ms)   noexcept { decayMs  = juce::jmax(1.0f, ms); calcDecay(); }
        void setSustain(float v)   noexcept { sustain = juce::jlimit(0.0f, 1.0f, v); }
        void setRelease(float ms)  noexcept { releaseMs = juce::jmax(1.0f, ms); calcRelease(); }
        void setPunch(float p)     noexcept { punch = juce::jlimit(0.0f, 1.0f, p); }

        void noteOn() noexcept
        {
            state = State::Attack;
            target = 1.0f + punch;  // overshoot for punch
            punchHold = int(punch * 0.020f * sr); // ~20 ms max
            // recompute coeffs in case parameters changed
            calcAttack();
        }

        void noteOff() noexcept
        {
            state = State::Release;
            target = 0.0f;
            calcRelease();
        }

        inline float process() noexcept
        {
            switch (state)
            {
            case State::Idle:
                value = 0.0f;
                break;
            case State::Attack:
                if (punchHold > 0)
                {
                    value = target * (1.0f - attackCoeff) + value * attackCoeff;
                    --punchHold;
                    if (punchHold == 0) target = 1.0f;
                }
                else
                {
                    value = 1.0f * (1.0f - attackCoeff) + value * attackCoeff;
                    if (std::fabs(value - 1.0f) < 0.001f)
                    {
                        value = 1.0f;
                        state = State::Decay;
                        target = sustain;
                    }
                }
                break;
            case State::Decay:
                value = sustain * (1.0f - decayCoeff) + value * decayCoeff;
                if (std::fabs(value - sustain) < 0.001f)
                {
                    value = sustain;
                    state = State::Sustain;
                }
                break;
            case State::Sustain:
                value = sustain;
                break;
            case State::Release:
                value = 0.0f * (1.0f - releaseCoeff) + value * releaseCoeff;
                if (std::fabs(value) < 0.0005f)
                {
                    value = 0.0f;
                    state = State::Idle;
                }
                break;
            }
            return juce::jlimit(0.0f, 2.0f, value);
        }

        bool isIdle() const noexcept { return state == State::Idle; }
        float currentValue() const noexcept { return value; }

    private:
        void calculateCoeffs() noexcept
        {
            calcAttack();
            calcDecay();
            calcRelease();
        }
        void calcAttack()  noexcept { attackCoeff  = msToCoeff(attackMs, float(sr)); }
        void calcDecay()   noexcept { decayCoeff   = msToCoeff(decayMs, float(sr)); }
        void calcRelease() noexcept { releaseCoeff = msToCoeff(releaseMs, float(sr)); }

        enum class State { Idle, Attack, Decay, Sustain, Release };

        double sr = 44100.0;
        float attackMs = 1.0f;
        float decayMs = 200.0f;
        float sustain = 0.8f;
        float releaseMs = 200.0f;
        float punch = 0.5f;

        float attackCoeff = 0.0f;
        float decayCoeff  = 0.0f;
        float releaseCoeff = 0.0f;

        float value = 0.0f;
        float target = 0.0f;
        int   punchHold = 0;
        State state = State::Idle;
    };
}
