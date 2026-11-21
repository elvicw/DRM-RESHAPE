#include "DrumShaperDSP.h"
#include <algorithm>
#include <complex>

namespace drumshaper
{
DrumShaperDSP::Coefficients DrumShaperDSP::makeCoefficients(const BandSettings& s, double sr) noexcept
{
    const auto f = std::clamp<double>(s.frequency, 10.0, sr * 0.45);
    const auto q = std::clamp<double>(s.q, 0.1, 10.0);
    const auto A = std::pow(10.0, std::clamp<double>(s.gainDb, -24.0, 24.0) / 40.0);
    constexpr double pi = 3.14159265358979323846;
    const auto w = 2.0 * pi * f / sr;
    const auto c = std::cos(w), sn = std::sin(w);
    const auto alpha = sn / (2.0 * q);
    double b0, b1, b2, a0, a1, a2;
    if (s.shape == Shape::bell)
    {
        b0 = 1 + alpha * A; b1 = -2 * c; b2 = 1 - alpha * A;
        a0 = 1 + alpha / A; a1 = -2 * c; a2 = 1 - alpha / A;
    }
    else
    {
        const auto beta = 2.0 * std::sqrt(A) * alpha;
        if (s.shape == Shape::lowShelf)
        {
            b0=A*((A+1)-(A-1)*c+beta); b1=2*A*((A-1)-(A+1)*c); b2=A*((A+1)-(A-1)*c-beta);
            a0=(A+1)+(A-1)*c+beta; a1=-2*((A-1)+(A+1)*c); a2=(A+1)+(A-1)*c-beta;
        }
        else
        {
            b0=A*((A+1)+(A-1)*c+beta); b1=-2*A*((A-1)+(A+1)*c); b2=A*((A+1)+(A-1)*c-beta);
            a0=(A+1)-(A-1)*c+beta; a1=2*((A-1)-(A+1)*c); a2=(A+1)-(A-1)*c-beta;
        }
    }
    return { b0/a0, b1/a0, b2/a0, a1/a0, a2/a0 };
}

double DrumShaperDSP::run(const Coefficients& c, State& s, double x) noexcept
{
    const auto y = c.b0 * x + s.z1;
    s.z1 = c.b1 * x - c.a1 * y + s.z2;
    s.z2 = c.b2 * x - c.a2 * y;
    return std::isfinite(y) ? y : 0.0;
}

void DrumShaperDSP::prepare(double sampleRate, int, int) { rate = sampleRate; reset(); }
void DrumShaperDSP::reset() noexcept { for (auto& ch : states) for (auto& s : ch) s = {}; for (auto& ch : detectorStates) for (auto& s : ch) s = {}; for (auto& ch : detectorEnvelope) ch = {}; for (auto& ch : gainReduction) ch = {}; }

void DrumShaperDSP::setSettings(const Settings& s) noexcept
{
    dynamics[0] = s.tightDynamics; dynamics[1] = s.smoothDynamics;
    for (size_t i=0; i<6; ++i)
    {
        auto band = s.bands[i];
        if (band.frequency != currentBands[i].frequency || band.q != currentBands[i].q || band.gainDb != currentBands[i].gainDb || band.enabled != currentBands[i].enabled || band.shape != currentBands[i].shape)
        {
            coefficients[i] = makeCoefficients(band, rate);
            currentBands[i] = band;
        }
    }
    for (int d=0; d<2; ++d)
    {
        auto detectorBand = s.bands[(size_t)(d == 0 ? 3 : 4)];
        if (detectorBand.frequency == detectorFrequencyCache[(size_t)d] && detectorBand.q == detectorQCache[(size_t)d]) continue;
        detectorFrequencyCache[(size_t)d] = detectorBand.frequency;
        detectorQCache[(size_t)d] = detectorBand.q;
        const auto w = 2.0 * 3.14159265358979323846 * std::clamp<double>(detectorBand.frequency, 10.0, rate * 0.45) / rate;
        const auto alpha = std::sin(w) / (2.0 * std::clamp<double>(detectorBand.q, 0.1, 10.0));
        const auto a0 = 1.0 + alpha;
        const Coefficients bp { alpha/a0, 0.0, -alpha/a0, -2.0*std::cos(w)/a0, (1.0-alpha)/a0 };
        detectorCoefficients[0][d] = detectorCoefficients[1][d] = bp;
        dynamicCoefficients[0][d] = dynamicCoefficients[1][d] = coefficients[(size_t)(d == 0 ? 3 : 4)];
    }
}

void DrumShaperDSP::updateDynamicReduction(int band, float reductionDb) noexcept
{
    if (band < 0 || band >= 6) return;
    dynamicReduction[(size_t)band] = std::clamp(reductionDb, 0.0f, 24.0f);
}

void DrumShaperDSP::process(float* const* data, int numChannels, int numSamples) noexcept
{
    const auto channels = std::min(numChannels, 2);
    const auto attackAlpha = [this](float ms) { return std::exp(-1.0 / (rate * std::max(0.0001, (double)ms * 0.001))); };
    for (int n=0; n<numSamples; ++n)
    {
        for (int ch=0; ch<channels; ++ch)
        {
            const double input = data[ch][n];
            for (int d=0; d<2; ++d)
            {
                if (!dynamics[d].enabled) { gainReduction[(size_t)ch][d] = 0.0; continue; }
                const auto detector = run(detectorCoefficients[(size_t)ch][d], detectorStates[(size_t)ch][d], input);
                const auto absolute = std::abs(detector);
                const auto& cfg = dynamics[d];
                const auto coeff = attackAlpha(absolute > detectorEnvelope[(size_t)ch][d] ? cfg.attackMs : cfg.releaseMs);
                auto& env = detectorEnvelope[(size_t)ch][d]; env = coeff * env + (1.0 - coeff) * absolute;
                const auto level = 20.0 * std::log10(std::max(env, 1.0e-8));
                const auto target = std::clamp((level - cfg.thresholdDb) / 12.0, 0.0, 1.0) * cfg.maxReductionDb;
                auto& gr = gainReduction[(size_t)ch][d];
                const auto grCoeff = attackAlpha(target > gr ? cfg.attackMs : cfg.releaseMs);
                gr = grCoeff * gr + (1.0 - grCoeff) * target;
            }
            auto x = input;
            for (size_t b=0; b<6; ++b)
            {
                if (!currentBands[b].enabled) continue;
                if (b == 3 || b == 4)
                {
                    const int d = b == 3 ? 0 : 1;
                    if ((n & 15) == 0)
                    {
                        auto dynamicBand = currentBands[b];
                        dynamicBand.gainDb -= (float)gainReduction[(size_t)ch][d];
                        dynamicCoefficients[(size_t)ch][d] = makeCoefficients(dynamicBand, rate);
                    }
                    x = run(dynamicCoefficients[(size_t)ch][d], states[(size_t)ch][b], x);
                }
                else x = run(coefficients[b], states[(size_t)ch][b], x);
            }
            data[ch][n] = std::isfinite(x) ? (float)x : 0.0f;
        }
    }
}

float DrumShaperDSP::responseAt(const Settings& settings, double f, int count) noexcept
{
    double magnitude = 1.0;
    for (int i=0; i<std::min(count, 6); ++i)
    {
        auto s = settings.bands[(size_t)i];
        if (!s.enabled) continue;
        const auto c = makeCoefficients(s, settings.sampleRate);
        constexpr double pi = 3.14159265358979323846;
        const auto w = 2.0 * pi * f / settings.sampleRate;
        const auto z1 = std::polar(1.0, -w), z2 = z1*z1;
        const auto numerator = c.b0 + c.b1*z1 + c.b2*z2;
        const auto denominator = 1.0 + c.a1*z1 + c.a2*z2;
        magnitude *= std::abs(numerator / denominator);
    }
    return (float)(20.0 * std::log10(std::max(magnitude, 1.0e-9)));
}
}
