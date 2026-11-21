#pragma once
#include <JuceHeader.h>
#include "DrumShaperDSP.h"

class DRMRESHAPEAudioProcessor final : public juce::AudioProcessor
{
public:
    DRMRESHAPEAudioProcessor();
    ~DRMRESHAPEAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DRM-RESHAPE"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    drumshaper::Settings getDspSettings() const noexcept;
    float getInputPeak() const noexcept { return inputPeak.load(); }
    float getOutputPeak() const noexcept { return outputPeak.load(); }
    float getSpectrum(bool post, int bin) const noexcept { return (post ? spectrumPost : spectrumPre)[(size_t)juce::jlimit(0, 1024, bin)].load(std::memory_order_relaxed); }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    using RawParameter = std::atomic<float>*;
    struct BandParameterRefs { RawParameter gain=nullptr, frequency=nullptr, q=nullptr, enabled=nullptr; };
    std::array<BandParameterRefs, 6> bandParameters;
    std::array<std::array<RawParameter, 5>, 2> dynamicParameters {};
    RawParameter inputParameter=nullptr, outputParameter=nullptr, mixParameter=nullptr;
    RawParameter compensationEnabledParameter=nullptr, compensationParameter=nullptr, bypassParameter=nullptr;
    static float read(RawParameter p) noexcept { return p != nullptr ? p->load(std::memory_order_relaxed) : 0.0f; }
    drumshaper::DrumShaperDSP dsp;
    juce::AudioBuffer<float> dryBuffer;
    juce::dsp::FFT analyzerFFT { 11 };
    std::array<float, 2048> analyzerInput {}, analyzerOutput {}, analyzerWindow {};
    std::array<float, 4096> fftScratch {};
    std::array<std::atomic<float>, 1025> spectrumPre {}, spectrumPost {};
    int analyzerPosition = 0;
    juce::SmoothedValue<float> inputGain, outputGain, wetMix;
    std::atomic<float> inputPeak { 0.0f }, outputPeak { 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DRMRESHAPEAudioProcessor)
};
