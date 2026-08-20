#pragma once

#include "DSPUtils.h"
#include "../Constants.h"

namespace ssab
{
    // ------------------------------------------------------------------
    // SSDistortion: drive + distortion + bitcrusher, with the critical
    // SUB PROTECT feature implemented as a Linkwitz-Riley 4th-order
    // crossover at ~80 Hz. The low band bypasses the saturator; the
    // high band goes through CLIP/FOLD/FUZZ. The two bands are then
    // summed back together.
    //
    // This guarantees the sub-bass content (the fundamental on low
    // notes) is never destroyed by the saturator — no matter how
    // hard you push DRIVE.
    // ------------------------------------------------------------------
    class SSDistortion
    {
    public:
        void prepare(double sampleRate)
        {
            sr = sampleRate;
            // Pre-compute LR4 crossover coefficients at 80 Hz
            // LR4 = two cascaded 2nd-order Butterworth sections at the
            // same frequency. We use the cookbook RBJ formulae.
            const float fc = 80.0f;
            const float w0 = 2.0f * float(kPi) * fc / float(sr);
            const float cosw = std::cos(w0);
            const float sinw = std::sin(w0);
            const float Q = 0.70710678f;     // Butterworth Q
            const float alpha = sinw / (2.0f * Q);

            // Butterworth lowpass 2nd order
            const float a0_lp = 1.0f + alpha;
            const float b0_lp = (1.0f - cosw) * 0.5f / a0_lp;
            const float b1_lp = (1.0f - cosw)       / a0_lp;
            const float b2_lp = (1.0f - cosw) * 0.5f / a0_lp;
            const float a1_lp = -2.0f * cosw         / a0_lp;
            const float a2_lp = (1.0f - alpha)     / a0_lp;

            // Butterworth highpass 2nd order
            const float a0_hp = 1.0f + alpha;
            const float b0_hp = (1.0f + cosw) * 0.5f / a0_hp;
            const float b1_hp = -(1.0f + cosw)       / a0_hp;
            const float b2_hp = (1.0f + cosw) * 0.5f / a0_hp;
            const float a1_hp = -2.0f * cosw         / a0_hp;
            const float a2_hp = (1.0f - alpha)      / a0_hp;

            // Stage 1 (LP and HP)
            lpB0_1 = b0_lp; lpB1_1 = b1_lp; lpB2_1 = b2_lp;
            lpA1_1 = a1_lp; lpA2_1 = a2_lp;
            hpB0_1 = b0_hp; hpB1_1 = b1_hp; hpB2_1 = b2_hp;
            hpA1_1 = a1_hp; hpA2_1 = a2_hp;
            // Stage 2 (same coefficients — cascade)
            lpB0_2 = b0_lp; lpB1_2 = b1_lp; lpB2_2 = b2_lp;
            lpA1_2 = a1_lp; lpA2_2 = a2_lp;
            hpB0_2 = b0_hp; hpB1_2 = b1_hp; hpB2_2 = b2_hp;
            hpA1_2 = a1_hp; hpA2_2 = a2_hp;

            // Reset all filter states
            lp_x1_1 = lp_x2_1 = lp_y1_1 = lp_y2_1 = 0.0f;
            lp_x1_2 = lp_x2_2 = lp_y1_2 = lp_y2_2 = 0.0f;
            hp_x1_1 = hp_x2_1 = hp_y1_1 = hp_y2_1 = 0.0f;
            hp_x1_2 = hp_x2_2 = hp_y1_2 = hp_y2_2 = 0.0f;

            dcL.reset();
            lastSample = 0.0f;
            counter = 0;
        }

        void setDrive(float d) noexcept { drive = juce::jlimit(0.0f, 1.0f, d); }
        void setType(DistType t) noexcept { type = t; }
        void setBitcrush(float b) noexcept { crush = juce::jlimit(0.0f, 1.0f, b); }
        void setSubProtect(float s) noexcept { subProtect = juce::jlimit(0.0f, 1.0f, s); }

