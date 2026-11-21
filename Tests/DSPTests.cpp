#include "DrumShaperDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>

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
    staticDsp.setSettings(staticSettings); dynamicDsp.setSettings(dynamicSettings);
    float* staticChannel[] { staticSignal.data() }; float* dynamicChannel[] { dynamicSignal.data() };
    staticDsp.process(staticChannel, 1, signalLength); dynamicDsp.process(dynamicChannel, 1, signalLength);
    double staticEnergy=0, dynamicEnergy=0;
    for (int n=4000; n<signalLength; ++n) { staticEnergy += staticSignal[(size_t)n]*staticSignal[(size_t)n]; dynamicEnergy += dynamicSignal[(size_t)n]*dynamicSignal[(size_t)n]; }
    assert(dynamicEnergy < staticEnergy * 0.5);

    std::array<float, 512> perfLeft {}, perfRight {};
    float* perfChannels[] { perfLeft.data(), perfRight.data() };
    const auto start=std::chrono::steady_clock::now();
    for (int block=0; block<2000; ++block) dsp.process(perfChannels, 2, 512);
    const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    assert(elapsed < 10.0);
    std::cout << "DSP response, dynamic reduction, stability, parameter updates, and 2000-block CPU check passed (" << elapsed << " s)\n";
}
