#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
const juce::StringArray bandNames { "LOW", "MID", "HIGH", "TIGHT", "SMOOTH", "AIR" };
juce::AudioParameterFloat* floatParam(const juce::String& id, const juce::String& name, float lo, float hi, float def, const juce::String& unit = {})
{
    return new juce::AudioParameterFloat(juce::ParameterID { id, 1 }, name, juce::NormalisableRange<float>(lo, hi), def, unit);
}
}

juce::AudioProcessorValueTreeState::ParameterLayout DRMRESHAPEAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterChoice>(ParameterID { "mode", 1 }, "Instrument", StringArray { "Kick", "Snare", "Toms", "Overheads", "Room", "Full Kit" }, 0));
    const float freqDefaults[] { 65, 800, 7000, 280, 4800, 14000 };
    const float gainRanges[] { 15, 15, 15, 12, 12, 15 };
    for (int i=0; i<6; ++i)
    {
        auto id = bandNames[i].toLowerCase();
        p.emplace_back(floatParam(id + "Gain", bandNames[i] + " Gain", -gainRanges[i], gainRanges[i], 0, " dB"));
        p.emplace_back(floatParam(id + "Freq", bandNames[i] + " Frequency", 20, 20000, freqDefaults[i], " Hz"));
        p.emplace_back(floatParam(id + "Q", bandNames[i] + " Q", 0.2f, 5.0f, i == 3 || i == 4 ? 1.2f : 0.707f));
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID { id + "On", 1 }, bandNames[i] + " Enabled", true));
    }
    p.emplace_back(floatParam("input", "Input", -24, 24, 0, " dB"));
    p.emplace_back(floatParam("output", "Output", -24, 24, 0, " dB"));
    p.emplace_back(floatParam("mix", "Mix", 0, 100, 100, "%"));
    p.push_back(std::make_unique<AudioParameterBool>(ParameterID { "comp", 1 }, "Output Compensation", false));
    p.push_back(std::make_unique<AudioParameterBool>(ParameterID { "bypass", 1 }, "Global Bypass", false));
    p.emplace_back(floatParam("compensation", "Compensation", -12, 12, 0, " dB"));
    for (auto name : { String("tight"), String("smooth") })
    {
        p.push_back(std::make_unique<AudioParameterBool>(ParameterID { name + "Dyn", 1 }, name + " Dynamic", false));
        p.emplace_back(floatParam(name + "Threshold", name + " Threshold", -60, 0, -24, " dB"));
        p.emplace_back(floatParam(name + "Attack", name + " Attack", 0.1f, 100, 10, " ms"));
        p.emplace_back(floatParam(name + "Release", name + " Release", 10, 1000, 120, " ms"));
        p.emplace_back(floatParam(name + "MaxGR", name + " Maximum Reduction", 0, 18, 6, " dB"));
    }
    return { p.begin(), p.end() };
}

DRMRESHAPEAudioProcessor::DRMRESHAPEAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true).withOutput("Output", AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

float DRMRESHAPEAudioProcessor::value(const juce::AudioProcessorValueTreeState& s, const char* id) noexcept
{
    if (auto* p = s.getRawParameterValue(id)) return p->load(std::memory_order_relaxed);
    return 0.0f;
}

drumshaper::Settings DRMRESHAPEAudioProcessor::getDspSettings() const noexcept
{
    using namespace drumshaper;
    Settings s;
    s.sampleRate = getSampleRate() > 0 ? getSampleRate() : 44100.0;
    constexpr Shape shapes[] { Shape::lowShelf, Shape::bell, Shape::highShelf, Shape::bell, Shape::bell, Shape::highShelf };
    for (int i=0; i<6; ++i)
    {
        auto id = bandNames[i].toLowerCase();
        s.bands[(size_t)i] = { shapes[i], value(parameters, (id + "Freq").toRawUTF8()), value(parameters, (id + "Q").toRawUTF8()), value(parameters, (id + "Gain").toRawUTF8()), value(parameters, (id + "On").toRawUTF8()) > 0.5f };
    }
    auto setup = [this](const juce::String& name) {
        return DynamicsSettings { value(parameters, (name+"Dyn").toRawUTF8()) > 0.5f,
            value(parameters, (name+"Threshold").toRawUTF8()), value(parameters, (name+"Attack").toRawUTF8()),
            value(parameters, (name+"Release").toRawUTF8()), value(parameters, (name+"MaxGR").toRawUTF8()) };
    };
    s.tightDynamics = setup("tight"); s.smoothDynamics = setup("smooth");
    return s;
}

void DRMRESHAPEAudioProcessor::prepareToPlay(double sr, int block)
{
    dsp.prepare(sr, block, getTotalNumOutputChannels());
    dryBuffer.setSize(getTotalNumOutputChannels(), block, false, false, true);
    analyzerPosition = 0;
    for (size_t i=0; i<analyzerWindow.size(); ++i)
        analyzerWindow[i] = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * (float)i / (float)(analyzerWindow.size()-1));
    for (auto& bin : spectrumPre) bin.store(0.0f);
    for (auto& bin : spectrumPost) bin.store(0.0f);
    inputGain.reset(sr, 0.02); outputGain.reset(sr, 0.02); wetMix.reset(sr, 0.02);
    inputGain.setCurrentAndTargetValue(1); outputGain.setCurrentAndTargetValue(1); wetMix.setCurrentAndTargetValue(1);
}

