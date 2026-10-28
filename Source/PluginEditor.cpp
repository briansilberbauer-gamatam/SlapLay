#include "PluginEditor.h"

#include "Parameters.h"
#include "Presets.h"
#include "ui/Controls.h"
#include "ui/Theme.h"
#include "ui/WaveformDisplay.h"

#include <array>
#include <cmath>
#include <utility>

using namespace studio::ui;

namespace
{
constexpr int designWidth = 1180;
constexpr int designHeight = 720;

juce::String formatDb (float value)
{
    if (std::abs (value) < 0.05f)
        return "0.0 dB";

    return (value > 0.0f ? "+" : "") + juce::String (value, 1) + " dB";
}

juce::String formatPercent (float value)
{
    return juce::String (juce::roundToInt (value)) + " %";
}

juce::String formatMilliseconds (float value)
{
    return juce::String (value, 1) + " ms";
}

juce::String formatHertz (float value)
{
    if (value >= 1000.0f)
        return juce::String (value / 1000.0f, 1) + " kHz";

    return juce::String (juce::roundToInt (value)) + " Hz";
}
} // namespace

class SlapLayEditorContent final : public juce::Component,
                                   private juce::Timer
{
public:
    explicit SlapLayEditorContent (SlapLayAudioProcessor& p)
        : processor (p),
          apvts (p.getValueTreeState())
    {
        setOpaque (true);

        timeKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::delayTime,
                                                "DELAY TIME", formatMilliseconds);
        feedbackKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::feedback,
                                                     "FEEDBACK", formatPercent);
        tapeKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::tapeDrive,
                                                "TAPE DRIVE", formatPercent);
        lowCutKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::lowCut,
                                                  "LOW CUT", formatHertz);
        highCutKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::highCut,
                                                   "HIGH CUT", formatHertz);
        wowKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::wow,
                                               "WOW", formatPercent);
        flutterKnob = std::make_unique<ParamKnob> (apvts, slaplay::params::flutter,
                                                   "FLUTTER", formatPercent);

        for (auto* knob : { timeKnob.get(), feedbackKnob.get(), tapeKnob.get(),
                            lowCutKnob.get(), highCutKnob.get(), wowKnob.get(),
                            flutterKnob.get() })
        {
            addAndMakeVisible (*knob);
        }

        inputSlider = std::make_unique<GlassSlider> (apvts, slaplay::params::inputGain,
                                                     "INPUT", formatDb);
        mixSlider = std::make_unique<GlassSlider> (apvts, slaplay::params::mix,
                                                   "WET / DRY", formatPercent);
        outputSlider = std::make_unique<GlassSlider> (apvts, slaplay::params::outputGain,
                                                      "OUTPUT", formatDb);

        for (auto* slider : { inputSlider.get(), mixSlider.get(), outputSlider.get() })
            addAndMakeVisible (*slider);

        filterButton = std::make_unique<TactileButton> (apvts, slaplay::params::filterEnabled,
                                                        "FILTER");
        tapeButton = std::make_unique<TactileButton> (apvts, slaplay::params::tapeEnabled,
                                                      "TAPE");
        modButton = std::make_unique<TactileButton> (apvts, slaplay::params::modulationEnabled,
                                                     "MOD");
        bypassButton = std::make_unique<TactileButton> (apvts, slaplay::params::bypass,
                                                        "BYPASS");

        addAndMakeVisible (*filterButton);
        addAndMakeVisible (*tapeButton);
        addAndMakeVisible (*modButton);
        addAndMakeVisible (*bypassButton);

        waveformDisplay = std::make_unique<WaveformDisplay> (processor);
        addAndMakeVisible (*waveformDisplay);

        outputMeter = std::make_unique<OutputMeter> (processor);
        addAndMakeVisible (*outputMeter);

        pillBar = std::make_unique<HeaderPillBar> (processor);
        pillBar->onPresetChanged = [this] { repaint(); };
        addAndMakeVisible (*pillBar);

        lastPreset = processor.getCurrentPresetIndex();
        lastDirty = processor.isPresetDirty();
        startTimerHz (12);
    }

    ~SlapLayEditorContent() override
    {
        stopTimer();
    }

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        drawDesktopHardwareChassis (g, bounds);

        paintBrandHeader (g);
        paintHeaderAnnotations (g);
        paintDivider (g);
        paintSignalChips (g);
    }

    void resized() override
    {
        auto unitBounds = getLocalBounds().reduced (18, 16);
        auto metalBase = unitBounds.reduced (5);
        auto glassPanel = metalBase.reduced (6);
        auto innerGlass = glassPanel.reduced (18, 16);

        // The mixer bay is carved off first so it spans the full interior height.
        auto rightMixerBay = innerGlass.removeFromRight (204);
        innerGlass.removeFromRight (14);

        auto headerRow = innerGlass.removeFromTop (58);
        pillBar->setBounds (headerRow.getCentreX() - 190, headerRow.getY() + 11, 380, 36);

        innerGlass.removeFromTop (12);
        waveformDisplay->setBounds (innerGlass.removeFromTop (270));
        innerGlass.removeFromTop (10);
        innerGlass.removeFromTop (26); // painted feedback-loop chip strip
        innerGlass.removeFromTop (8);

        auto bottomDeck = innerGlass;

        auto buttonRow = bottomDeck.removeFromBottom (42);
        bottomDeck.removeFromBottom (10);
        auto knobRow = bottomDeck;

        auto primaryArea = knobRow.removeFromLeft (540);
        const int primaryWidth = primaryArea.getWidth() / 3;
        timeKnob->setBounds (primaryArea.removeFromLeft (primaryWidth));
        feedbackKnob->setBounds (primaryArea.removeFromLeft (primaryWidth));
        tapeKnob->setBounds (primaryArea);

        knobRow.removeFromLeft (16);
        auto secondaryArea = knobRow;
        const int secondaryWidth = secondaryArea.getWidth() / 2;

        auto secondaryTop = secondaryArea.removeFromTop (secondaryArea.getHeight() / 2);
        lowCutKnob->setBounds (secondaryTop.removeFromLeft (secondaryWidth));
        highCutKnob->setBounds (secondaryTop);
        wowKnob->setBounds (secondaryArea.removeFromLeft (secondaryWidth));
        flutterKnob->setBounds (secondaryArea);

        constexpr int buttonWidth = 140;
        constexpr int buttonGap = 16;
        constexpr int buttonCount = 4;
        const int totalButtonWidth = buttonWidth * buttonCount + buttonGap * (buttonCount - 1);
        int buttonX = buttonRow.getCentreX() - totalButtonWidth / 2;

        for (auto* button : { filterButton.get(), tapeButton.get(), modButton.get(),
                              bypassButton.get() })
        {
            button->setBounds (buttonX, buttonRow.getY() + 2, buttonWidth,
                               buttonRow.getHeight() - 4);
            buttonX += buttonWidth + buttonGap;
        }

        // Lower section of the mixer bay: horizontal output meter below the mixers.
        auto outputMeterArea = rightMixerBay.removeFromBottom (72);
        outputMeter->setBounds (outputMeterArea.reduced (6, 4));
        rightMixerBay.removeFromBottom (8);

        constexpr int mixerCount = 3;
        const int columnWidth = rightMixerBay.getWidth() / mixerCount;
        std::array<GlassSlider*, mixerCount> mixers { inputSlider.get(), mixSlider.get(),
                                                     outputSlider.get() };

        for (int i = 0; i < mixerCount; ++i)
        {
            auto column = rightMixerBay.removeFromLeft (i == mixerCount - 1
                                                            ? rightMixerBay.getWidth()
                                                            : columnWidth);
            mixers[static_cast<size_t> (i)]->setBounds (column.reduced (4, 0));
        }
    }

