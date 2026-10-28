#include "Controls.h"

#include "Parameters.h"
#include "Presets.h"

#include <algorithm>
#include <cmath>

namespace studio::ui
{
namespace
{
float normaliseDb (float linear, float floorDb = -48.0f)
{
    if (! std::isfinite (linear) || linear <= 0.0f)
        return 0.0f;

    const float db = juce::Decibels::gainToDecibels (linear, floorDb);
    return juce::jlimit (0.0f, 1.0f, (db - floorDb) / -floorDb);
}

juce::String formatNormalisedLevelDb (float normalised)
{
    if (! std::isfinite (normalised) || normalised <= 0.0005f)
        return "-inf";

    return juce::String (juce::jlimit (-48.0f, 0.0f, -48.0f + 48.0f * normalised), 1);
}
} // namespace

//======================================================================================
ParamKnob::ParamKnob (juce::AudioProcessorValueTreeState& state,
                      const juce::String& id,
                      juce::String caption,
                      std::function<juce::String (float)> formatter)
    : apvts (state),
      parameterID (id),
      captionText (std::move (caption)),
      formatValue (std::move (formatter))
{
    setOpaque (false);
    setInterceptsMouseClicks (true, true);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                juce::MathConstants<float>::pi * 2.75f, true);
    slider.setMouseDragSensitivity (180);
    slider.setScrollWheelEnabled (true);
    addAndMakeVisible (slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);

    if (auto* parameter = apvts.getParameter (parameterID))
    {
        const float defaultValue = parameter->getNormalisableRange().convertFrom0to1 (
            parameter->getDefaultValue());
        slider.setDoubleClickReturnValue (true, defaultValue);
        slider.setTooltip (parameter->getName (64));
    }

    slider.onValueChange = [this] { refreshValueText(); };

    refreshValueText();
    startTimerHz (24);
}

ParamKnob::~ParamKnob()
{
    stopTimer();
}

void ParamKnob::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    const auto captionArea = bounds.removeFromTop (15.0f);
    g.setColour (hover ? textPrimary : textSecondary);
    g.setFont (font (7.9f, Weight::semibold, 0.14f));
    g.drawFittedText (captionText, captionArea.toNearestInt(),
                      juce::Justification::centred, 1);

    const auto valueArea = bounds.removeFromBottom (15.0f);
    g.setColour (textPrimary);
    g.setFont (font (8.3f, Weight::semibold, 0.05f));
    g.drawFittedText (valueText, valueArea.toNearestInt(),
                      juce::Justification::centred, 1);
}

void ParamKnob::resized()
{
    auto area = getLocalBounds().withTrimmedTop (15).withTrimmedBottom (15);
    const int diameter = juce::jmin (area.getWidth(), area.getHeight());
    slider.setBounds (area.withSizeKeepingCentre (diameter, diameter));
}

void ParamKnob::mouseEnter (const juce::MouseEvent&)
{
    hover = true;
    repaint();
}

void ParamKnob::mouseExit (const juce::MouseEvent&)
{
    hover = false;
    repaint();
}

void ParamKnob::timerCallback()
{
    refreshValueText();
}

void ParamKnob::refreshValueText()
{
    if (formatValue == nullptr)
        return;

    const auto newText = formatValue (static_cast<float> (slider.getValue()));

    if (newText != valueText)
    {
        valueText = newText;
        repaint();
    }
}

//======================================================================================
GlassSlider::GlassSlider (juce::AudioProcessorValueTreeState& state,
                          const juce::String& id,
                          juce::String caption,
                          std::function<juce::String (float)> formatter)
    : apvts (state),
      parameterID (id),
      captionText (std::move (caption)),
      formatValue (std::move (formatter))
{
    setOpaque (false);
    setInterceptsMouseClicks (true, true);

    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setMouseDragSensitivity (190);
    slider.setScrollWheelEnabled (true);
    addAndMakeVisible (slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);

    if (auto* parameter = apvts.getParameter (parameterID))
    {
        const float defaultValue = parameter->getNormalisableRange().convertFrom0to1 (
            parameter->getDefaultValue());
        slider.setDoubleClickReturnValue (true, defaultValue);
        slider.setTooltip (parameter->getName (64));
    }

    slider.onValueChange = [this] { refreshValueText(); };

    refreshValueText();
    startTimerHz (24);
}

GlassSlider::~GlassSlider()
{
    stopTimer();
}

void GlassSlider::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    const auto captionArea = bounds.removeFromTop (20.0f);
    g.setColour (hover ? textPrimary : textSecondary);
    g.setFont (font (7.1f, Weight::semibold, 0.10f));
    g.drawFittedText (captionText, captionArea.toNearestInt(),
                      juce::Justification::centred, 2);

    const auto valueArea = bounds.removeFromBottom (15.0f);
    g.setColour (textPrimary);
    g.setFont (font (7.6f, Weight::semibold, 0.02f));
    g.drawFittedText (valueText, valueArea.toNearestInt(),
                      juce::Justification::centred, 1);
}

