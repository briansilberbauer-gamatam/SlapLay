#pragma once

#include <juce_core/juce_core.h>

#include <cmath>

namespace slaplay::dsp
{
    /** One-pole low-pass building block (6 dB/octave). */
    struct OnePoleLowPass
    {
        void prepare (double newSampleRate) noexcept
        {
            sampleRate = juce::jmax (8000.0, newSampleRate);
            reset();
        }

        void setCutoff (float cutoffHz) noexcept
        {
            const float nyquistLimit = static_cast<float> (sampleRate) * 0.45f;
            const float clamped = juce::jlimit (10.0f, nyquistLimit, cutoffHz);
            coefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                           * clamped / static_cast<float> (sampleRate));
        }

        void reset() noexcept { state = 0.0f; }

        float process (float sample) noexcept
        {
            state += coefficient * (sample - state);
            return state;
        }

    private:
        double sampleRate = 48000.0;
        float coefficient = 0.0f;
        float state = 0.0f;
    };

    /** One-pole high-pass building block (6 dB/octave). */
    struct OnePoleHighPass
    {
        void prepare (double newSampleRate) noexcept
        {
            sampleRate = juce::jmax (8000.0, newSampleRate);
            reset();
        }

        void setCutoff (float cutoffHz) noexcept
        {
            const float nyquistLimit = static_cast<float> (sampleRate) * 0.45f;
            const float clamped = juce::jlimit (10.0f, nyquistLimit, cutoffHz);
            coefficient = std::exp (-juce::MathConstants<float>::twoPi
                                    * clamped / static_cast<float> (sampleRate));
        }

        void reset() noexcept
        {
            previousInput = 0.0f;
            previousOutput = 0.0f;
        }

        float process (float sample) noexcept
        {
            const float output = coefficient * (previousOutput + sample - previousInput);
            previousInput = sample;
            previousOutput = output;
            return output;
        }

    private:
        double sampleRate = 48000.0;
        float coefficient = 0.0f;
        float previousInput = 0.0f;
        float previousOutput = 0.0f;
    };

    /**
        The vintage feedback-loop tone stage: a 12 dB/octave low cut followed by
        a 12 dB/octave high cut. Both slopes are built from two cascaded
        one-pole sections, which gives the mellow, phase-heavy character of a
        bucket-brigade or tape echo without resonant peaks.

        This filter is placed *only* inside the delay feedback path. The first
        repeat that reaches the output is the raw saturated tap; each successive
        repeat is progressively darker and thinner.

        A permanent 12 Hz DC blocker after the tone stack keeps the feedback loop
        from accumulating an offset when the tone filters are switched out.
    */
    class FeedbackEQ
    {
    public:
        void prepare (double newSampleRate)
        {
            sampleRate = juce::jmax (8000.0, newSampleRate);

            lowPass1.prepare (sampleRate);
            lowPass2.prepare (sampleRate);
            highPass1.prepare (sampleRate);
            highPass2.prepare (sampleRate);
            dcBlocker.prepare (sampleRate);
            dcBlocker.setCutoff (12.0f);

            reset();
            setCutoffs (150.0f, 3800.0f);
        }

        void setCutoffs (float lowCutHz, float highCutHz) noexcept
        {
            const float safeHigh = juce::jlimit (200.0f, static_cast<float> (sampleRate) * 0.45f,
                                                highCutHz);
            const float safeLow = juce::jlimit (20.0f, safeHigh * 0.85f, lowCutHz);

            highPass1.setCutoff (safeLow);
            highPass2.setCutoff (safeLow);
            lowPass1.setCutoff (safeHigh);
            lowPass2.setCutoff (safeHigh);
        }

        void setEnabled (bool shouldBeEnabled) noexcept { enabled = shouldBeEnabled; }
        bool isEnabled() const noexcept { return enabled; }

        void reset() noexcept
        {
            lowPass1.reset();
            lowPass2.reset();
            highPass1.reset();
            highPass2.reset();
            dcBlocker.reset();
        }

        float process (float sample) noexcept
        {
            // DC safety is always present in the loop.
            sample = dcBlocker.process (sample);

            if (! enabled)
                return sample;

            sample = highPass2.process (highPass1.process (sample));
            return lowPass2.process (lowPass1.process (sample));
        }

    private:
        double sampleRate = 48000.0;
        bool enabled = true;

        OnePoleLowPass lowPass1, lowPass2;
        OnePoleHighPass highPass1, highPass2, dcBlocker;
    };
} // namespace slaplay::dsp
