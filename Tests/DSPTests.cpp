#include "DrumShaperDSP.h"
#include "DrumShaperCharacter.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>
#include <vector>

int main()
{
    using namespace drumshaper;
    Settings settings;
    settings.sampleRate = 48000;
    settings.bands[0] = { Shape::lowShelf, 80, 0.707f, 6, true };
    assert(DrumShaperDSP::responseAt(settings, 25) > 5.0f);
    assert(std::abs(DrumShaperDSP::responseAt(settings, 15000)) < 0.5f);
    settings.bands[0].enabled = false;
    assert(std::abs(DrumShaperDSP::responseAt(settings, 25)) < 0.01f);

    DrumShaperDSP dsp;
    dsp.prepare(48000, 256, 2);
    float left[256], right[256];
    for (int n=0; n<256; ++n) left[n]=right[n]=(n == 0 ? 1.0f : 0.0f);
    float* channels[] { left, right };
    settings.bands[0].enabled = true;
    dsp.setSettings(settings);
    dsp.process(channels, 2, 256);
    for (int n=0; n<256; ++n) assert(std::isfinite(left[n]) && std::abs(left[n]) < 4.0f);

    for (int i=0; i<1000; ++i)
    {
        settings.bands[1] = { Shape::bell, 50.0f + (i % 200) * 90.0f, 0.2f + (i % 20) * 0.2f, (i % 30)-15.0f, true };
        dsp.setSettings(settings);
        dsp.process(channels, 2, 256);
    }
    for (float x : left) assert(std::isfinite(x));

    constexpr int signalLength = 24000;
    std::array<float, signalLength> staticSignal {}, dynamicSignal {};
    for (int n=0; n<signalLength; ++n) staticSignal[(size_t)n] = dynamicSignal[(size_t)n] = 0.7f * std::sin(2.0 * 3.141592653589793 * 240.0 * n / 48000.0);
    Settings dynamicSettings;
    dynamicSettings.sampleRate = 48000;
    dynamicSettings.bands[3] = { Shape::bell, 240, 1.2f, 0, true };
    dynamicSettings.tightDynamics = { true, -40, 0.1f, 30, 12 };
    DrumShaperDSP staticDsp, dynamicDsp;
    staticDsp.prepare(48000, 256, 1); dynamicDsp.prepare(48000, 256, 1);
    auto staticSettings=dynamicSettings; staticSettings.tightDynamics.enabled=false;
    staticDsp.setSettings(staticSettings, signalLength); dynamicDsp.setSettings(dynamicSettings, signalLength);
    float* staticChannel[] { staticSignal.data() }; float* dynamicChannel[] { dynamicSignal.data() };
    staticDsp.process(staticChannel, 1, signalLength); dynamicDsp.process(dynamicChannel, 1, signalLength);
    double staticEnergy=0, dynamicEnergy=0;
    for (int n=4000; n<signalLength; ++n) { staticEnergy += staticSignal[(size_t)n]*staticSignal[(size_t)n]; dynamicEnergy += dynamicSignal[(size_t)n]*dynamicSignal[(size_t)n]; }
    assert(dynamicEnergy < staticEnergy * 0.5);

    constexpr int characterSamples = 48000;
    std::array<float, characterSamples> dryTone {}, compressedTone {}, drivenTone {};
    for (int n=0; n<characterSamples; ++n)
        dryTone[(size_t)n] = compressedTone[(size_t)n] = drivenTone[(size_t)n] = 0.75f * std::sin(2.0 * 3.141592653589793 * 90.0 * n / 48000.0);
    DrumShaperCharacter compressor;
    compressor.prepare(48000);
    compressor.setParameters(100.0f, 0.0f);
    float* compressedChannel[] { compressedTone.data() };
    compressor.process(compressedChannel, 1, characterSamples);
    double dryEnergy=0.0, compressedEnergy=0.0;
    for (int n=12000; n<characterSamples; ++n)
    {
        dryEnergy += (double)dryTone[(size_t)n] * dryTone[(size_t)n];
        compressedEnergy += (double)compressedTone[(size_t)n] * compressedTone[(size_t)n];
        assert(std::isfinite(compressedTone[(size_t)n]));
    }
    assert(compressedEnergy < dryEnergy * 0.35);
    assert(compressor.getGainReductionDb() > 8.0f);

    for (auto type : { CompressorType::transparent, CompressorType::vintage, CompressorType::slam })
    {
        std::vector<float> offTone(dryTone.begin(), dryTone.end()), onTone(dryTone.begin(), dryTone.end());
        DrumShaperCharacter off, on;
        off.prepare(48000); on.prepare(48000);
        off.setParameters(0.0f, 0.0f, type); on.setParameters(100.0f, 0.0f, type);
        float* offChannel[] { offTone.data() };
        float* onChannel[] { onTone.data() };
        off.process(offChannel, 1, characterSamples); on.process(onChannel, 1, characterSamples);
        double offError = 0.0, onEnergy = 0.0, dryTotal = 0.0;
        for (int n=12000; n<characterSamples; ++n)
        {
            offError = std::max(offError, std::abs((double)offTone[(size_t)n] - dryTone[(size_t)n]));
            onEnergy += (double)onTone[(size_t)n] * onTone[(size_t)n];
            dryTotal += (double)dryTone[(size_t)n] * dryTone[(size_t)n];
            assert(std::isfinite(onTone[(size_t)n]));
        }
        assert(offError < 1.0e-4);
        assert(on.getGainReductionDb() > 2.0f);
        assert(onEnergy != dryTotal);
    }

    // Running compression and saturation as separate stages matches the combined stage.
    std::vector<float> combinedTone(dryTone.begin(), dryTone.end()), splitTone(dryTone.begin(), dryTone.end());
    DrumShaperCharacter combined, split;
    combined.prepare(48000); split.prepare(48000);
    combined.setParameters(60.0f, 9.0f, CompressorType::vintage); split.setParameters(60.0f, 9.0f, CompressorType::vintage);
    float* combinedChannel[] { combinedTone.data() };
    float* splitChannel[] { splitTone.data() };
    combined.process(combinedChannel, 1, characterSamples);
    split.process(splitChannel, 1, characterSamples, CharacterStage::compression);
    split.process(splitChannel, 1, characterSamples, CharacterStage::saturation);
    for (int n=0; n<characterSamples; ++n) assert(std::abs(combinedTone[(size_t)n] - splitTone[(size_t)n]) < 1.0e-3f);

    DrumShaperCharacter saturator;
    saturator.prepare(48000);
    saturator.setParameters(0.0f, 18.0f);
    float* drivenChannel[] { drivenTone.data() };
    saturator.process(drivenChannel, 1, characterSamples);
    double nonlinearError=0.0;
    for (int n=12000; n<characterSamples; ++n)
    {
        nonlinearError += std::abs((double)drivenTone[(size_t)n] - dryTone[(size_t)n]);
        assert(std::isfinite(drivenTone[(size_t)n]));
    }
    assert(nonlinearError / (double)(characterSamples-12000) > 0.01);

    std::array<float, 512> perfLeft {}, perfRight {};
    float* perfChannels[] { perfLeft.data(), perfRight.data() };
    DrumShaperCharacter characterPerformance;
    characterPerformance.prepare(48000);
    characterPerformance.setParameters(40.0f, 6.0f);
    const auto start=std::chrono::steady_clock::now();
    for (int block=0; block<2000; ++block)
    {
        dsp.process(perfChannels, 2, 512);
        characterPerformance.process(perfChannels, 2, 512);
    }
    const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    assert(elapsed < 10.0);
    std::cout << "DSP response, dynamic reduction, compression, saturation, stability, parameter updates, and 2000-block CPU check passed (" << elapsed << " s)\n";
}
