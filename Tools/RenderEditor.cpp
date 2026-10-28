#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <cmath>
#include <iostream>
#include <memory>

class SlapLayRenderApplication final : public juce::JUCEApplication,
                                       private juce::Timer
{
public:
    const juce::String getApplicationName() override { return "SlapLay Screenshot"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted (const juce::String&) override {}
    void systemRequestedQuit() override {}
    void shutdown() override {}

    void initialise (const juce::String& commandLine) override
    {
        outputFile = juce::File::getCurrentWorkingDirectory().getChildFile ("dist/slaplay-ui.png");

        for (const auto& argument : juce::StringArray::fromTokens (commandLine, true))
        {
            if (argument.startsWith ("--out="))
            {
                const auto path = argument.substring (6);
                outputFile = juce::File::isAbsolutePath (path)
                                 ? juce::File (path)
                                 : juce::File::getCurrentWorkingDirectory().getChildFile (path);
            }
        }

        outputFile = outputFile.getFullPathName();
        outputFile.getParentDirectory().createDirectory();

        processor = std::make_unique<SlapLayAudioProcessor>();
        processor->setPlayConfigDetails (2, 2, 48000.0, 1024);
        processor->prepareToPlay (48000.0, 1024);
        processor->setCurrentProgram (0); // Sunset Slap, with the full tape path active.

        // Park the pointer away from the unit before it appears, so the captured
        // snapshot never carries a stale hover highlight.
        juce::Desktop::getInstance().getMainMouseSource().setScreenPosition ({ 5.0f, 5.0f });

        editor = std::unique_ptr<juce::AudioProcessorEditor> (processor->createEditor());

        if (editor == nullptr)
        {
            std::cerr << "Could not create the plugin editor.\n";
            setApplicationReturnValue (1);
            quit();
            return;
        }

        editor->setSize (1180, 720);
        editor->addToDesktop (0);
        editor->setTopLeftPosition (30, 30);
        editor->setVisible (true);
        editor->toFront (true);

        testBuffer.setSize (2, 1024);
        startTimerHz (30);

        juce::Timer::callAfterDelay (1800, [this] { capture(); });
    }

private:
    void timerCallback() override
    {
        if (processor == nullptr)
            return;

        for (int i = 0; i < testBuffer.getNumSamples(); ++i)
        {
            const double t = static_cast<double> (sampleCounter + i) / 48000.0;

            // A plucked guitar-like burst every 0.55 s, so the echo taps and the
            // decaying repeats are both visible in the live waveform screen.
            const double pluckPeriod = 0.55;
            const double phase = std::fmod (t, pluckPeriod);
            const float envelope = static_cast<float> (std::exp (-phase * 7.0));
            const double fundamental = juce::MathConstants<double>::twoPi * 196.0 * t;

            const float body = static_cast<float> (
                0.30 * std::sin (fundamental)
                + 0.12 * std::sin (2.0 * fundamental)
                + 0.05 * std::sin (3.0 * fundamental));

            const float pick = 0.05f * static_cast<float> (
                std::sin (juce::MathConstants<double>::twoPi * 2400.0 * t))
                * static_cast<float> (std::exp (-phase * 45.0));

            const float sample = envelope * (body + pick);

            testBuffer.setSample (0, i, sample);
            testBuffer.setSample (1, i, sample * 0.92f);
        }

        sampleCounter += testBuffer.getNumSamples();
        juce::MidiBuffer midi;
        processor->processBlock (testBuffer, midi);
    }

    void capture()
    {
        if (editor == nullptr)
            return;

        stopTimer();
        const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 2.0f);

        if (image.isValid())
        {
            outputFile.deleteFile();

            if (auto stream = std::unique_ptr<juce::FileOutputStream> (outputFile.createOutputStream()))
            {
                juce::PNGImageFormat png;
                const bool written = png.writeImageToStream (image, *stream);
                stream->flush();

                if (written)
                    std::cout << "Rendered " << outputFile.getFullPathName() << '\n';
                else
                    std::cerr << "Failed to encode PNG.\n";
            }
            else
            {
                std::cerr << "Could not open " << outputFile.getFullPathName() << '\n';
            }
        }
        else
        {
            std::cerr << "Editor snapshot was invalid.\n";
        }

        processor->releaseResources();
        editor->removeFromDesktop();
        editor.reset();
        processor.reset();
        quit();
    }

    std::unique_ptr<SlapLayAudioProcessor> processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor;
    juce::File outputFile;
    juce::AudioBuffer<float> testBuffer;
    juce::int64 sampleCounter = 0;
};

START_JUCE_APPLICATION (SlapLayRenderApplication)
