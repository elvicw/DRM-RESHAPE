#include "PluginEditor.h"

namespace
{
const juce::StringArray labels { "LOW", "MID", "HIGH", "TIGHT", "SMOOTH", "AIR" };
const juce::StringArray ids { "low", "mid", "high", "tight", "smooth", "air" };
const juce::Colour bg { 0xff1b1c1c }, panel { 0xffc7c1b2 }, faceplateLight { 0xffd8d2c4 }, display { 0xff151e20 }, text { 0xff272925 }, titleText { 0xffece8dc }, muted { 0xff69675f }, accent { 0xffd59b4d }, teal { 0xff527e79 }, grid { 0xff344144 };
void setupSlider(juce::Slider& s, juce::Colour colour, bool compact=false)
{
    s.setSliderStyle(compact ? juce::Slider::RotaryHorizontalVerticalDrag : juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    s.setColour(juce::Slider::rotarySliderFillColourId, colour);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff514e47));
    s.setColour(juce::Slider::thumbColourId, accent);
    s.setColour(juce::Slider::textBoxTextColourId, text);
    s.setColour(juce::Slider::textBoxBackgroundColourId, faceplateLight);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xffaaa493));
    s.setNumDecimalPlacesToDisplay(1);
}

void drawHardwareMeter(juce::Graphics& g, float x, float y, float width, float height,
                       const juce::String& name, float peakDb)
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
    const float normalized = juce::jlimit(0.0f, 1.0f, (peakDb + 24.0f) / 30.0f);
    const float angle = juce::MathConstants<float>::pi + normalized * juce::MathConstants<float>::pi;
    g.setColour(juce::Colour(0xffa44b36));
    g.drawLine(cx, cy, cx + (radius - 9.0f) * std::cos(angle), cy + (radius - 9.0f) * std::sin(angle), 1.6f);
    g.fillEllipse(cx - 2.5f, cy - 2.5f, 5.0f, 5.0f);
}
}

