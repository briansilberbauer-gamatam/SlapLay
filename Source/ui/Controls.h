#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "Theme.h"

#include <functional>
#include <memory>

namespace studio::ui
{
    //==================================================================================
    // Low-profile heavy-machined knurled metal knob with caption and value readout.
    //==================================================================================
    class ParamKnob final : public juce::Component,
                            private juce::Timer
    {
    public:
        ParamKnob (juce::AudioProcessorValueTreeState&,
                   const juce::String& parameterID,
                   juce::String caption,
                   std::function<juce::String (float)> formatter);

        ~ParamKnob() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseEnter (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;

    private:
        void timerCallback() override;
        void refreshValueText();

        juce::AudioProcessorValueTreeState& apvts;
        juce::String parameterID;
        juce::String captionText;
        std::function<juce::String (float)> formatValue;

        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::String valueText;
        bool hover = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamKnob)
    };

    //==================================================================================
    // Ultra-thin vertical glass slider inset into the full-height right mixer bay.
    //==================================================================================
    class GlassSlider final : public juce::Component,
                              private juce::Timer
    {
    public:
        GlassSlider (juce::AudioProcessorValueTreeState&,
                     const juce::String& parameterID,
                     juce::String caption,
                     std::function<juce::String (float)> formatter);

        ~GlassSlider() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseEnter (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;

    private:
        void timerCallback() override;
        void refreshValueText();

        juce::AudioProcessorValueTreeState& apvts;
        juce::String parameterID;
        juce::String captionText;
        std::function<juce::String (float)> formatValue;

        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::String valueText;
        bool hover = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlassSlider)
    };

    //==================================================================================
    // Rectangular low-profile tactile button attached to a boolean parameter.
    //==================================================================================
    class TactileButton final : public juce::ToggleButton
    {
    public:
        TactileButton (juce::AudioProcessorValueTreeState&,
                       const juce::String& parameterID,
                       const juce::String& labelText);

    private:
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TactileButton)
    };

    //==================================================================================
    // Horizontal stereo output meter inside the lower right mixer bay.
    //==================================================================================
    class OutputMeter final : public juce::Component,
                              private juce::Timer
    {
    public:
        explicit OutputMeter (SlapLayAudioProcessor&);

        void paint (juce::Graphics&) override;

    private:
        void timerCallback() override;
        void drawChannel (juce::Graphics&, juce::Rectangle<float> bounds,
                          const juce::String& label, float normalisedLevel,
                          float normalisedPeak) const;

        SlapLayAudioProcessor& processor;
        float levelLeft = 0.0f;
        float levelRight = 0.0f;
        float peakLeft = 0.0f;
        float peakRight = 0.0f;
        int peakHoldLeft = 0;
        int peakHoldRight = 0;
        int clipHold = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputMeter)
    };

    //==================================================================================
    // Slender top-centre pill readout with preset name and chevron navigation.
    //==================================================================================
    class HeaderPillBar final : public juce::Component,
                                private juce::Timer
    {
    public:
        explicit HeaderPillBar (SlapLayAudioProcessor&);

        void paint (juce::Graphics&) override;
        void mouseMove (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;

        std::function<void()> onPresetChanged;

    private:
        enum class Zone { none, previous, next, readout };

        void timerCallback() override;
        void step (int delta);
        void showPresetMenu();
        Zone zoneAt (float x) const;

        SlapLayAudioProcessor& processor;
        int lastPreset = -1;
        bool lastDirty = false;
        Zone hoverZone = Zone::none;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderPillBar)
    };
} // namespace studio::ui
