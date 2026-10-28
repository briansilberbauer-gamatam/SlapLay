#pragma once

#include <juce_core/juce_core.h>

#include <cmath>

namespace slaplay::dsp
{
    /**
        Tape-style soft clipper.

        The transfer curve is tanh(x * drive) / drive, so the small-signal gain
        stays at unity while peaks are progressively compressed. Blending the
        shaped signal with the input by `amount` lets the TAPE DRIVE control go
        from completely clean to heavily saturated without a level jump at zero.
    */
    class TapeSaturation
    {
    public:
        void setAmount (float newAmount) noexcept
        {
            amount = juce::jlimit (0.0f, 1.0f, newAmount);
        }

        float getAmount() const noexcept { return amount; }

        float process (float sample) const noexcept
        {
            if (amount <= 1.0e-5f)
                return sample;

            const float drive = 1.0f + 4.0f * amount;
            const float shaped = std::tanh (sample * drive) / drive;

            return sample + amount * (shaped - sample);
        }

    private:
        float amount = 0.0f;
    };
} // namespace slaplay::dsp
