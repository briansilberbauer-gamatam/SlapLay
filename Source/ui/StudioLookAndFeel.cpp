#include "StudioLookAndFeel.h"

#include <cmath>

namespace studio::ui
{
StudioLookAndFeel::StudioLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, studioSurfaceDark);
    setColour (juce::Label::textColourId, textPrimary);
    setColour (juce::TextButton::buttonColourId, obsidianWell);
    setColour (juce::TextButton::buttonOnColourId, crimsonAccent);
    setColour (juce::TextButton::textColourOffId, textSecondary);
    setColour (juce::TextButton::textColourOnId, textPrimary);
    setColour (juce::ComboBox::backgroundColourId, obsidianScreen);
    setColour (juce::ComboBox::textColourId, textPrimary);
    setColour (juce::ComboBox::outlineColourId, gunmetalLight.withAlpha (0.65f));
    setColour (juce::ComboBox::arrowColourId, crimsonAccent);
    setColour (juce::ComboBox::focusedOutlineColourId, crimsonAccent);
    setColour (juce::PopupMenu::backgroundColourId, obsidianScreen);
    setColour (juce::PopupMenu::textColourId, textSecondary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, crimsonSubtle);
    setColour (juce::PopupMenu::highlightedTextColourId, textPrimary);
    setColour (juce::TooltipWindow::backgroundColourId, obsidianScreen);
    setColour (juce::TooltipWindow::textColourId, textPrimary);
    setColour (juce::TooltipWindow::outlineColourId, crimsonAccent);
    setColour (juce::Slider::rotarySliderFillColourId, crimsonAccent);
    setColour (juce::Slider::rotarySliderOutlineColourId, obsidianWell);
    setColour (juce::Slider::thumbColourId, crimsonAccent);
    setColour (juce::ToggleButton::textColourId, textSecondary);
    setColour (juce::ToggleButton::tickColourId, crimsonAccent);
    setColour (juce::ToggleButton::tickDisabledColourId, obsidianWell);
}

void StudioLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                static_cast<float> (width), static_cast<float> (height))
                            .reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    if (radius < 10.0f)
        return;

    const auto centre = bounds.getCentre();
    const float prop  = juce::jlimit (0.0f, 1.0f, sliderPosProportional);
    const float angle = rotaryStartAngle + prop * (rotaryEndAngle - rotaryStartAngle);

    // 1. Minimalist perimeter tick marks etched into the glass.
    const float scaleR = radius - 1.5f;
    for (int i = 0; i <= 24; ++i)
    {
        const float tickProp = static_cast<float> (i) / 24.0f;
        const float tickAngle = rotaryStartAngle + tickProp * (rotaryEndAngle - rotaryStartAngle);
        const bool isMajor = (i % 4 == 0);
        const auto p1 = centre.getPointOnCircumference (scaleR - (isMajor ? 4.0f : 2.2f), tickAngle);
        const auto p2 = centre.getPointOnCircumference (scaleR, tickAngle);

        const bool lit = (tickProp <= prop);
        g.setColour (lit ? crimsonAccent.withAlpha (isMajor ? 0.85f : 0.55f)
                         : textMuted.withAlpha (isMajor ? 0.55f : 0.28f));
        g.drawLine ({ p1, p2 }, isMajor ? 1.3f : 0.8f);
    }

    // 2. Recessed glass/metal well and subtle crimson LED under-glow.
    const float wellR = radius * 0.86f;
    const juce::Rectangle<float> wellBounds (centre.x - wellR, centre.y - wellR,
                                             wellR * 2.0f, wellR * 2.0f);
    g.setColour (obsidianWell);
    g.fillEllipse (wellBounds);

    {
        juce::ColourGradient underGlow (crimsonAccent.withAlpha (0.18f + 0.28f * prop), centre.x, centre.y,
                                        juce::Colours::transparentBlack, centre.x + wellR, centre.y, true);
        g.setGradientFill (underGlow);
        g.fillEllipse (wellBounds.expanded (2.0f));

        juce::Path activeUnderArc;
        activeUnderArc.addCentredArc (centre.x, centre.y, wellR - 1.2f, wellR - 1.2f,
                                      0.0f, rotaryStartAngle, angle, true);
        g.setColour (crimsonGlow.withAlpha (0.45f));
        g.strokePath (activeUnderArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                             juce::PathStrokeType::rounded));
        g.setColour (crimsonAccent.withAlpha (0.85f));
        g.strokePath (activeUnderArc, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                                             juce::PathStrokeType::rounded));
    }

    // 3. Soft upper-left lit / lower-right shadow for a low recessed profile.
    const float rimR = radius * 0.80f;
    const juce::Rectangle<float> rimBounds (centre.x - rimR, centre.y - rimR,
                                            rimR * 2.0f, rimR * 2.0f);
    g.setColour (dropShadowDeep.withAlpha (0.65f));
    g.fillEllipse (rimBounds.translated (1.5f, 2.2f));

    // 4. Heavy-machined outer rim with fine vertical knurled ridges.
    juce::ColourGradient rimGradient (gunmetalLight.brighter (0.32f), rimBounds.getTopLeft(),
                                      obsidianScreen, rimBounds.getBottomRight(), false);
    g.setGradientFill (rimGradient);
    g.fillEllipse (rimBounds);

    constexpr int numKnurls = 64;
    for (int i = 0; i < numKnurls; ++i)
    {
        const float knurlAngle = angle + static_cast<float> (i)
                                             * (juce::MathConstants<float>::twoPi
                                                / static_cast<float> (numKnurls));
        const auto k1 = centre.getPointOnCircumference (rimR * 0.86f, knurlAngle);
        const auto k2 = centre.getPointOnCircumference (rimR * 0.99f, knurlAngle);

        // Upper-left soft-box lighting modulates the knurl specular intensity.
        const float lightBias = 0.5f - 0.5f * std::sin (knurlAngle
                                                        + juce::MathConstants<float>::pi * 0.25f);
        g.setColour (i % 2 == 0
                         ? juce::Colours::white.withAlpha (0.06f + 0.16f * lightBias)
                         : juce::Colours::black.withAlpha (0.65f));
        g.drawLine ({ k1, k2 }, 1.0f);
    }

    juce::Path rimHighlight;
    rimHighlight.addCentredArc (centre.x, centre.y, rimR * 0.995f, rimR * 0.995f, 0.0f,
                                juce::MathConstants<float>::pi * 1.55f,
                                juce::MathConstants<float>::pi * 1.95f, true);
    g.setColour (glassRimHighlight.withAlpha (0.30f));
    g.strokePath (rimHighlight, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

    // 5. Flat, low-profile machined metal top cap.
    const float capR = rimR * 0.84f;
    const juce::Rectangle<float> capBounds (centre.x - capR, centre.y - capR,
                                            capR * 2.0f, capR * 2.0f);
    juce::ColourGradient capGradient (chamferLight.brighter (0.20f), capBounds.getTopLeft(),
                                      gunmetalDeep, capBounds.getBottomRight(), false);
    capGradient.addColour (0.46, gunmetalBase.brighter (0.16f));
    g.setGradientFill (capGradient);
    g.fillEllipse (capBounds);

    // Upper-left soft-box sheen across the flat machined cap.
    {
        juce::ColourGradient capSheen (juce::Colours::white.withAlpha (0.10f),
                                       capBounds.getTopLeft().translated (capR * 0.30f, capR * 0.30f),
                                       juce::Colours::transparentWhite, capBounds.getCentre(), true);
        g.setGradientFill (capSheen);
        g.fillEllipse (capBounds);
    }

    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path capClip;
        capClip.addEllipse (capBounds);
        g.reduceClipRegion (capClip);

        for (int gy = static_cast<int> (capBounds.getY()); gy < static_cast<int> (capBounds.getBottom()); gy += 2)
        {
            g.setColour (juce::Colours::white.withAlpha (0.035f));
            g.fillRect (capBounds.getX(), static_cast<float> (gy), capBounds.getWidth(), 1.0f);
        }
    }

    juce::Path capSpecular;
    capSpecular.addCentredArc (centre.x, centre.y, capR * 0.92f, capR * 0.92f, 0.0f,
                               juce::MathConstants<float>::pi * 1.50f,
                               juce::MathConstants<float>::pi * 1.96f, true);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.strokePath (capSpecular, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

    g.setColour (chamferLight.withAlpha (0.75f));
    g.drawEllipse (capBounds, 1.1f);
    g.setColour (obsidianScreen.withAlpha (0.80f));
    g.drawEllipse (capBounds.expanded (0.7f), 0.9f);

    // 6. Tiny minimalist tick mark on the cap for position reference.
    const auto tickInner = centre.getPointOnCircumference (capR * 0.64f, angle);
    const auto tickOuter = centre.getPointOnCircumference (capR * 0.88f, angle);
    g.setColour (crimsonGlow.withAlpha (0.45f));
    g.drawLine ({ tickInner, tickOuter }, 3.0f);
    g.setColour (textPrimary);
    g.drawLine ({ tickInner, tickOuter }, 1.5f);
}

void StudioLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearBarVertical)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                          minSliderPos, maxSliderPos, style, slider);
        return;
    }

    const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                static_cast<float> (width), static_cast<float> (height));
    const float centreX = bounds.getCentreX();
    const float trackTop = bounds.getY() + 8.0f;
    const float trackBot = bounds.getBottom() - 8.0f;
    const float trackH   = juce::jmax (10.0f, trackBot - trackTop);
    const float clampedY = juce::jlimit (trackTop, trackBot, sliderPos);

    // 1. Minimalist horizontal tick marks on both sides of the recessed track.
    constexpr int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        const float tickProp = static_cast<float> (i) / static_cast<float> (numTicks - 1);
        const float tickY = trackTop + tickProp * trackH;
        const bool isMajor = (i % 5 == 0);
        const float tickLen = isMajor ? 7.0f : 4.5f;
        constexpr float gap = 6.5f;

        g.setColour (isMajor ? textSecondary.withAlpha (0.70f) : textMuted.withAlpha (0.48f));
        g.fillRect (centreX - gap - tickLen, tickY - 0.5f, tickLen, 1.0f);
        g.fillRect (centreX + gap, tickY - 0.5f, tickLen, 1.0f);
    }

    // 2. Subtly recessed ultra-thin linear track in the glass surface.
    const juce::Rectangle<float> trackRect (centreX - 2.0f, trackTop, 4.0f, trackH);
    g.setColour (obsidianWell);
    g.fillRoundedRectangle (trackRect, 2.0f);
    g.setColour (glassRimHighlight.withAlpha (0.14f));
    g.drawRoundedRectangle (trackRect.expanded (0.5f), 2.2f, 0.7f);

    // 3. Muted crimson active level fill inside the recessed track. Bipolar
    //    trims fill outward from the centre detent; unipolar levels fill upward.
    const bool bipolar = slider.getMinimum() < -0.001 && slider.getMaximum() > 0.001;
    float fillTop = clampedY;
    float fillBottom = trackBot;

    if (bipolar)
    {
        const double range = slider.getMaximum() - slider.getMinimum();
        const double zeroProp = range > 0.0 ? (slider.getMaximum() - 0.0) / range : 0.5;
        const float zeroY = trackTop + static_cast<float> (zeroProp) * trackH;

        fillTop = juce::jmin (clampedY, zeroY);
        fillBottom = juce::jmax (clampedY, zeroY);

        g.setColour (textMuted.withAlpha (0.75f));
        g.fillRect (centreX - 5.0f, zeroY - 0.5f, 10.0f, 1.0f);
    }

    if (fillBottom - fillTop > 0.5f)
    {
        const juce::Rectangle<float> activeRect (centreX - 1.5f, fillTop, 3.0f, fillBottom - fillTop);
        g.setColour (crimsonGlow.withAlpha (0.38f));
        g.fillRoundedRectangle (activeRect.expanded (1.5f, 0.5f), 2.0f);
        g.setColour (crimsonAccent);
        g.fillRoundedRectangle (activeRect, 1.5f);
    }

    // 4. Low-profile flush touch / mechanical slider cap.
    const juce::Rectangle<float> thumb (centreX - 11.0f, clampedY - 4.5f, 22.0f, 9.0f);
    g.setColour (dropShadowDeep.withAlpha (0.55f));
    g.fillRoundedRectangle (thumb.translated (0.8f, 1.4f), 2.0f);

    juce::ColourGradient thumbGradient (gunmetalLight.brighter (0.18f), thumb.getTopLeft(),
                                        gunmetalDeep, thumb.getBottomRight(), false);
    g.setGradientFill (thumbGradient);
    g.fillRoundedRectangle (thumb, 2.0f);

    g.setColour (glassRimHighlight.withAlpha (0.28f));
    g.drawRoundedRectangle (thumb, 2.0f, 0.8f);

    // Crisp minimalist crimson reference line across the thumb centre.
    g.setColour (crimsonBright);
    g.fillRect (thumb.getX() + 3.0f, clampedY - 0.5f, thumb.getWidth() - 6.0f, 1.2f);
}

void StudioLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused (shouldDrawButtonAsDown);

    const auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    constexpr float corner = 3.5f;
    const bool active = button.getToggleState();

    // 1. Subtle recessed seat in the glass surface.
    g.setColour (obsidianWell);
    g.fillRoundedRectangle (bounds.expanded (1.0f), corner + 0.8f);

    // 2. Low-profile flush button surface.
    juce::ColourGradient capGradient (active ? gunmetalLight.interpolatedWith (crimsonGlow, 0.14f)
                                             : gunmetalBase,
                                      bounds.getTopLeft(),
                                      gunmetalDeep,
                                      bounds.getBottomRight(),
                                      false);
    g.setGradientFill (capGradient);
    g.fillRoundedRectangle (bounds, corner);

    // 3. Thin understated red glowing border when active; matte dark rim when inactive.
    if (active)
    {
        g.setColour (crimsonGlow.withAlpha (0.36f));
        g.drawRoundedRectangle (bounds.expanded (1.0f), corner + 0.5f, 2.8f);

        g.setColour (crimsonAccent);
        g.drawRoundedRectangle (bounds, corner, 1.2f);

        g.setColour (crimsonBright.withAlpha (0.75f));
        g.drawRoundedRectangle (bounds.reduced (0.4f), corner, 0.6f);
    }
    else
    {
        g.setColour (shouldDrawButtonAsHighlighted ? chamferLight
                                                   : gunmetalLight.withAlpha (0.65f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }

    // 4. Crisp clean sans-serif typography.
    g.setColour (active ? textPrimary : textSecondary);
    g.setFont (font (10.5f, active ? Weight::semibold : Weight::regular, 0.10f));
    g.drawText (button.getButtonText().toUpperCase(), bounds, juce::Justification::centred, false);
}

void StudioLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour&, bool isHighlighted,
                                              bool isDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    constexpr float corner = 3.5f;
    const bool active = button.getToggleState();

    g.setColour (obsidianWell);
    g.fillRoundedRectangle (bounds.expanded (1.0f), corner + 0.8f);

    juce::ColourGradient capGradient (active ? gunmetalLight.interpolatedWith (crimsonGlow, 0.14f)
                                             : gunmetalBase,
                                      bounds.getTopLeft(),
                                      gunmetalDeep,
                                      bounds.getBottomRight(),
                                      false);
    g.setGradientFill (capGradient);
    g.fillRoundedRectangle (bounds, corner);

    if (active)
    {
        g.setColour (crimsonGlow.withAlpha (0.36f));
        g.drawRoundedRectangle (bounds.expanded (1.0f), corner + 0.5f, 2.8f);
        g.setColour (crimsonAccent);
        g.drawRoundedRectangle (bounds, corner, 1.2f);
    }
    else
    {
        g.setColour (isHighlighted || isDown ? chamferLight : gunmetalLight.withAlpha (0.65f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
}

void StudioLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                        bool, bool)
{
    g.setColour (button.getToggleState() ? textPrimary : textSecondary);
    g.setFont (font (10.5f, Weight::semibold, 0.10f));
    g.drawText (button.getButtonText().toUpperCase(), button.getLocalBounds(),
                juce::Justification::centred, false);
}

juce::Font StudioLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return font (10.5f, Weight::semibold, 0.10f);
}

void StudioLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f,
                                                 static_cast<float> (width),
                                                 static_cast<float> (height));

    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds.reduced (0.5f), 3.0f);

    g.setColour (gunmetalLight.withAlpha (0.60f));
    g.drawRoundedRectangle (bounds.reduced (0.8f), 2.6f, 0.8f);
    g.setColour (crimsonAccent.withAlpha (0.45f));
    g.drawRoundedRectangle (bounds.reduced (2.0f), 2.0f, 0.7f);

    const juce::Rectangle<float> arrowZone (static_cast<float> (buttonX),
                                            static_cast<float> (buttonY),
                                            static_cast<float> (buttonW),
                                            static_cast<float> (buttonH));
    const auto arrowCentre = arrowZone.getCentre();

    juce::Path arrow;
    arrow.startNewSubPath (arrowCentre.x - 4.5f, arrowCentre.y - 2.0f);
    arrow.lineTo (arrowCentre.x, arrowCentre.y + 2.8f);
    arrow.lineTo (arrowCentre.x + 4.5f, arrowCentre.y - 2.0f);
    g.setColour (box.isMouseOver() ? crimsonBright : crimsonAccent);
    g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

