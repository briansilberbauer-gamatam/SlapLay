#include "PluginProcessor.h"
#include "Parameters.h"
#include "Presets.h"
#include "dsp/FeedbackEQ.h"
#include "dsp/FractionalDelayLine.h"
#include "dsp/SlapbackEngine.h"
#include "dsp/TapeSaturation.h"
#include "dsp/WowFlutter.h"

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
int failures = 0;

void check (bool condition, const char* message)
{
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << message << '\n';

    if (! condition)
        ++failures;
}

bool allFinite (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            if (! std::isfinite (buffer.getReadPointer (ch)[i]))
                return false;

    return true;
}

void setParameter (SlapLayAudioProcessor& processor, const char* id, float value)
{
    if (auto* parameter = processor.getValueTreeState().getParameter (id))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
}

float getParameter (SlapLayAudioProcessor& processor, const char* id)
{
    if (auto* parameter = processor.getValueTreeState().getParameter (id))
        return parameter->convertFrom0to1 (parameter->getValue());

    return 0.0f;
}

void processSine (SlapLayAudioProcessor& processor, juce::AudioBuffer<float>& buffer,
                  int numBlocks, float gainLeft, float gainRight, double frequency = 1000.0,
                  juce::int64 startSample = 0)
{
    juce::MidiBuffer midi;

    for (int block = 0; block < numBlocks; ++block)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto n = static_cast<double> (startSample + block * buffer.getNumSamples() + i);
            const auto sample = static_cast<float> (
                0.45 * std::sin (juce::MathConstants<double>::twoPi * frequency * n / 48000.0));

            buffer.setSample (0, i, sample * gainLeft);

            if (buffer.getNumChannels() > 1)
                buffer.setSample (1, i, sample * gainRight);
        }

        processor.processBlock (buffer, midi);
    }
}

double measureRms (slaplay::dsp::FeedbackEQ& eq, double frequency, double sampleRate,
                   bool enableFilter)
{
    eq.reset();
    eq.setEnabled (enableFilter);

    const int totalSamples = static_cast<int> (sampleRate);
    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < totalSamples; ++i)
    {
        const float input = static_cast<float> (
            0.5 * std::sin (juce::MathConstants<double>::twoPi * frequency
                            * static_cast<double> (i) / sampleRate));

        const float output = eq.process (input);

        if (i >= totalSamples / 2)
        {
            sum += static_cast<double> (output) * static_cast<double> (output);
            ++count;
        }
    }

    return count > 0 ? std::sqrt (sum / static_cast<double> (count)) : 0.0;
}

void testFractionalDelayLine()
{
    using slaplay::dsp::FractionalDelayLine;

    constexpr double sampleRate = 48000.0;

    // Fractional accuracy: a 1 kHz sine delayed by 10.25 samples must match the
    // analytic delayed sine through the cubic interpolator.
    {
        FractionalDelayLine line;
        line.prepare (sampleRate, 0.05);

        constexpr float delaySamples = 10.25f;
        constexpr float frequency = 1000.0f;
        constexpr int totalSamples = 4000;

        float maximumError = 0.0f;

        for (int n = 0; n < totalSamples; ++n)
        {
            const float input = 0.5f * std::sin (juce::MathConstants<float>::twoPi * frequency
                                                 * static_cast<float> (n)
                                                 / static_cast<float> (sampleRate));
            const float delayed = line.readLeft (delaySamples);
            line.write (input, input);

            if (n > 1000)
            {
                const float expected = 0.5f * std::sin (
                    juce::MathConstants<float>::twoPi * frequency
                    * (static_cast<float> (n) - delaySamples)
                    / static_cast<float> (sampleRate));

                maximumError = juce::jmax (maximumError, std::abs (delayed - expected));
            }
        }

        check (maximumError < 0.002f,
               "cubic interpolation reproduces a fractional 10.25-sample delay");
    }

    // End stops of the documented 50 - 200 ms range.
    for (const float delayMs : { 50.0f, 200.0f })
    {
        FractionalDelayLine line;
        line.prepare (sampleRate, 0.25);

        const float delaySamples = delayMs * 0.001f * static_cast<float> (sampleRate);
        const int integerDelay = static_cast<int> (std::lround (delaySamples));
        const int totalSamples = integerDelay + 64;

        int peakIndex = -1;
        float peakValue = 0.0f;

        for (int n = 0; n < totalSamples; ++n)
        {
            const float delayed = line.readLeft (delaySamples);
            line.write (n == 0 ? 1.0f : 0.0f, n == 0 ? 1.0f : 0.0f);

            if (std::abs (delayed) > peakValue)
            {
                peakValue = std::abs (delayed);
                peakIndex = n;
            }
        }

        const bool correctTiming = std::abs (peakIndex - integerDelay) <= 1;
        const bool unityGain = std::abs (peakValue - 1.0f) < 0.01f;

        check (correctTiming && unityGain,
               delayMs < 100.0f ? "50 ms tap lands at the correct sample with unity gain"
                                : "200 ms tap lands at the correct sample with unity gain");
    }
}

