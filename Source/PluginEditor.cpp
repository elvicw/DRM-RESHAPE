#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace
{
const juce::StringArray labels { "LOW", "MID", "HIGH", "TIGHT", "SMOOTH", "AIR" };
const juce::StringArray ids { "low", "mid", "high", "tight", "smooth", "air" };
const juce::Colour bg { 0xff202124 }, panel { 0xff292b2d }, faceplateLight { 0xffd8d0bd }, display { 0xff111315 }, text { 0xffd8d0bd }, titleText { 0xffe5dfd1 }, muted { 0xff85847c }, accent { 0xffd69a38 }, teal { 0xff547a91 }, green { 0xff82916b }, steel { 0xff73777a }, grid { 0xff34383a };

class HardwareLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    HardwareLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxBackgroundColourId, display);
        setColour(juce::Slider::textBoxOutlineColourId, steel.withAlpha(0.65f));
        setColour(juce::ToggleButton::textColourId, text);
        setColour(juce::ComboBox::textColourId, text);
        setColour(juce::ComboBox::backgroundColourId, display);
        setColour(juce::ComboBox::outlineColourId, steel);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float position, float startAngle, float endAngle, juce::Slider& slider) override
    {
        const auto size = (float)juce::jmin(width, height);
        const auto cx = (float)x + (float)width * 0.5f;
        const auto cy = (float)y + (float)height * 0.5f;
        const auto radius = size * 0.34f;
        const auto angle = startAngle + position * (endAngle - startAngle);
        const auto style = (int)slider.getProperties().getWithDefault("hardwareStyle", 0);
        const auto highlight = slider.isMouseOverOrDragging() ? juce::Colour(0xffe6b45c) : accent;

        // Engraved scale on the faceplate, with major marks at sensible intervals.
        g.setColour(steel.withAlpha(0.68f));
        constexpr int tickCount = 21;
        for (int tick = 0; tick < tickCount; ++tick)
        {
            const auto t = (float)tick / (float)(tickCount - 1);
            const auto a = startAngle + t * (endAngle - startAngle);
            const auto major = tick % 5 == 0;
            const auto outer = radius + size * 0.115f;
            const auto inner = outer - size * (major ? 0.075f : 0.042f);
            g.drawLine(cx + inner * std::cos(a), cy + inner * std::sin(a),
                       cx + outer * std::cos(a), cy + outer * std::sin(a), major ? 1.15f : 0.7f);
        }

        // Recess shadow and steel mounting collar.
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.fillEllipse(cx - radius - 1.5f, cy - radius + size * 0.035f, (radius + 1.5f) * 2.0f, (radius + 1.5f) * 2.0f);
        juce::ColourGradient collar(juce::Colour(0xff9b9da0), cx - radius, cy - radius,
                                    juce::Colour(0xff45494b), cx + radius, cy + radius, false);
        g.setGradientFill(collar);
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colour(0xff111315));
        g.fillEllipse(cx - radius * 0.91f, cy - radius * 0.91f, radius * 1.82f, radius * 1.82f);

        const auto bodyRadius = radius * (style == 2 ? 0.74f : 0.80f);
        const auto body = juce::Rectangle<float>(cx - bodyRadius, cy - bodyRadius * 0.91f,
                                                 bodyRadius * 2.0f, bodyRadius * 1.82f);
        // Lower body makes the control read as a cylinder rather than a flat disk.
        juce::ColourGradient side(juce::Colour(0xff53565a), cx, body.getY(),
                                  juce::Colour(0xff090a0b), cx, body.getBottom(), false);
        g.setGradientFill(side);
        g.fillEllipse(body.translated(0.0f, size * 0.045f));
        g.setColour(juce::Colour(0xff070809));
        g.fillEllipse(body);

        if (style == 1)
        {
            g.setColour(juce::Colour(0xff626568).withAlpha(0.72f));
            const auto ribRadius = bodyRadius * 0.94f;
            for (int rib = 0; rib < 28; ++rib)
            {
                const auto a = juce::MathConstants<float>::twoPi * (float)rib / 28.0f;
                const auto px = cx + ribRadius * std::cos(a);
                const auto py = cy + ribRadius * 0.78f * std::sin(a);
                g.drawLine(px, py, px + 1.15f * std::cos(a), py + 1.15f * std::sin(a), 0.85f);
            }
        }

        juce::ColourGradient cap(style == 2 ? juce::Colour(0xffb8bab8) : juce::Colour(0xff343638),
                                 cx - bodyRadius * 0.65f, cy - bodyRadius * 0.72f,
                                 style == 2 ? juce::Colour(0xff555a5c) : juce::Colour(0xff151718),
                                 cx + bodyRadius * 0.85f, cy + bodyRadius * 0.82f, false);
        g.setGradientFill(cap);
        g.fillEllipse(cx - bodyRadius * 0.86f, cy - bodyRadius * 0.78f,
                      bodyRadius * 1.72f, bodyRadius * 1.56f);
        g.setColour(juce::Colour(0xffaeb0a9).withAlpha(0.28f));
        juce::Path reflection;
        reflection.addArc(cx - bodyRadius * 0.74f, cy - bodyRadius * 0.68f,
                          bodyRadius * 1.48f, bodyRadius * 1.36f,
                          juce::MathConstants<float>::pi * 1.15f, juce::MathConstants<float>::pi * 1.82f, true);
        g.strokePath(reflection, juce::PathStrokeType(1.2f));

        // Value pointer is drawn last and always follows the attached slider value.
        const auto pointerLength = bodyRadius * 0.70f;
        const auto px = cx + pointerLength * std::sin(angle);
        const auto py = cy - pointerLength * std::cos(angle);
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.drawLine(cx + 1.0f, cy + 1.5f, px + 1.0f, py + 1.5f, size * 0.035f);
        g.setColour(style == 2 ? juce::Colour(0xfff0e5c8) : juce::Colour(0xffe8dfc8));
        g.drawLine(cx, cy, px, py, size * 0.033f);
        g.setColour(highlight);
        g.fillEllipse(cx - size * 0.027f, cy - size * 0.027f, size * 0.054f, size * 0.054f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto labelArea = bounds.removeFromRight(juce::jmax(48.0f, bounds.getWidth() * 0.58f));
        const auto housing = juce::Rectangle<float>(bounds.getX() + 2.0f, bounds.getCentreY() - 9.0f,
                                                     juce::jmin(31.0f, bounds.getWidth() - 4.0f), 18.0f);
        g.setColour(juce::Colours::black.withAlpha(0.55f)); g.fillRoundedRectangle(housing.translated(0, 1.5f), 3.0f);
        g.setColour(juce::Colour(0xff55595b)); g.fillRoundedRectangle(housing, 3.0f);
        g.setColour(juce::Colour(0xff121416)); g.fillRoundedRectangle(housing.reduced(2.0f), 2.0f);
        const auto on = button.getToggleState();
        auto lever = housing.reduced(3.0f).withWidth((housing.getWidth() - 6.0f) * 0.5f);
        if (on) lever.setX(housing.getRight() - lever.getWidth() - 3.0f);
        g.setColour(on ? juce::Colour(0xffc6c0b2) : juce::Colour(0xff74777a)); g.fillRoundedRectangle(lever, 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.18f)); g.drawLine(lever.getX()+2, lever.getY()+1, lever.getRight()-2, lever.getY()+1, 0.8f);
        if (on) { g.setColour(accent); g.fillEllipse(housing.getRight()+3.0f, housing.getCentreY()-2.0f, 4.0f, 4.0f); }
        g.setColour(button.findColour(juce::ToggleButton::textColourId));
        g.setFont(juce::Font(10.5f, juce::Font::bold));
        g.drawText(button.getButtonText(), labelArea.toNearestInt(), juce::Justification::centredLeft, false);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool over, bool down) override
    {
        auto r = button.getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(juce::Colours::black.withAlpha(0.45f)); g.fillRoundedRectangle(r.translated(0, 2.0f), 3.0f);
        juce::ColourGradient metal(juce::Colour(0xff45484a), r.getX(), r.getY(), juce::Colour(0xff252729), r.getX(), r.getBottom(), false);
        g.setGradientFill(metal); g.fillRoundedRectangle(r, 3.0f);
        g.setColour(steel); g.drawRoundedRectangle(r, 3.0f, 1.0f);
        if (over || down) { g.setColour(accent.withAlpha(down ? 0.46f : 0.22f)); g.drawRoundedRectangle(r.reduced(1.5f), 2.0f, 1.0f); }
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool over, int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float>(0, 0, (float)w, (float)h).reduced(1.0f);
        g.setColour(juce::Colours::black.withAlpha(0.45f)); g.fillRect(r.translated(0, 1.0f));
        g.setColour(juce::Colour(0xff111315)); g.fillRect(r);
        g.setColour(over ? accent : steel); g.drawRect(r, 1.0f);
        const auto cx=(float)w-15.0f, cy=(float)h*0.5f;
        g.setColour(box.findColour(juce::ComboBox::textColourId));
        juce::Path arrow; arrow.startNewSubPath(cx-4,cy-2); arrow.lineTo(cx,cy+2); arrow.lineTo(cx+4,cy-2);
        g.strokePath(arrow, juce::PathStrokeType(1.5f));
    }
};