void GlassSlider::resized()
{
    auto area = getLocalBounds().withTrimmedTop (20).withTrimmedBottom (15);
    const int trackWidth = juce::jmin (58, area.getWidth());
    slider.setBounds (area.withSizeKeepingCentre (trackWidth, area.getHeight()));
}

void GlassSlider::mouseEnter (const juce::MouseEvent&)
{
    hover = true;
    repaint();
}

void GlassSlider::mouseExit (const juce::MouseEvent&)
{
    hover = false;
    repaint();
}

void GlassSlider::timerCallback()
{
    refreshValueText();
}

void GlassSlider::refreshValueText()
{
    if (formatValue == nullptr)
        return;

    const auto newText = formatValue (static_cast<float> (slider.getValue()));

    if (newText != valueText)
    {
        valueText = newText;
        repaint();
    }
}

//======================================================================================
TactileButton::TactileButton (juce::AudioProcessorValueTreeState& state,
                              const juce::String& parameterID,
                              const juce::String& labelText)
    : juce::ToggleButton (labelText)
{
    setClickingTogglesState (true);
    setButtonText (labelText);

    if (auto* parameter = state.getParameter (parameterID))
        setTooltip (parameter->getName (64));

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        state, parameterID, *this);
}

//======================================================================================
OutputMeter::OutputMeter (SlapLayAudioProcessor& p)
    : processor (p)
{
    setOpaque (false);
    startTimerHz (30);
}

void OutputMeter::timerCallback()
{
    auto follow = [] (float& display, float target)
    {
        if (target > display)
            display = target;
        else
            display += (target - display) * 0.22f;
    };

    follow (levelLeft, normaliseDb (processor.getOutputLevelLeft()));
    follow (levelRight, normaliseDb (processor.getOutputLevelRight()));

    auto updatePeak = [] (float current, float& held, int& holdCounter)
    {
        if (current >= held)
        {
            held = current;
            holdCounter = 30;
        }
        else if (holdCounter > 0)
        {
            --holdCounter;
        }
        else
        {
            held = juce::jmax (current, held - 0.004f);
        }
    };

    updatePeak (levelLeft, peakLeft, peakHoldLeft);
    updatePeak (levelRight, peakRight, peakHoldRight);

    if (processor.getOutputLevel() >= 0.985f)
        clipHold = 55;
    else if (clipHold > 0)
        --clipHold;

    repaint();
}

void OutputMeter::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    drawGlassWell (g, bounds, 3.0f);

    auto inner = bounds.reduced (8.0f, 5.0f);
    auto header = inner.removeFromTop (14.0f);

    g.setColour (textMuted);
    g.setFont (font (5.9f, Weight::semibold, 0.18f));
    g.drawText ("OUT METER", header.withTrimmedRight (68.0f),
                juce::Justification::centredLeft, false);

    const float peakNormalised = juce::jmax (levelLeft, levelRight);
    g.setColour (peakNormalised > 0.90f ? crimsonBright : textPrimary);
    g.setFont (font (7.2f, Weight::semibold, 0.04f));
    g.drawText (formatNormalisedLevelDb (peakNormalised) + " dB", header,
                juce::Justification::centredRight, false);

    if (clipHold > 0)
    {
        g.setColour (crimsonBright);
        g.fillEllipse (bounds.getRight() - 10.0f, bounds.getY() + 4.0f, 4.0f, 4.0f);
        g.setColour (crimsonGlow.withAlpha (0.35f));
        g.fillEllipse (bounds.getRight() - 13.5f, bounds.getY() + 0.5f, 11.0f, 11.0f);
    }

    inner.removeFromTop (1.0f);
    auto scaleArea = inner.removeFromBottom (11.0f);
    inner.removeFromBottom (1.0f);

    auto leftRow = inner.removeFromTop (inner.getHeight() * 0.5f).reduced (0.0f, 1.0f);
    auto rightRow = inner.reduced (0.0f, 1.0f);

    drawChannel (g, leftRow, "L", levelLeft, peakLeft);
    drawChannel (g, rightRow, "R", levelRight, peakRight);

    // Minimalist dB scale under the twin bars, sharing the bar's left edge.
    const float barX = bounds.getX() + 18.0f;
    const float barWidth = bounds.getWidth() - 26.0f;
    constexpr int markers = 6;
    const float markerDb[markers] { -48.0f, -36.0f, -24.0f, -12.0f, -6.0f, 0.0f };

    g.setFont (font (5.3f, Weight::regular, 0.02f));

    for (int i = 0; i < markers; ++i)
    {
        const float prop = (markerDb[i] + 48.0f) / 48.0f;
        const float x = barX + prop * barWidth;

        g.setColour (textMuted.withAlpha (0.55f));
        g.fillRect (x - 0.25f, scaleArea.getY(), 0.5f, 2.5f);

        g.setColour (textMuted.withAlpha (0.72f));
        const juce::String label { juce::roundToInt (markerDb[i]) };
        g.drawText (label,
                    juce::Rectangle<float> (x - 10.0f, scaleArea.getY() + 2.0f, 20.0f,
                                            scaleArea.getHeight() - 2.0f),
                    juce::Justification::centred, false);
    }
}