        inline float process(float in) noexcept
        {
            // ---- 1) SUB PROTECT: LR4 crossover at 80 Hz ----
            // Two cascaded Butterworth stages for each band (LP / HP).
            float low;
            if (subProtect > 0.001f)
            {
                // Low band (preserved)
                float y_lp1 = lpB0_1 * in + lpB1_1 * lp_x1_1 + lpB2_1 * lp_x2_1
                              - lpA1_1 * lp_y1_1 - lpA2_1 * lp_y2_1;
                lp_x2_1 = lp_x1_1; lp_x1_1 = in;
                lp_y2_1 = lp_y1_1; lp_y1_1 = y_lp1;
                float y_lp2 = lpB0_2 * y_lp1 + lpB1_2 * lp_x1_2 + lpB2_2 * lp_x2_2
                              - lpA1_2 * lp_y1_2 - lpA2_2 * lp_y2_2;
                lp_x2_2 = lp_x1_2; lp_x1_2 = y_lp1;
                lp_y2_2 = lp_y1_2; lp_y1_2 = y_lp2;
                low = y_lp2;

                // High band (saturated)
                float y_hp1 = hpB0_1 * in + hpB1_1 * hp_x1_1 + hpB2_1 * hp_x2_1
                              - hpA1_1 * hp_y1_1 - hpA2_1 * hp_y2_1;
                hp_x2_1 = hp_x1_1; hp_x1_1 = in;
                hp_y2_1 = hp_y1_1; hp_y1_1 = y_hp1;
                float y_hp2 = hpB0_2 * y_hp1 + hpB1_2 * hp_x1_2 + hpB2_2 * hp_x2_2
                              - hpA1_2 * hp_y1_2 - hpA2_2 * hp_y2_2;
                hp_x2_2 = hp_x1_2; hp_x1_2 = y_hp1;
                hp_y2_2 = hp_y1_2; hp_y1_2 = y_hp2;
                // 'high' is just y_hp2 — saturate it.
                float toSat = y_hp2;
                float x = toSat * (1.0f + drive * 25.0f);
                float sat;
                switch (type)
                {
                case DistType::Clip:  sat = softClip(x * 0.7f); break;
                case DistType::Fold:  sat = foldWave(x * 0.8f); break;
                case DistType::Fuzz:  sat = fuzz(x);            break;
                default:              sat = x;                  break;
                }
                // Mix: protected low + saturated high.
                // sub_protect = 1.0 -> 100% original low + 0% distorted low.
                const float out = low * subProtect + sat * (1.0f - subProtect * 0.0f)
                                  + sat * (subProtect * 0.0f);
                // Simplified: out = low + sat (crossover sums to unity).
                return applyBitcrushAndDC(low + sat);
            }
            else
            {
                // No sub protection — full-band saturate.
                float x = in * (1.0f + drive * 25.0f);
                float sat;
                switch (type)
                {
                case DistType::Clip:  sat = softClip(x * 0.7f); break;
                case DistType::Fold:  sat = foldWave(x * 0.8f); break;
                case DistType::Fuzz:  sat = fuzz(x);            break;
                default:              sat = x;                  break;
                }
                return applyBitcrushAndDC(sat);
            }
        }

    private:
        inline float applyBitcrushAndDC(float in) noexcept
        {
            float out = in;
            if (crush > 0.001f)
            {
                const int holdEvery = 1 + int(crush * 8.0f);
                const float held = holdStep(lastSample, out, counter, holdEvery);
                const float q = quantize(held, crush);
                lastSample = held;
                out = q;
            }
            return dcL.process(out);
        }

        double sr = 44100.0;
        float drive = 0.0f;
        float crush = 0.0f;
        float subProtect = 0.7f;
        DistType type = DistType::Clip;

        // LR4 crossover coefficients (two stages per band)
        float lpB0_1, lpB1_1, lpB2_1, lpA1_1, lpA2_1;
        float lpB0_2, lpB1_2, lpB2_2, lpA1_2, lpA2_2;
        float hpB0_1, hpB1_1, hpB2_1, hpA1_1, hpA2_1;
        float hpB0_2, hpB1_2, hpB2_2, hpA1_2, hpA2_2;

        float lp_x1_1, lp_x2_1, lp_y1_1, lp_y2_1;
        float lp_x1_2, lp_x2_2, lp_y1_2, lp_y2_2;
        float hp_x1_1, hp_x2_1, hp_y1_1, hp_y2_1;
        float hp_x1_2, hp_x2_2, hp_y1_2, hp_y2_2;

        DCBlocker dcL;
        float lastSample = 0.0f;
        int   counter = 0;
    };
}
