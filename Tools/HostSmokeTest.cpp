#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <cmath>
#include <iostream>

namespace
{
int failures = 0;

void check (bool condition, const char* message)
{
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << message << '\n';

    if (! condition)
        ++failures;
}

void fillSignal (juce::AudioBuffer<float>& buffer, int offset)
{
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto n = static_cast<double> (offset + i);
        const auto fundamental = juce::MathConstants<double>::twoPi * 196.0 * n / 48000.0;
        const float left = static_cast<float> (
            0.30 * std::sin (fundamental) + 0.08 * std::sin (2.0 * fundamental));
        const float right = static_cast<float> (
            0.24 * std::sin (fundamental) + 0.03 * std::sin (3.0 * fundamental));

        buffer.setSample (0, i, left);
        buffer.setSample (1, i, right);
    }
}

bool allFinite (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            if (! std::isfinite (buffer.getReadPointer (ch)[i]))
                return false;

    return true;
}
} // namespace

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    if (argc < 2)
    {
        std::cerr << "Usage: SlapLayHostSmokeTest /path/to/SlapLay.vst3\n";
        return 2;
    }

    const juce::File pluginFile { juce::String (argv[1]) };

    if (! pluginFile.exists())
    {
        std::cerr << "Plugin bundle not found: " << pluginFile.getFullPathName() << '\n';
        return 2;
    }

    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();

    juce::OwnedArray<juce::PluginDescription> descriptions;

    for (auto* format : formatManager.getFormats())
        if (auto* vst3 = dynamic_cast<juce::VST3PluginFormat*> (format))
            vst3->findAllTypesForFile (descriptions, pluginFile.getFullPathName());

    check (! descriptions.isEmpty(), "VST3 scanner discovers the plugin");

    if (descriptions.isEmpty())
        return 1;

    auto& description = *descriptions[0];

    std::cout << "  INFO  " << description.name << " / " << description.manufacturerName
              << " / " << description.version << '\n';

    juce::String error;
    auto instance = formatManager.createPluginInstance (description, 48000.0, 512, error);

    check (instance != nullptr, "plugin instance can be created");

    if (instance == nullptr)
    {
        std::cerr << "  ERROR " << error << '\n';
        return 1;
    }

    auto parameters = instance->getParameters();

    std::cout << "  INFO  parameter count: " << parameters.size() << "\n";

    for (int i = 0; i < parameters.size(); ++i)
        std::cout << "  INFO  param " << i << ": " << parameters[i]->getName (64) << '\n';

    check (parameters.size() >= 14, "all 14 user parameters are exposed to the host");
    check (instance->getNumPrograms() == 10, "all ten factory programs are exposed to the host");

    instance->setPlayConfigDetails (2, 2, 48000.0, 512);
    instance->prepareToPlay (48000.0, 512);

    check (instance->getLatencySamples() == 0,
           "zero-latency tape delay reports no host latency");

    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;
    bool finite = true;

    for (int block = 0; block < 80; ++block)
    {
        fillSignal (buffer, block * 512);
        instance->processBlock (buffer, midi);
        finite = finite && allFinite (buffer);
    }

    check (finite, "audio thread produces finite output through the VST3 wrapper");
    check (buffer.getMagnitude (0, 0, buffer.getNumSamples()) > 0.0001f,
           "processed signal remains audible");

    const float markerValue = 0.37f;
    parameters[3]->setValueNotifyingHost (markerValue);

    juce::MemoryBlock state;
    instance->getStateInformation (state);
    check (state.getSize() > 0, "state can be serialised for the host");

    parameters[3]->setValueNotifyingHost (0.83f);
    instance->setStateInformation (state.getData(), static_cast<int> (state.getSize()));

    check (std::abs (parameters[3]->getValue() - markerValue) < 0.01f,
           "serialised parameter state is restored");

    instance->releaseResources();

    std::cout << (failures == 0 ? "\nHost smoke test passed.\n" : "\nHost smoke test failed.\n");
    return failures == 0 ? 0 : 1;
}
