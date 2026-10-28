#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>

namespace slaplay::dsp
{
    /**
        Stereo variable-length delay line with 4-point cubic Hermite interpolation.

        The write head is always one sample ahead of a read at the minimum delay,
        so a delay of exactly N samples returns the sample written N steps ago.
        The fractional read is performed with a Catmull-Rom style cubic, which
        keeps the tape-style wow/flutter modulation free of the hard edge that
        linear interpolation produces on short delays.
    */
    class FractionalDelayLine
    {
    public:
        void prepare (double sampleRate, double maxDelaySeconds)
        {
            const auto required = static_cast<int> (std::ceil (sampleRate * maxDelaySeconds)) + 8;
            size = std::max (8, required);

            buffer.setSize (2, size, false, true, false);
            reset();
        }

        void reset() noexcept
        {
            buffer.clear();
            writeIndex = 0;
        }

        void write (float left, float right) noexcept
        {
            buffer.setSample (0, writeIndex, left);
            buffer.setSample (1, writeIndex, right);

            if (++writeIndex >= size)
                writeIndex = 0;
        }

        float readLeft (float delaySamples) const noexcept
        {
            return readInterpolated (0, delaySamples);
        }

        float readRight (float delaySamples) const noexcept
        {
            return readInterpolated (1, delaySamples);
        }

        int getSize() const noexcept { return size; }

        float getMaximumDelaySamples() const noexcept
        {
            return static_cast<float> (std::max (0, size - 4));
        }

    private:
        float readInterpolated (int channel, float delaySamples) const noexcept
        {
            const float clamped = juce::jlimit (1.0f,
                                                static_cast<float> (std::max (1, size - 4)),
                                                delaySamples);

            float readPosition = static_cast<float> (writeIndex) - clamped;

            if (readPosition < 0.0f)
                readPosition += static_cast<float> (size);

            // Guard against a pathological delay that wrapped past the buffer.
            if (readPosition >= static_cast<float> (size))
                readPosition -= static_cast<float> (size);

            const int index1 = static_cast<int> (std::floor (readPosition));
            const float fraction = readPosition - static_cast<float> (index1);

            int index0 = index1 - 1;
            int index2 = index1 + 1;
            int index3 = index1 + 2;

            if (index0 < 0)
                index0 += size;
            if (index2 >= size)
                index2 -= size;
            if (index3 >= size)
                index3 -= size;

            const auto* data = buffer.getReadPointer (channel);

            const float y0 = data[index0];
            const float y1 = data[index1];
            const float y2 = data[index2];
            const float y3 = data[index3];

            // Catmull-Rom / cubic Hermite coefficients.
            const float c0 = y1;
            const float c1 = 0.5f * (y2 - y0);
            const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

            return ((c3 * fraction + c2) * fraction + c1) * fraction + c0;
        }

        juce::AudioBuffer<float> buffer;
        int size = 0;
        int writeIndex = 0;

        JUCE_LEAK_DETECTOR (FractionalDelayLine)
    };
} // namespace slaplay::dsp
