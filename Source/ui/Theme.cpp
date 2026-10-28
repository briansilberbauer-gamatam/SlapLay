#include "Theme.h"

#include <BinaryData.h>

#include <cmath>

namespace studio::ui
{
namespace
{
juce::Typeface::Ptr makeTypeface (const void* data, int size)
{
    return juce::Typeface::createSystemTypefaceFor (data, static_cast<size_t> (size));
}

const juce::Typeface::Ptr& regularTypeface()
{
    static const auto typeface = makeTypeface (BinaryData::LatoRegular_ttf, BinaryData::LatoRegular_ttfSize);
    return typeface;
}

const juce::Typeface::Ptr& semiboldTypeface()
{
    static const auto typeface = makeTypeface (BinaryData::LatoSemibold_ttf, BinaryData::LatoSemibold_ttfSize);
    return typeface;
}

const juce::Typeface::Ptr& blackTypeface()
{
    static const auto typeface = makeTypeface (BinaryData::LatoBlack_ttf, BinaryData::LatoBlack_ttfSize);
    return typeface;
}
} // namespace

juce::Font font (float height, Weight weight, float tracking)
{
    const auto& typeface = weight == Weight::black ? blackTypeface()
                         : weight == Weight::semibold ? semiboldTypeface()
                                                      : regularTypeface();

    juce::Font result { juce::FontOptions (typeface).withHeight (height) };

    if (! juce::approximatelyEqual (tracking, 0.0f))
        result = result.withExtraKerningFactor (tracking);

    return result;
}

void drawDesktopHardwareChassis (juce::Graphics& g, juce::Rectangle<float> fullBounds)
{
    // 1. Dark studio surface with an upper-left soft-box falloff.
    juce::ColourGradient studioGradient (studioSurfaceLight, fullBounds.getTopLeft(),
                                         studioSurfaceDark, fullBounds.getBottomRight(), true);
    g.setGradientFill (studioGradient);
    g.fillRect (fullBounds);

    // Inset desktop unit so it rests visibly on the studio surface.
    const auto unitBounds = fullBounds.reduced (18.0f, 16.0f);
    constexpr float outerCorner = 14.0f;

    // 2. Soft-box cast shadow toward the bottom right.
    for (int i = 5; i >= 1; --i)
    {
        const float spread = static_cast<float> (i) * 3.2f;
        const float offset = static_cast<float> (i) * 2.4f;
        g.setColour (dropShadowDeep.withAlpha (0.16f));
        g.fillRoundedRectangle (unitBounds.expanded (spread * 0.4f).translated (offset, offset * 1.15f),
                                outerCorner + spread * 0.25f);
    }

    // 3. Dimensional lower-right matte chamfered base extrusion.
    const auto extrudedBase = unitBounds.translated (3.5f, 5.0f);
    juce::ColourGradient skirtGradient (chamferLight, extrudedBase.getTopLeft(),
                                        chamferDark, extrudedBase.getBottomRight(), false);
    g.setGradientFill (skirtGradient);
    g.fillRoundedRectangle (extrudedBase, outerCorner);

    // 4. Smoothly chamfered matte outer metal rim.
    juce::ColourGradient chamferGradient (chamferLight, unitBounds.getTopLeft(),
                                          chamferDark, unitBounds.getBottomRight(), false);
    g.setGradientFill (chamferGradient);
    g.fillRoundedRectangle (unitBounds, outerCorner);

    // 5. Dark gunmetal brushed metal base with fine horizontal grain.
    const auto metalBase = unitBounds.reduced (4.5f);
    constexpr float metalCorner = 10.5f;
    juce::ColourGradient metalGradient (gunmetalLight, metalBase.getTopLeft(),
                                        gunmetalDeep, metalBase.getBottomRight(), false);
    metalGradient.addColour (0.45, gunmetalBase);
    g.setGradientFill (metalGradient);
    g.fillRoundedRectangle (metalBase, metalCorner);

    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path clip;
        clip.addRoundedRectangle (metalBase, metalCorner);
        g.reduceClipRegion (clip);

        const int yStart = static_cast<int> (metalBase.getY());
        const int yEnd   = static_cast<int> (metalBase.getBottom());
        const float x1   = metalBase.getX();
        const float w    = metalBase.getWidth();

        for (int y = yStart; y < yEnd; y += 2)
        {
            const float fy = static_cast<float> (y);
            g.setColour (juce::Colours::white.withAlpha ((y % 6 == 0) ? 0.032f : 0.016f));
            g.fillRect (x1, fy, w, 1.0f);
            g.setColour (juce::Colours::black.withAlpha (0.075f));
            g.fillRect (x1, fy + 1.0f, w, 1.0f);
        }
    }

    // 6. Thick, seamless transparent glass overlay. No screws or fasteners.
    const auto glassPanel = metalBase.reduced (6.0f);
    constexpr float glassCorner = 8.0f;

    juce::ColourGradient glassBodyGradient (glassTintTop, glassPanel.getTopLeft(),
                                            glassTintBottom, glassPanel.getBottomRight(), false);
    g.setGradientFill (glassBodyGradient);
    g.fillRoundedRectangle (glassPanel, glassCorner);

