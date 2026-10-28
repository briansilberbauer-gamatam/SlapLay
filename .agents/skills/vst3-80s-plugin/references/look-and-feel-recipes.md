# Futuristic Studio Hardware (Gunmetal, Glass & Muted Crimson) — JUCE 8 Drawing Recipes

Use these reference implementations in `Source/ui/Theme.cpp`, `Source/ui/StudioLookAndFeel.cpp`, `Source/ui/Controls.cpp`, and `Source/ui/WaveformDisplay.cpp` to achieve the **Sleek Futuristic Studio Hardware (Brushed Gunmetal Chassis + Flush Glass Overlay + Muted Crimson LED Edge-Glow)** look and feel.

---

## 1. Desktop Unit Chassis, Brushed Gunmetal Base & Crimson-Edged Glass Overlay

Renders the desktop audio unit sitting on a dark studio surface with upper-left soft-box lighting, smoothly chamfered matte gunmetal edges, fine horizontal brushed metal grain, a thick seamless transparent glass overlay (**zero visible screws or fasteners**), and subtle crimson LED edge-glow/refraction:

```cpp
void drawDesktopHardwareChassis (juce::Graphics& g, juce::Rectangle<float> fullBounds)
{
    using namespace studio::ui;

    // 1. Dark studio surface background with upper-left soft-box falloff
    juce::ColourGradient studioGrad (studioSurfaceLight, fullBounds.getTopLeft(),
                                     studioSurfaceDark, fullBounds.getBottomRight(), true);
    g.setGradientFill (studioGrad);
    g.fillRect (fullBounds);

    // Inset desktop unit on the dark studio surface (three-quarter / dimensional presence)
    const auto unitBounds = fullBounds.reduced (18.0f, 16.0f);
    constexpr float outerCorner = 14.0f;

    // 2. Soft-box cast shadow toward bottom-right
    for (int i = 5; i >= 1; --i)
    {
        const float spread = static_cast<float> (i) * 3.2f;
        const float offset = static_cast<float> (i) * 2.4f;
        g.setColour (dropShadowDeep.withAlpha (0.16f));
        g.fillRoundedRectangle (unitBounds.expanded (spread * 0.4f).translated (offset, offset * 1.15f),
                                outerCorner + spread * 0.25f);
    }

    // 3. Dimensional 3/4 lower-right matte chamfered base extrusion
    const auto extrudedBase = unitBounds.translated (3.5f, 5.0f);
    juce::ColourGradient skirtGrad (chamferLight, extrudedBase.getTopLeft(),
                                    chamferDark, extrudedBase.getBottomRight(), false);
    g.setGradientFill (skirtGrad);
    g.fillRoundedRectangle (extrudedBase, outerCorner);

    // 4. Smoothly chamfered matte outer metal rim
    juce::ColourGradient chamferGrad (chamferLight, unitBounds.getTopLeft(),
                                      chamferDark, unitBounds.getBottomRight(), false);
    g.setGradientFill (chamferGrad);
    g.fillRoundedRectangle (unitBounds, outerCorner);

    // 5. Dark gunmetal gray brushed metal base with fine horizontal grain texture
    const auto metalBase = unitBounds.reduced (4.5f);
    constexpr float metalCorner = 10.5f;
    juce::ColourGradient metalGrad (gunmetalLight, metalBase.getTopLeft(),
                                    gunmetalDeep, metalBase.getBottomRight(), false);
    metalGrad.addColour (0.45, gunmetalBase);
    g.setGradientFill (metalGrad);
    g.fillRoundedRectangle (metalBase, metalCorner);

    // Fine horizontal brushed metal grain clipped to the gunmetal chassis
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

    // 6. Thick, seamless transparent glass overlay (NO screws or mounting hardware)
    const auto glassPanel = metalBase.reduced (6.0f);
    constexpr float glassCorner = 8.0f;

    juce::ColourGradient glassBodyGrad (glassTintTop, glassPanel.getTopLeft(),
                                        glassTintBottom, glassPanel.getBottomRight(), false);
    g.setGradientFill (glassBodyGrad);
    g.fillRoundedRectangle (glassPanel, glassCorner);

    // 7. Crimson LED edge-glow & inward refraction through the clear glass perimeter
    for (int ring = 4; ring >= 1; --ring)
    {
        const float inset = static_cast<float> (ring) * 2.2f;
        const float alpha = (5 - ring) * 0.038f;
        g.setColour (crimsonGlow.withAlpha (alpha));
        g.drawRoundedRectangle (glassPanel.reduced (inset),
                                juce::jmax (2.0f, glassCorner - inset * 0.5f), 2.2f);
    }

    // Crisp low-profile crimson LED edge light-pipe (strongest on vertical edges)
    juce::ColourGradient edgeGlowGrad (crimsonBright.withAlpha (0.78f), glassPanel.getTopLeft(),
                                       crimsonAccent.withAlpha (0.52f), glassPanel.getBottomRight(), false);
    edgeGlowGrad.addColour (0.5, crimsonAccent.withAlpha (0.85f));
    g.setGradientFill (edgeGlowGrad);
    g.drawRoundedRectangle (glassPanel, glassCorner, 1.35f);

    // Polished glass top-left specular bevel
    g.setColour (glassRimHighlight);
    g.drawRoundedRectangle (glassPanel.reduced (1.2f), glassCorner - 1.0f, 0.8f);

    // 8. Upper-left studio soft-box glossy specular reflection across the glass
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

        juce::ColourGradient sheenGrad (juce::Colours::white.withAlpha (0.065f), glassPanel.getTopLeft(),
                                        juce::Colours::transparentWhite,
                                        glassPanel.getPointOnCircumference (glassPanel.getWidth() * 0.45f, 2.2f), false);
        g.setGradientFill (sheenGrad);
        g.fillPath (sheen);
    }
}
```

