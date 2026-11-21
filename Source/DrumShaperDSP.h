#pragma once

#include <array>
#include <cmath>

namespace drumshaper
{
enum class Shape { lowShelf, bell, highShelf };

struct BandSettings
{
    Shape shape = Shape::bell;
    float frequency = 1000.0f;
    float q = 0.707f;
    float gainDb = 0.0f;
    bool enabled = true;
};

struct DynamicsSettings
{
    bool enabled = false;
    float thresholdDb = -24.0f;
    float attackMs = 10.0f;
    float releaseMs = 120.0f;
    float maxReductionDb = 6.0f;
};

struct Settings
{
    std::array<BandSettings, 6> bands;
    DynamicsSettings tightDynamics;
    DynamicsSettings smoothDynamics;
    double sampleRate = 44100.0;
};

class DrumShaperDSP
{
public:
    void prepare(double sampleRate, int maximumBlockSize, int channels);
    void reset() noexcept;
    void setSettings(const Settings& settings) noexcept;
    void process(float* const* channels, int numChannels, int numSamples) noexcept;
    void updateDynamicReduction(int band, float reductionDb) noexcept;
    static float responseAt(const Settings& settings, double frequency, int bands = 6) noexcept;

private:
    struct Coefficients { double b0=1, b1=0, b2=0, a1=0, a2=0; };
    struct State { double z1=0, z2=0; };
    static Coefficients makeCoefficients(const BandSettings&, double sampleRate) noexcept;
    static double run(const Coefficients&, State&, double input) noexcept;
    std::array<Coefficients, 6> coefficients{};
    std::array<std::array<State, 6>, 2> states{};
    std::array<BandSettings, 6> currentBands{};
    std::array<float, 6> dynamicReduction{};
    DynamicsSettings dynamics[2];
    std::array<std::array<Coefficients, 2>, 2> detectorCoefficients{};
    std::array<std::array<Coefficients, 2>, 2> dynamicCoefficients{};
    std::array<std::array<State, 2>, 2> detectorStates{};
    std::array<std::array<double, 2>, 2> detectorEnvelope{};
    std::array<std::array<double, 2>, 2> gainReduction{};
    std::array<float, 2> detectorFrequencyCache { -1.0f, -1.0f };
    std::array<float, 2> detectorQCache { -1.0f, -1.0f };
    double rate = 44100.0;
};
}