void setupSlider(juce::Slider& s, juce::Colour colour, bool compact=false)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, compact ? 62 : 70, 18);
    s.setColour(juce::Slider::rotarySliderFillColourId, colour);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff514e47));
    s.setColour(juce::Slider::thumbColourId, accent);
    s.setColour(juce::Slider::textBoxTextColourId, text);
    s.setColour(juce::Slider::textBoxBackgroundColourId, display);
    s.setColour(juce::Slider::textBoxOutlineColourId, steel);
    s.setColour(juce::Slider::textBoxHighlightColourId, accent);
    s.setNumDecimalPlacesToDisplay(1);
}

void drawHardwareMeter(juce::Graphics& g, float x, float y, float width, float height,
                       const juce::String& name, float reading, float minimum, float maximum)
{
    const auto bounds = juce::Rectangle<float>(x, y, width, height);
    g.setColour(juce::Colour(0xff77746a)); g.fillRoundedRectangle(bounds.expanded(1.5f), 5.0f);
    g.setColour(juce::Colour(0xffddd5bd)); g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xff706b5c)); g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    g.setColour(juce::Colour(0xff3c3c35)); g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.drawText(name, (int)x + 5, (int)y + 2, (int)width - 10, 12, juce::Justification::centred);
    const float cx = x + width * 0.5f, cy = y + height - 4.0f, radius = width * 0.43f;
    for (int i=0; i<=12; ++i)
    {
        const float angle = juce::MathConstants<float>::pi + juce::MathConstants<float>::pi * (float)i / 12.0f;
        const float inner = radius - (i % 3 == 0 ? 7.0f : 4.0f);
        g.drawLine(cx + inner * std::cos(angle), cy + inner * std::sin(angle),
                   cx + radius * std::cos(angle), cy + radius * std::sin(angle), i % 3 == 0 ? 1.0f : 0.6f);
    }
    const float normalized = juce::jlimit(0.0f, 1.0f, (reading - minimum) / (maximum - minimum));
    const float angle = juce::MathConstants<float>::pi + normalized * juce::MathConstants<float>::pi;
    g.setColour(juce::Colour(0xffa44b36));
    g.drawLine(cx, cy, cx + (radius - 9.0f) * std::cos(angle), cy + (radius - 9.0f) * std::sin(angle), 1.6f);
    g.fillEllipse(cx - 2.5f, cy - 2.5f, 5.0f, 5.0f);
}
}