---

## 2. Top Header Pill-Shaped Digital Readout (`HeaderPillBar`)

A slender, pill-shaped digital readout screen at the top center displaying plugin/preset info framed by minimalist chevron buttons (`<` `>`):

```cpp
void drawHeaderPillReadout (juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            const juce::String& presetTitle,
                            juce::Rectangle<float>& outPrevButton,
                            juce::Rectangle<float>& outNextButton)
{
    using namespace studio::ui;
    const float radius = bounds.getHeight() * 0.5f;

    // Subtle crimson edge refraction around the pill cutout
    g.setColour (crimsonGlow.withAlpha (0.28f));
    g.drawRoundedRectangle (bounds.expanded (1.2f), radius + 1.2f, 2.0f);

    // Recessed obsidian digital screen
    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds, radius);

    // Thin crimson + glass rim
    g.setColour (crimsonAccent.withAlpha (0.55f));
    g.drawRoundedRectangle (bounds, radius, 1.0f);
    g.setColour (glassRimHighlight.withAlpha (0.18f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), radius - 1.0f, 0.75f);

    // Minimalist left/right chevron hit zones
    const float navW = bounds.getHeight() * 1.15f;
    outPrevButton = bounds.withWidth (navW);
    outNextButton = bounds.withLeft (bounds.getRight() - navW);

    auto drawChevron = [&] (juce::Rectangle<float> area, bool pointLeft)
    {
        const auto c = area.getCentre();
        const float s = 4.2f;
        const float dir = pointLeft ? -1.0f : 1.0f;
        juce::Path p;
        p.startNewSubPath (c.x - dir * s * 0.5f, c.y - s);
        p.lineTo (c.x + dir * s * 0.6f, c.y);
        p.lineTo (c.x - dir * s * 0.5f, c.y + s);
        g.setColour (textSecondary);
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
    };

    drawChevron (outPrevButton, true);
    drawChevron (outNextButton, false);

    // Subtle vertical divider hairlines
    g.setColour (obsidianGrid);
    g.fillRect (outPrevButton.getRight(), bounds.getY() + 6.0f, 1.0f, bounds.getHeight() - 12.0f);
    g.fillRect (outNextButton.getX(),     bounds.getY() + 6.0f, 1.0f, bounds.getHeight() - 12.0f);

    // Crisp tracked digital readout text
    const auto textArea = bounds.withTrimmedLeft (navW + 6.0f).withTrimmedRight (navW + 6.0f);
    g.setColour (textPrimary);
    g.setFont (font (11.5f, FontWeight::semibold, 0.16f));
    g.drawText (presetTitle.toUpperCase(), textArea, juce::Justification::centred, false);
}
```

---

## 3. Central Embedded Real-Time Waveform Visualizer (`WaveformDisplay`)

A large, central waveform display screen embedded under the glass, featuring an obsidian background, subtle frequency/dB scale markers, and a glowing red real-time audio waveform:

```cpp
void drawWaveformVisualizer (juce::Graphics& g,
                             juce::Rectangle<float> bounds,
                             const float* minSamples,
                             const float* maxSamples,
                             int numColumns)
{
    using namespace studio::ui;
    constexpr float corner = 6.0f;

    // 1. Recessed obsidian screen under glass
    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds, corner);

    const auto inner = bounds.reduced (2.0f);
    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path screenClip;
        screenClip.addRoundedRectangle (inner, corner - 1.0f);
        g.reduceClipRegion (screenClip);

        // 2. Subtle dB and time/frequency grid hairlines
        constexpr int numHorizontalLines = 5; // +6, 0, -6, -12, -18 dB
        const char* dbLabels[numHorizontalLines] = { "0 dB", "-6 dB", "-12 dB", "-18 dB", "-24 dB" };
        g.setFont (font (7.5f, FontWeight::regular, 0.06f));

        for (int i = 0; i < numHorizontalLines; ++i)
        {
            const float prop = static_cast<float> (i + 1) / static_cast<float> (numHorizontalLines + 1);
            const float y = inner.getY() + prop * inner.getHeight();
            g.setColour (i == 2 ? obsidianGrid.brighter (0.12f) : obsidianGrid);
            g.fillRect (inner.getX(), y, inner.getWidth(), 1.0f);

            g.setColour (textMuted);
            g.drawText (dbLabels[i],
                        juce::Rectangle<float> (inner.getX() + 8.0f, y - 12.0f, 42.0f, 11.0f),
                        juce::Justification::centredLeft, false);
        }

        constexpr int numVerticalLines = 8;
        for (int i = 1; i < numVerticalLines; ++i)
        {
            const float prop = static_cast<float> (i) / static_cast<float> (numVerticalLines);
            const float x = inner.getX() + prop * inner.getWidth();
            g.setColour (obsidianGrid);
            g.fillRect (x, inner.getY(), 1.0f, inner.getHeight());
        }

        // 3. Real-time glowing red audio waveform
        if (numColumns > 1 && minSamples != nullptr && maxSamples != nullptr)
        {
            const float midY  = inner.getCentreY();
            const float halfH = inner.getHeight() * 0.44f;

            juce::Path fillPath;
            juce::Path topTrace;
            juce::Path bottomTrace;

            for (int i = 0; i < numColumns; ++i)
            {
                const float normX = static_cast<float> (i) / static_cast<float> (numColumns - 1);
                const float x     = inner.getX() + normX * inner.getWidth();
                const float yMax  = midY - juce::jlimit (-1.0f, 1.0f, maxSamples[i]) * halfH;

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
                const float x     = inner.getX() + normX * inner.getWidth();
                const float yMin  = midY - juce::jlimit (-1.0f, 1.0f, minSamples[i]) * halfH;
                fillPath.lineTo (x, yMin);

                if (i == numColumns - 1)
                    bottomTrace.startNewSubPath (x, yMin);
                else
                    bottomTrace.lineTo (x, yMin);
            }
            fillPath.closeSubPath();

            // Pass 1: Soft crimson inner envelope fill
            juce::ColourGradient envGrad (crimsonAccent.withAlpha (0.24f), inner.getX(), midY - halfH,
                                          crimsonGlow.withAlpha (0.06f),   inner.getX(), midY + halfH, false);
            g.setGradientFill (envGrad);
            g.fillPath (fillPath);

            // Pass 2: Wide ambient crimson LED bloom
            g.setColour (crimsonGlow.withAlpha (0.28f));
            g.strokePath (topTrace,    juce::PathStrokeType (5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.strokePath (bottomTrace, juce::PathStrokeType (5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Pass 3: Primary muted crimson waveform body
            g.setColour (crimsonAccent);
            g.strokePath (topTrace,    juce::PathStrokeType (2.1f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.strokePath (bottomTrace, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Pass 4: Crisp bright crimson core filament
            g.setColour (crimsonBright);
            g.strokePath (topTrace, juce::PathStrokeType (0.95f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 4. Glossy upper-left glass reflection over the embedded display
        juce::Path glassSheen;
        glassSheen.startNewSubPath (inner.getX(), inner.getY());
        glassSheen.lineTo (inner.getX() + inner.getWidth() * 0.42f, inner.getY());
        glassSheen.lineTo (inner.getX() + inner.getWidth() * 0.18f, inner.getBottom());
        glassSheen.lineTo (inner.getX(), inner.getBottom());
        glassSheen.closeSubPath();
        g.setColour (juce::Colours::white.withAlpha (0.032f));
        g.fillPath (glassSheen);
    }

    // 5. Recessed glass bezel & subtle crimson inner edge refraction
    g.setColour (crimsonAccent.withAlpha (0.42f));
    g.drawRoundedRectangle (bounds, corner, 1.1f);
    g.setColour (glassRimHighlight.withAlpha (0.16f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), corner - 1.0f, 0.8f);
}
```

