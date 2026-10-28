#include "PluginProcessor.h"

#include "Parameters.h"
#include "PluginEditor.h"
#include "Presets.h"

#include <algorithm>
#include <cmath>

namespace
{
using APVTS = juce::AudioProcessorValueTreeState;

std::unique_ptr<juce::AudioParameterFloat> makeFloat (const char* id,
                                                     const char* name,
                                                     juce::NormalisableRange<float> range,
                                                     float defaultValue,
                                                     const char* label)
{
    return std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id, 1 },
        name,
        range,
        defaultValue,
        juce::AudioParameterFloatAttributes().withLabel (label));
}

std::unique_ptr<juce::AudioParameterBool> makeBool (const char* id,
                                                    const char* name,
                                                    bool defaultValue)
{
    return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, defaultValue);
}
} // namespace

SlapLayAudioProcessor::SlapLayAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    inputGainParam = apvts.getRawParameterValue (slaplay::params::inputGain);
    outputGainParam = apvts.getRawParameterValue (slaplay::params::outputGain);
    mixParam = apvts.getRawParameterValue (slaplay::params::mix);
    bypassParam = apvts.getRawParameterValue (slaplay::params::bypass);
    delayTimeParam = apvts.getRawParameterValue (slaplay::params::delayTime);
    feedbackParam = apvts.getRawParameterValue (slaplay::params::feedback);
    lowCutParam = apvts.getRawParameterValue (slaplay::params::lowCut);
    highCutParam = apvts.getRawParameterValue (slaplay::params::highCut);
    tapeDriveParam = apvts.getRawParameterValue (slaplay::params::tapeDrive);
    wowParam = apvts.getRawParameterValue (slaplay::params::wow);
    flutterParam = apvts.getRawParameterValue (slaplay::params::flutter);
    filterEnabledParam = apvts.getRawParameterValue (slaplay::params::filterEnabled);
    tapeEnabledParam = apvts.getRawParameterValue (slaplay::params::tapeEnabled);
    modulationEnabledParam = apvts.getRawParameterValue (slaplay::params::modulationEnabled);

    for (const auto* parameterID : slaplay::params::allParameterIDs)
        apvts.addParameterListener (parameterID, this);
}

SlapLayAudioProcessor::~SlapLayAudioProcessor()
{
    for (const auto* parameterID : slaplay::params::allParameterIDs)
        apvts.removeParameterListener (parameterID, this);
}

juce::AudioProcessorValueTreeState::ParameterLayout SlapLayAudioProcessor::createParameterLayout()
{
    using namespace slaplay::params;

    APVTS::ParameterLayout layout;

    auto input = std::make_unique<juce::AudioProcessorParameterGroup> ("input", "Input", " | ");
    input->addChild (makeFloat (inputGain, "Input Gain", { -18.0f, 18.0f, 0.01f }, 0.0f, "dB"));
    layout.add (std::move (input));

    auto echo = std::make_unique<juce::AudioProcessorParameterGroup> ("echo", "Tape Echo", " | ");
    echo->addChild (makeFloat (delayTime, "Delay Time", { minimumDelayMs, maximumDelayMs, 0.1f },
                               defaultDelayMs, "ms"));
    echo->addChild (makeFloat (feedback, "Feedback", { 0.0f, 90.0f, 0.1f }, 36.0f, "%"));
    echo->addChild (makeFloat (lowCut, "Low Cut", { minimumLowCutHz, maximumLowCutHz, 1.0f, 0.45f },
                               150.0f, "Hz"));
    echo->addChild (makeFloat (highCut, "High Cut", { minimumHighCutHz, maximumHighCutHz, 1.0f, 0.50f },
                               3800.0f, "Hz"));
    layout.add (std::move (echo));

    auto tape = std::make_unique<juce::AudioProcessorParameterGroup> ("tape", "Tape Character", " | ");
    tape->addChild (makeFloat (tapeDrive, "Tape Drive", { 0.0f, 100.0f, 0.1f }, 38.0f, "%"));
    tape->addChild (makeFloat (wow, "Wow", { 0.0f, 100.0f, 0.1f }, 22.0f, "%"));
    tape->addChild (makeFloat (flutter, "Flutter", { 0.0f, 100.0f, 0.1f }, 12.0f, "%"));
    layout.add (std::move (tape));

    auto output = std::make_unique<juce::AudioProcessorParameterGroup> ("output", "Output", " | ");
    output->addChild (makeFloat (outputGain, "Output Gain", { -18.0f, 18.0f, 0.01f }, 0.0f, "dB"));
    output->addChild (makeFloat (mix, "Wet / Dry Mix", { 0.0f, 100.0f, 0.1f }, 32.0f, "%"));
    output->addChild (makeBool (filterEnabled, "Feedback Filter", true));
    output->addChild (makeBool (tapeEnabled, "Tape Saturation", true));
    output->addChild (makeBool (modulationEnabled, "Wow and Flutter", true));
    output->addChild (makeBool (bypass, "Bypass", false));
    layout.add (std::move (output));

    return layout;
}

void SlapLayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = std::max (8000.0, sampleRate);
    preparedBlockSize = std::max (1, samplesPerBlock);

    engine.prepare (currentSampleRate);

    inputBuffer.setSize (processingChannels, preparedBlockSize, false, false, true);
    wetBuffer.setSize (processingChannels, preparedBlockSize, false, false, true);

    inputGainSmoothed.reset (currentSampleRate, 0.020);
    outputGainSmoothed.reset (currentSampleRate, 0.020);
    mixSmoothed.reset (currentSampleRate, 0.025);

    inputGainSmoothed.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (inputGainParam->load()));
    outputGainSmoothed.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (outputGainParam->load()));
    mixSmoothed.setCurrentAndTargetValue (bypassParam->load() > 0.5f
                                              ? 0.0f
                                              : mixParam->load() * 0.01f);

    inputLevel.store (0.0f, std::memory_order_relaxed);
    outputLevel.store (0.0f, std::memory_order_relaxed);
    outputLevelLeft.store (0.0f, std::memory_order_relaxed);
    outputLevelRight.store (0.0f, std::memory_order_relaxed);

    // The delay is a pure floating-point read with no oversampling or lookahead,
    // so the plug-in reports zero latency and never shifts host timing.
    setLatencySamples (0);

    // Keep roughly 750 waveform columns per second at any sample rate, giving
    // the central display a ~0.34 second window across its 256 columns.
    waveformSamplesPerColumn = juce::jmax (8, juce::roundToInt (currentSampleRate / 750.0));
    resetWaveformHistory();
}

void SlapLayAudioProcessor::resetWaveformHistory() noexcept
{
    for (auto& value : waveformMin)
        value.store (0.0f, std::memory_order_relaxed);

    for (auto& value : waveformMax)
        value.store (0.0f, std::memory_order_relaxed);

    waveformWriteCount.store (0, std::memory_order_relaxed);
    waveformAccumulatedSamples = 0;
    waveformAccumulatedMin = 0.0f;
    waveformAccumulatedMax = 0.0f;
}

void SlapLayAudioProcessor::captureWaveform (const juce::AudioBuffer<float>& buffer,
                                             int numSamples) noexcept
{
    const int columnSize = juce::jmax (1, waveformSamplesPerColumn);
    const bool stereo = buffer.getNumChannels() > 1;

    auto sanitise = [] (float value)
    {
        return std::isfinite (value) ? juce::jlimit (-1.0f, 1.0f, value) : 0.0f;
    };

    for (int i = 0; i < numSamples; ++i)
    {
        const float left = sanitise (buffer.getSample (0, i));
        const float right = sanitise (stereo ? buffer.getSample (1, i) : left);
        const float columnMin = juce::jmin (left, right);
        const float columnMax = juce::jmax (left, right);

        if (waveformAccumulatedSamples == 0)
        {
            waveformAccumulatedMin = columnMin;
            waveformAccumulatedMax = columnMax;
        }
        else
        {
            waveformAccumulatedMin = juce::jmin (waveformAccumulatedMin, columnMin);
            waveformAccumulatedMax = juce::jmax (waveformAccumulatedMax, columnMax);
        }

        if (++waveformAccumulatedSamples < columnSize)
            continue;

        const auto writeIndex = waveformWriteCount.load (std::memory_order_relaxed);
        const auto slot = static_cast<size_t> (writeIndex
                                               % static_cast<juce::uint32> (waveformCapacity));
        waveformMin[slot].store (waveformAccumulatedMin, std::memory_order_relaxed);
        waveformMax[slot].store (waveformAccumulatedMax, std::memory_order_relaxed);
        waveformWriteCount.store (writeIndex + 1, std::memory_order_release);

        waveformAccumulatedSamples = 0;
    }
}