void OutputMeter::drawChannel (juce::Graphics& g, juce::Rectangle<float> bounds,
                               const juce::String& label, float normalisedLevel,
                               float normalisedPeak) const
{
    g.setColour (textMuted);
    g.setFont (font (5.8f, Weight::semibold, 0.04f));
    g.drawText (label, bounds.withWidth (9.0f), juce::Justification::centredLeft, false);

    const auto bar = bounds.withTrimmedLeft (10.0f).reduced (0.0f, 0.5f);

    g.setColour (obsidianWell);
    g.fillRoundedRectangle (bar, 1.5f);
    g.setColour (glassRimHighlight.withAlpha (0.12f));
    g.drawRoundedRectangle (bar.reduced (0.4f), 1.4f, 0.6f);

    constexpr int segments = 32;
    constexpr float gap = 1.0f;
    const float cellW = (bar.getWidth() - gap * static_cast<float> (segments - 1))
                        / static_cast<float> (segments);
    const int litCount = juce::roundToInt (juce::jlimit (0.0f, 1.0f, normalisedLevel)
                                           * static_cast<float> (segments));

    for (int i = 0; i < segments; ++i)
    {
        const auto cell = juce::Rectangle<float> (bar.getX() + static_cast<float> (i) * (cellW + gap),
                                                  bar.getY() + 1.0f, cellW, bar.getHeight() - 2.0f);
        const float prop = static_cast<float> (i) / static_cast<float> (segments - 1);
        const auto lit = crimsonGlow.interpolatedWith (crimsonAccent, juce::jmin (1.0f, prop * 1.6f))
                             .interpolatedWith (crimsonBright, juce::jmax (0.0f, prop - 0.75f) * 4.0f);

        g.setColour (i < litCount ? lit : crimsonSubtle.withAlpha (0.24f));
        g.fillRoundedRectangle (cell, 0.7f);
    }

    if (litCount > 0)
    {
        const auto litBounds = bar.withWidth (bar.getWidth()
                                              * static_cast<float> (litCount)
                                              / static_cast<float> (segments));
        g.setColour (crimsonGlow.withAlpha (0.24f));
        g.drawRoundedRectangle (litBounds.expanded (0.8f, 1.2f), 2.0f, 2.2f);
    }

    if (normalisedPeak > 0.01f)
    {
        const float peakX = bar.getX() + juce::jlimit (0.0f, 1.0f, normalisedPeak) * bar.getWidth();
        g.setColour (textPrimary.withAlpha (0.58f));
        g.fillRect (peakX - 0.5f, bar.getY(), 1.0f, bar.getHeight());
    }
}

//======================================================================================
HeaderPillBar::HeaderPillBar (SlapLayAudioProcessor& p)
    : processor (p)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    lastPreset = processor.getCurrentPresetIndex();
    lastDirty = processor.isPresetDirty();
    startTimerHz (12);
}