    // 7. Crimson LED edge-glow and inward refraction through the clear glass perimeter.
    for (int ring = 4; ring >= 1; --ring)
    {
        const float inset = static_cast<float> (ring) * 2.2f;
        const float alpha = (5 - ring) * 0.038f;
        g.setColour (crimsonGlow.withAlpha (alpha));
        g.drawRoundedRectangle (glassPanel.reduced (inset),
                                juce::jmax (2.0f, glassCorner - inset * 0.5f), 2.2f);
    }

    // Crisp low-profile crimson LED light-pipe, strongest along the vertical edges.
    juce::ColourGradient edgeGlowGradient (crimsonBright.withAlpha (0.78f), glassPanel.getTopLeft(),
                                           crimsonAccent.withAlpha (0.52f), glassPanel.getBottomRight(), false);
    edgeGlowGradient.addColour (0.5, crimsonAccent.withAlpha (0.85f));
    g.setGradientFill (edgeGlowGradient);
    g.drawRoundedRectangle (glassPanel, glassCorner, 1.35f);

    // Polished glass top-left specular bevel.
    g.setColour (glassRimHighlight);
    g.drawRoundedRectangle (glassPanel.reduced (1.2f), glassCorner - 1.0f, 0.8f);

    // 8. Upper-left studio soft-box glossy reflection sweeping across the glass.
    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path glassClip;
        glassClip.addRoundedRectangle (glassPanel, glassCorner);
        g.reduceClipRegion (glassClip);

        juce::Path sheen;
        sheen.startNewSubPath (glassPanel.getX(), glassPanel.getY());
        sheen.lineTo (glassPanel.getX() + glassPanel.getWidth() * 0.58f, glassPanel.getY());
        sheen.lineTo (glassPanel.getX() + glassPanel.getWidth() * 0.24f, glassPanel.getBottom());
        sheen.lineTo (glassPanel.getX(), glassPanel.getBottom());
        sheen.closeSubPath();

        juce::ColourGradient sheenGradient (juce::Colours::white.withAlpha (0.065f), glassPanel.getTopLeft(),
                                            juce::Colours::transparentWhite,
                                            glassPanel.getTopLeft().getPointOnCircumference (glassPanel.getWidth() * 0.45f, 2.2f),
                                            false);
        g.setGradientFill (sheenGradient);
        g.fillPath (sheen);
    }
}

void drawEngravedLabel (juce::Graphics& g, juce::Rectangle<float> bounds,
                        const juce::String& title, const juce::String& subtitle)
{
    const auto titleArea = bounds.withHeight (12.0f);
    const auto subtitleArea = bounds.withTrimmedTop (13.0f).withHeight (10.0f);

    g.setColour (crimsonAccent.withAlpha (0.85f));
    g.fillRect (bounds.getX(), bounds.getY() + 2.0f, 2.0f, 8.0f);

    g.setColour (textSecondary);
    g.setFont (font (8.2f, Weight::semibold, 0.17f));
    g.drawText (title, titleArea.withTrimmedLeft (7.0f), juce::Justification::centredLeft, false);

    g.setColour (textSecondary.withAlpha (0.68f));
    g.setFont (font (6.3f, Weight::regular, 0.13f));
    g.drawText (subtitle, subtitleArea.withTrimmedLeft (7.0f), juce::Justification::centredLeft, false);
}

void drawGlassWell (juce::Graphics& g, juce::Rectangle<float> bounds, float corner)
{
    g.setColour (obsidianWell.withAlpha (0.92f));
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (juce::Colours::black.withAlpha (0.38f));
    g.fillRoundedRectangle (bounds.withHeight (juce::jmin (4.0f, bounds.getHeight() * 0.35f)), corner);

    g.setColour (glassRimHighlight.withAlpha (0.16f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 0.7f);

    g.setColour (crimsonAccent.withAlpha (0.16f));
    g.drawRoundedRectangle (bounds.reduced (1.4f), juce::jmax (1.0f, corner - 1.2f), 0.7f);
}

void drawSignalChip (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text)
{
    g.setColour (obsidianScreen.withAlpha (0.92f));
    g.fillRoundedRectangle (bounds, 2.0f);

    g.setColour (gunmetalLight.withAlpha (0.42f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 2.0f, 0.7f);

    g.setColour (crimsonAccent.withAlpha (0.30f));
    g.fillRect (bounds.getX() + 4.0f, bounds.getBottom() - 1.5f, bounds.getWidth() - 8.0f, 0.8f);

    g.setColour (textSecondary);
    g.setFont (font (5.9f, Weight::semibold, 0.15f));
    g.drawText (text, bounds, juce::Justification::centred, false);
}

void drawCrimsonDot (juce::Graphics& g, juce::Point<float> centre, float radius)
{
    g.setColour (crimsonGlow.withAlpha (0.28f));
    g.fillEllipse (centre.x - radius * 2.4f, centre.y - radius * 2.4f,
                   radius * 4.8f, radius * 4.8f);
    g.setColour (crimsonAccent);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (crimsonBright);
    g.fillEllipse (centre.x - radius * 0.45f, centre.y - radius * 0.45f,
                   radius * 0.9f, radius * 0.9f);
}
} // namespace studio::ui