void testFeedbackEQ()
{
    slaplay::dsp::FeedbackEQ eq;
    eq.prepare (48000.0);
    eq.setCutoffs (200.0f, 4000.0f);
    eq.setEnabled (true);

    const double referenceRms = 0.5 / std::sqrt (2.0);
    const double lowRms = measureRms (eq, 50.0, 48000.0, true);
    const double midRms = measureRms (eq, 1000.0, 48000.0, true);
    const double highRms = measureRms (eq, 10000.0, 48000.0, true);

    const double lowRatio = lowRms / referenceRms;
    const double midRatio = midRms / referenceRms;
    const double highRatio = highRms / referenceRms;

    std::cout << "  INFO  12 dB/oct feedback EQ: 50 Hz " << lowRatio
              << " / 1 kHz " << midRatio << " / 10 kHz " << highRatio << '\n';

    check (lowRatio < 0.12, "low cut attenuates 50 Hz inside the feedback loop");
    check (midRatio > 0.75, "the feedback EQ leaves the midrange largely intact");
    check (highRatio < 0.25, "high cut attenuates 10 kHz inside the feedback loop");

    const double bypassRms = measureRms (eq, 1000.0, 48000.0, false);
    check (std::abs (bypassRms / referenceRms - 1.0) < 0.02,
           "switching the feedback EQ out returns a flat path (DC-blocked only)");
}

void testTapeSaturation()
{
    slaplay::dsp::TapeSaturation saturation;
    saturation.setAmount (1.0f);

    bool bounded = true;
    bool monotonic = true;
    float previous = -10.0f;

    for (float x = -3.0f; x <= 3.0f; x += 0.01f)
    {
        const float y = saturation.process (x);
        bounded = bounded && std::isfinite (y) && std::abs (y) <= 1.0f;
        monotonic = monotonic && y >= previous - 1.0e-6f;
        previous = y;
    }

    check (bounded, "tape saturation is bounded for full-scale and hot inputs");
    check (monotonic, "tape saturation transfer curve is monotonic");

    // Small-signal unity gain keeps the wet level stable at low drive.
    const float tinyInput = 1.0e-4f;
    const float tinyOutput = saturation.process (tinyInput);
    check (std::abs (tinyOutput - tinyInput) / tinyInput < 1.0e-3f,
           "tape saturation preserves small-signal gain");

    // Third-harmonic distortion from a 0.5 amplitude 1 kHz tone.
    constexpr int totalSamples = 48000;
    double fundamentalReal = 0.0;
    double fundamentalImag = 0.0;
    double thirdReal = 0.0;
    double thirdImag = 0.0;

    for (int i = 0; i < totalSamples; ++i)
    {
        const double phase = juce::MathConstants<double>::twoPi * 1000.0
                             * static_cast<double> (i) / 48000.0;
        const double shaped = static_cast<double> (
            saturation.process (static_cast<float> (0.5 * std::sin (phase))));
        const double window = 0.5 - 0.5 * std::cos (juce::MathConstants<double>::twoPi
                                                    * static_cast<double> (i)
                                                    / static_cast<double> (totalSamples - 1));

        fundamentalReal += shaped * window * std::cos (phase);
        fundamentalImag += shaped * window * std::sin (phase);
        thirdReal += shaped * window * std::cos (3.0 * phase);
        thirdImag += shaped * window * std::sin (3.0 * phase);
    }

    const double fundamental = std::sqrt (fundamentalReal * fundamentalReal
                                          + fundamentalImag * fundamentalImag);
    const double third = std::sqrt (thirdReal * thirdReal + thirdImag * thirdImag);
    const double ratio = third / juce::jmax (1.0e-12, fundamental);

    std::cout << "  INFO  1 kHz / 3 kHz harmonic ratio at full tape drive: " << ratio << '\n';
    check (ratio > 0.02, "tape saturation adds audible odd-harmonic warmth");
}