---

## 4. Low-Profile Heavy-Machined Knurled Metal Knobs (`drawRotarySlider`)

Large, heavy-machined metal primary knobs (`"Drive"`, `"Tone"`, etc.) with fine knurled vertical ridges on the outer rim, recessed low-profile height, subtle crimson LED under-glow, and a tiny minimalist tick mark on the cap:

```cpp
void StudioLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider&)
{
    using namespace studio::ui;

    const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                static_cast<float> (width), static_cast<float> (height))
                            .reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    if (radius < 10.0f)
        return;

    const auto centre = bounds.getCentre();
    const float prop  = juce::jlimit (0.0f, 1.0f, sliderPosProportional);
    const float angle = rotaryStartAngle + prop * (rotaryEndAngle - rotaryStartAngle);

    // 1. Minimalist perimeter tick dots/hairlines etched into the glass
    const float scaleR = radius - 1.5f;
    for (int i = 0; i <= 24; ++i)
    {
        const float tProp = static_cast<float> (i) / 24.0f;
        const float a     = rotaryStartAngle + tProp * (rotaryEndAngle - rotaryStartAngle);
        const bool isMajor = (i % 4 == 0);
        const auto p1 = centre.getPointOnCircumference (scaleR - (isMajor ? 4.0f : 2.2f), a);
        const auto p2 = centre.getPointOnCircumference (scaleR, a);

        const bool lit = (tProp <= prop);
        g.setColour (lit ? crimsonAccent.withAlpha (isMajor ? 0.85f : 0.55f)
                         : textMuted.withAlpha (isMajor ? 0.55f : 0.28f));
        g.drawLine ({ p1, p2 }, isMajor ? 1.3f : 0.8f);
    }

    // 2. Recessed glass/metal well + subtle crimson LED under-glow beneath the knob
    const float wellR = radius * 0.86f;
    const juce::Rectangle<float> wellBounds (centre.x - wellR, centre.y - wellR, wellR * 2.0f, wellR * 2.0f);
    g.setColour (obsidianWell);
    g.fillEllipse (wellBounds);

    // Diffused crimson LED under-glow bleeding from under the knob skirt
    {
        juce::ColourGradient underGlow (crimsonAccent.withAlpha (0.18f + 0.28f * prop), centre.x, centre.y,
                                        juce::Colours::transparentBlack, centre.x + wellR, centre.y, true);
        g.setGradientFill (underGlow);
        g.fillEllipse (wellBounds.expanded (2.0f));

        juce::Path activeUnderArc;
        activeUnderArc.addCentredArc (centre.x, centre.y, wellR - 1.2f, wellR - 1.2f,
                                      0.0f, rotaryStartAngle, angle, true);
        g.setColour (crimsonGlow.withAlpha (0.45f));
        g.strokePath (activeUnderArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (crimsonAccent.withAlpha (0.85f));
        g.strokePath (activeUnderArc, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Soft upper-left lit / lower-right cast shadow (low-profile recessed height)
    const float rimR = radius * 0.80f;
    const juce::Rectangle<float> rimBounds (centre.x - rimR, centre.y - rimR, rimR * 2.0f, rimR * 2.0f);
    g.setColour (dropShadowDeep.withAlpha (0.65f));
    g.fillEllipse (rimBounds.translated (1.5f, 2.2f));

    // 4. Heavy-machined metal outer rim with fine vertical knurled ridges
    juce::ColourGradient rimGrad (gunmetalLight.brighter (0.15f), rimBounds.getTopLeft(),
                                  obsidianScreen, rimBounds.getBottomRight(), false);
    g.setGradientFill (rimGrad);
    g.fillEllipse (rimBounds);

    constexpr int numKnurls = 64;
    for (int i = 0; i < numKnurls; ++i)
    {
        const float kAngle = angle + static_cast<float> (i) * (juce::MathConstants<float>::twoPi / static_cast<float> (numKnurls));
        const auto k1 = centre.getPointOnCircumference (rimR * 0.86f, kAngle);
        const auto k2 = centre.getPointOnCircumference (rimR * 0.99f, kAngle);

        // Upper-left studio soft-box lighting modulates knurl specular intensity
        const float lightBias = 0.5f - 0.5f * std::sin (kAngle + juce::MathConstants<float>::pi * 0.25f);
        g.setColour (i % 2 == 0
                         ? juce::Colours::white.withAlpha (0.06f + 0.16f * lightBias)
                         : juce::Colours::black.withAlpha (0.65f));
        g.drawLine ({ k1, k2 }, 1.0f);
    }

    // 5. Flat, low-profile machined metal top cap
    const float capR = rimR * 0.84f;
    const juce::Rectangle<float> capBounds (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f);
    juce::ColourGradient capGrad (gunmetalLight.brighter (0.08f), capBounds.getTopLeft(),
                                  gunmetalDeep, capBounds.getBottomRight(), false);
    capGrad.addColour (0.5, gunmetalBase);
    g.setGradientFill (capGrad);
    g.fillEllipse (capBounds);

    // Fine horizontal brushed grain on the flat metal cap
    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path capClip;
        capClip.addEllipse (capBounds);
        g.reduceClipRegion (capClip);

        for (int gy = static_cast<int> (capBounds.getY()); gy < static_cast<int> (capBounds.getBottom()); gy += 2)
        {
            g.setColour (juce::Colours::white.withAlpha (0.022f));
            g.fillRect (capBounds.getX(), static_cast<float> (gy), capBounds.getWidth(), 1.0f);
        }
    }

    // Chamfered specular ring on flat cap edge
    g.setColour (glassRimHighlight.withAlpha (0.24f));
    g.drawEllipse (capBounds, 0.9f);
    g.setColour ( obsidianScreen.withAlpha (0.7f));
    g.drawEllipse (capBounds.expanded (0.8f), 0.8f);

    // 6. Tiny minimalist tick mark on the cap for position reference
    const auto tickInner = centre.getPointOnCircumference (capR * 0.64f, angle);
    const auto tickOuter = centre.getPointOnCircumference (capR * 0.88f, angle);
    g.setColour (crimsonGlow.withAlpha (0.45f));
    g.drawLine ({ tickInner, tickOuter }, 3.0f);
    g.setColour (textPrimary);
    g.drawLine ({ tickInner, tickOuter }, 1.5f);
}
```