private:
    void timerCallback() override
    {
        const int preset = processor.getCurrentPresetIndex();
        const bool dirty = processor.isPresetDirty();

        if (preset != lastPreset || dirty != lastDirty)
        {
            lastPreset = preset;
            lastDirty = dirty;
            repaint();
        }
    }

    void paintBrandHeader (juce::Graphics& g)
    {
        g.setColour (textSecondary.withAlpha (0.85f));
        g.setFont (font (6.3f, Weight::semibold, 0.22f));
        g.drawText ("BLS AUDIO  /  TAPE SERIES", 47, 44, 260, 9,
                    juce::Justification::centredLeft, false);

        g.setColour (textPrimary);
        g.setFont (font (21.0f, Weight::semibold, 0.04f));
        g.drawText ("SLAPLAY", 47, 52, 260, 27,
                    juce::Justification::centredLeft, false);

        g.setColour (textSecondary);
        g.setFont (font (6.6f, Weight::regular, 0.14f));
        g.drawText ("TAPE SLAPBACK DELAY  /  50-200 MS",
                    47, 80, 280, 9, juce::Justification::centredLeft, false);

        g.setColour (crimsonAccent);
        g.fillRect (47.0f, 93.0f, 54.0f, 1.4f);
    }

    void paintHeaderAnnotations (juce::Graphics& g)
    {
        g.setColour (textSecondary.withAlpha (0.62f));
        g.setFont (font (6.0f, Weight::regular, 0.10f));
        g.drawText ("SHORT TAPE DELAY / CUBIC INTERPOLATION",
                    676, 62, 239, 9, juce::Justification::centredRight, false);
        g.drawText ("12 dB/OCT FEEDBACK EQ / 0 LATENCY",
                    676, 75, 239, 9, juce::Justification::centredRight, false);
    }

    void paintDivider (juce::Graphics& g)
    {
        const float dividerX = 922.0f;

        g.setColour (obsidianGrid);
        g.fillRect (dividerX, 43.0f, 1.0f, 634.0f);

        juce::ColourGradient accent (crimsonAccent.withAlpha (0.38f), dividerX, 43.0f,
                                     crimsonAccent.withAlpha (0.08f), dividerX, 677.0f, false);
        g.setGradientFill (accent);
        g.fillRect (dividerX - 0.6f, 43.0f, 0.8f, 634.0f);

        g.setColour (crimsonGlow.withAlpha (0.10f));
        g.fillRect (dividerX - 3.0f, 43.0f, 5.0f, 634.0f);
    }

    void paintSignalChips (juce::Graphics& g)
    {
        g.setColour (obsidianGrid);
        g.fillRect (47.0f, 390.0f, 868.0f, 1.0f);

        g.setColour (textSecondary.withAlpha (0.90f));
        g.setFont (font (6.3f, Weight::semibold, 0.18f));
        g.drawText ("FEEDBACK LOOP", 47, 399, 110, 9,
                    juce::Justification::centredLeft, false);

        const std::array<const char*, 4> stages { "TAPE SAT", "LOW CUT", "HIGH CUT",
                                                  "WOW / FLUTTER" };
        float chipX = 152.0f;
        constexpr float chipW = 118.0f;
        constexpr float chipGap = 14.0f;

        for (size_t i = 0; i < stages.size(); ++i)
        {
            drawSignalChip (g, { chipX, 394.0f, chipW, 18.0f }, stages[i]);

            if (i + 1 < stages.size())
            {
                g.setColour (crimsonAccent.withAlpha (0.80f));
                g.setFont (font (7.5f, Weight::semibold, 0.0f));
                g.drawText (">", juce::Rectangle<float> (chipX + chipW, 394.0f,
                                                         chipGap, 18.0f),
                            juce::Justification::centred, false);
            }

            chipX += chipW + chipGap;
        }

        const int preset = processor.getCurrentPresetIndex();
        const bool dirty = processor.isPresetDirty();

        g.setColour (dirty ? crimsonBright.withAlpha (0.90f) : textSecondary.withAlpha (0.82f));
        g.setFont (font (6.2f, Weight::semibold, 0.10f));
        g.drawFittedText (slaplay::getFactoryPresetCharacter (preset),
                          juce::Rectangle<int> (676, 395, 239, 12),
                          juce::Justification::centredRight, 1);
    }

    SlapLayAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;

    std::unique_ptr<ParamKnob> timeKnob;
    std::unique_ptr<ParamKnob> feedbackKnob;
    std::unique_ptr<ParamKnob> tapeKnob;
    std::unique_ptr<ParamKnob> lowCutKnob;
    std::unique_ptr<ParamKnob> highCutKnob;
    std::unique_ptr<ParamKnob> wowKnob;
    std::unique_ptr<ParamKnob> flutterKnob;

    std::unique_ptr<GlassSlider> inputSlider;
    std::unique_ptr<GlassSlider> mixSlider;
    std::unique_ptr<GlassSlider> outputSlider;

    std::unique_ptr<TactileButton> filterButton;
    std::unique_ptr<TactileButton> tapeButton;
    std::unique_ptr<TactileButton> modButton;
    std::unique_ptr<TactileButton> bypassButton;

    std::unique_ptr<WaveformDisplay> waveformDisplay;
    std::unique_ptr<OutputMeter> outputMeter;
    std::unique_ptr<HeaderPillBar> pillBar;

    int lastPreset = -1;
    bool lastDirty = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlapLayEditorContent)
};

