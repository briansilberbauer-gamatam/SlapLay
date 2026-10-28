#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

#include <array>

namespace studio::ui
{
    // Central obsidian visualizer: dB/time grid plus a live glowing crimson waveform.
    class WaveformDisplay final : public juce::Component,
                                  private juce::Timer
    {
    public:
        explicit WaveformDisplay (SlapLayAudioProcessor&);

        void paint (juce::Graphics&) override;

    private:
        void timerCallback() override;

        SlapLayAudioProcessor& processor;
        static constexpr int viewColumns = 256;

        std::array<float, viewColumns> minTrace {};
        std::array<float, viewColumns> maxTrace {};
        int numColumns = 0;

        const std::atomic<float>* delayParameter = nullptr;
        const std::atomic<float>* feedbackParameter = nullptr;
        const std::atomic<float>* modulationParameter = nullptr;
        const std::atomic<float>* tapeParameter = nullptr;

        float displayDelayMs = 110.0f;
        float displayFeedback = 36.0f;
        bool modulationEnabled = true;
        bool tapeEnabled = true;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
    };
} // namespace studio::ui