---

## 5. Low-Profile Vertical Touch / Ultra-Thin Mechanical Sliders, Horizontal Output Meter & Right-Side Full-Height Mixer Bay (`drawLinearSlider` + `drawRightSideMixerBay` + `drawHorizontalOutputMeter`)

The three mixer sliders for **`Input`** (`dB`), **`Wet/Dry Mix`** (`%`), and **`Output`** (`dB`), along with a **Horizontal Output Level Meter positioned vertically below the mixers**, **must** be placed on the **right side of the component** inside the **Right-Side Full-Height Mixer Bay**.

### A. Right-Side Full-Height Mixer Bay Backdrop, Layout & Horizontal Output Meter Below Mixers (`drawRightSideMixerBay` + `layoutRightSideMixerBay` + `drawHorizontalOutputMeter`)

```cpp
// Call inside ContentComponent::paint(g) after drawDesktopHardwareChassis(g, getLocalBounds().toFloat()):
void drawRightSideMixerBay (juce::Graphics& g, juce::Rectangle<float> mixerBayBounds)
{
    using namespace studio::ui;

    // 1. Subtle smoked glass mixer bay recess spanning the full interior height on the right
    g.setColour (obsidianScreen.withAlpha (0.28f));
    g.fillRoundedRectangle (mixerBayBounds, 6.0f);

    // 2. Full-height vertical divider hairline on the left edge of the mixer bay
    const float divX = mixerBayBounds.getX() - 7.0f;
    g.setColour (obsidianGrid);
    g.fillRect (divX, mixerBayBounds.getY() + 4.0f, 1.0f, mixerBayBounds.getHeight() - 8.0f);

    // Subtle crimson vertical edge refraction along the mixer divider
    juce::ColourGradient divGlow (juce::Colours::transparentBlack, divX, mixerBayBounds.getY(),
                                  juce::Colours::transparentBlack, divX, mixerBayBounds.getBottom(), false);
    divGlow.addColour (0.5, crimsonAccent.withAlpha (0.32f));
    g.setGradientFill (divGlow);
    g.fillRect (divX + 1.0f, mixerBayBounds.getY() + 12.0f, 1.0f, mixerBayBounds.getHeight() - 24.0f);
}

// Call inside ContentComponent::resized() FIRST on innerGlass so the mixer bay takes the full height,
// with the Horizontal Output Meter carved out vertically BELOW the Input/Mix/Output mixer sliders:
void layoutRightSideMixerBay (juce::Rectangle<int>& innerGlass,
                              juce::Slider& inputSlider,   juce::Label& inputTitle,  juce::Label& inputValue,
                              juce::Slider& mixSlider,     juce::Label& mixTitle,    juce::Label& mixValue,
                              juce::Slider& outputSlider,  juce::Label& outputTitle, juce::Label& outputValue,
                              juce::Rectangle<float>& outMixerBayBounds,
                              juce::Rectangle<float>& outHorizontalOutputMeterBounds)
{
    constexpr int mixerBayWidth = 204;
    auto rightMixerBay = innerGlass.removeFromRight (mixerBayWidth);
    outMixerBayBounds  = rightMixerBay.toFloat();
    innerGlass.removeFromRight (14); // Divider clearance between main workspace and right mixer bay

    // 1. Carve out the Horizontal Output Level Meter vertically BELOW the mixer sliders
    constexpr int outputMeterHeight = 72;
    auto meterArea = rightMixerBay.removeFromBottom (outputMeterHeight).reduced (6, 4);
    outHorizontalOutputMeterBounds = meterArea.toFloat();

    rightMixerBay.removeFromBottom (8); // Vertical separation between mixer sliders and horizontal Output meter

    // 2. Lay out the 3 vertical mixer sliders (Input, Wet/Dry Mix, Output) in the upper section
    juce::Slider* sliders[3] = { &inputSlider, &mixSlider, &outputSlider };
    juce::Label*  titles[3]  = { &inputTitle,  &mixTitle,  &outputTitle };
    juce::Label*  values[3]  = { &inputValue,  &mixValue,  &outputValue };

    const int colWidth = rightMixerBay.getWidth() / 3;
    for (int i = 0; i < 3; ++i)
    {
        auto col = rightMixerBay.removeFromLeft (i == 2 ? rightMixerBay.getWidth() : colWidth)
                                .reduced (4, 4);
        titles[i]->setBounds (col.removeFromTop (22));
        values[i]->setBounds (col.removeFromBottom (22));
        sliders[i]->setBounds (col); // Vertical slider occupies the upper mixer bay span above the Output meter
    }
}

// Render the Horizontal Output Level Meter vertically below the mixer sliders:
void drawHorizontalOutputMeter (juce::Graphics& g,
                                juce::Rectangle<float> bounds,
                                float outLeftDb,
                                float outRightDb,
                                float peakHoldDb)
{
    using namespace studio::ui;

    // 1. Recessed obsidian screen well embedded under the glass
    g.setColour (obsidianScreen);
    g.fillRoundedRectangle (bounds, 5.0f);

    g.setColour (glassRimHighlight.withAlpha (0.18f));
    g.drawRoundedRectangle (bounds, 5.0f, 0.8f);
    g.setColour (crimsonAccent.withAlpha (0.28f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), 4.2f, 0.6f);

    auto inner = bounds.reduced (8.0f, 5.0f);

    // 2. Top header row: "OUT METER" title on the left, live peak dB readout on the right
    auto headerRow = inner.removeFromTop (14.0f);
    g.setColour (textSecondary);
    g.setFont (font (8.0f, FontWeight::semibold, 0.10f));
    g.drawText ("OUT METER", headerRow, juce::Justification::centredLeft, false);

    const juce::String peakText = (peakHoldDb <= -59.5f)
        ? juce::String ("-INF dB")
        : juce::String (peakHoldDb, 1) + " dB";
    g.setColour (peakHoldDb > -0.5f ? crimsonBright : textPrimary);
    g.setFont (font (8.5f, FontWeight::medium, 0.04f));
    g.drawText (peakText, headerRow, juce::Justification::centredRight, false);

    inner.removeFromTop (3.0f);
    auto scaleRow = inner.removeFromBottom (11.0f);

    // 3. Horizontal stereo (L / R) meter tracks (-60 dB on left to 0 dB on right)
    const float labelW   = 11.0f;
    const float barLeft  = inner.getX() + labelW;
    const float barRight = inner.getRight() - 2.0f;
    const float barW     = juce::jmax (20.0f, barRight - barLeft);
    constexpr float barH = 6.5f;
    const float midY     = inner.getCentreY();

    const juce::Rectangle<float> leftTrack  (barLeft, midY - barH - 1.5f, barW, barH);
    const juce::Rectangle<float> rightTrack (barLeft, midY + 1.5f,        barW, barH);

    g.setColour (textMuted);
    g.setFont (font (7.5f, FontWeight::semibold));
    g.drawText ("L", juce::Rectangle<float> (inner.getX(), leftTrack.getY() - 1.0f, labelW - 2.0f, barH + 2.0f),
                juce::Justification::centredLeft, false);
    g.drawText ("R", juce::Rectangle<float> (inner.getX(), rightTrack.getY() - 1.0f, labelW - 2.0f, barH + 2.0f),
                juce::Justification::centredLeft, false);

    g.setColour (obsidianWell);
    g.fillRoundedRectangle (leftTrack, 2.0f);
    g.fillRoundedRectangle (rightTrack, 2.0f);

    auto dbToNorm = [] (float db) noexcept
    {
        return juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    };

    // 4. Glowing crimson horizontal L & R level bars filling from left to right
    auto drawChannelFill = [&] (juce::Rectangle<float> track, float levelDb)
    {
        const float norm  = dbToNorm (levelDb);
        const float fillW = norm * track.getWidth();
        if (fillW <= 0.5f)
            return;

        const auto fillRect = track.withWidth (fillW);

        // Soft crimson outer bloom
        g.setColour (crimsonGlow.withAlpha (0.35f));
        g.fillRoundedRectangle (fillRect.expanded (0.5f, 1.2f), 2.0f);

        // Horizontal gradient from muted crimson on the left to hot crimson near 0 dB on the right
        juce::ColourGradient barGrad (crimsonAccent, track.getX(),     fillRect.getCentreY(),
                                      crimsonBright, track.getRight(), fillRect.getCentreY(), false);
        g.setGradientFill (barGrad);
        g.fillRoundedRectangle (fillRect, 1.5f);

        // Crisp right-edge peak hairline
        g.setColour (crimsonBright);
        g.fillRect (fillRect.getRight() - 1.5f, fillRect.getY(), 1.5f, fillRect.getHeight());
    };

    drawChannelFill (leftTrack,  outLeftDb);
    drawChannelFill (rightTrack, outRightDb);

    // Ballistic peak-hold vertical marker across both L/R tracks
    if (peakHoldDb > -59.0f)
    {
        const float peakX = barLeft + dbToNorm (peakHoldDb) * barW;
        g.setColour (crimsonBright.withAlpha (0.85f));
        g.fillRect (peakX - 0.6f, leftTrack.getY(), 1.2f, rightTrack.getBottom() - leftTrack.getY());
    }

    // 5. Bottom horizontal dB scale ticks & labels (-48, -36, -24, -12, -6, 0)
    constexpr float tickDbs[] = { -48.0f, -36.0f, -24.0f, -12.0f, -6.0f, 0.0f };
    g.setFont (font (7.0f, FontWeight::regular));
    for (float db : tickDbs)
    {
        const float x = barLeft + dbToNorm (db) * barW;
        g.setColour (obsidianGrid.brighter (0.15f));
        g.fillRect (x - 0.5f, rightTrack.getBottom() + 1.0f, 1.0f, 2.5f);

        g.setColour (textMuted);
        g.drawText (juce::String (static_cast<int> (db)),
                    juce::Rectangle<float> (x - 10.0f, scaleRow.getY(), 20.0f, scaleRow.getHeight()),
                    juce::Justification::centred, false);
    }
}
```