DRMRESHAPEAudioProcessorEditor::DRMRESHAPEAudioProcessorEditor(DRMRESHAPEAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1080, 800);
    title.setText("DRM-RESHAPE", juce::dontSendNotification); title.setFont(juce::Font(26.0f, juce::Font::bold)); title.setColour(juce::Label::textColourId, titleText); addAndMakeVisible(title);
    subtitle.setText("DRUM TONE SHAPER   /   YEAHWAAH AUDIO", juce::dontSendNotification); subtitle.setFont(juce::Font(10.0f, juce::Font::bold)); subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffb8b3a7)); addAndMakeVisible(subtitle);
    mode.addItemList({ "KICK", "SNARE", "TOMS", "OVERHEADS", "ROOM", "FULL KIT" }, 1); mode.setColour(juce::ComboBox::backgroundColourId, panel); mode.setColour(juce::ComboBox::textColourId, text); mode.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff343a42)); addAndMakeVisible(mode);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "mode", mode);
    mode.onChange = [this] { updateModeDefaults(); };
    advanced.setColour(juce::TextButton::buttonColourId, faceplateLight); advanced.setColour(juce::TextButton::textColourOffId, text); advanced.onClick = [this] { showAdvanced = !showAdvanced; advanced.setButtonText(showAdvanced ? "ADVANCED −" : "ADVANCED +"); resized(); repaint(); }; addAndMakeVisible(advanced);
    dynamicsPage.setColour(juce::TextButton::buttonColourId, panel); dynamicsPage.setColour(juce::TextButton::textColourOffId, teal); dynamicsPage.onClick = [this] { showDynamics = !showDynamics; dynamicsPage.setButtonText(showDynamics ? "BANDS" : "DYNAMICS"); resized(); repaint(); }; addAndMakeVisible(dynamicsPage); dynamicsPage.setVisible(false);
    for (int i=0; i<6; ++i)
    {
        bandLabels[(size_t)i].setText(labels[i], juce::dontSendNotification); bandLabels[(size_t)i].setJustificationType(juce::Justification::centred); bandLabels[(size_t)i].setFont(juce::Font(13, juce::Font::bold)); bandLabels[(size_t)i].setColour(juce::Label::textColourId, i == 3 || i == 4 ? teal : text); addAndMakeVisible(bandLabels[(size_t)i]);
        bandEnabled[(size_t)i].setButtonText("ON"); bandEnabled[(size_t)i].setColour(juce::ToggleButton::textColourId, muted); addAndMakeVisible(bandEnabled[(size_t)i]);
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, ids[i] + "On", bandEnabled[(size_t)i]));
        setupSlider(gainControls[(size_t)i], i == 3 || i == 4 ? teal : accent); gainControls[(size_t)i].setNumDecimalPlacesToDisplay(1); gainControls[(size_t)i].setTextValueSuffix(" dB"); addAndMakeVisible(gainControls[(size_t)i]);
        auto stem = ids[i];
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, stem + "Gain", gainControls[(size_t)i]));
        setupSlider(freqControls[(size_t)i], accent, true); freqControls[(size_t)i].setNumDecimalPlacesToDisplay(0); freqControls[(size_t)i].setTextValueSuffix(" Hz"); addAndMakeVisible(freqControls[(size_t)i]);
        freqControls[(size_t)i].setSliderStyle(juce::Slider::LinearHorizontal); freqControls[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 58, 18);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, stem + "Freq", freqControls[(size_t)i]));
        setupSlider(qControls[(size_t)i], teal, true); qControls[(size_t)i].setNumDecimalPlacesToDisplay(2); addAndMakeVisible(qControls[(size_t)i]);
        qControls[(size_t)i].setSliderStyle(juce::Slider::LinearHorizontal); qControls[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 58, 18);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, stem + "Q", qControls[(size_t)i]));
    }
    auto setupUtility = [this](juce::Slider& s, juce::Label& l, const juce::String& name, const juce::String& id, const juce::String& suffix) {
        setupSlider(s, accent); s.setTextValueSuffix(suffix); addAndMakeVisible(s); l.setText(name, juce::dontSendNotification); l.setJustificationType(juce::Justification::centred); l.setColour(juce::Label::textColourId, muted); addAndMakeVisible(l);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, id, s));
    };
    setupUtility(input, inputLabel, "INPUT", "input", " dB"); setupUtility(output, outputLabel, "OUTPUT", "output", " dB"); setupUtility(mix, mixLabel, "MIX", "mix", "%");
    for (int i=0; i<8; ++i)
    {
        const int group=i/4, control=i%4;
        const juce::String prefix=group==0 ? "TIGHT" : "SMOOTH";
        const juce::String key=prefix.toLowerCase();
        const juce::StringArray suffixes { "Threshold", "Attack", "Release", "MaxGR" };
        const juce::StringArray units { " dB", " ms", " ms", " dB" };
        dynamicsLabels[(size_t)i].setText(prefix + " " + suffixes[control].toUpperCase(), juce::dontSendNotification); dynamicsLabels[(size_t)i].setJustificationType(juce::Justification::centred); dynamicsLabels[(size_t)i].setColour(juce::Label::textColourId, muted); addAndMakeVisible(dynamicsLabels[(size_t)i]);
        setupSlider(dynamicsControls[(size_t)i], teal); dynamicsControls[(size_t)i].setNumDecimalPlacesToDisplay(1); dynamicsControls[(size_t)i].setTextValueSuffix(units[control]); addAndMakeVisible(dynamicsControls[(size_t)i]);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, key + suffixes[control], dynamicsControls[(size_t)i]));
    }
    for (int i=0; i<2; ++i)
    {
        const auto key=i==0 ? "tightDyn" : "smoothDyn";
        dynamicsEnabled[(size_t)i].setButtonText(i==0 ? "TIGHT DYNAMIC" : "SMOOTH DYNAMIC"); dynamicsEnabled[(size_t)i].setColour(juce::ToggleButton::textColourId, teal); addAndMakeVisible(dynamicsEnabled[(size_t)i]);
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, key, dynamicsEnabled[(size_t)i]));
    }
    globalBypass.setButtonText("BYPASS"); globalBypass.setColour(juce::ToggleButton::textColourId, text); addAndMakeVisible(globalBypass);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "bypass", globalBypass));
    outputCompensation.setButtonText("LEVEL COMP"); outputCompensation.setColour(juce::ToggleButton::textColourId, muted); addAndMakeVisible(outputCompensation);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "comp", outputCompensation));
    resized();
    startTimerHz(24);
}

