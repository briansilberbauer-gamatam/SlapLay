#pragma once

#include "Parameters.h"

#include <vector>

namespace slaplay
{
    struct PresetValue
    {
        const char* id;
        float value;
    };

    struct FactoryPreset
    {
        const char* name;
        const char* character;
        std::vector<PresetValue> overrides;
    };

    inline const std::vector<FactoryPreset>& getFactoryPresets()
    {
        using namespace params;

        static const std::vector<FactoryPreset> presets
        {
            {
                "Sunset Slap", "110 MS / WARM TAPE / PARALLEL",
                {
                    { delayTime, 110.0f }, { feedback, 32.0f }, { lowCut, 160.0f },
                    { highCut, 3600.0f }, { tapeDrive, 42.0f }, { wow, 18.0f },
                    { flutter, 10.0f }, { mix, 32.0f }, { inputGain, 0.0f },
                    { outputGain, 0.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Rockabilly '59", "95 MS / BRIGHT / TIGHT REPEATS",
                {
                    { delayTime, 95.0f }, { feedback, 24.0f }, { lowCut, 120.0f },
                    { highCut, 6500.0f }, { tapeDrive, 26.0f }, { wow, 8.0f },
                    { flutter, 6.0f }, { mix, 38.0f }, { inputGain, 0.0f },
                    { outputGain, -1.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Tape Slap 120", "120 MS / DARK / HEAVY WOW",
                {
                    { delayTime, 120.0f }, { feedback, 45.0f }, { lowCut, 220.0f },
                    { highCut, 2800.0f }, { tapeDrive, 56.0f }, { wow, 40.0f },
                    { flutter, 24.0f }, { mix, 34.0f }, { inputGain, 0.0f },
                    { outputGain, -1.5f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Long Train", "185 MS / HIGH FEEDBACK / DUB",
                {
                    { delayTime, 185.0f }, { feedback, 72.0f }, { lowCut, 180.0f },
                    { highCut, 2400.0f }, { tapeDrive, 66.0f }, { wow, 30.0f },
                    { flutter, 18.0f }, { mix, 40.0f }, { inputGain, 0.0f },
                    { outputGain, -3.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Slapback 68", "68 MS / SHORT ROCKABILLY / CLEAN",
                {
                    { delayTime, 68.0f }, { feedback, 18.0f }, { lowCut, 110.0f },
                    { highCut, 7000.0f }, { tapeDrive, 16.0f }, { wow, 5.0f },
                    { flutter, 4.0f }, { mix, 42.0f }, { inputGain, 0.0f },
                    { outputGain, -0.5f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Bucket Brigade", "145 MS / 12 dB FILTER / LO-FI",
                {
                    { delayTime, 145.0f }, { feedback, 58.0f }, { lowCut, 380.0f },
                    { highCut, 2100.0f }, { tapeDrive, 74.0f }, { wow, 55.0f },
                    { flutter, 38.0f }, { mix, 45.0f }, { inputGain, 0.0f },
                    { outputGain, -4.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Stadium Slap", "200 MS / WIDE / LONG THROW",
                {
                    { delayTime, 200.0f }, { feedback, 40.0f }, { lowCut, 150.0f },
                    { highCut, 3200.0f }, { tapeDrive, 30.0f }, { wow, 20.0f },
                    { flutter, 12.0f }, { mix, 46.0f }, { inputGain, 0.0f },
                    { outputGain, -2.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Studio Tight", "80 MS / CLEAN / LOW MIX",
                {
                    { delayTime, 80.0f }, { feedback, 15.0f }, { lowCut, 90.0f },
                    { highCut, 9000.0f }, { tapeDrive, 8.0f }, { wow, 4.0f },
                    { flutter, 2.0f }, { mix, 24.0f }, { inputGain, 0.0f },
                    { outputGain, 0.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Ghost Echo", "160 MS / SATURATED / DARK LOOP",
                {
                    { delayTime, 160.0f }, { feedback, 66.0f }, { lowCut, 300.0f },
                    { highCut, 1800.0f }, { tapeDrive, 88.0f }, { wow, 62.0f },
                    { flutter, 46.0f }, { mix, 38.0f }, { inputGain, 1.0f },
                    { outputGain, -5.0f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            },
            {
                "Space Slap", "175 MS / SLOW WOW / CINEMATIC",
                {
                    { delayTime, 175.0f }, { feedback, 52.0f }, { lowCut, 200.0f },
                    { highCut, 2600.0f }, { tapeDrive, 40.0f }, { wow, 82.0f },
                    { flutter, 20.0f }, { mix, 55.0f }, { inputGain, 0.0f },
                    { outputGain, -3.5f }, { tapeEnabled, 1.0f }, { filterEnabled, 1.0f },
                    { modulationEnabled, 1.0f }, { bypass, 0.0f }
                }
            }
        };

        return presets;
    }

    inline int getNumFactoryPresets()
    {
        return static_cast<int> (getFactoryPresets().size());
    }

    inline juce::String getFactoryPresetName (int index)
    {
        const auto& presets = getFactoryPresets();

        if (index < 0 || index >= static_cast<int> (presets.size()))
            return {};

        return presets[static_cast<size_t> (index)].name;
    }

    inline juce::String getFactoryPresetCharacter (int index)
    {
        const auto& presets = getFactoryPresets();

        if (index < 0 || index >= static_cast<int> (presets.size()))
            return {};

        return presets[static_cast<size_t> (index)].character;
    }
} // namespace slaplay