DRMRESHAPEAudioProcessorEditor::DRMRESHAPEAudioProcessorEditor(DRMRESHAPEAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), hardwareLookAndFeel(std::make_unique<HardwareLookAndFeel>())
{
    setLookAndFeel(hardwareLookAndFeel.get());
    setSize(1120, 820);
    auto setDefaultOnDoubleClick = [this](juce::Slider& slider, const juce::String& parameterID)
    {
        if (const auto* parameter = audioProcessor.parameters.getParameter(parameterID))
        {
            const auto value = parameter->getNormalisableRange().convertFrom0to1(parameter->getDefaultValue());
            slider.setDoubleClickReturnValue(true, value);
            slider.setScrollWheelEnabled(true);
        }
    };
    title.setText("DRM-RESHAPE", juce::dontSendNotification); title.setFont(juce::Font(26.0f, juce::Font::bold)); title.setColour(juce::Label::textColourId, titleText); addAndMakeVisible(title);
    subtitle.setText("DRUM TONE SHAPER   /   YEAHWAAH AUDIO", juce::dontSendNotification); subtitle.setFont(juce::Font(10.0f, juce::Font::bold)); subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffb8b3a7)); addAndMakeVisible(subtitle);
    mode.addItemList({ "KICK", "SNARE", "TOMS", "OVERHEADS", "ROOM", "FULL KIT" }, 1); mode.setColour(juce::ComboBox::backgroundColourId, display); mode.setColour(juce::ComboBox::textColourId, text); mode.setColour(juce::ComboBox::outlineColourId, steel); addAndMakeVisible(mode);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "mode", mode);
    mode.onChange = [this] { updateModeDefaults(); };
    advanced.setColour(juce::TextButton::buttonColourId, panel); advanced.setColour(juce::TextButton::textColourOffId, text); advanced.onClick = [this] { showAdvanced = !showAdvanced; advanced.setButtonText(showAdvanced ? "ADVANCED -" : "ADVANCED +"); resized(); repaint(); }; addAndMakeVisible(advanced);
    dynamicsPage.setColour(juce::TextButton::buttonColourId, panel); dynamicsPage.setColour(juce::TextButton::textColourOffId, teal); dynamicsPage.onClick = [this] { showDynamics = !showDynamics; dynamicsPage.setButtonText(showDynamics ? "BANDS" : "DYNAMICS"); resized(); repaint(); }; addAndMakeVisible(dynamicsPage); dynamicsPage.setVisible(false);
    for (int i=0; i<6; ++i)
    {
        bandLabels[(size_t)i].setText(labels[i], juce::dontSendNotification); bandLabels[(size_t)i].setJustificationType(juce::Justification::centred); bandLabels[(size_t)i].setFont(juce::Font(13, juce::Font::bold)); bandLabels[(size_t)i].setColour(juce::Label::textColourId, i == 3 || i == 4 ? teal : text); addAndMakeVisible(bandLabels[(size_t)i]);
        bandEnabled[(size_t)i].setButtonText("ON"); bandEnabled[(size_t)i].setColour(juce::ToggleButton::textColourId, muted); addAndMakeVisible(bandEnabled[(size_t)i]);
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, ids[i] + "On", bandEnabled[(size_t)i]));
        setupSlider(gainControls[(size_t)i], i == 3 || i == 4 ? teal : accent); gainControls[(size_t)i].getProperties().set("hardwareStyle", 0); gainControls[(size_t)i].setNumDecimalPlacesToDisplay(1); gainControls[(size_t)i].setTextValueSuffix(" dB"); addAndMakeVisible(gainControls[(size_t)i]); gainControls[(size_t)i].setLookAndFeel(hardwareLookAndFeel.get());
        auto stem = ids[i];
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, stem + "Gain", gainControls[(size_t)i]));
        setDefaultOnDoubleClick(gainControls[(size_t)i], stem + "Gain");
        setupSlider(freqControls[(size_t)i], accent, true); freqControls[(size_t)i].getProperties().set("hardwareStyle", 2); freqControls[(size_t)i].setNumDecimalPlacesToDisplay(0); freqControls[(size_t)i].setTextValueSuffix(" Hz"); addAndMakeVisible(freqControls[(size_t)i]); freqControls[(size_t)i].setLookAndFeel(hardwareLookAndFeel.get());
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, stem + "Freq", freqControls[(size_t)i]));
        setDefaultOnDoubleClick(freqControls[(size_t)i], stem + "Freq");
        setupSlider(qControls[(size_t)i], teal, true); qControls[(size_t)i].getProperties().set("hardwareStyle", 1); qControls[(size_t)i].setNumDecimalPlacesToDisplay(2); addAndMakeVisible(qControls[(size_t)i]); qControls[(size_t)i].setLookAndFeel(hardwareLookAndFeel.get());
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, stem + "Q", qControls[(size_t)i]));
        setDefaultOnDoubleClick(qControls[(size_t)i], stem + "Q");
    }
    auto setupUtility = [this, &setDefaultOnDoubleClick](juce::Slider& s, juce::Label& l, const juce::String& name, const juce::String& id, const juce::String& suffix) {
        setupSlider(s, accent); s.getProperties().set("hardwareStyle", 2); s.setTextValueSuffix(suffix); addAndMakeVisible(s); s.setLookAndFeel(hardwareLookAndFeel.get()); l.setText(name, juce::dontSendNotification); l.setJustificationType(juce::Justification::centred); l.setColour(juce::Label::textColourId, muted); addAndMakeVisible(l);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, id, s));
        setDefaultOnDoubleClick(s, id);
    };
    setupUtility(input, inputLabel, "INPUT", "input", " dB"); setupUtility(output, outputLabel, "OUTPUT", "output", " dB"); setupUtility(mix, mixLabel, "MIX", "mix", "%");
    setupUtility(compression, compressionLabel, "COMPRESSION", "compression", "%");
    setupUtility(drive, driveLabel, "DRIVE", "drive", " dB");
    compression.getProperties().set("hardwareStyle", 0);
    drive.getProperties().set("hardwareStyle", 2);
    for (int i=0; i<8; ++i)
    {
        const int group=i/4, control=i%4;
        const juce::String prefix=group==0 ? "TIGHT" : "SMOOTH";
        const juce::String key=prefix.toLowerCase();
        const juce::StringArray suffixes { "Threshold", "Attack", "Release", "MaxGR" };
        const juce::StringArray units { " dB", " ms", " ms", " dB" };
        dynamicsLabels[(size_t)i].setText(prefix + " " + suffixes[control].toUpperCase(), juce::dontSendNotification); dynamicsLabels[(size_t)i].setJustificationType(juce::Justification::centred); dynamicsLabels[(size_t)i].setColour(juce::Label::textColourId, muted); addAndMakeVisible(dynamicsLabels[(size_t)i]);
        setupSlider(dynamicsControls[(size_t)i], teal, true); dynamicsControls[(size_t)i].getProperties().set("hardwareStyle", 1); dynamicsControls[(size_t)i].setNumDecimalPlacesToDisplay(1); dynamicsControls[(size_t)i].setTextValueSuffix(units[control]); addAndMakeVisible(dynamicsControls[(size_t)i]); dynamicsControls[(size_t)i].setLookAndFeel(hardwareLookAndFeel.get());
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, key + suffixes[control], dynamicsControls[(size_t)i]));
        setDefaultOnDoubleClick(dynamicsControls[(size_t)i], key + suffixes[control]);
    }
    for (int i=0; i<2; ++i)
    {
        const auto key=i==0 ? "tightDyn" : "smoothDyn";
        dynamicsEnabled[(size_t)i].setButtonText(i==0 ? "TIGHT DYNAMIC" : "SMOOTH DYNAMIC"); dynamicsEnabled[(size_t)i].setColour(juce::ToggleButton::textColourId, teal); addAndMakeVisible(dynamicsEnabled[(size_t)i]);
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, key, dynamicsEnabled[(size_t)i]));
    }
    globalBypass.setButtonText("BYPASS"); globalBypass.setColour(juce::ToggleButton::textColourId, text); addAndMakeVisible(globalBypass);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "bypass", globalBypass));
    outputCompensation.setButtonText("LEVEL COMP"); outputCompensation.setColour(juce::ToggleButton::textColourId, muted); addAndMakeVisible(outputCompensation);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "comp", outputCompensation));
    resized();
    startTimerHz(24);
}