void HeaderPillBar::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float radius = bounds.getHeight() * 0.5f;

    // Subtle crimson refraction around the pill cutout.
    g.setColour (crimsonGlow.withAlpha (0.28f));
    g.drawRoundedRectangle (bounds.expanded (1.2f), radius + 1.2f, 2.0f);

    // Recessed obsidian digital screen.
    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds, radius);

    // Thin crimson + glass rim.
    g.setColour (crimsonAccent.withAlpha (0.55f));
    g.drawRoundedRectangle (bounds, radius, 1.0f);
    g.setColour (glassRimHighlight.withAlpha (0.18f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), radius - 1.0f, 0.75f);

    const float navW = bounds.getHeight() * 1.15f;
    const auto previousArea = bounds.withWidth (navW);
    const auto nextArea = bounds.withLeft (bounds.getRight() - navW);

    auto drawChevron = [&] (juce::Rectangle<float> area, bool pointLeft, bool hot)
    {
        if (hot)
        {
            g.setColour (crimsonSubtle.withAlpha (0.38f));
            g.fillRoundedRectangle (area.reduced (4.0f, 3.0f), radius);
        }

        const auto centre = area.getCentre();
        constexpr float size = 4.2f;
        const float dir = pointLeft ? -1.0f : 1.0f;

        juce::Path chevron;
        chevron.startNewSubPath (centre.x - dir * size * 0.5f, centre.y - size);
        chevron.lineTo (centre.x + dir * size * 0.6f, centre.y);
        chevron.lineTo (centre.x - dir * size * 0.5f, centre.y + size);

        g.setColour (hot ? crimsonBright : textSecondary);
        g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    };

    drawChevron (previousArea, true, hoverZone == Zone::previous);
    drawChevron (nextArea, false, hoverZone == Zone::next);

    // Divider hairlines.
    g.setColour (obsidianGrid);
    g.fillRect (previousArea.getRight() + 1.0f, bounds.getY() + 7.0f,
                1.0f, bounds.getHeight() - 14.0f);
    g.fillRect (nextArea.getX() - 1.0f, bounds.getY() + 7.0f,
                1.0f, bounds.getHeight() - 14.0f);

    // Preset readout.
    auto textArea = bounds.withTrimmedLeft (navW + 10.0f).withTrimmedRight (navW + 10.0f);
    const auto statusArea = textArea.removeFromRight (54.0f);

    drawCrimsonDot (g, { textArea.getX() + 2.0f, textArea.getCentreY() }, 2.0f);

    const auto nameArea = textArea.withTrimmedLeft (12.0f);
    const auto presetName = lastPreset >= 0 ? processor.getProgramName (lastPreset)
                                            : juce::String ("INIT");
    const auto readout = juce::String (lastPreset + 1).paddedLeft ('0', 2)
                         + "  //  " + presetName.toUpperCase();

    g.setColour (textPrimary);
    g.setFont (font (10.8f, Weight::semibold, 0.16f));
    g.drawFittedText (readout, nameArea.toNearestInt(), juce::Justification::centred, 1);

    g.setColour (lastDirty ? crimsonBright : textSecondary.withAlpha (0.78f));
    g.setFont (font (6.2f, Weight::semibold, 0.14f));
    g.drawText (lastDirty ? "EDITED" : "FACTORY", statusArea,
                juce::Justification::centredRight, false);
}

void HeaderPillBar::mouseMove (const juce::MouseEvent& event)
{
    const auto newZone = zoneAt (static_cast<float> (event.x));

    if (newZone != hoverZone)
    {
        hoverZone = newZone;
        repaint();
    }
}

void HeaderPillBar::mouseExit (const juce::MouseEvent&)
{
    if (hoverZone != Zone::none)
    {
        hoverZone = Zone::none;
        repaint();
    }
}

void HeaderPillBar::mouseUp (const juce::MouseEvent& event)
{
    switch (zoneAt (static_cast<float> (event.x)))
    {
        case Zone::previous: step (-1); break;
        case Zone::next:     step (1);  break;
        case Zone::readout:  showPresetMenu(); break;
        case Zone::none:     break;
    }
}

HeaderPillBar::Zone HeaderPillBar::zoneAt (float x) const
{
    const float navW = static_cast<float> (getHeight()) * 1.15f;

    if (x < navW)
        return Zone::previous;

    if (x > static_cast<float> (getWidth()) - navW)
        return Zone::next;

    return Zone::readout;
}

void HeaderPillBar::timerCallback()
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

void HeaderPillBar::step (int delta)
{
    const int count = processor.getNumPrograms();

    if (count <= 0)
        return;

    int next = (processor.getCurrentPresetIndex() + delta) % count;

    if (next < 0)
        next += count;

    processor.loadFactoryPreset (next);
    lastPreset = next;
    lastDirty = processor.isPresetDirty();
    repaint();

    if (onPresetChanged)
        onPresetChanged();
}

void HeaderPillBar::showPresetMenu()
{
    juce::PopupMenu menu;

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        menu.addItem (i + 1,
                      juce::String (i + 1).paddedLeft ('0', 2) + "   " + processor.getProgramName (i),
                      true, i == lastPreset);

    juce::Component::SafePointer<HeaderPillBar> safeThis (this);

    menu.showMenuAsync (juce::PopupMenu::Options()
                            .withTargetComponent (this)
                            .withMinimumWidth (getWidth()),
                        [safeThis] (int result) mutable
                        {
                            if (safeThis == nullptr || result <= 0)
                                return;

                            safeThis->processor.loadFactoryPreset (result - 1);
                            safeThis->lastPreset = result - 1;
                            safeThis->lastDirty = safeThis->processor.isPresetDirty();
                            safeThis->repaint();

                            if (safeThis->onPresetChanged)
                                safeThis->onPresetChanged();
                        });
}
} // namespace studio::ui
