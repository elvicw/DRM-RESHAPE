#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Illuminated push button used for LEVEL COMP; the label is part of the button assembly.
class IlluminatedButton final : public juce::Button
{
public:
    explicit IlluminatedButton(const juce::String& name) : juce::Button(name) { setClickingTogglesState(true); }
    void paintButton(juce::Graphics&, bool isMouseOver, bool isButtonDown) override;
};

class DRMRESHAPEAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DRMRESHAPEAudioProcessorEditor(DRMRESHAPEAudioProcessor&);
    ~DRMRESHAPEAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    void timerCallback() override;
    void updateModeDefaults();
    void applyAdvancedState();
    void rebuildChassis(float pixelScale);
    float refHeight() const noexcept;
    void drawStaticChassis(juce::Graphics&) const;
    void drawAnalyzer(juce::Graphics&) const;
    void drawMeters(juce::Graphics&) const;

    DRMRESHAPEAudioProcessor& audioProcessor;
    std::unique_ptr<juce::LookAndFeel> hardwareLookAndFeel;
    juce::ComboBox mode;
    juce::TextButton advanced { "ADVANCED +" };
    std::array<juce::Slider, 6> gainControls, freqControls, qControls;
    juce::Slider input, output, mix, compression, drive;
    std::array<juce::Slider, 8> dynamicsControls;
    std::array<juce::ToggleButton, 2> dynamicsEnabled;
    juce::TextButton dynamicsPage { "DYNAMICS" };
    IlluminatedButton levelComp { "LEVEL COMP" };
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    juce::ComponentBoundsConstrainer constrainer;
    juce::Image chassis;
    float chassisScale = 0.0f;
    int chassisWidth = 0, chassisHeight = 0;
    bool showAdvanced = false;
    bool showDynamics = false;
    bool showPre = true, showPost = true, showResponse = true;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DRMRESHAPEAudioProcessorEditor)
};
