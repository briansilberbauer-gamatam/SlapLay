#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "dsp/SlapbackEngine.h"

#include <array>
#include <atomic>
#include <memory>

class SlapLayAudioProcessor final : public juce::AudioProcessor,
                                    private juce::AudioProcessorValueTreeState::Listener
{
public:
    SlapLayAudioProcessor();
    ~SlapLayAudioProcessor() override;

    using juce::AudioProcessor::processBlock;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // Let the host's own bypass control the panel switch, so there is only one
    // Bypass parameter and zero-latency dry audio is guaranteed in the wrapper.
    juce::AudioProcessorParameter* getBypassParameter() const override;

    const juce::String getName() const override { return "SlapLay"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return apvts; }
    const juce::AudioProcessorValueTreeState& getValueTreeState() const noexcept { return apvts; }

    void loadFactoryPreset (int index);
    int getCurrentPresetIndex() const noexcept { return currentPreset.load (std::memory_order_relaxed); }
    bool isPresetDirty() const noexcept { return presetDirty.load (std::memory_order_relaxed); }

    float getInputLevel() const noexcept { return inputLevel.load (std::memory_order_relaxed); }
    float getOutputLevel() const noexcept { return outputLevel.load (std::memory_order_relaxed); }
    float getOutputLevelLeft() const noexcept { return outputLevelLeft.load (std::memory_order_relaxed); }
    float getOutputLevelRight() const noexcept { return outputLevelRight.load (std::memory_order_relaxed); }

    // Lock-free display history for the central waveform screen. The audio thread
    // publishes min/max columns; the message thread copies them in chronological
    // order without locking or allocating.
    static constexpr int waveformCapacity = 1024;
    int copyWaveform (float* minColumns, float* maxColumns, int maxColumnCount) const noexcept;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void applyPreset (int index);
    void resetWaveformHistory() noexcept;
    void captureWaveform (const juce::AudioBuffer<float>& buffer, int numSamples) noexcept;

    static constexpr int processingChannels = 2;

    juce::AudioProcessorValueTreeState apvts;

    slaplay::dsp::SlapbackEngine engine;
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> wetBuffer;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;

    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float>* delayTimeParam = nullptr;
    std::atomic<float>* feedbackParam = nullptr;
    std::atomic<float>* lowCutParam = nullptr;
    std::atomic<float>* highCutParam = nullptr;
    std::atomic<float>* tapeDriveParam = nullptr;
    std::atomic<float>* wowParam = nullptr;
    std::atomic<float>* flutterParam = nullptr;
    std::atomic<float>* filterEnabledParam = nullptr;
    std::atomic<float>* tapeEnabledParam = nullptr;
    std::atomic<float>* modulationEnabledParam = nullptr;

    std::atomic<int> currentPreset { 0 };
    std::atomic<bool> presetDirty { false };
    std::atomic<bool> applyingPreset { false };

    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };
    std::atomic<float> outputLevelLeft { 0.0f };
    std::atomic<float> outputLevelRight { 0.0f };

    std::array<std::atomic<float>, waveformCapacity> waveformMin {};
    std::array<std::atomic<float>, waveformCapacity> waveformMax {};
    std::atomic<juce::uint32> waveformWriteCount { 0 };
    int waveformSamplesPerColumn = 64;
    int waveformAccumulatedSamples = 0;
    float waveformAccumulatedMin = 0.0f;
    float waveformAccumulatedMax = 0.0f;

    double currentSampleRate = 48000.0;
    int preparedBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlapLayAudioProcessor)
};