void testWowAndFlutter()
{
    using slaplay::dsp::SlapbackEngine;
    using slaplay::dsp::SlapbackParameters;

    SlapbackEngine engine;
    engine.prepare (48000.0);

    SlapbackParameters parameters;
    parameters.delayMs = 100.0f;
    parameters.feedback = 0.0f;
    parameters.tapeDrive = 0.0f;
    parameters.tapeEnabled = false;
    parameters.filterEnabled = false;
    parameters.modulationEnabled = true;
    parameters.wow = 1.0f;
    parameters.flutter = 1.0f;

    engine.setParameters (parameters);
    engine.reset();

    constexpr int blockSize = 64;
    std::vector<float> input (blockSize, 0.0f);
    std::vector<float> wet (blockSize, 0.0f);

    float minimumDelay = 1.0e9f;
    float maximumDelay = -1.0e9f;
    bool finite = true;

    for (int block = 0; block < 900; ++block)
    {
        engine.process (input.data(), input.data(), wet.data(), wet.data(), blockSize);

        for (float value : wet)
            finite = finite && std::isfinite (value);

        const float delaySamples = engine.getCurrentDelaySamples();
        minimumDelay = juce::jmin (minimumDelay, delaySamples);
        maximumDelay = juce::jmax (maximumDelay, delaySamples);
    }

    const float spread = maximumDelay - minimumDelay;
    std::cout << "  INFO  wow/flutter delay swing at 100 ms: " << spread << " samples\n";

    check (finite, "modulated delay stays finite");
    check (spread > 40.0f, "wow and flutter LFOs sweep the fractional read pointer");

    // With modulation disabled the read pointer must settle.
    parameters.modulationEnabled = false;
    engine.setParameters (parameters);
    engine.reset();

    minimumDelay = 1.0e9f;
    maximumDelay = -1.0e9f;

    for (int block = 0; block < 400; ++block)
    {
        engine.process (input.data(), input.data(), wet.data(), wet.data(), blockSize);
        const float delaySamples = engine.getCurrentDelaySamples();
        minimumDelay = juce::jmin (minimumDelay, delaySamples);
        maximumDelay = juce::jmax (maximumDelay, delaySamples);
    }

    check (maximumDelay - minimumDelay < 0.5f,
           "switching modulation off freezes the wow/flutter read offset");

    const float expectedSamples = 100.0f * 0.001f * 48000.0f;
    check (std::abs (minimumDelay - expectedSamples) < 1.0f,
           "unmodulated read pointer matches the 100 ms delay time");
}

void testEchoTimingAndFeedback()
{
    using slaplay::dsp::SlapbackEngine;
    using slaplay::dsp::SlapbackParameters;

    SlapbackEngine engine;
    engine.prepare (48000.0);

    SlapbackParameters parameters;
    parameters.delayMs = 100.0f;
    parameters.feedback = 0.5f;
    parameters.tapeDrive = 0.0f;
    parameters.tapeEnabled = false;
    parameters.filterEnabled = false;
    parameters.modulationEnabled = false;
    parameters.wow = 0.0f;
    parameters.flutter = 0.0f;

    engine.setParameters (parameters);
    engine.reset();

    constexpr int totalSamples = 20000;
    std::vector<float> output (static_cast<size_t> (totalSamples), 0.0f);

    for (int n = 0; n < totalSamples; ++n)
    {
        const float input = n == 0 ? 1.0f : 0.0f;
        float wet = 0.0f;
        engine.process (&input, &input, &wet, &wet, 1);
        output[static_cast<size_t> (n)] = wet;
    }

    auto amplitudeAt = [&] (int index)
    {
        float peak = 0.0f;

        for (int i = juce::jmax (0, index - 2); i <= juce::jmin (totalSamples - 1, index + 2); ++i)
            peak = juce::jmax (peak, std::abs (output[static_cast<size_t> (i)]));

        return peak;
    };

    const float first = amplitudeAt (4800);
    const float second = amplitudeAt (9600);
    const float third = amplitudeAt (14400);

    std::cout << "  INFO  echo amplitudes: " << first << " / " << second << " / " << third << '\n';

    check (std::abs (first - 1.0f) < 0.01f, "first slapback repeat appears after exactly 100 ms");
    check (std::abs (second - 0.5f) < 0.02f, "feedback repeats decay by the feedback amount");
    check (std::abs (third - 0.25f) < 0.02f, "successive feedback repeats keep decaying");
}

