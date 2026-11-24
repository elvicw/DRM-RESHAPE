#pragma once

#include <atomic>
#include <cmath>

namespace drumshaper
{
enum class CompressorType { transparent, vintage, slam };
// Lets the host place compression and saturation at different points in the chain.
enum class CharacterStage { both, compression, saturation };

// Separate post-EQ character stage for the requested wideband compression and
// soft-saturation macros. It allocates no memory and is safe to call per block.
class DrumShaperCharacter
{
public:
    void prepare(double sampleRate) noexcept;
    void setParameters(float compressionPercent, float driveDb, CompressorType type = CompressorType::transparent) noexcept;
    void reset() noexcept;
    void process(float* const* channels, int numChannels, int numSamples, CharacterStage stage = CharacterStage::both) noexcept;
    float getGainReductionDb() const noexcept { return gainReductionDb.load(std::memory_order_relaxed); }

private:
    double rate = 44100.0;
    double envelope = 0.0;
    double attackCoefficient = 0.0;
    double releaseCoefficient = 0.0;
    double vintageAttack = 0.0, vintageReleaseFast = 0.0, vintageReleaseSlow = 0.0;
    double slamAttack = 0.0, slamRelease = 0.0;
    float compression = 0.0f, targetCompression = 0.0f;
    float drive = 0.0f, targetDrive = 0.0f;
    CompressorType type = CompressorType::transparent;
    double reductionState = 0.0;
    std::atomic<float> gainReductionDb { 0.0f };
};
}