DRMRESHAPEAudioProcessorEditor::~DRMRESHAPEAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    for (auto& slider : gainControls) slider.setLookAndFeel(nullptr);
    for (auto& slider : freqControls) slider.setLookAndFeel(nullptr);
    for (auto& slider : qControls) slider.setLookAndFeel(nullptr);
    for (auto& slider : dynamicsControls) slider.setLookAndFeel(nullptr);
    input.setLookAndFeel(nullptr); output.setLookAndFeel(nullptr); mix.setLookAndFeel(nullptr);
    compression.setLookAndFeel(nullptr); drive.setLookAndFeel(nullptr);
}

void DRMRESHAPEAudioProcessorEditor::updateModeDefaults()
{
    const int m = mode.getSelectedItemIndex();
    const float values[6][6] = { {65,800,7000,280,4800,14000}, {150,1800,7500,450,5500,14500}, {90,700,6500,320,4200,13000}, {120,1000,9000,300,6000,15500}, {55,500,8500,220,5000,15000}, {60,650,8000,250,5000,15000} };
    for (int i=0; i<6; ++i)
        if (auto* param = audioProcessor.parameters.getParameter(ids[i] + "Freq")) param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(values[juce::jlimit(0,5,m)][i]));
}

void DRMRESHAPEAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(juce::Colour(0xff292b2d)); g.fillRect(12, 8, getWidth()-24, getHeight()-16);
    g.setColour(steel); g.drawRect(12, 8, getWidth()-24, getHeight()-16, 1);
    // Fine brushed-metal grain, kept low-contrast to protect label legibility.
    g.setColour(juce::Colours::white.withAlpha(0.012f));
    for (int y=12; y<getHeight()-12; y+=5) g.drawHorizontalLine(y, 14, getWidth()-14);
    g.setColour(juce::Colour(0xff17191a)); g.fillRect(18, 14, getWidth()-36, 62);
    g.setColour(steel.withAlpha(0.8f)); g.drawHorizontalLine(78, 22, getWidth()-22);
    const auto inputDb=juce::Decibels::gainToDecibels(audioProcessor.getInputPeak(), -60.0f);
    const auto outputDb=juce::Decibels::gainToDecibels(audioProcessor.getOutputPeak(), -60.0f);
    drawHardwareMeter(g, (float)getWidth()-356.0f, 14.0f, 108.0f, 52.0f, "INPUT  /  dB", inputDb, -60.0f, 0.0f);
    drawHardwareMeter(g, (float)getWidth()-240.0f, 14.0f, 108.0f, 52.0f, "OUTPUT  /  dB", outputDb, -60.0f, 0.0f);
    drawHardwareMeter(g, (float)getWidth()-124.0f, 14.0f, 108.0f, 52.0f, "GR  /  dB", audioProcessor.getCompressionReductionDb(), 0.0f, 24.0f);
    g.setColour(juce::Colour(0xff090b0c)); g.fillRect(24, 88, getWidth()-48, 274);
    g.setColour(steel.withAlpha(0.6f)); g.drawRect(juce::Rectangle<float>(24.0f, 88.0f, (float)getWidth()-48.0f, 274.0f), 1.0f);
    constexpr float plotTop=130.0f, plotBottom=338.0f;
    const float plotLeft=64.0f, plotRight=(float)getWidth()-74.0f, plotWidth=plotRight-plotLeft;
    const auto frequencyToX=[plotLeft,plotWidth](double hz) {
        return plotLeft + (float)(std::log(std::clamp(hz,20.0,20000.0)/20.0)/std::log(1000.0))*plotWidth;
    };
    g.setColour(grid);
    const double gridFrequencies[] { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    for (const auto hz : gridFrequencies) g.drawVerticalLine((int)frequencyToX(hz), (int)plotTop, (int)plotBottom);
    for (int i=0; i<5; ++i) g.drawHorizontalLine((int)(plotTop+i*(plotBottom-plotTop)/4.0f), (int)plotLeft, plotRight);
    g.setColour(juce::Colour(0xffa7ada6)); g.setFont(juce::Font(9.0f, juce::Font::bold));
    const double labelFrequencies[] { 20, 50, 100, 500, 1000, 5000, 10000, 20000 };
    const juce::StringArray freqText { "20", "50", "100", "500", "1k", "5k", "10k", "20k" };
    for (int i=0; i<freqText.size(); ++i) g.drawText(freqText[i], (int)frequencyToX(labelFrequencies[i])-22, 343, 44, 14, juce::Justification::centred);
    g.setColour(muted); g.setFont(juce::Font(8.5f)); g.drawText("FFT", 22, 99, 38, 13, juce::Justification::centredRight);
    const juce::StringArray fftDbLabels { "0", "-18", "-36", "-54", "-72" };
    for (int i=0; i<fftDbLabels.size(); ++i)
    {
        const auto y=(int)(plotTop+i*(plotBottom-plotTop)/4.0f)-7;
        g.drawText(fftDbLabels[i], 24, y, 34, 14, juce::Justification::centredRight);
    }
    g.setColour(accent.withAlpha(0.8f)); g.drawText("EQ dB", (int)plotRight+5, 99, 56, 13, juce::Justification::centredLeft);
    const juce::StringArray eqDbLabels { "+18", "0", "-18" };
    for (int i=0; i<eqDbLabels.size(); ++i)
    {
        const auto y=(int)(plotTop+i*(plotBottom-plotTop)/2.0f)-7;
        g.drawText(eqDbLabels[i], (int)plotRight+5, y, 50, 14, juce::Justification::centredLeft);
    }
    auto s = audioProcessor.getDspSettings();
    const double sampleRate=audioProcessor.getSampleRate()>0.0 ? audioProcessor.getSampleRate() : 44100.0;
    auto drawSpectrum = [this, &g, sampleRate, plotLeft, plotWidth, plotTop, plotBottom](bool post, juce::Colour colour) {
        juce::Path spectrum;
        for (int x=0; x<=700; ++x)
        {
            const double hz=20.0*std::pow(1000.0, x/700.0);
            const int bin=juce::jlimit(1,1024,(int)std::round(hz*2048.0/sampleRate));
            const float magnitude=audioProcessor.getSpectrum(post,bin);
            const float db=juce::jlimit(-72.0f,0.0f,juce::Decibels::gainToDecibels(magnitude, -72.0f));
            const float px=plotLeft+(float)x/700.0f*plotWidth, py=plotBottom-(db+72.0f)*((plotBottom-plotTop)/72.0f);
            if (x==0) spectrum.startNewSubPath(px,py); else spectrum.lineTo(px,py);
        }
        g.setColour(colour.withAlpha(post ? 0.48f : 0.28f)); g.strokePath(spectrum, juce::PathStrokeType(post ? 1.7f : 1.0f));
    };
    drawSpectrum(false, teal);
    drawSpectrum(true, green);
    juce::Path response;
    for (int x=0; x<=700; ++x)
    {
        const double norm=x/700.0, hz=20.0*std::pow(1000.0,norm);
        const auto db=juce::jlimit(-18.0f,18.0f,drumshaper::DrumShaperDSP::responseAt(s,hz));
        const float px=plotLeft+norm*plotWidth, py=(plotTop+plotBottom)*0.5f-db*((plotBottom-plotTop)/36.0f);
        if (x==0) response.startNewSubPath(px,py); else response.lineTo(px,py);
    }
    g.setColour(accent); g.strokePath(response, juce::PathStrokeType(2.5f));
    g.setColour(text); g.setFont(juce::Font(10.0f, juce::Font::bold)); g.drawText("PRE   POST   /   FREQUENCY RESPONSE", 42, 91, 320, 16, juce::Justification::left);
    g.setColour(panel); g.fillRect(22, 378, getWidth()-44, getHeight()-430);
    g.setColour(steel); g.drawHorizontalLine(378, 22, getWidth()-22);
    g.setColour(steel.withAlpha(0.7f)); g.drawHorizontalLine(654, 34, getWidth()-34);
    g.setColour(juce::Colour(0xff111315));
    for (const auto point : { juce::Point<int>(34,390), juce::Point<int>(getWidth()-34,390), juce::Point<int>(34,getHeight()-34), juce::Point<int>(getWidth()-34,getHeight()-34) })
    {
        g.setColour(juce::Colour(0xff999383)); g.fillEllipse((float)point.x-3, (float)point.y-3, 6, 6);
        g.setColour(juce::Colour(0xff68655d)); g.drawLine((float)point.x-1.5f,(float)point.y+1.5f,(float)point.x+1.5f,(float)point.y-1.5f,0.8f);
    }
    g.setColour(muted); g.setFont(juce::Font(10.0f, juce::Font::bold)); g.drawText(showAdvanced ? (showDynamics ? "DYNAMIC PROCESSING" : "EQUALIZER / FREQUENCY / Q") : "EQUALIZER / TONE", 40, 386, 290, 18, juce::Justification::left);
    g.setColour(steel.withAlpha(0.55f)); g.drawVerticalLine(370, 675, getHeight()-35);
    g.drawVerticalLine(640, 675, getHeight()-35);
    g.setColour(juce::Colour(0xff292e35)); g.drawHorizontalLine(780, 0, getWidth());
    g.setColour(muted); g.setFont(juce::Font(9.0f)); g.drawText("MODEL DRM-01    /    MINIMUM PHASE    /    ZERO LATENCY    /    STEREO", 22, getHeight()-23, getWidth()-44, 14, juce::Justification::centred);
    g.setColour(teal); g.drawText("IN " + juce::String(juce::Decibels::gainToDecibels(audioProcessor.getInputPeak(), -60.0f), 1) + " dB", getWidth()-180, 93, 72, 16, juce::Justification::right);
    g.setColour(green); g.drawText("OUT " + juce::String(juce::Decibels::gainToDecibels(audioProcessor.getOutputPeak(), -60.0f), 1) + " dB", getWidth()-102, 93, 72, 16, juce::Justification::right);
}