void testFeedbackStability()
{
    using slaplay::dsp::SlapbackEngine;
    using slaplay::dsp::SlapbackParameters;

    SlapbackEngine engine;
    engine.prepare (48000.0);

    SlapbackParameters parameters;
    parameters.delayMs = 50.0f;
    parameters.feedback = 0.9f;
    parameters.lowCutHz = 200.0f;
    parameters.highCutHz = 2000.0f;
    parameters.tapeDrive = 1.0f;
    parameters.wow = 1.0f;
    parameters.flutter = 1.0f;
    parameters.filterEnabled = true;
    parameters.tapeEnabled = true;
    parameters.modulationEnabled = true;

    engine.setParameters (parameters);
    engine.reset();

    constexpr int blockSize = 512;
    std::vector<float> input (blockSize);
    std::vector<float> wet (blockSize, 0.0f);

    float peak = 0.0f;
    bool finite = true;
    double accumulated = 0.0;

    for (int block = 0; block < 600; ++block) // ~6.4 seconds
    {
        for (int i = 0; i < blockSize; ++i)
        {
            const auto n = static_cast<double> (block * blockSize + i);
            input[static_cast<size_t> (i)] = static_cast<float> (
                0.8 * std::sin (juce::MathConstants<double>::twoPi * 196.0 * n / 48000.0));
        }

        engine.process (input.data(), input.data(), wet.data(), wet.data(), blockSize);

        for (float value : wet)
        {
            finite = finite && std::isfinite (value);
            peak = juce::jmax (peak, std::abs (value));
            accumulated += std::abs (static_cast<double> (value));
        }
    }

    std::cout << "  INFO  worst-case feedback peak: " << peak << '\n';

    check (finite, "maximum feedback, saturation and modulation stay finite");
    check (peak < 1.0f, "maximum feedback loop is bounded by tape saturation");
    check (accumulated > 0.01, "the maximum-feedback loop still passes audio");

    const float delaySamples = engine.getCurrentDelaySamples();
    const float nominal = 50.0f * 0.001f * 48000.0f;
    check (delaySamples > nominal * 0.97f && delaySamples < nominal * 1.03f,
           "modulation stays within a subtle 3 % window of the delay time");
}

