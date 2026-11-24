#include "DrumShaperCharacter.h"
#include <algorithm>

namespace drumshaper
{
void DrumShaperCharacter::prepare(double sampleRate) noexcept
{
    rate = std::max(1.0, sampleRate);
    attackCoefficient = std::exp(-1.0 / (rate * 0.005));
    releaseCoefficient = std::exp(-1.0 / (rate * 0.100));
    const auto coefficient = [this](double ms) { return std::exp(-1.0 / (rate * ms * 0.001)); };
    vintageAttack = coefficient(0.4);
    vintageReleaseFast = coefficient(60.0);
    vintageReleaseSlow = coefficient(450.0);
    slamAttack = coefficient(15.0);
    slamRelease = coefficient(140.0);
    reset();
}

void DrumShaperCharacter::setParameters(float compressionPercent, float driveDb, CompressorType newType) noexcept
{
    type = newType;
    targetCompression = std::clamp(compressionPercent, 0.0f, 100.0f) * 0.01f;
    targetDrive = std::clamp(driveDb, 0.0f, 24.0f);
}

void DrumShaperCharacter::reset() noexcept
{
    envelope = 0.0;
    reductionState = 0.0;
    gainReductionDb.store(0.0f, std::memory_order_relaxed);
}

void DrumShaperCharacter::process(float* const* channels, int numChannels, int numSamples, CharacterStage stage) noexcept
{
    const bool doCompression = stage != CharacterStage::saturation, doSaturation = stage != CharacterStage::compression;
    const auto channelCount = std::clamp(numChannels, 0, 2);
    if (channelCount == 0 || numSamples <= 0)
        return;

    float blockReduction = 0.0f;
    const auto smoothing = 1.0f - std::exp(-1.0f / (0.010f * (float)rate));
    for (int n = 0; n < numSamples; ++n)
    {
        if (doCompression) compression += smoothing * (targetCompression - compression);
        if (doSaturation) drive += smoothing * (targetDrive - drive);

        double detector = 0.0;
        for (int ch = 0; ch < channelCount; ++ch)
            detector = std::max(detector, std::abs((double)channels[ch][n]));

        // Gain computer with a soft knee; returns dB of reduction.
        const auto reductionDb = [](double levelDb, double thresholdDb, double ratio, double kneeDb)
        {
            const auto slope = 1.0 - 1.0 / ratio;
            const auto over = levelDb - thresholdDb;
            if (2.0 * over < -kneeDb) return 0.0;
            if (2.0 * std::abs(over) <= kneeDb) return slope * (over + kneeDb * 0.5) * (over + kneeDb * 0.5) / (2.0 * kneeDb);
            return slope * over;
        };

        double compressorGain = 1.0;
        if (doCompression)
        {
            double attackCoeff = attackCoefficient, releaseCoeff = releaseCoefficient;
            if (type == CompressorType::vintage)
            {
                // Program-dependent release: sustained reduction recovers more slowly.
                const auto t = std::clamp(reductionState / 12.0, 0.0, 1.0);
                attackCoeff = vintageAttack;
                releaseCoeff = vintageReleaseFast + t * (vintageReleaseSlow - vintageReleaseFast);
            }
            else if (type == CompressorType::slam)
            {
                attackCoeff = slamAttack;
                releaseCoeff = slamRelease;
            }

            const auto envelopeCoefficient = detector > envelope ? attackCoeff : releaseCoeff;
            envelope = envelopeCoefficient * envelope + (1.0 - envelopeCoefficient) * detector;
            const auto inputDb = 20.0 * std::log10(std::max(envelope, 1.0e-9));

            // Every type is transparent at 0% and needs no makeup at that position.
            float reduction = 0.0f;
            double parallelBlend = 1.0;
            switch (type)
            {
                case CompressorType::transparent:
                {
                    // Unity ratio toward 8:1 above a fixed -18 dBFS threshold.
                    const auto ratio = 1.0 + 7.0 * (double)compression;
                    const auto overThreshold = std::max(0.0, inputDb + 18.0);
                    reduction = (float)(overThreshold * (1.0 - 1.0 / ratio));
                    compressorGain = std::pow(10.0, -(double)reduction / 20.0);
                    break;
                }
                case CompressorType::vintage:
                {
                    // FET-style: fast attack, ratio rising from 1:1 to 12:1 (about 4:1 at half way).
                    const auto ratio = 1.0 + 11.0 * (double)compression * (double)compression;
                    const auto db = reductionDb(inputDb, -20.0, ratio, 2.0);
                    reductionState = db;
                    reduction = (float)db;
                    compressorGain = std::pow(10.0, -db / 20.0);
                    break;
                }
                case CompressorType::slam:
                {
                    // Heavy 20:1 squash blended in parallel with the dry signal, with makeup on the squashed path.
                    const auto db = reductionDb(inputDb, -30.0, 20.0, 3.0);
                    const auto squashed = std::pow(10.0, (-db + 6.0) / 20.0);
                    parallelBlend = 0.75 * (double)compression;
                    compressorGain = (1.0 - parallelBlend) + parallelBlend * squashed;
                    reduction = (float)std::max(0.0, -20.0 * std::log10(std::max(compressorGain, 1.0e-6)));
                    break;
                }
            }
            blockReduction = std::max(blockReduction, reduction);
        }

        const auto driveGain = std::pow(10.0, (double)drive / 20.0);
        const auto saturationNorm = std::tanh(driveGain);
        for (int ch = 0; ch < channelCount; ++ch)
        {
            auto sample = (double)channels[ch][n] * compressorGain;
            if (doCompression && type == CompressorType::vintage && compression > 0.001f)
            {
                // Gentle FET-style colour that scales with the amount.
                const auto colour = 0.35 * (double)compression;
                sample = (1.0 - colour) * sample + colour * std::tanh(1.5 * sample) / 1.5;
            }
            if (doSaturation && drive > 0.001f)
                sample = std::tanh(sample * driveGain) / saturationNorm;
            channels[ch][n] = std::isfinite(sample) ? (float)sample : 0.0f;
        }
    }
    if (doCompression) gainReductionDb.store(blockReduction, std::memory_order_relaxed);
}
}