//======================================================================================
SlapLayAudioProcessorEditor::SlapLayAudioProcessorEditor (SlapLayAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setOpaque (true);

    content = std::make_unique<SlapLayEditorContent> (processor);
    addAndMakeVisible (*content);

    setResizable (true, true);
    setResizeLimits (885, 540, 1770, 1080);

    if (auto* boundsConstrainer = getConstrainer())
        boundsConstrainer->setFixedAspectRatio (static_cast<double> (designWidth)
                                                / static_cast<double> (designHeight));

    setSize (designWidth, designHeight);
}

SlapLayAudioProcessorEditor::~SlapLayAudioProcessorEditor()
{
    content.reset();
    setLookAndFeel (nullptr);
}

void SlapLayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (studioSurfaceDark);
}

void SlapLayAudioProcessorEditor::resized()
{
    if (content == nullptr)
        return;

    const float scale = juce::jmin (static_cast<float> (getWidth()) / static_cast<float> (designWidth),
                                    static_cast<float> (getHeight()) / static_cast<float> (designHeight));
    const float offsetX = (static_cast<float> (getWidth()) - static_cast<float> (designWidth) * scale) * 0.5f;
    const float offsetY = (static_cast<float> (getHeight()) - static_cast<float> (designHeight) * scale) * 0.5f;

    content->setBounds (0, 0, designWidth, designHeight);
    content->setTransform (juce::AffineTransform::scale (scale).translated (offsetX, offsetY));
}