### B. Ultra-Thin Recessed Vertical Glass Slider Rendering (`drawLinearSlider`)

Vertical touch-style or ultra-thin mechanical sliders (`"Input"`, `"Mix"`, `"Output"`) integrated smoothly into the right-side full-height glass mixer bay with subtly recessed linear tracks and minimalist tick marks alongside them:

```cpp
void StudioLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                          const juce::Slider::SliderStyle style, juce::Slider&)
{
    using namespace studio::ui;

    if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearBarVertical)
        return;

    const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                static_cast<float> (width), static_cast<float> (height));
    const float centerX = bounds.getCentreX();
    const float trackTop = bounds.getY() + 8.0f;
    const float trackBot = bounds.getBottom() - 8.0f;
    const float trackH   = juce::jmax (10.0f, trackBot - trackTop);
    const float clampedY = juce::jlimit (trackTop, trackBot, sliderPos);

    // 1. Minimalist horizontal tick marks alongside both sides of the full-height recessed track
    constexpr int numTicks = 21;
    for (int i = 0; i < numTicks; ++i)
    {
        const float prop  = static_cast<float> (i) / static_cast<float> (numTicks - 1);
        const float tickY = trackTop + prop * trackH;
        const bool isMajor = (i % 5 == 0);
        const float tickLen = isMajor ? 7.0f : 4.0f;
        const float gap     = 6.5f;

        g.setColour (isMajor ? textSecondary.withAlpha (0.55f) : textMuted.withAlpha (0.35f));
        g.fillRect (centerX - gap - tickLen, tickY - 0.5f, tickLen, 1.0f);
        g.fillRect (centerX + gap,           tickY - 0.5f, tickLen, 1.0f);
    }

    // 2. Subtly recessed ultra-thin linear track in the glass surface
    const juce::Rectangle<float> trackRect (centerX - 2.0f, trackTop, 4.0f, trackH);
    g.setColour (obsidianWell);
    g.fillRoundedRectangle (trackRect, 2.0f);
    g.setColour (glassRimHighlight.withAlpha (0.14f));
    g.drawRoundedRectangle (trackRect.expanded (0.5f), 2.2f, 0.7f);

    // 3. Muted crimson active level fill inside the recessed track
    const float fillH = trackBot - clampedY;
    if (fillH > 0.5f)
    {
        const juce::Rectangle<float> activeRect (centerX - 1.5f, clampedY, 3.0f, fillH);
        g.setColour (crimsonGlow.withAlpha (0.38f));
        g.fillRoundedRectangle (activeRect.expanded (1.5f, 0.5f), 2.0f);
        g.setColour (crimsonAccent);
        g.fillRoundedRectangle (activeRect, 1.5f);
    }

    // 4. Low-profile flush touch / mechanical slider cap
    const juce::Rectangle<float> thumb (centerX - 12.0f, clampedY - 4.5f, 24.0f, 9.0f);
    g.setColour (dropShadowDeep.withAlpha (0.55f));
    g.fillRoundedRectangle (thumb.translated (0.8f, 1.4f), 2.0f);

    juce::ColourGradient thumbGrad (gunmetalLight.brighter (0.18f), thumb.getTopLeft(),
                                    gunmetalDeep, thumb.getBottomRight(), false);
    g.setGradientFill (thumbGrad);
    g.fillRoundedRectangle (thumb, 2.0f);

    g.setColour (glassRimHighlight.withAlpha (0.28f));
    g.drawRoundedRectangle (thumb, 2.0f, 0.8f);

    // Crisp minimalist crimson reference line across thumb center
    g.setColour (crimsonBright);
    g.fillRect (thumb.getX() + 3.0f, clampedY - 0.5f, thumb.getWidth() - 6.0f, 1.2f);
}
```