bool DRMRESHAPEAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == AudioChannelSet::mono() || out == AudioChannelSet::stereo()) && out == layouts.getMainInputChannelSet();
}

void DRMRESHAPEAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto channels = getTotalNumInputChannels(), samples = buffer.getNumSamples();
    for (int ch=getTotalNumOutputChannels(); ch<buffer.getNumChannels(); ++ch) buffer.clear(ch, 0, samples);
    auto settings = getDspSettings(); dsp.setSettings(settings);
    inputGain.setTargetValue(juce::Decibels::decibelsToGain(value(parameters, "input")));
    auto outputDb = value(parameters, "output") + (value(parameters, "comp") > 0.5f ? value(parameters, "compensation") : 0.0f);
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(outputDb));
    wetMix.setTargetValue(value(parameters, "mix") / 100.0f);
    const bool bypass = value(parameters, "bypass") > 0.5f;
    float inPeak=0, outPeak=0;
    const bool canMix = dryBuffer.getNumSamples() >= samples && dryBuffer.getNumChannels() >= channels;
    for (int n=0; n<samples; ++n)
    {
        const auto gain = inputGain.getNextValue();
        for (int ch=0; ch<channels; ++ch)
        {
            auto* x = buffer.getWritePointer(ch);
            inPeak = std::max(inPeak, std::abs(x[n]));
            x[n] *= gain;
            if (canMix) dryBuffer.getWritePointer(ch)[n] = x[n];
        }
    }
    if (!bypass)
        dsp.process(buffer.getArrayOfWritePointers(), channels, samples);
    for (int n=0; n<samples; ++n)
    {
        const auto mix = wetMix.getNextValue();
        const auto gain = outputGain.getNextValue();
        for (int ch=0; ch<channels; ++ch)
        {
            auto* x=buffer.getWritePointer(ch);
            if (canMix && !bypass) x[n] = dryBuffer.getReadPointer(ch)[n] + mix * (x[n] - dryBuffer.getReadPointer(ch)[n]);
            x[n] *= gain; outPeak=std::max(outPeak, std::abs(x[n]));
        }
    }
    if (canMix)
        for (int n=0; n<samples; ++n)
        {
            float pre=0.0f, post=0.0f;
            for (int ch=0; ch<channels; ++ch) { pre += dryBuffer.getReadPointer(ch)[n]; post += buffer.getReadPointer(ch)[n]; }
            analyzerInput[(size_t)analyzerPosition] = pre / (float)channels;
            analyzerOutput[(size_t)analyzerPosition] = post / (float)channels;
            if (++analyzerPosition == 2048)
            {
                auto publish = [this](const std::array<float, 2048>& signal, std::array<std::atomic<float>, 1025>& destination) {
                    for (size_t i=0; i<signal.size(); ++i) { fftScratch[i] = signal[i] * analyzerWindow[i]; fftScratch[i+2048] = 0.0f; }
                    analyzerFFT.performFrequencyOnlyForwardTransform(fftScratch.data());
                    for (int b=0; b<=1024; ++b) destination[(size_t)b].store(fftScratch[(size_t)b] / 512.0f, std::memory_order_relaxed);
                };
                publish(analyzerInput, spectrumPre); publish(analyzerOutput, spectrumPost); analyzerPosition=0;
            }
        }
    inputPeak.store(inPeak, std::memory_order_relaxed); outputPeak.store(outPeak, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* DRMRESHAPEAudioProcessor::createEditor() { return new DRMRESHAPEAudioProcessorEditor(*this); }
void DRMRESHAPEAudioProcessor::getStateInformation(juce::MemoryBlock& dest) { auto state=parameters.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, dest); }
void DRMRESHAPEAudioProcessor::setStateInformation(const void* data, int size) { std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size)); if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DRMRESHAPEAudioProcessor(); }