int SlapLayAudioProcessor::copyWaveform (float* minColumns, float* maxColumns,
                                         int maxColumnCount) const noexcept
{
    if (minColumns == nullptr || maxColumns == nullptr || maxColumnCount <= 0)
        return 0;

    const auto written = waveformWriteCount.load (std::memory_order_acquire);
    const int available = static_cast<int> (
        juce::jmin<juce::uint32> (written, static_cast<juce::uint32> (waveformCapacity)));
    const int count = juce::jmin (available, maxColumnCount);
    const auto start = written - static_cast<juce::uint32> (count);

    for (int i = 0; i < count; ++i)
    {
        const auto slot = static_cast<size_t> ((start + static_cast<juce::uint32> (i))
                                               % static_cast<juce::uint32> (waveformCapacity));
        minColumns[i] = waveformMin[slot].load (std::memory_order_relaxed);
        maxColumns[i] = waveformMax[slot].load (std::memory_order_relaxed);
    }

    return count;
}

void SlapLayAudioProcessor::releaseResources()
{
    engine.reset();
    inputBuffer.setSize (processingChannels, 0);
    wetBuffer.setSize (processingChannels, 0);
}

void SlapLayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples == 0)
        return;

    if (numChannels < 1 || numSamples > preparedBlockSize
        || numSamples > inputBuffer.getNumSamples()
        || numSamples > wetBuffer.getNumSamples())
    {
        buffer.clear();
        return;
    }

    for (int ch = processingChannels; ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    const float inputLeftPeak = buffer.getMagnitude (0, 0, numSamples);
    const float inputRightPeak = numChannels > 1 ? buffer.getMagnitude (1, 0, numSamples)
                                                 : inputLeftPeak;
    const float inputPeak = juce::jmax (inputLeftPeak, inputRightPeak);

    const bool bypassed = bypassParam->load() > 0.5f;

    inputGainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (inputGainParam->load()));
    outputGainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (outputGainParam->load()));
    mixSmoothed.setTargetValue (bypassed ? 0.0f : mixParam->load() * 0.01f);

    slaplay::dsp::SlapbackParameters engineParameters;
    engineParameters.delayMs = delayTimeParam->load();
    engineParameters.feedback = feedbackParam->load() * 0.01f;
    engineParameters.lowCutHz = lowCutParam->load();
    engineParameters.highCutHz = highCutParam->load();
    engineParameters.tapeDrive = tapeDriveParam->load() * 0.01f;
    engineParameters.wow = wowParam->load() * 0.01f;
    engineParameters.flutter = flutterParam->load() * 0.01f;
    engineParameters.filterEnabled = filterEnabledParam->load() > 0.5f;
    engineParameters.tapeEnabled = tapeEnabledParam->load() > 0.5f;
    engineParameters.modulationEnabled = modulationEnabledParam->load() > 0.5f;
    engine.setParameters (engineParameters);

    const float* inputLeft = buffer.getReadPointer (0);
    const float* inputRight = numChannels > 1 ? buffer.getReadPointer (1) : inputLeft;

    auto* gainedLeft = inputBuffer.getWritePointer (0);
    auto* gainedRight = inputBuffer.getWritePointer (1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float inputGain = inputGainSmoothed.getNextValue();
        gainedLeft[i] = inputLeft[i] * inputGain;
        gainedRight[i] = inputRight[i] * inputGain;
    }

    engine.process (gainedLeft, gainedRight,
                    wetBuffer.getWritePointer (0), wetBuffer.getWritePointer (1),
                    numSamples);

    const float* wetLeft = wetBuffer.getReadPointer (0);
    const float* wetRight = wetBuffer.getReadPointer (1);

    auto* outputLeft = buffer.getWritePointer (0);
    auto* outputRight = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = mixSmoothed.getNextValue();
        const float outputGain = outputGainSmoothed.getNextValue();

        const float dryLeft = gainedLeft[i];
        const float dryRight = gainedRight[i];

        outputLeft[i] = (dryLeft + mix * (wetLeft[i] - dryLeft)) * outputGain;

        if (outputRight != nullptr)
            outputRight[i] = (dryRight + mix * (wetRight[i] - dryRight)) * outputGain;
    }

    const float outputLeftPeak = buffer.getMagnitude (0, 0, numSamples);
    const float outputRightPeak = numChannels > 1 ? buffer.getMagnitude (1, 0, numSamples)
                                                  : outputLeftPeak;
    const float outputPeak = juce::jmax (outputLeftPeak, outputRightPeak);

    inputLevel.store (std::isfinite (inputPeak) ? inputPeak : 0.0f, std::memory_order_relaxed);
    outputLevel.store (std::isfinite (outputPeak) ? outputPeak : 0.0f, std::memory_order_relaxed);
    outputLevelLeft.store (std::isfinite (outputLeftPeak) ? outputLeftPeak : 0.0f,
                           std::memory_order_relaxed);
    outputLevelRight.store (std::isfinite (outputRightPeak) ? outputRightPeak : 0.0f,
                            std::memory_order_relaxed);

    captureWaveform (buffer, numSamples);
}