void DRMRESHAPEAudioProcessorEditor::updateModeDefaults()
{
    const int m = mode.getSelectedItemIndex();
    const float values[6][6] = { {65,800,7000,280,4800,14000}, {150,1800,7500,450,5500,14500}, {90,700,6500,320,4200,13000}, {120,1000,9000,300,6000,15500}, {55,500,8500,220,5000,15000}, {60,650,8000,250,5000,15000} };
    for (int i=0; i<6; ++i)
        if (auto* param = processor.parameters.getParameter(ids[i] + "Freq")) param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(values[juce::jlimit(0,5,m)][i]));
}

void DRMRESHAPEAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(juce::Colour(0xff292b2a)); g.fillRoundedRectangle(18, 9, getWidth()-36, 64, 8);
    g.setColour(juce::Colour(0xff77756d)); g.drawRoundedRectangle(18, 9, getWidth()-36, 64, 8, 1.0f);
    g.setColour(accent); g.drawHorizontalLine(74, 24, getWidth()-24);
    const auto inputDb=juce::Decibels::gainToDecibels(processor.getInputPeak(), -60.0f);
    const auto outputDb=juce::Decibels::gainToDecibels(processor.getOutputPeak(), -60.0f);
    drawHardwareMeter(g, (float)getWidth()-313.0f, 14.0f, 132.0f, 52.0f, "INPUT  /  dB", inputDb);
    drawHardwareMeter(g, (float)getWidth()-169.0f, 14.0f, 132.0f, 52.0f, "OUTPUT  /  dB", outputDb);
    g.setColour(display); g.fillRoundedRectangle(22, 82, getWidth()-44, 252, 10);
    g.setColour(grid);
    for (int i=0; i<9; ++i) { const float x=42.0f+i*(getWidth()-84.0f)/8.0f; g.drawVerticalLine((int)x, 100, 310); }
    for (int i=0; i<5; ++i) { const float y=110.0f+i*48.0f; g.drawHorizontalLine((int)y, 42, getWidth()-42); }
    g.setColour(juce::Colour(0xffa7ada6)); g.setFont(10.0f);
    const juce::StringArray freqText { "20", "50", "100", "500", "1k", "5k", "10k", "20k" };
    for (int i=0; i<freqText.size(); ++i) g.drawText(freqText[i], 36 + i*(getWidth()-85)/7, 315, 40, 14, juce::Justification::left);
    auto s = processor.getDspSettings();
    const double sampleRate=processor.getSampleRate()>0.0 ? processor.getSampleRate() : 44100.0;
    auto drawSpectrum = [this, &g, sampleRate](bool post, juce::Colour colour) {
        juce::Path spectrum;
        for (int x=0; x<=700; ++x)
        {
            const double hz=20.0*std::pow(1000.0, x/700.0);
            const int bin=juce::jlimit(1,1024,(int)std::round(hz*2048.0/sampleRate));
            const float magnitude=processor.getSpectrum(post,bin);
            const float db=juce::jlimit(-72.0f,0.0f,juce::Decibels::gainToDecibels(magnitude, -72.0f));
            const float px=42.0f+(float)x/700.0f*(float)(getWidth()-84), py=310.0f-(db+72.0f)*(190.0f/72.0f);
            if (x==0) spectrum.startNewSubPath(px,py); else spectrum.lineTo(px,py);
        }
        g.setColour(colour.withAlpha(post ? 0.48f : 0.28f)); g.strokePath(spectrum, juce::PathStrokeType(post ? 1.7f : 1.0f));
    };
    drawSpectrum(false, juce::Colour(0xffc2c5ba));
    drawSpectrum(true, teal);
    juce::Path response;
    for (int x=0; x<=700; ++x)
    {
        const double norm=x/700.0, hz=20.0*std::pow(1000.0,norm);
        const auto db=juce::jlimit(-18.0f,18.0f,drumshaper::DrumShaperDSP::responseAt(s,hz));
        const float px=42.0f+norm*(getWidth()-84.0f), py=214.0f-db*5.7f;
        if (x==0) response.startNewSubPath(px,py); else response.lineTo(px,py);
    }
    g.setColour(accent); g.strokePath(response, juce::PathStrokeType(2.5f));
    g.setColour(teal); g.setFont(10.0f); g.drawText("PRE / POST SPECTRUM   •   EQ RESPONSE", 42, 92, 260, 16, juce::Justification::left);
    g.setColour(panel); g.fillRoundedRectangle(22, 356, getWidth()-44, 420, 10);
    g.setColour(juce::Colour(0xff827d70)); g.drawRoundedRectangle(22, 356, getWidth()-44, 420, 10, 1.0f);
    for (int y=360; y<773; y+=4) { g.setColour(juce::Colours::white.withAlpha(0.025f)); g.drawHorizontalLine(y, 32, getWidth()-32); }
    for (const auto point : { juce::Point<int>(34,368), juce::Point<int>(getWidth()-34,368), juce::Point<int>(34,764), juce::Point<int>(getWidth()-34,764) })
    {
        g.setColour(juce::Colour(0xff999383)); g.fillEllipse((float)point.x-3, (float)point.y-3, 6, 6);
        g.setColour(juce::Colour(0xff68655d)); g.drawLine((float)point.x-1.5f,(float)point.y+1.5f,(float)point.x+1.5f,(float)point.y-1.5f,0.8f);
    }
    g.setColour(muted); g.drawText(showAdvanced ? (showDynamics ? "DYNAMIC TIGHT / SMOOTH" : "FREQUENCY / Q") : "SHAPE THE KIT", 42, 367, 280, 18, juce::Justification::left);
    g.setColour(juce::Colour(0xff657174)); g.drawRoundedRectangle(22, 82, getWidth()-44, 252, 10, 1.0f);
    g.setColour(juce::Colour(0xff292e35)); g.drawHorizontalLine(780, 0, getWidth());
    g.setColour(juce::Colour(0xffb8b3a7)); g.setFont(10); g.drawText("MINIMUM PHASE  •  ZERO LATENCY  •  STEREO", 22, 782, getWidth()-44, 16, juce::Justification::centred);
    g.setColour(teal); g.drawText("IN " + juce::String(juce::Decibels::gainToDecibels(processor.getInputPeak(), -60.0f), 1) + " dB", getWidth()-180, 92, 72, 16, juce::Justification::right);
    g.drawText("OUT " + juce::String(juce::Decibels::gainToDecibels(processor.getOutputPeak(), -60.0f), 1) + " dB", getWidth()-102, 92, 72, 16, juce::Justification::right);
}

