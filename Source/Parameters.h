#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>

namespace slaplay::params
{
    // Input / output trims.
    inline constexpr const char* inputGain          = "inputGain";
    inline constexpr const char* outputGain         = "outputGain";
    inline constexpr const char* mix                = "mix";
    inline constexpr const char* bypass             = "bypass";

    // Short tape delay core.
    inline constexpr const char* delayTime          = "delayTime";
    inline constexpr const char* feedback           = "feedback";

    // Feedback-only 12 dB/octave tone shaping.
    inline constexpr const char* lowCut             = "lowCut";
    inline constexpr const char* highCut            = "highCut";

    // Tape degradation and mechanical speed instability.
    inline constexpr const char* tapeDrive          = "tapeDrive";
    inline constexpr const char* wow                = "wow";
    inline constexpr const char* flutter            = "flutter";

    // Stage bypass switches.
    inline constexpr const char* filterEnabled      = "filterEnabled";
    inline constexpr const char* tapeEnabled        = "tapeEnabled";
    inline constexpr const char* modulationEnabled  = "modulationEnabled";

    inline constexpr std::array<const char*, 14> allParameterIDs
    {
        inputGain, outputGain, mix, bypass,
        delayTime, feedback, lowCut, highCut,
        tapeDrive, wow, flutter,
        filterEnabled, tapeEnabled, modulationEnabled
    };

    inline constexpr float minimumDelayMs = 50.0f;
    inline constexpr float maximumDelayMs = 200.0f;
    inline constexpr float defaultDelayMs = 110.0f;

    inline constexpr float minimumLowCutHz  = 40.0f;
    inline constexpr float maximumLowCutHz  = 900.0f;
    inline constexpr float minimumHighCutHz = 1000.0f;
    inline constexpr float maximumHighCutHz = 12000.0f;
} // namespace slaplay::params
