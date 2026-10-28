#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "ui/StudioLookAndFeel.h"

#include <memory>

class SlapLayEditorContent;

class SlapLayAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SlapLayAudioProcessorEditor (SlapLayAudioProcessor&);
    ~SlapLayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SlapLayAudioProcessor& processor;
    studio::ui::StudioLookAndFeel lookAndFeel;
    std::unique_ptr<SlapLayEditorContent> content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlapLayAudioProcessorEditor)
};
