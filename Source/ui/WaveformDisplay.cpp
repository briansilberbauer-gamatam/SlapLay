#include "WaveformDisplay.h"

#include "Parameters.h"
#include "Theme.h"

#include <cmath>

namespace studio::ui
{
WaveformDisplay::WaveformDisplay (SlapLayAudioProcessor& p)
    : processor (p),
      delayParameter (p.getValueTreeState().getRawParameterValue (slaplay::params::delayTime)),
      feedbackParameter (p.getValueTreeState().getRawParameterValue (slaplay::params::feedback)),
      modulationParameter (p.getValueTreeState().getRawParameterValue (slaplay::params::modulationEnabled)),
      tapeParameter (p.getValueTreeState().getRawParameterValue (slaplay::params::tapeEnabled))
{
    setOpaque (false);
    setInterceptsMouseClicks (false, false);
    startTimerHz (45);
}

void WaveformDisplay::timerCallback()
{
    numColumns = processor.copyWaveform (minTrace.data(), maxTrace.data(), viewColumns);

    if (delayParameter != nullptr)
        displayDelayMs = delayParameter->load();

    if (feedbackParameter != nullptr)
        displayFeedback = feedbackParameter->load();

    modulationEnabled = modulationParameter != nullptr && modulationParameter->load() > 0.5f;
    tapeEnabled = tapeParameter != nullptr && tapeParameter->load() > 0.5f;

    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    constexpr float corner = 6.0f;

    // Soft crimson bloom around the recessed screen.
    g.setColour (crimsonGlow.withAlpha (0.20f));
    g.drawRoundedRectangle (bounds.expanded (1.2f), corner + 1.0f, 2.4f);

    // Recessed obsidian screen under the glass.
    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds, corner);

    const auto inner = bounds.reduced (2.0f);

    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path screenClip;
        screenClip.addRoundedRectangle (inner, corner - 1.0f);
        g.reduceClipRegion (screenClip);

        const auto graph = inner.withTrimmedTop (16.0f)
                                .withTrimmedBottom (12.0f)
                                .withTrimmedLeft (34.0f)
                                .withTrimmedRight (8.0f);

        // Subtle dB scale hairlines with minimalist labels.
        constexpr int numHorizontalLines = 5;
        const char* dbLabels[numHorizontalLines] = { "0 dB", "-6 dB", "-12 dB", "-18 dB", "-24 dB" };
        g.setFont (font (6.2f, Weight::regular, 0.04f));

        for (int i = 0; i < numHorizontalLines; ++i)
        {
            const float prop = static_cast<float> (i + 1) / static_cast<float> (numHorizontalLines + 1);
            const float y = graph.getY() + prop * graph.getHeight();

            g.setColour (i == 2 ? obsidianGrid.brighter (0.12f) : obsidianGrid);
            g.fillRect (graph.getX(), y, graph.getWidth(), 1.0f);

            g.setColour (textSecondary.withAlpha (0.55f));
            g.drawText (dbLabels[i],
                        juce::Rectangle<float> (inner.getX() + 6.0f, y - 4.5f, 26.0f, 9.0f),
                        juce::Justification::centredLeft, false);
        }

        // Time divisions.
        constexpr int numVerticalLines = 8;
        for (int i = 1; i < numVerticalLines; ++i)
        {
            const float prop = static_cast<float> (i) / static_cast<float> (numVerticalLines);
            const float x = graph.getX() + prop * graph.getWidth();
            g.setColour (obsidianGrid);
            g.fillRect (x, graph.getY(), 1.0f, graph.getHeight());
        }

        // Zero line.
        g.setColour (obsidianGrid.brighter (0.25f));
        g.fillRect (graph.getX(), graph.getCentreY(), graph.getWidth(), 1.0f);

