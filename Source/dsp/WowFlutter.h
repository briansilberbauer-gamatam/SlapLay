#pragma once

#include <juce_core/juce_core.h>

#include <cmath>

namespace slaplay::dsp
{
    /**
        Mechanical tape speed instability.

        Two free-running sine LFOs produce a fractional *speed* deviation rather
        than a fixed time offset. The engine multiplies the delay length by
        (1 + modulation), which is exactly what a stretched or slipping tape does:
        a longer delay exhibits more absolute wow than a short one, while the
        pitch wobble remains proportional to the delay time.

        Wow is the slow 0.55 Hz drift; flutter is the faster 6.3 Hz shimmer.
    */
    class WowFlutter
    {
    public:
        static constexpr float wowRateHz = 0.55f;
        static constexpr float flutterRateHz = 6.3f;

        // Maximum fractional speed deviations at 100 % depth.
        static constexpr float wowDepthAtFull = 0.0060f;     // 0.6 %
        static constexpr float flutterDepthAtFull = 0.0025f; // 0.25 %

        void prepare (double newSampleRate)
        {
            sampleRate = juce::jmax (8000.0, newSampleRate);

            wowIncrement = juce::MathConstants<float>::twoPi * wowRateHz
                           / static_cast<float> (sampleRate);
            flutterIncrement = juce::MathConstants<float>::twoPi * flutterRateHz
                               / static_cast<float> (sampleRate);

            reset();
        }

        void reset() noexcept
        {
            wowPhase = 0.0f;
            flutterPhase = 1.15f;
        }

        void setDepths (float wowAmount, float flutterAmount) noexcept
        {
            wowDepth = juce::jlimit (0.0f, 1.0f, wowAmount) * wowDepthAtFull;
            flutterDepth = juce::jlimit (0.0f, 1.0f, flutterAmount) * flutterDepthAtFull;
        }

        /** Current fractional speed error, roughly -0.0085 .. +0.0085 at full depth. */
        float getSpeedModulation() const noexcept
        {
            return wowDepth * std::sin (wowPhase)
                   + flutterDepth * std::sin (flutterPhase);
        }

        void advance() noexcept
        {
            wowPhase += wowIncrement;

            if (wowPhase >= juce::MathConstants<float>::twoPi)
                wowPhase -= juce::MathConstants<float>::twoPi;

            flutterPhase += flutterIncrement;

            if (flutterPhase >= juce::MathConstants<float>::twoPi)
                flutterPhase -= juce::MathConstants<float>::twoPi;
        }

    private:
        double sampleRate = 48000.0;
        float wowPhase = 0.0f;
        float flutterPhase = 1.15f;
        float wowIncrement = 0.0f;
        float flutterIncrement = 0.0f;
        float wowDepth = 0.0f;
        float flutterDepth = 0.0f;
    };
} // namespace slaplay::dsp