---

## 6. Low-Profile Rectangular Tactile Buttons (`drawToggleButton`)

Rectangular, low-profile tactile buttons (`"Bypass"`, `"Filter"`, `"Comp"`, `"Limiter"`) with crisp sans-serif typography, outlined by a thin, understated red glowing border when active:

```cpp
void StudioLookAndFeel::drawToggleButton (juce::Graphics& g,
                                          juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool /*shouldDrawButtonAsDown*/)
{
    using namespace studio::ui;

    const auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    constexpr float corner = 3.5f;
    const bool active = button.getToggleState();

    // 1. Subtle recessed seat in the glass surface
    g.setColour (obsidianWell);
    g.fillRoundedRectangle (bounds.expanded (1.0f), corner + 0.8f);

    // 2. Low-profile flush button surface
    juce::ColourGradient capGrad (active ? gunmetalLight.interpolatedWith (crimsonGlow, 0.14f)
                                         : gunmetalBase,
                                  bounds.getTopLeft(),
                                  gunmetalDeep,
                                  bounds.getBottomRight(),
                                  false);
    g.setGradientFill (capGrad);
    g.fillRoundedRectangle (bounds, corner);

    // 3. Border: Thin, understated red glowing border when active; matte dark rim when inactive
    if (active)
    {
        // Soft diffused crimson halo
        g.setColour (crimsonGlow.withAlpha (0.36f));
        g.drawRoundedRectangle (bounds.expanded (1.0f), corner + 0.5f, 2.8f);

        // Crisp 1.2px crimson border
        g.setColour (crimsonAccent);
        g.drawRoundedRectangle (bounds, corner, 1.2f);

        // Subtle upper-left specular catch on the glowing border
        g.setColour (crimsonBright.withAlpha (0.75f));
        g.drawRoundedRectangle (bounds.reduced (0.4f), corner, 0.6f);
    }
    else
    {
        g.setColour (shouldDrawButtonAsHighlighted ? chamferLight : gunmetalLight.withAlpha (0.65f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }

    // 4. Crisp, clean sans-serif typography
    g.setColour (active ? textPrimary : textSecondary);
    g.setFont (font (10.5f, active ? FontWeight::semibold : FontWeight::regular, 0.10f));
    g.drawText (button.getButtonText().toUpperCase(), bounds, juce::Justification::centred, false);
}
```
