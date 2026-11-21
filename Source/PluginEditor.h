#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DRMRESHAPEAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DRMRESHAPEAudioProcessorEditor(DRMRESHAPEAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    void updateModeDefaults();
    DRMRESHAPEAudioProcessor& processor;
    juce::ComboBox mode;
    juce::TextButton advanced { "ADVANCED +" };
    juce::Label title, subtitle;
    std::array<juce::Slider, 6> gainControls, freqControls, qControls;
    std::array<juce::ToggleButton, 6> bandEnabled;
    std::array<juce::Label, 6> bandLabels;
    juce::Slider input, output, mix;
    juce::Label inputLabel, outputLabel, mixLabel;
    std::array<juce::Slider, 8> dynamicsControls;
    std::array<juce::Label, 8> dynamicsLabels;
    std::array<juce::ToggleButton, 2> dynamicsEnabled;
    juce::TextButton dynamicsPage { "DYNAMICS" };
    juce::ToggleButton globalBypass, outputCompensation;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    bool showAdvanced = false;
    bool showDynamics = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DRMRESHAPEAudioProcessorEditor)
};