void DRMRESHAPEAudioProcessorEditor::resized()
{
    title.setBounds(26, 18, 310, 34); subtitle.setBounds(29, 52, 280, 18); mode.setBounds(365, 28, 150, 32); advanced.setBounds(530, 28, 120, 32);
    const int colWidth=(getWidth()-100)/6;
    for (int i=0; i<6; ++i)
    {
        const int x=38+i*colWidth;
        bandLabels[(size_t)i].setBounds(x, 390, colWidth-10, 22);
        bandEnabled[(size_t)i].setBounds(x, 409, colWidth-10, 22);
        gainControls[(size_t)i].setVisible(!showAdvanced || !showDynamics);
        gainControls[(size_t)i].setBounds(x, 432, colWidth-10, 118);
        freqControls[(size_t)i].setVisible(showAdvanced && !showDynamics); qControls[(size_t)i].setVisible(showAdvanced && !showDynamics);
        if (showAdvanced && !showDynamics) { freqControls[(size_t)i].setBounds(x, 556, colWidth-12, 38); qControls[(size_t)i].setBounds(x, 600, colWidth-12, 38); }
    }
    dynamicsPage.setVisible(showAdvanced); dynamicsPage.setBounds(getWidth()-140, 364, 100, 28);
    for (int i=0; i<8; ++i) { dynamicsLabels[(size_t)i].setVisible(showAdvanced && showDynamics); dynamicsControls[(size_t)i].setVisible(showAdvanced && showDynamics); const int group=i/4, control=i%4; const int x=42 + control*250; const int y=462 + group*130; dynamicsLabels[(size_t)i].setBounds(x,y,220,18); dynamicsControls[(size_t)i].setBounds(x,y+18,220,90); }
    for (int i=0; i<2; ++i) { dynamicsEnabled[(size_t)i].setVisible(showAdvanced && showDynamics); dynamicsEnabled[(size_t)i].setBounds(42, 408+i*130, 220, 28); }
    const int y=676;
    inputLabel.setBounds(40,y,100,18); input.setBounds(40,y+18,110,76);
    outputLabel.setBounds(160,y,100,18); output.setBounds(160,y+18,110,76);
    mixLabel.setBounds(280,y,100,18); mix.setBounds(280,y+18,110,76);
    outputCompensation.setBounds(420, y+28, 125, 28); globalBypass.setBounds(getWidth()-150, y+28, 110, 28);
}