juce::AudioProcessorParameter* SlapLayAudioProcessor::getBypassParameter() const
{
    return apvts.getParameter (slaplay::params::bypass);
}

juce::AudioProcessorEditor* SlapLayAudioProcessor::createEditor()
{
    return new SlapLayAudioProcessorEditor (*this);
}

int SlapLayAudioProcessor::getNumPrograms()
{
    return slaplay::getNumFactoryPresets();
}

int SlapLayAudioProcessor::getCurrentProgram()
{
    return currentPreset.load (std::memory_order_relaxed);
}

void SlapLayAudioProcessor::setCurrentProgram (int index)
{
    loadFactoryPreset (index);
}

const juce::String SlapLayAudioProcessor::getProgramName (int index)
{
    return slaplay::getFactoryPresetName (index);
}

void SlapLayAudioProcessor::changeProgramName (int, const juce::String&)
{
}

void SlapLayAudioProcessor::loadFactoryPreset (int index)
{
    applyPreset (index);
}

void SlapLayAudioProcessor::applyPreset (int index)
{
    const auto& presets = slaplay::getFactoryPresets();

    if (index < 0 || index >= static_cast<int> (presets.size()))
        return;

    applyingPreset.store (true, std::memory_order_relaxed);

    for (auto* parameter : getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (parameter))
            ranged->setValueNotifyingHost (ranged->getDefaultValue());

    for (const auto& value : presets[static_cast<size_t> (index)].overrides)
    {
        if (auto* parameter = apvts.getParameter (value.id))
        {
            const auto& range = parameter->getNormalisableRange();
            const float deNormalised = juce::jlimit (range.start, range.end, value.value);
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (deNormalised));
        }
    }

    currentPreset.store (index, std::memory_order_relaxed);
    presetDirty.store (false, std::memory_order_relaxed);
    applyingPreset.store (false, std::memory_order_relaxed);
}

void SlapLayAudioProcessor::parameterChanged (const juce::String&, float)
{
    if (! applyingPreset.load (std::memory_order_relaxed))
        presetDirty.store (true, std::memory_order_relaxed);
}

void SlapLayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", currentPreset.load (std::memory_order_relaxed), nullptr);
    state.setProperty ("presetDirty", presetDirty.load (std::memory_order_relaxed), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void SlapLayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);

    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    auto tree = juce::ValueTree::fromXml (*xml);

    if (! tree.isValid())
        return;

    const int presetIndex = static_cast<int> (tree.getProperty ("presetIndex", 0));
    const bool dirty = static_cast<bool> (tree.getProperty ("presetDirty", false));

    applyingPreset.store (true, std::memory_order_relaxed);
    apvts.replaceState (tree);
    applyingPreset.store (false, std::memory_order_relaxed);

    currentPreset.store (juce::jlimit (0, slaplay::getNumFactoryPresets() - 1, presetIndex),
                         std::memory_order_relaxed);
    presetDirty.store (dirty, std::memory_order_relaxed);
}

bool SlapLayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainIn != mainOut)
        return false;

    return mainIn == juce::AudioChannelSet::mono() || mainIn == juce::AudioChannelSet::stereo();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SlapLayAudioProcessor();
}