        // Live crimson waveform in three glow passes.
        if (numColumns > 1)
        {
            const float midY  = graph.getCentreY();
            const float halfH = graph.getHeight() * 0.46f;
            constexpr float visualGain = 2.4f;

            juce::Path fillPath;
            juce::Path topTrace;
            juce::Path bottomTrace;

            for (int i = 0; i < numColumns; ++i)
            {
                const float normX = static_cast<float> (i) / static_cast<float> (numColumns - 1);
                const float x     = graph.getX() + normX * graph.getWidth();
                const float yMax  = midY - juce::jlimit (-1.0f, 1.0f,
                                                         maxTrace[static_cast<size_t> (i)] * visualGain) * halfH;

                if (i == 0)
                {
                    fillPath.startNewSubPath (x, yMax);
                    topTrace.startNewSubPath (x, yMax);
                }
                else
                {
                    fillPath.lineTo (x, yMax);
                    topTrace.lineTo (x, yMax);
                }
            }

            for (int i = numColumns - 1; i >= 0; --i)
            {
                const float normX = static_cast<float> (i) / static_cast<float> (numColumns - 1);
                const float x     = graph.getX() + normX * graph.getWidth();
                const float yMin  = midY - juce::jlimit (-1.0f, 1.0f,
                                                         minTrace[static_cast<size_t> (i)] * visualGain) * halfH;
                fillPath.lineTo (x, yMin);

                if (i == numColumns - 1)
                    bottomTrace.startNewSubPath (x, yMin);
                else
                    bottomTrace.lineTo (x, yMin);
            }

            fillPath.closeSubPath();

            // Pass 1: soft crimson inner envelope.
            juce::ColourGradient envelopeGradient (crimsonAccent.withAlpha (0.24f),
                                                   graph.getX(), midY - halfH,
                                                   crimsonGlow.withAlpha (0.06f),
                                                   graph.getX(), midY + halfH, false);
            g.setGradientFill (envelopeGradient);
            g.fillPath (fillPath);

            // Pass 2: wide ambient crimson LED bloom.
            g.setColour (crimsonGlow.withAlpha (0.28f));
            g.strokePath (topTrace, juce::PathStrokeType (5.5f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
            g.strokePath (bottomTrace, juce::PathStrokeType (5.5f, juce::PathStrokeType::curved,
                                                             juce::PathStrokeType::rounded));

            // Pass 3: primary muted crimson body.
            g.setColour (crimsonAccent);
            g.strokePath (topTrace, juce::PathStrokeType (2.1f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
            g.strokePath (bottomTrace, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved,
                                                             juce::PathStrokeType::rounded));

            // Pass 4: crisp bright core filament.
            g.setColour (crimsonBright);
            g.strokePath (topTrace, juce::PathStrokeType (0.95f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
        }
        else
        {
            g.setColour (crimsonAccent.withAlpha (0.35f));
            g.fillRect (graph.getX(), graph.getCentreY() - 0.5f, graph.getWidth(), 1.0f);
        }

        // Glossy upper-left glass reflection over the embedded display.
        juce::Path glassSheen;
        glassSheen.startNewSubPath (inner.getX(), inner.getY());
        glassSheen.lineTo (inner.getX() + inner.getWidth() * 0.42f, inner.getY());
        glassSheen.lineTo (inner.getX() + inner.getWidth() * 0.18f, inner.getBottom());
        glassSheen.lineTo (inner.getX(), inner.getBottom());
        glassSheen.closeSubPath();
        g.setColour (juce::Colours::white.withAlpha (0.030f));
        g.fillPath (glassSheen);

        // Header label and live signal-path tag.
        g.setColour (textSecondary.withAlpha (0.85f));
        g.setFont (font (6.2f, Weight::semibold, 0.18f));
        g.drawText ("REAL-TIME OUTPUT / TAPE ECHO", inner.getX() + 7.0f, inner.getY() + 3.0f,
                    inner.getWidth() * 0.5f, 9.0f, juce::Justification::centredLeft, false);

        const auto pathTag = tapeEnabled ? juce::String ("TAPE SAT")
                                         : juce::String ("CLEAN TAP");
        g.setColour (crimsonAccent);
        g.setFont (font (6.2f, Weight::semibold, 0.16f));
        g.drawText (pathTag, inner.getRight() - 90.0f, inner.getY() + 3.0f,
                    84.0f, 9.0f, juce::Justification::centredRight, false);

        g.setColour (crimsonBright);
        g.fillEllipse (inner.getRight() - 97.0f, inner.getY() + 6.0f, 3.2f, 3.2f);
        g.setColour (crimsonGlow.withAlpha (0.30f));
        g.fillEllipse (inner.getRight() - 100.0f, inner.getY() + 3.0f, 9.2f, 9.2f);

        // Bottom readouts: time reference and live delay/feedback/modulation state.
        g.setColour (textSecondary.withAlpha (0.62f));
        g.setFont (font (5.9f, Weight::semibold, 0.12f));
        g.drawText ("dBFS / TIME", inner.getX() + 7.0f, inner.getBottom() - 10.0f,
                    80.0f, 8.0f, juce::Justification::centredLeft, false);

        const auto status = "DELAY " + juce::String (juce::roundToInt (displayDelayMs)) + " ms"
                            + "   //   FB " + juce::String (juce::roundToInt (displayFeedback)) + " %"
                            + "   //   " + (modulationEnabled ? "WOW + FLUTTER" : "MOD OFF");

        g.setColour (modulationEnabled ? crimsonAccent : textMuted);
        g.setFont (font (5.9f, Weight::semibold, 0.10f));
        g.drawText (status, inner.getRight() - 260.0f, inner.getBottom() - 10.0f,
                    254.0f, 8.0f, juce::Justification::centredRight, false);
    }

    // Recessed glass bezel and subtle crimson inner refraction.
    g.setColour (crimsonAccent.withAlpha (0.42f));
    g.drawRoundedRectangle (bounds, corner, 1.1f);
    g.setColour (glassRimHighlight.withAlpha (0.16f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), corner - 1.0f, 0.8f);
}
} // namespace studio::ui
