#pragma once

#include "FeedbackEQ.h"
#include "FractionalDelayLine.h"
#include "TapeSaturation.h"
#include "WowFlutter.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>

namespace slaplay::dsp
{
    struct SlapbackParameters
    {
        float delayMs = 110.0f;
        float feedback = 0.36f;   // 0 .. 0.9
        float lowCutHz = 150.0f;
        float highCutHz = 3800.0f;
        float tapeDrive = 0.38f;  // 0 .. 1
        float wow = 0.22f;        // 0 .. 1
        float flutter = 0.12f;    // 0 .. 1
        bool filterEnabled = true;
        bool tapeEnabled = true;
        bool modulationEnabled = true;
    };

    /**
        The complete slapback voice.

        Signal flow per channel and sample:

            input ──► write head ──► fractional read ──► tape saturation ──► wet out
                            ▲                                     │
                            │                                     ▼
                            └──── feedback gain ◄── DC block ◄── low cut ◄── high cut

        The 12 dB/octave tone stack lives exclusively in the feedback path, so
        only the repeats darken and thin out. Saturation is applied to the
        delayed signal itself, and the wow/flutter LFOs scale the delay length
        as a tiny speed error.
    */
    class SlapbackEngine
    {
    public:
        void prepare (double newSampleRate)
        {
            sampleRate = juce::jmax (8000.0, newSampleRate);

            delayLine.prepare (sampleRate, 0.25);
            eqLeft.prepare (sampleRate);
            eqRight.prepare (sampleRate);
            modulator.prepare (sampleRate);

            delayMsSmoothed.reset (sampleRate, 0.060);
            feedbackSmoothed.reset (sampleRate, 0.020);
            tapeAmountSmoothed.reset (sampleRate, 0.030);
            wowSmoothed.reset (sampleRate, 0.050);
            flutterSmoothed.reset (sampleRate, 0.050);
            lowCutSmoothed.reset (sampleRate, 0.040);
            highCutSmoothed.reset (sampleRate, 0.040);

            delayMsSmoothed.setCurrentAndTargetValue (110.0f);
            feedbackSmoothed.setCurrentAndTargetValue (0.36f);
            tapeAmountSmoothed.setCurrentAndTargetValue (0.38f);
            wowSmoothed.setCurrentAndTargetValue (0.22f);
            flutterSmoothed.setCurrentAndTargetValue (0.12f);
            lowCutSmoothed.setCurrentAndTargetValue (150.0f);
            highCutSmoothed.setCurrentAndTargetValue (3800.0f);

            reset();
        }

        void reset() noexcept
        {
            delayLine.reset();
            eqLeft.reset();
            eqRight.reset();
            modulator.reset();
            sampleCounter = 0;

            snap (delayMsSmoothed);
            snap (feedbackSmoothed);
            snap (tapeAmountSmoothed);
            snap (wowSmoothed);
            snap (flutterSmoothed);
            snap (lowCutSmoothed);
            snap (highCutSmoothed);

            lastDelaySamples = delayMsSmoothed.getCurrentValue() * 0.001f
                               * static_cast<float> (sampleRate);
        }

        void setParameters (const SlapbackParameters& parameters) noexcept
        {
            delayMsSmoothed.setTargetValue (juce::jlimit (10.0f, 220.0f, parameters.delayMs));
            feedbackSmoothed.setTargetValue (juce::jlimit (0.0f, 0.9f, parameters.feedback));
            tapeAmountSmoothed.setTargetValue (parameters.tapeEnabled
                                                   ? juce::jlimit (0.0f, 1.0f, parameters.tapeDrive)
                                                   : 0.0f);
            wowSmoothed.setTargetValue (parameters.modulationEnabled
                                            ? juce::jlimit (0.0f, 1.0f, parameters.wow)
                                            : 0.0f);
            flutterSmoothed.setTargetValue (parameters.modulationEnabled
                                                ? juce::jlimit (0.0f, 1.0f, parameters.flutter)
                                                : 0.0f);
            lowCutSmoothed.setTargetValue (juce::jlimit (20.0f, 1200.0f, parameters.lowCutHz));
            highCutSmoothed.setTargetValue (juce::jlimit (200.0f,
                                                         static_cast<float> (sampleRate) * 0.45f,
                                                         parameters.highCutHz));

            eqLeft.setEnabled (parameters.filterEnabled);
            eqRight.setEnabled (parameters.filterEnabled);
        }

        void process (const float* inputLeft, const float* inputRight,
                      float* wetLeft, float* wetRight, int numSamples) noexcept
        {
            const float maximumDelay = delayLine.getMaximumDelaySamples();

            for (int i = 0; i < numSamples; ++i)
            {
                const float delayMs = delayMsSmoothed.getNextValue();
                const float feedback = feedbackSmoothed.getNextValue();
                const float tapeAmount = tapeAmountSmoothed.getNextValue();
                const float wowAmount = wowSmoothed.getNextValue();
                const float flutterAmount = flutterSmoothed.getNextValue();
                const float lowCut = lowCutSmoothed.getNextValue();
                const float highCut = highCutSmoothed.getNextValue();

                if ((sampleCounter & 15u) == 0)
                {
                    eqLeft.setCutoffs (lowCut, highCut);
                    eqRight.setCutoffs (lowCut, highCut);
                }

                modulator.setDepths (wowAmount, flutterAmount);

                const float speedError = modulator.getSpeedModulation();
                modulator.advance();

                const float delaySamples = juce::jlimit (
                    2.0f, maximumDelay,
                    delayMs * 0.001f * static_cast<float> (sampleRate) * (1.0f + speedError));

                lastDelaySamples = delaySamples;

                const float delayedLeft = delayLine.readLeft (delaySamples);
                const float delayedRight = delayLine.readRight (delaySamples);

                tapeLeft.setAmount (tapeAmount);
                tapeRight.setAmount (tapeAmount);

                const float saturatedLeft = tapeLeft.process (delayedLeft);
                const float saturatedRight = tapeRight.process (delayedRight);

                const float feedbackLeft = eqLeft.process (saturatedLeft);
                const float feedbackRight = eqRight.process (saturatedRight);

                delayLine.write (inputLeft[i] + feedbackLeft * feedback,
                                 inputRight[i] + feedbackRight * feedback);

                wetLeft[i] = saturatedLeft;
                wetRight[i] = saturatedRight;

                ++sampleCounter;
            }
        }

        /** The most recent interpolated delay length in samples (used by tests/UI). */
        float getCurrentDelaySamples() const noexcept { return lastDelaySamples; }

        double getSampleRate() const noexcept { return sampleRate; }

    private:
        template <typename Smoother>
        static void snap (Smoother& smoother) noexcept
        {
            smoother.setCurrentAndTargetValue (smoother.getTargetValue());
        }

        double sampleRate = 48000.0;
        juce::uint32 sampleCounter = 0;

        FractionalDelayLine delayLine;
        FeedbackEQ eqLeft, eqRight;
        TapeSaturation tapeLeft, tapeRight;
        WowFlutter modulator;

        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayMsSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> feedbackSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tapeAmountSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wowSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flutterSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lowCutSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> highCutSmoothed;

        float lastDelaySamples = 4800.0f;
    };
} // namespace slaplay::dsp