void DRMRESHAPEAudioProcessorEditor::resized()
{
    title.setBounds(26, 16, 300, 32); subtitle.setBounds(29, 49, 280, 16); mode.setBounds(390, 28, 160, 30); advanced.setBounds(558, 28, 122, 30);
    const int left=34, usable=getWidth()-68, colWidth=usable/6;
    for (int i=0; i<6; ++i)
    {
        const int x=left+i*colWidth;
        bandLabels[(size_t)i].setVisible(!showAdvanced || !showDynamics);
        bandEnabled[(size_t)i].setVisible(!showAdvanced || !showDynamics);
        bandLabels[(size_t)i].setBounds(x, 407, colWidth-10, 20);
        bandEnabled[(size_t)i].setBounds(x+2, 427, colWidth-10, 22);
        gainControls[(size_t)i].setVisible(!showAdvanced || !showDynamics);
        gainControls[(size_t)i].setBounds(x, 448, colWidth-12, 112);
        freqControls[(size_t)i].setVisible(showAdvanced && !showDynamics); qControls[(size_t)i].setVisible(showAdvanced && !showDynamics);
        if (showAdvanced && !showDynamics) { freqControls[(size_t)i].setBounds(x, 557, colWidth-12, 48); qControls[(size_t)i].setBounds(x, 605, colWidth-12, 46); }
    }
    dynamicsPage.setVisible(showAdvanced); dynamicsPage.setBounds(getWidth()-154, 384, 112, 26);
    for (int i=0; i<8; ++i)
    {
        dynamicsLabels[(size_t)i].setVisible(showAdvanced && showDynamics);
        dynamicsControls[(size_t)i].setVisible(showAdvanced && showDynamics);
        const int x=left+(i%4)*(usable/4), y=444+(i/4)*102;
        dynamicsLabels[(size_t)i].setBounds(x,y,usable/4-12,16);
        dynamicsControls[(size_t)i].setBounds(x,y+13,usable/4-12,86);
    }
    for (int i=0; i<2; ++i)
    {
        dynamicsEnabled[(size_t)i].setVisible(showAdvanced && showDynamics);
        dynamicsEnabled[(size_t)i].setBounds(42+i*230, 408, 220, 26);
    }
    const int y=669;
    inputLabel.setBounds(30,y,100,16); input.setBounds(29,y+13,100,88);
    outputLabel.setBounds(140,y,100,16); output.setBounds(139,y+13,100,88);
    mixLabel.setBounds(250,y,100,16); mix.setBounds(249,y+13,100,88);
    compressionLabel.setBounds(408,y,104,16); compression.setBounds(406,y+13,104,88);
    driveLabel.setBounds(528,y,96,16); drive.setBounds(526,y+13,96,88);
    outputCompensation.setBounds(664, y+29, 152, 28); globalBypass.setBounds(getWidth()-174, y+29, 140, 28);
}
