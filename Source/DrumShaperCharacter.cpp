#include "DrumShaperCharacter.h"
#include <algorithm>

namespace drumshaper
{
void DrumShaperCharacter::prepare(double sampleRate) noexcept
{
    rate = std::max(1.0, sampleRate);
    attackCoefficient = std::exp(-1.0 / (rate * 0.005));
    releaseCoefficient = std::exp(-1.0 / (rate * 0.100));
    reset();
}

void DrumShaperCharacter::setParameters(float compressionPercent, float driveDb) noexcept
{
    targetCompression = std::clamp(compressionPercent, 0.0f, 100.0f) * 0.01f;
    targetDrive = std::clamp(driveDb, 0.0f, 24.0f);
}

void DrumShaperCharacter::reset() noexcept
{
    envelope = 0.0;
    gainReductionDb.store(0.0f, std::memory_order_relaxed);
}

void DrumShaperCharacter::process(float* const* channels, int numChannels, int numSamples) noexcept
{
    const auto channelCount = std::clamp(numChannels, 0, 2);
    if (channelCount == 0 || numSamples <= 0)
        return;

    float blockReduction = 0.0f;
    const auto smoothing = 1.0f - std::exp(-1.0f / (0.010f * (float)rate));
    for (int n = 0; n < numSamples; ++n)
    {
        compression += smoothing * (targetCompression - compression);
        drive += smoothing * (targetDrive - drive);

        double detector = 0.0;
        for (int ch = 0; ch < channelCount; ++ch)
            detector = std::max(detector, std::abs((double)channels[ch][n]));

        const auto envelopeCoefficient = detector > envelope ? attackCoefficient : releaseCoefficient;
        envelope = envelopeCoefficient * envelope + (1.0 - envelopeCoefficient) * detector;

        // The macro blends from unity ratio toward 8:1 above a fixed -18 dBFS
        // threshold. Its zero position is transparent and requires no makeup.
        const auto ratio = 1.0 + 7.0 * (double)compression;
        const auto inputDb = 20.0 * std::log10(std::max(envelope, 1.0e-9));
        const auto overThreshold = std::max(0.0, inputDb + 18.0);
        const auto reduction = (float)(overThreshold * (1.0 - 1.0 / ratio));
        blockReduction = std::max(blockReduction, reduction);
        const auto compressorGain = std::pow(10.0, -(double)reduction / 20.0);

        const auto driveGain = std::pow(10.0, (double)drive / 20.0);
        const auto saturationNorm = std::tanh(driveGain);
        for (int ch = 0; ch < channelCount; ++ch)
        {
            auto sample = (double)channels[ch][n] * compressorGain;
            if (drive > 0.001f)
                sample = std::tanh(sample * driveGain) / saturationNorm;
            channels[ch][n] = std::isfinite(sample) ? (float)sample : 0.0f;
        }
    }
    gainReductionDb.store(blockReduction, std::memory_order_relaxed);
}
}
