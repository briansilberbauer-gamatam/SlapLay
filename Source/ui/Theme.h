#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace studio::ui
{
    // --- Dark Studio Surface & Ambient Soft-Box Backdrop ---
    inline const juce::Colour studioSurfaceLight { 0xff12151c }; // Upper-left soft-box lit desk surface
    inline const juce::Colour studioSurfaceDark  { 0xff07080b }; // Lower-right shadowed studio surface
    inline const juce::Colour dropShadowDeep     { 0xff020305 }; // Desktop unit cast shadow

    // --- Brushed Gunmetal Base Chassis & Chamfered Edges ---
    inline const juce::Colour gunmetalLight      { 0xff2b3038 }; // Upper-left brushed metal highlight
    inline const juce::Colour gunmetalBase       { 0xff1d2026 }; // Primary gunmetal gray chassis
    inline const juce::Colour gunmetalDeep       { 0xff13151a }; // Lower-right chassis shade & recesses
    inline const juce::Colour chamferLight       { 0xff343a44 }; // Upper-left matte chamfered edge catch
    inline const juce::Colour chamferDark        { 0xff0e1014 }; // Lower-right chamfered skirt extrusion

    // --- Obsidian Black Screens & Recessed Cavities ---
    inline const juce::Colour obsidianScreen     { 0xff090a0d }; // Main visualizer & top pill readout background
    inline const juce::Colour obsidianWell       { 0xff0d0f13 }; // Low-profile knob & slider mounting recess
    inline const juce::Colour obsidianGrid       { 0xff181b22 }; // Subtle dB / frequency scale grid hairlines

    // --- Transparent Glass Overlay & Soft-Box Reflections ---
    inline const juce::Colour glassTintTop       { 0x1affffff }; // Upper-left soft-box glass sheen
    inline const juce::Colour glassTintBottom    { 0x26050608 }; // Deep smoked glass depth tint
    inline const juce::Colour glassSpecular      { 0x14ffffff }; // Glossy diagonal soft-box reflection band
    inline const juce::Colour glassRimHighlight  { 0x33ffffff }; // Crisp 1px polished glass top/left bevel

    // --- Muted, Sophisticated Crimson Red Accents ---
    inline const juce::Colour crimsonAccent      { 0xffc82230 }; // Primary muted crimson LED & active border
    inline const juce::Colour crimsonBright      { 0xffef3b49 }; // Hot core of waveform trace & edge-glow center
    inline const juce::Colour crimsonGlow        { 0xff8c141f }; // Diffused crimson refraction & knob under-glow
    inline const juce::Colour crimsonSubtle      { 0xff3d1218 }; // Dormant/dim crimson scale & track undertone

    // --- Clean Minimalist Sans-Serif Typography ---
    inline const juce::Colour textPrimary        { 0xffeceff4 }; // Active labels, pill readout & cap tick marks
    inline const juce::Colour textSecondary      { 0xff929aa8 }; // Control titles ("Drive", "Tone", "Mix", "Output")
    inline const juce::Colour textMuted          { 0xff545b68 }; // Subtle dB/Hz scale markers & inactive buttons

    enum class Weight
    {
        regular,
        semibold,
        black
    };

    juce::Font font (float height, Weight weight = Weight::regular, float tracking = 0.0f);

    // Dark studio surface, dimensional chamfered gunmetal unit, brushed grain,
    // flush screwless glass overlay and the crimson LED edge-glow.
    void drawDesktopHardwareChassis (juce::Graphics&, juce::Rectangle<float> bounds);

    // Engraved section label with a tiny crimson index tick and a muted subtitle.
    void drawEngravedLabel (juce::Graphics&, juce::Rectangle<float> bounds,
                            const juce::String& title, const juce::String& subtitle);

    // Recessed obsidian glass well used behind meters and readouts.
    void drawGlassWell (juce::Graphics&, juce::Rectangle<float> bounds, float corner);

    // Small signal-path chip used in the footer.
    void drawSignalChip (juce::Graphics&, juce::Rectangle<float> bounds, const juce::String& text);

    // Soft crimson LED bead with a diffused halo.
    void drawCrimsonDot (juce::Graphics&, juce::Point<float> centre, float radius);
} // namespace studio::ui