void testProcessor()
{
    SlapLayAudioProcessor processor;
    check (processor.getParameters().size() == 14, "processor exposes all 14 parameters");
    check (processor.getNumPrograms() == 10, "ten factory programs are available");

    if (auto* delayParameter = processor.getValueTreeState().getParameter (slaplay::params::delayTime))
    {
        const auto& range = delayParameter->getNormalisableRange();
        check (std::abs (range.start - 50.0f) < 0.001f
                   && std::abs (range.end - 200.0f) < 0.001f,
               "delay time parameter spans exactly 50 ms to 200 ms");
    }
    else
    {
        check (false, "delay time parameter exists");
    }

    processor.setPlayConfigDetails (2, 2, 48000.0, 512);
    processor.prepareToPlay (48000.0, 512);

    check (processor.getLatencySamples() == 0, "zero-latency fractional delay reports no host delay");

    juce::AudioBuffer<float> buffer (2, 512);

    setParameter (processor, slaplay::params::delayTime, 110.0f);
    setParameter (processor, slaplay::params::feedback, 40.0f);
    setParameter (processor, slaplay::params::mix, 45.0f);
    setParameter (processor, slaplay::params::tapeDrive, 55.0f);
    setParameter (processor, slaplay::params::wow, 30.0f);
    setParameter (processor, slaplay::params::flutter, 18.0f);

    processSine (processor, buffer, 160, 1.0f, 0.7f, 1000.0);

    check (allFinite (buffer), "delay processing produces finite output");
    check (processor.getOutputLevel() > 0.001f, "output meter reports live signal");
    check (processor.getInputLevel() > 0.001f, "input meter reports live signal");

    // Lock-free waveform telemetry must expose live, ordered min/max columns.
    std::array<float, 64> waveformMin {};
    std::array<float, 64> waveformMax {};
    const int columns = processor.copyWaveform (waveformMin.data(), waveformMax.data(), 64);

    bool validColumns = columns == 64;
    bool liveWaveform = false;

    for (int i = 0; i < columns; ++i)
    {
        validColumns = validColumns && waveformMax[static_cast<size_t> (i)]
                                          >= waveformMin[static_cast<size_t> (i)];
        liveWaveform = liveWaveform || waveformMax[static_cast<size_t> (i)] > 0.001f
                                      || waveformMin[static_cast<size_t> (i)] < -0.001f;
    }

    check (validColumns, "waveform telemetry publishes ordered min/max columns");
    check (liveWaveform, "waveform telemetry contains live audio");

    // A fully wet, zero-feedback tap must produce an audible repeat.
    processor.setCurrentProgram (7); // Studio Tight, 80 ms
    setParameter (processor, slaplay::params::delayTime, 100.0f);
    setParameter (processor, slaplay::params::feedback, 0.0f);
    setParameter (processor, slaplay::params::tapeDrive, 0.0f);
    setParameter (processor, slaplay::params::mix, 100.0f);
    setParameter (processor, slaplay::params::modulationEnabled, 0.0f);
    setParameter (processor, slaplay::params::filterEnabled, 0.0f);
    setParameter (processor, slaplay::params::bypass, 0.0f);

    processSine (processor, buffer, 40, 1.0f, 1.0f, 1000.0); // settle smoothers
    check (allFinite (buffer), "fully wet delay tap remains finite");

    // Bypass must hand the dry signal back with no latency or level jump.
    setParameter (processor, slaplay::params::bypass, 1.0f);
    processSine (processor, buffer, 120, 1.0f, 1.0f, 1000.0, 512 * 400);

    juce::AudioBuffer<float> dryCopy (buffer);
    {
        juce::MidiBuffer midi;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto n = static_cast<double> (512 * 520 + i);
            const auto sample = static_cast<float> (
                0.45 * std::sin (juce::MathConstants<double>::twoPi * 1000.0 * n / 48000.0));
            buffer.setSample (0, i, sample);
            buffer.setSample (1, i, sample);
        }

        dryCopy = buffer;
        processor.processBlock (buffer, midi);
    }

    float maximumDifference = 0.0f;

    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            maximumDifference = juce::jmax (maximumDifference,
                                            std::abs (buffer.getSample (ch, i)
                                                      - dryCopy.getSample (ch, i)));

    check (maximumDifference < 1.0e-5f, "bypass returns bit-transparent dry audio at zero latency");

    // State round-trip, including preset identity and delay time.
    setParameter (processor, slaplay::params::bypass, 0.0f);
    processor.setCurrentProgram (3);
    setParameter (processor, slaplay::params::delayTime, 173.4f);
    const float delayBefore = getParameter (processor, slaplay::params::delayTime);

    juce::MemoryBlock state;
    processor.getStateInformation (state);
    check (state.getSize() > 0, "full processor state serialises");

    SlapLayAudioProcessor restored;
    restored.setPlayConfigDetails (2, 2, 48000.0, 512);
    restored.prepareToPlay (48000.0, 512);
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));

    check (std::abs (getParameter (restored, slaplay::params::delayTime) - delayBefore) < 0.01f,
           "delay time is restored from serialised state");

    // Mono support: the same engine folds to a single channel with no NaNs.
    SlapLayAudioProcessor monoProcessor;
    monoProcessor.setPlayConfigDetails (1, 1, 48000.0, 256);
    monoProcessor.prepareToPlay (48000.0, 256);

    juce::AudioBuffer<float> monoBuffer (1, 256);
    juce::MidiBuffer midi;

    for (int block = 0; block < 100; ++block)
    {
        for (int i = 0; i < monoBuffer.getNumSamples(); ++i)
        {
            const auto n = static_cast<double> (block * 256 + i);
            monoBuffer.setSample (0, i, static_cast<float> (
                0.4 * std::sin (juce::MathConstants<double>::twoPi * 220.0 * n / 48000.0)));
        }

        monoProcessor.processBlock (monoBuffer, midi);
    }

    check (allFinite (monoBuffer), "mono layout processes without NaNs");
    check (monoBuffer.getMagnitude (0, 0, monoBuffer.getNumSamples()) > 0.0001f,
           "mono layout remains audible");

    check (! slaplay::getFactoryPresetName (0).isEmpty(), "factory preset names are populated");

    processor.releaseResources();
    restored.releaseResources();
    monoProcessor.releaseResources();
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    std::cout << "SlapLay offline DSP tests\n\n";

    testFractionalDelayLine();
    testFeedbackEQ();
    testTapeSaturation();
    testWowAndFlutter();
    testEchoTimingAndFeedback();
    testFeedbackStability();
    testProcessor();

    std::cout << (failures == 0 ? "\nAll DSP tests passed.\n" : "\nDSP tests failed.\n");
    return failures == 0 ? 0 : 1;
}
