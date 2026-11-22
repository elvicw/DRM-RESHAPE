#pragma once

#include <atomic>
#include <cmath>

namespace drumshaper
{
// Separate post-EQ character stage for the requested wideband compression and
// soft-saturation macros. It allocates no memory and is safe to call per block.
class DrumShaperCharacter
{
public:
    void prepare(double sampleRate) noexcept;
    void setParameters(float compressionPercent, float driveDb) noexcept;
    void reset() noexcept;
    void process(float* const* channels, int numChannels, int numSamples) noexcept;
    float getGainReductionDb() const noexcept { return gainReductionDb.load(std::memory_order_relaxed); }

private:
    double rate = 44100.0;
    double envelope = 0.0;
    double attackCoefficient = 0.0;
    double releaseCoefficient = 0.0;
    float compression = 0.0f, targetCompression = 0.0f;
    float drive = 0.0f, targetDrive = 0.0f;
    std::atomic<float> gainReductionDb { 0.0f };
};
}