juce::Font StudioLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return font (10.0f, Weight::semibold, 0.055f);
}

void StudioLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (10, 1, box.getWidth() - 32, box.getHeight() - 2);
    label.setFont (font (10.0f, Weight::semibold, 0.055f));
    label.setColour (juce::Label::textColourId, textPrimary);
    label.setJustificationType (juce::Justification::centredLeft);
}

void StudioLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f,
                                                 static_cast<float> (width),
                                                 static_cast<float> (height));

    g.setColour (dropShadowDeep.withAlpha (0.65f));
    g.fillRoundedRectangle (bounds.translated (1.5f, 2.0f), 4.0f);

    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds.reduced (0.5f), 4.0f);
    g.setColour (crimsonAccent.withAlpha (0.40f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (glassRimHighlight.withAlpha (0.22f));
    g.drawHorizontalLine (1, 2.0f, bounds.getWidth() - 2.0f);
}

void StudioLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                           bool isSeparator, bool isActive, bool isHighlighted,
                                           bool isTicked, bool hasSubMenu, const juce::String& text,
                                           const juce::String&, const juce::Drawable*,
                                           const juce::Colour*)
{
    auto bounds = area.toFloat().reduced (1.0f);

    if (isSeparator)
    {
        g.setColour (obsidianGrid);
        g.fillRect (bounds.getX() + 8.0f, bounds.getCentreY(),
                    bounds.getWidth() - 16.0f, 1.0f);
        return;
    }

    if (isHighlighted)
    {
        g.setColour (crimsonSubtle.withAlpha (isActive ? 0.95f : 0.35f));
        g.fillRoundedRectangle (bounds, 2.0f);
        g.setColour (crimsonAccent.withAlpha (0.65f));
        g.drawRoundedRectangle (bounds.reduced (0.3f), 2.0f, 0.7f);
    }

    if (isTicked)
    {
        g.setColour (crimsonAccent);
        g.fillEllipse (bounds.getX() + 9.0f, bounds.getCentreY() - 2.0f, 4.0f, 4.0f);
        g.setColour (crimsonGlow.withAlpha (0.35f));
        g.fillEllipse (bounds.getX() + 6.5f, bounds.getCentreY() - 4.5f, 9.0f, 9.0f);
    }

    g.setColour (isActive ? (isHighlighted ? textPrimary : textSecondary) : textMuted);
    g.setFont (font (10.0f, Weight::semibold, 0.06f));
    g.drawText (text, bounds.reduced (22.0f, 0.0f), juce::Justification::centredLeft, false);

    if (hasSubMenu)
    {
        const auto centre = bounds.getCentre();
        juce::Path arrow;
        arrow.startNewSubPath (centre.x + bounds.getWidth() * 0.36f, centre.y - 4.0f);
        arrow.lineTo (centre.x + bounds.getWidth() * 0.43f, centre.y);
        arrow.lineTo (centre.x + bounds.getWidth() * 0.36f, centre.y + 4.0f);
        g.setColour (textSecondary);
        g.strokePath (arrow, juce::PathStrokeType (1.2f));
    }
}

juce::Font StudioLookAndFeel::getPopupMenuFont()
{
    return font (10.0f, Weight::semibold, 0.06f);
}

void StudioLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                                   int standardMenuItemHeight, int& idealWidth,
                                                   int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 140;
        idealHeight = 9;
        return;
    }

    idealHeight = juce::jmax (24, standardMenuItemHeight);
    idealWidth = juce::jlimit (200, 560, text.length() * 7 + 70);
}

void StudioLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f,
                                                 static_cast<float> (width),
                                                 static_cast<float> (height)).reduced (1.0f);
    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (crimsonAccent.withAlpha (0.65f));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
    g.setColour (textPrimary);
    g.setFont (font (10.0f, Weight::regular, 0.02f));
    g.drawFittedText (text, bounds.toNearestInt().reduced (8, 3),
                      juce::Justification::centredLeft, 2);
}
} // namespace studio::ui
