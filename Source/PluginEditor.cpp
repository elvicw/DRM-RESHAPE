#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
// The whole editor is laid out in reference-image pixels and scaled to the window.
constexpr float kRefWidth = 1418.0f, kRefBaseHeight = 1109.0f, kAdvancedExtra = 262.0f;

const juce::StringArray labels { "LOW", "MID", "HIGH", "TIGHT", "SMOOTH", "AIR" };
const juce::StringArray ids { "low", "mid", "high", "tight", "smooth", "air" };

const juce::Colour cream { 0xffd6cbb0 }, creamDim { 0xff8f8672 }, creamBright { 0xffe8dfc6 },
                   redLed { 0xffff3b22 }, amber { 0xffe9a23a }, ember { 0xffff6a2a },
                   metalDark { 0xff17181a }, metalMid { 0xff232527 }, metalLight { 0xff303234 },
                   blackFace { 0xff060607 }, gridLine { 0xff25201d };

float textWidth(const juce::Font& f, const juce::String& t)
{
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(f, t, 0.0f, 0.0f);
    return glyphs.getBoundingBox(0, -1, true).getWidth();
}

juce::Font font(float size, bool bold = true)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

void drawScrew(juce::Graphics& g, float cx, float cy, float r, float turn)
{
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillEllipse(cx - r, cy - r + 1.5f, r * 2.0f, r * 2.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff8a8780), cx - r, cy - r, juce::Colour(0xff25262a), cx + r, cy + r, false));
    g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
    g.setColour(juce::Colour(0xff0f1011));
    g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);
    const auto dx = std::cos(turn) * r * 0.7f, dy = std::sin(turn) * r * 0.7f;
    g.setColour(juce::Colour(0xff111213));
    g.drawLine(cx - dx, cy - dy, cx + dx, cy + dy, r * 0.28f);
}

void drawRecess(juce::Graphics& g, juce::Rectangle<float> r, float corner, juce::Colour fill)
{
    g.setColour(juce::Colours::white.withAlpha(0.09f)); g.fillRoundedRectangle(r.expanded(1.5f).translated(0, 1.5f), corner + 1.0f);
    g.setColour(juce::Colours::black.withAlpha(0.85f)); g.fillRoundedRectangle(r.expanded(1.5f).translated(0, -1.0f), corner + 1.0f);
    g.setColour(fill); g.fillRoundedRectangle(r, corner);
}

void drawTextCentred(juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, float size, juce::Colour c, bool bold = true, float kerning = 0.04f)
{
    g.setColour(c); g.setFont(font(size, bold).withExtraKerningFactor(kerning));
    g.drawText(t, r, juce::Justification::centred, false);
}


// Seven-segment cell: bit order a(top), b(top-right), c(bottom-right), d(bottom), e(bottom-left), f(top-left), g(middle).
void drawSegmentCell(juce::Graphics& g, juce::Rectangle<float> cell, unsigned mask, juce::Colour lit)
{
    const auto t = cell.getHeight() * 0.12f, x0 = cell.getX(), x1 = cell.getRight(), y0 = cell.getY(), y1 = cell.getBottom(), ym = cell.getCentreY();
    const auto horizontal = [&](float y) {
        juce::Path p; p.startNewSubPath(x0 + t * 0.5f, y); p.lineTo(x0 + t * 1.3f, y - t * 0.5f); p.lineTo(x1 - t * 1.3f, y - t * 0.5f);
        p.lineTo(x1 - t * 0.5f, y); p.lineTo(x1 - t * 1.3f, y + t * 0.5f); p.lineTo(x0 + t * 1.3f, y + t * 0.5f); p.closeSubPath(); return p; };
    const auto vertical = [&](float x, float ya, float yb) {
        juce::Path p; p.startNewSubPath(x, ya + t * 0.5f); p.lineTo(x + t * 0.5f, ya + t * 1.3f); p.lineTo(x + t * 0.5f, yb - t * 1.3f);
        p.lineTo(x, yb - t * 0.5f); p.lineTo(x - t * 0.5f, yb - t * 1.3f); p.lineTo(x - t * 0.5f, ya + t * 1.3f); p.closeSubPath(); return p; };
    const juce::Path segs[] { horizontal(y0 + t * 0.5f), vertical(x1 - t * 0.5f, y0, ym), vertical(x1 - t * 0.5f, ym, y1), horizontal(y1 - t * 0.5f),
                              vertical(x0 + t * 0.5f, ym, y1), vertical(x0 + t * 0.5f, y0, ym), horizontal(ym) };
    for (int i = 0; i < 7; ++i)
    {
        const auto on = (mask >> i) & 1u;
        if (on) { g.setColour(lit.withAlpha(0.22f)); g.strokePath(segs[i], juce::PathStrokeType(t * 0.9f, juce::PathStrokeType::curved)); }
        g.setColour(on ? lit : lit.withAlpha(0.06f));
        g.fillPath(segs[i]);
    }
}

unsigned segmentMask(juce::juce_wchar c)
{
    static const unsigned digits[10] { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
    if (c >= '0' && c <= '9') return digits[c - '0'];
    return c == '-' ? 0x40u : 0u;
}

// Fixed "-dd.d" cell layout so unlit segments stay visible like real hardware.
void drawSevenSegmentReadout(juce::Graphics& g, juce::Rectangle<float> area, float value, juce::Colour lit)
{
    value = juce::jlimit(-99.9f, 99.9f, value);
    if (std::abs(value) < 0.05f) value = 0.0f;
    const auto tenths = (int)std::lround(std::abs(value) * 10.0f);
    const unsigned masks[] { value < 0.0f ? 0x40u : 0u, tenths >= 100 ? segmentMask((juce::juce_wchar)('0' + tenths / 100)) : 0u,
                             segmentMask((juce::juce_wchar)('0' + (tenths / 10) % 10)), segmentMask((juce::juce_wchar)('0' + tenths % 10)) };
    const auto cellH = area.getHeight() * 0.8f, cellW = cellH * 0.5f, gap = cellW * 0.42f;
    const auto totalW = 4.0f * cellW + 3.0f * gap;
    auto x = area.getCentreX() - totalW * 0.5f;
    const auto y = area.getCentreY() - cellH * 0.5f;
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::shear(-0.16f, 0.0f).translated(0.16f * area.getCentreY(), 0.0f));
    for (int i = 0; i < 4; ++i)
    {
        drawSegmentCell(g, { x, y, cellW, cellH }, masks[i], lit);
        x += cellW + gap;
        if (i == 2) { g.setColour(lit); g.fillEllipse(x - gap * 0.72f, y + cellH - cellW * 0.22f, cellW * 0.26f, cellW * 0.26f); }
    }
}

class HardwareLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    HardwareLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, creamBright);
        setColour(juce::Slider::textBoxBackgroundColourId, blackFace);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxHighlightColourId, ember.withAlpha(0.5f));
        setColour(juce::ToggleButton::textColourId, cream);
        setColour(juce::ComboBox::textColourId, creamBright);
        setColour(juce::ComboBox::backgroundColourId, blackFace);
        setColour(juce::ComboBox::outlineColourId, creamDim);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff111213));
        setColour(juce::PopupMenu::textColourId, cream);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff5a2a1c));
        setColour(juce::PopupMenu::highlightedTextColourId, creamBright);
    }

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        const auto b = slider.getLocalBounds();
        juce::Slider::SliderLayout layout;
        layout.sliderBounds = b.withHeight(b.getWidth());
        const auto w = slider.getTextBoxWidth(), h = slider.getTextBoxHeight();
        layout.textBoxBounds = juce::Rectangle<int>(w, h).withCentre({ b.getCentreX(), b.getBottom() - h / 2 });
        return layout;
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        if (dynamic_cast<juce::Slider*>(label.getParentComponent()) == nullptr)
        {
            juce::LookAndFeel_V4::drawLabel(g, label);
            return;
        }
        auto r = label.getLocalBounds().toFloat();
        const auto corner = r.getHeight() * 0.08f;
        g.setColour(juce::Colours::black.withAlpha(0.9f)); g.fillRoundedRectangle(r, corner);
        g.setColour(blackFace); g.fillRoundedRectangle(r.reduced(1.0f), corner);
        g.setColour(creamDim.withAlpha(0.75f)); g.drawRoundedRectangle(r.reduced(0.5f), corner, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.07f)); g.drawLine(r.getX() + 2, r.getBottom() - 1.5f, r.getRight() - 2, r.getBottom() - 1.5f, 1.0f);
        if (!label.isBeingEdited())
        {
            g.setColour(creamBright);
            g.setFont(font(r.getHeight() * 0.56f).withExtraKerningFactor(0.03f));
            g.drawText(label.getText(), r.reduced(3.0f, 0.0f), juce::Justification::centred, false);
        }
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float position, float startAngle, float endAngle, juce::Slider& slider) override
    {
        const auto size = (float)juce::jmin(width, height);
        const auto cx = (float)x + (float)width * 0.5f;
        const auto cy = (float)y + (float)height * 0.5f;
        const auto r = size * 0.37f;
        const auto angle = startAngle + position * (endAngle - startAngle);
        const auto at = [&](float a, float radius) { return juce::Point<float>(cx + radius * std::sin(a), cy - radius * std::cos(a)); };

        // Engraved scale marks around the knob.
        constexpr int ticks = 11;
        for (int i = 0; i < ticks; ++i)
        {
            const auto a = startAngle + (float)i / (float)(ticks - 1) * (endAngle - startAngle);
            const auto major = i == 0 || i == ticks - 1 || i == ticks / 2;
            const auto p1 = at(a, r * 1.12f), p2 = at(a, r * (major ? 1.29f : 1.22f));
            g.setColour(cream.withAlpha(major ? 0.95f : 0.7f));
            g.drawLine({ p1, p2 }, juce::jmax(1.2f, size * (major ? 0.011f : 0.008f)));
        }
        const auto lo = slider.getProperties().getWithDefault("endLow", "").toString();
        const auto hi = slider.getProperties().getWithDefault("endHigh", "").toString();
        g.setColour(cream);
        g.setFont(font(size * 0.085f));
        const auto pl = at(startAngle, r * 1.28f).translated(0.0f, size * 0.075f);
        const auto ph = at(endAngle, r * 1.28f).translated(0.0f, size * 0.075f);
        g.drawText(lo, juce::Rectangle<float>(size * 0.34f, size * 0.12f).withCentre(pl.translated(-size * 0.035f, 0)), juce::Justification::centred, false);
        g.drawText(hi, juce::Rectangle<float>(size * 0.34f, size * 0.12f).withCentre(ph.translated(size * 0.035f, 0)), juce::Justification::centred, false);

        // Knob body: drop shadow, black knurled collar, machined metal cap.
        g.setColour(juce::Colours::black.withAlpha(0.65f));
        g.fillEllipse(cx - r * 1.04f, cy - r * 0.98f + r * 0.1f, r * 2.08f, r * 2.08f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3a3b3c), cx - r, cy - r, juce::Colour(0xff050505), cx + r, cy + r, false));
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour(juce::Colour(0xff6a6c6e).withAlpha(0.38f));
        for (int i = 0; i < 60; ++i)
        {
            const auto a = juce::MathConstants<float>::twoPi * (float)i / 60.0f;
            g.drawLine({ at(a, r * 0.90f), at(a, r * 0.985f) }, 1.0f);
        }
        g.setColour(juce::Colour(0xff0c0c0d)); g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.2f);

        const auto cap = r * 0.74f;
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffc1beb2), cx - cap * 0.7f, cy - cap * 0.8f, juce::Colour(0xff3d3e3e), cx + cap * 0.8f, cy + cap * 0.9f, false));
        g.fillEllipse(cx - cap, cy - cap, cap * 2.0f, cap * 2.0f);
        for (int i = 1; i <= 5; ++i)
        {
            const auto rr = cap * (float)i / 5.5f;
            g.setColour((i % 2 ? juce::Colours::black : juce::Colours::white).withAlpha(0.07f));
            g.drawEllipse(cx - rr, cy - rr, rr * 2.0f, rr * 2.0f, 0.8f);
        }
        g.setColour(juce::Colours::black.withAlpha(0.55f)); g.drawEllipse(cx - cap, cy - cap, cap * 2.0f, cap * 2.0f, 1.2f);
        if (slider.isMouseOverOrDragging())
        {
            g.setColour(ember.withAlpha(0.55f)); g.drawEllipse(cx - r * 1.04f, cy - r * 1.04f, r * 2.08f, r * 2.08f, 1.4f);
        }

        // Pointer follows the attached parameter value.
        const auto tip = at(angle, cap * 0.92f), base = at(angle, cap * 0.12f);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawLine({ base.translated(1.0f, 1.5f), tip.translated(1.0f, 1.5f) }, juce::jmax(2.5f, r * 0.09f));
        g.setColour(creamBright);
        g.drawLine({ base, tip }, juce::jmax(2.2f, r * 0.075f));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool) override
    {
        auto b = button.getLocalBounds().toFloat();
        const auto on = button.getToggleState();
        const auto lamp = juce::Rectangle<float>(b.getHeight() * 0.8f, b.getHeight() * 0.8f).withCentre({ b.getX() + b.getHeight() * 0.5f, b.getCentreY() });
        drawRecess(g, lamp, 3.0f, juce::Colour(0xff1a0d0a));
        if (on)
        {
            g.setColour(ember.withAlpha(0.35f)); g.fillRoundedRectangle(lamp.expanded(3.0f), 5.0f);
            g.setColour(ember); g.fillRoundedRectangle(lamp.reduced(2.5f), 2.0f);
        }
        g.setColour(on ? creamBright : creamDim);
        g.setFont(font(b.getHeight() * 0.5f).withExtraKerningFactor(0.05f));
        g.drawText(button.getButtonText(), b.withTrimmedLeft(b.getHeight() * 1.0f), juce::Justification::centredLeft, false);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool over, bool down) override
    {
        auto r = button.getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.7f)); g.fillRoundedRectangle(r.translated(0, 3.0f), 6.0f);
        const auto top = down ? juce::Colour(0xff2b2c2d) : juce::Colour(0xff6d6c66), bottom = down ? juce::Colour(0xff4a4a47) : juce::Colour(0xff2c2d2e);
        g.setGradientFill(juce::ColourGradient(top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(juce::Colours::white.withAlpha(0.18f)); g.drawRoundedRectangle(r.reduced(1.0f), 5.0f, 1.0f);
        g.setColour(over ? ember : juce::Colour(0xff0b0b0c)); g.drawRoundedRectangle(r, 6.0f, 1.4f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool down) override
    {
        auto r = button.getLocalBounds().toFloat();
        if (down) r = r.translated(0, 1.0f);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.setFont(font(r.getHeight() * 0.4f).withExtraKerningFactor(0.06f));
        g.drawText(button.getButtonText(), r.translated(0, 1.0f), juce::Justification::centred, false);
        g.setColour(button.findColour(juce::TextButton::textColourOffId));
        g.drawText(button.getButtonText(), r, juce::Justification::centred, false);
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override { return font((float)box.getHeight() * 0.42f).withExtraKerningFactor(0.05f); }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(box.getLocalBounds().withTrimmedLeft(box.getHeight() / 4).withTrimmedRight(box.getHeight()));
        label.setFont(getComboBoxFont(box));
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool over, int, int, int, int, juce::ComboBox&) override
    {
        auto r = juce::Rectangle<float>(0, 0, (float)w, (float)h).reduced(1.5f);
        g.setColour(juce::Colours::white.withAlpha(0.1f)); g.fillRoundedRectangle(r.translated(0, 1.5f), 3.0f);
        g.setColour(blackFace); g.fillRoundedRectangle(r, 3.0f);
        g.setColour(over ? ember : creamBright.withAlpha(0.85f)); g.drawRoundedRectangle(r, 3.0f, 1.6f);
        const auto cx = (float)w - (float)h * 0.45f, cy = (float)h * 0.5f, a = (float)h * 0.1f;
        juce::Path arrow; arrow.startNewSubPath(cx - a * 1.4f, cy - a); arrow.lineTo(cx, cy + a); arrow.lineTo(cx + a * 1.4f, cy - a);
        g.setColour(creamBright); g.strokePath(arrow, juce::PathStrokeType((float)h * 0.07f));
    }
};

void setupKnob(juce::Slider& s, const juce::String& name, const juce::String& suffix, const juce::String& lo, const juce::String& hi, int decimals = 1)
{
    s.setName(name);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
    s.setNumDecimalPlacesToDisplay(decimals);
    s.setTextValueSuffix(suffix);
    s.getProperties().set("endLow", lo);
    s.getProperties().set("endHigh", hi);
}
}

void IlluminatedButton::paintButton(juce::Graphics& g, bool over, bool down)
{
    const auto on = getToggleState();
    auto b = getLocalBounds().toFloat();
    const auto u = b.getHeight() / 120.0f;
    auto plate = b.removeFromTop(40.0f * u).reduced(b.getWidth() * 0.04f, 0.0f);
    auto lensArea = b.reduced(b.getWidth() * 0.14f, 4.0f * u);

    // Label plate belongs to the button assembly and brightens with the lens.
    g.setColour(juce::Colours::black.withAlpha(0.7f)); g.fillRoundedRectangle(plate.translated(0, 2.0f * u), 3.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3c3c3a), plate.getX(), plate.getY(), juce::Colour(0xff222324), plate.getX(), plate.getBottom(), false));
    g.fillRoundedRectangle(plate, 3.0f);
    g.setColour(juce::Colours::black); g.drawRoundedRectangle(plate, 3.0f, 1.2f);
    g.setColour(on ? creamBright : creamDim);
    g.setFont(font(plate.getHeight() * 0.5f).withExtraKerningFactor(0.06f));
    g.drawText("LEVEL COMP", plate, juce::Justification::centred, false);

    drawRecess(g, lensArea, 6.0f * u, juce::Colour(0xff0a0a0b));
    auto lens = lensArea.reduced(6.0f * u);
    if (on)
    {
        g.setColour(ember.withAlpha(0.28f)); g.fillRoundedRectangle(lens.expanded(7.0f * u), 9.0f * u);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffff9a4a), lens.getX(), lens.getY(), juce::Colour(0xffd8300f), lens.getX(), lens.getBottom(), false));
    }
    else
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff4a2a24), lens.getX(), lens.getY(), juce::Colour(0xff251210), lens.getX(), lens.getBottom(), false));
    g.fillRoundedRectangle(lens.translated(0, down ? 1.5f * u : 0.0f), 5.0f * u);
    g.setColour(juce::Colours::white.withAlpha(on ? 0.35f : 0.08f));
    g.drawLine(lens.getX() + 6.0f * u, lens.getY() + 4.0f * u, lens.getRight() - 6.0f * u, lens.getY() + 4.0f * u, 1.5f);
    g.setColour(over ? creamBright : juce::Colour(0xff6c685c)); g.drawRoundedRectangle(lens, 5.0f * u, 1.4f);
    g.setColour(on ? juce::Colour(0xff2a0a02) : creamDim);
    g.setFont(font(lens.getHeight() * 0.34f).withExtraKerningFactor(0.1f));
    g.drawText(on ? "ON" : "OFF", lens, juce::Justification::centred, false);
}

DRMRESHAPEAudioProcessorEditor::DRMRESHAPEAudioProcessorEditor(DRMRESHAPEAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), hardwareLookAndFeel(std::make_unique<HardwareLookAndFeel>())
{
    setLookAndFeel(hardwareLookAndFeel.get());
    auto setDefaultOnDoubleClick = [this](juce::Slider& slider, const juce::String& parameterID)
    {
        if (const auto* parameter = audioProcessor.parameters.getParameter(parameterID))
        {
            const auto value = parameter->getNormalisableRange().convertFrom0to1(parameter->getDefaultValue());
            slider.setDoubleClickReturnValue(true, value);
            slider.setScrollWheelEnabled(true);
        }
    };
    auto attach = [this, &setDefaultOnDoubleClick](juce::Slider& s, const juce::String& id)
    {
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, id, s));
        setDefaultOnDoubleClick(s, id);
        s.setLookAndFeel(hardwareLookAndFeel.get());
        addAndMakeVisible(s);
    };

    mode.addItemList({ "KICK", "SNARE", "TOMS", "OVERHEADS", "ROOM", "FULL KIT" }, 1);
    mode.setName("Instrument");
    addAndMakeVisible(mode);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "mode", mode);
    mode.onChange = [this] { updateModeDefaults(); };

    advanced.setColour(juce::TextButton::textColourOffId, cream);
    advanced.onClick = [this] { showAdvanced = !showAdvanced; applyAdvancedState(); };
    addAndMakeVisible(advanced);
    dynamicsPage.setColour(juce::TextButton::textColourOffId, cream);
    dynamicsPage.onClick = [this] { showDynamics = !showDynamics; applyAdvancedState(); };
    addChildComponent(dynamicsPage);

    for (int i = 0; i < 6; ++i)
    {
        const auto stem = ids[i];
        setupKnob(gainControls[(size_t)i], labels[i], " dB", juce::String::fromUTF8("\xe2\x88\x92"), "+");
        attach(gainControls[(size_t)i], stem + "Gain");
        setupKnob(freqControls[(size_t)i], labels[i] + juce::String(" frequency"), " Hz", "", "", 0);
        attach(freqControls[(size_t)i], stem + "Freq");
        setupKnob(qControls[(size_t)i], labels[i] + juce::String(" Q"), "", "", "", 2);
        attach(qControls[(size_t)i], stem + "Q");
    }
    setupKnob(input, "Input", " dB", "-24", "+24");
    setupKnob(output, "Output", " dB", "-24", "+24");
    setupKnob(mix, "Mix", "%", "0%", "100%");
    setupKnob(compression, "Compression", "%", "0%", "100%");
    setupKnob(drive, "Drive", " dB", "0", "24");
    attach(input, "input"); attach(output, "output"); attach(mix, "mix");
    attach(compression, "compression"); attach(drive, "drive");

    const juce::StringArray suffixes { "Threshold", "Attack", "Release", "MaxGR" };
    const juce::StringArray units { " dB", " ms", " ms", " dB" };
    for (int i = 0; i < 8; ++i)
    {
        const auto key = juce::String(i / 4 == 0 ? "tight" : "smooth");
        setupKnob(dynamicsControls[(size_t)i], key + " " + suffixes[i % 4], units[i % 4], "", "");
        attach(dynamicsControls[(size_t)i], key + suffixes[i % 4]);
    }
    for (int i = 0; i < 2; ++i)
    {
        dynamicsEnabled[(size_t)i].setButtonText(i == 0 ? "TIGHT DYNAMIC" : "SMOOTH DYNAMIC");
        dynamicsEnabled[(size_t)i].setLookAndFeel(hardwareLookAndFeel.get());
        addChildComponent(dynamicsEnabled[(size_t)i]);
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, i == 0 ? "tightDyn" : "smoothDyn", dynamicsEnabled[(size_t)i]));
    }
    // Level Comp is the only always-visible toggle. The "bypass" parameter is left untouched.
    addAndMakeVisible(levelComp);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "comp", levelComp));

    constrainer.setFixedAspectRatio((double)kRefWidth / (double)kRefBaseHeight);
    constrainer.setSizeLimits(710, 555, 1700, 1330);
    setConstrainer(&constrainer);
    setResizable(true, true);
    setSize(1100, (int)std::round(1100.0f * refHeight() / kRefWidth));
    startTimerHz(24);
}

DRMRESHAPEAudioProcessorEditor::~DRMRESHAPEAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
    for (auto* group : { &gainControls, &freqControls, &qControls })
        for (auto& s : *group) s.setLookAndFeel(nullptr);
    for (auto& s : dynamicsControls) s.setLookAndFeel(nullptr);
    for (auto& b : dynamicsEnabled) b.setLookAndFeel(nullptr);
    for (auto* s : { &input, &output, &mix, &compression, &drive }) s->setLookAndFeel(nullptr);
}

float DRMRESHAPEAudioProcessorEditor::refHeight() const noexcept
{
    return kRefBaseHeight + (showAdvanced ? kAdvancedExtra : 0.0f);
}

void DRMRESHAPEAudioProcessorEditor::applyAdvancedState()
{
    advanced.setButtonText(showAdvanced ? "ADVANCED -" : "ADVANCED +");
    dynamicsPage.setButtonText(showDynamics ? "BANDS" : "DYNAMICS");
    constrainer.setFixedAspectRatio((double)kRefWidth / (double)refHeight());
    chassis = {};
    const auto h = (int)std::round((float)getWidth() * refHeight() / kRefWidth);
    if (h == getHeight()) resized(); else setSize(getWidth(), h);
    repaint();
}

void DRMRESHAPEAudioProcessorEditor::timerCallback()
{
    // Numeric readouts hold their peak for 1.5 s, then fall at 15 dB/s so the digits stay readable.
    constexpr float dt = 1.0f / 24.0f;
    const float live[3] { juce::Decibels::gainToDecibels(audioProcessor.getInputPeak(), -60.0f),
                          juce::Decibels::gainToDecibels(audioProcessor.getOutputPeak(), -60.0f),
                          audioProcessor.getCompressionReductionDb() };
    for (int i = 0; i < 3; ++i)
    {
        if (live[i] >= heldReadings[i]) { heldReadings[i] = live[i]; holdAgeSeconds[i] = 0.0f; }
        else if ((holdAgeSeconds[i] += dt) > 1.5f) heldReadings[i] = juce::jmax(live[i], heldReadings[i] - 15.0f * dt);
    }
    const auto s = (float)getWidth() / kRefWidth;
    repaint(juce::Rectangle<float>(870.0f * s, 40.0f * s, 540.0f * s, 125.0f * s).toNearestIntEdges());
    repaint(juce::Rectangle<float>(20.0f * s, 170.0f * s, 1380.0f * s, 355.0f * s).toNearestIntEdges());
}

void DRMRESHAPEAudioProcessorEditor::updateModeDefaults()
{
    const int m = mode.getSelectedItemIndex();
    const float values[6][6] = { {65,800,7000,280,4800,14000}, {150,1800,7500,450,5500,14500}, {90,700,6500,320,4200,13000}, {120,1000,9000,300,6000,15500}, {55,500,8500,220,5000,15000}, {60,650,8000,250,5000,15000} };
    for (int i = 0; i < 6; ++i)
        if (auto* param = audioProcessor.parameters.getParameter(ids[i] + "Freq")) param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(values[juce::jlimit(0, 5, m)][i]));
}

void DRMRESHAPEAudioProcessorEditor::resized()
{
    const auto s = (float)getWidth() / kRefWidth;
    const auto E = showAdvanced ? kAdvancedExtra : 0.0f;
    const auto R = [s](float x, float y, float w, float h) { return juce::Rectangle<float>(x * s, y * s, w * s, h * s).toNearestInt(); };
    mode.setBounds(R(438, 80, 227, 40));
    advanced.setBounds(R(686, 72, 164, 54));

    // Knobs are placed by centre; the rotary square is the slider width.
    // The text box overlaps the lower edge of the rotary square.
    const auto placeKnob = [&](juce::Slider& sl, float cx, float cy, float size, float boxW, float boxH, float boxCentreY)
    {
        sl.setTextBoxStyle(juce::Slider::TextBoxBelow, false, (int)std::round(boxW * s), (int)std::round(boxH * s));
        const auto top = cy - size * 0.5f, bottom = boxCentreY + boxH * 0.5f;
        sl.setBounds(R(cx - size * 0.5f, top, size, bottom - top));
    };

    const auto showBands = showAdvanced && !showDynamics;
    for (int i = 0; i < 6; ++i)
    {
        const auto cx = 155.0f + (float)i * 221.2f;
        placeKnob(gainControls[(size_t)i], cx, 672.0f, 170.0f, 112.0f, 34.0f, 766.0f);
        freqControls[(size_t)i].setVisible(showBands);
        qControls[(size_t)i].setVisible(showBands);
        if (showBands)
        {
            placeKnob(freqControls[(size_t)i], cx - 56.0f, 955.0f, 104.0f, 92.0f, 28.0f, 1020.0f);
            placeKnob(qControls[(size_t)i], cx + 56.0f, 955.0f, 104.0f, 92.0f, 28.0f, 1020.0f);
        }
    }
    dynamicsPage.setVisible(showAdvanced);
    dynamicsPage.setBounds(R(1232, 824, 150, 34));
    for (int i = 0; i < 8; ++i)
    {
        dynamicsControls[(size_t)i].setVisible(showAdvanced && showDynamics);
        if (showAdvanced && showDynamics)
            placeKnob(dynamicsControls[(size_t)i], 100.0f + (float)i * 174.0f, 985.0f, 108.0f, 100.0f, 30.0f, 1048.0f);
    }
    for (int i = 0; i < 2; ++i)
    {
        dynamicsEnabled[(size_t)i].setVisible(showAdvanced && showDynamics);
        dynamicsEnabled[(size_t)i].setBounds(R(60.0f + (float)i * 330.0f, 866, 300, 36));
    }

    placeKnob(input, 115.0f, 925.0f + E, 145.0f, 108.0f, 34.0f, 1005.0f + E);
    placeKnob(output, 296.0f, 925.0f + E, 145.0f, 108.0f, 34.0f, 1005.0f + E);
    placeKnob(mix, 484.0f, 925.0f + E, 145.0f, 108.0f, 34.0f, 1005.0f + E);
    placeKnob(compression, 709.0f, 925.0f + E, 145.0f, 108.0f, 34.0f, 1005.0f + E);
    placeKnob(drive, 911.0f, 925.0f + E, 145.0f, 108.0f, 34.0f, 1005.0f + E);
    levelComp.setBounds(R(1117, 872.0f + E, 190, 120));

    if (chassis.isNull() || chassisWidth != getWidth() || chassisHeight != getHeight()) chassis = {};
}

void DRMRESHAPEAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    const auto s = (float)getWidth() / kRefWidth;
    const auto p = e.position / s;
    if (juce::Rectangle<float>(78, 186, 78, 36).contains(p)) showPre = !showPre;
    else if (juce::Rectangle<float>(166, 186, 90, 36).contains(p)) showPost = !showPost;
    else if (juce::Rectangle<float>(272, 186, 280, 36).contains(p)) showResponse = !showResponse;
    else return;
    repaint();
}

void DRMRESHAPEAudioProcessorEditor::rebuildChassis(float pixelScale)
{
    chassisScale = pixelScale; chassisWidth = getWidth(); chassisHeight = getHeight();
    chassis = juce::Image(juce::Image::ARGB, juce::jmax(1, (int)std::ceil((float)getWidth() * pixelScale)), juce::jmax(1, (int)std::ceil((float)getHeight() * pixelScale)), true);
    juce::Graphics g(chassis);
    g.addTransform(juce::AffineTransform::scale((float)chassis.getWidth() / kRefWidth, (float)chassis.getHeight() / refHeight()));
    drawStaticChassis(g);
}

void DRMRESHAPEAudioProcessorEditor::drawStaticChassis(juce::Graphics& g) const
{
    const auto H = refHeight(), E = showAdvanced ? kAdvancedExtra : 0.0f;
    // Aged charcoal chassis with fine speckle and a few scratches.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff1d1e20), 0, 0, juce::Colour(0xff111213), 0, H, false));
    g.fillRect(0.0f, 0.0f, kRefWidth, H);
    juce::Random rng(0x5eed);
    for (int i = 0; i < 9000; ++i)
    {
        g.setColour((rng.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha(rng.nextFloat() * 0.07f));
        g.fillRect(rng.nextFloat() * kRefWidth, rng.nextFloat() * H, 1.4f, 1.4f);
    }
    for (int i = 0; i < 40; ++i)
    {
        const auto x = rng.nextFloat() * kRefWidth, y = rng.nextFloat() * H, len = 20.0f + rng.nextFloat() * 90.0f;
        g.setColour(juce::Colours::white.withAlpha(0.025f + rng.nextFloat() * 0.025f));
        g.drawLine(x, y, x + len, y + (rng.nextFloat() - 0.5f) * 10.0f, 0.8f);
    }
    g.setColour(juce::Colours::black.withAlpha(0.9f)); g.drawRect(0.0f, 0.0f, kRefWidth, H, 4.0f);
    g.setColour(juce::Colour(0xff3a3b3b)); g.drawRect(4.0f, 4.0f, kRefWidth - 8.0f, H - 8.0f, 1.5f);

    const auto panel = [&](juce::Rectangle<float> r)
    {
        g.setColour(juce::Colours::black.withAlpha(0.8f)); g.fillRoundedRectangle(r.expanded(2.0f), 6.0f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff222426), r.getX(), r.getY(), juce::Colour(0xff17181a), r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle(r, 5.0f);
        g.setColour(juce::Colour(0xff3d3f40)); g.drawRoundedRectangle(r.reduced(0.5f), 5.0f, 1.2f);
        g.setColour(juce::Colours::white.withAlpha(0.05f)); g.drawLine(r.getX() + 6, r.getY() + 2, r.getRight() - 6, r.getY() + 2, 1.0f);
    };
    const auto sectionTitle = [&](const juce::String& t, float y, float x0, float x1)
    {
        const auto w = textWidth(font(23.0f).withExtraKerningFactor(0.05f), t);
        g.setColour(creamDim.withAlpha(0.55f));
        g.drawHorizontalLine((int)y, x0, x0 + 24.0f);
        g.drawHorizontalLine((int)y, x0 + 52.0f + w, x1);
        g.setColour(cream); g.setFont(font(23.0f).withExtraKerningFactor(0.05f));
        g.drawText(t, juce::Rectangle<float>(x0 + 30.0f, y - 14.0f, w + 20.0f, 28.0f), juce::Justification::centredLeft, false);
    };

    // Header.
    panel({ 14.0f, 36.0f, 1390.0f, 128.0f });
    drawRecess(g, { 30.0f, 48.0f, 836.0f, 106.0f }, 6.0f, juce::Colour(0xff1b1c1e));
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.setFont(font(50.0f).withExtraKerningFactor(0.03f)); g.drawText("DRM-RESHAPE", juce::Rectangle<float>(76.0f, 59.0f, 380.0f, 66.0f), juce::Justification::centredLeft, false);
    g.setColour(juce::Colour(0xffc9bc9a));
    g.drawText("DRM-RESHAPE", juce::Rectangle<float>(74.0f, 57.0f, 380.0f, 66.0f), juce::Justification::centredLeft, false);
    g.setColour(cream.withAlpha(0.9f)); g.setFont(font(16.0f, false).withExtraKerningFactor(0.1f));
    g.drawText("DRUM TONE SHAPER  /  YEAHWAAH AUDIO", juce::Rectangle<float>(74.0f, 122.0f, 380.0f, 22.0f), juce::Justification::centredLeft, false);

    // Meter housings: labels and scales are static, readouts are drawn live.
    const juce::String names[] { "INPUT / dB", "OUTPUT / dB", "GR / dB" };
    const juce::StringArray scaleIn { "-48", "-24", "-12", "0" }, scaleGr { "-24", "-12", "-6", "0" };
    const juce::Rectangle<float> meters[] { { 883, 46, 179, 116 }, { 1068, 46, 175, 116 }, { 1248, 46, 142, 116 } };
    for (int i = 0; i < 3; ++i)
    {
        const auto m = meters[i];
        drawRecess(g, m, 5.0f, juce::Colour(0xff0c0d0e));
        g.setColour(creamDim.withAlpha(0.5f)); g.drawRoundedRectangle(m.reduced(1.0f), 5.0f, 1.0f);
        drawTextCentred(g, names[i], { m.getX(), m.getY() + 8.0f, m.getWidth(), 24.0f }, 17.0f, cream);
        const auto& scale = i == 2 ? scaleGr : scaleIn;
        const float fr[] { 0.0f, 0.5f, 0.75f, 1.0f };
        for (int k = 0; k < 4; ++k)
            drawTextCentred(g, scale[k], juce::Rectangle<float>(44.0f, 18.0f).withCentre({ m.getX() + 16.0f + fr[k] * (m.getWidth() - 32.0f), m.getY() + m.getHeight() - 14.0f }), 14.0f, cream.withAlpha(0.9f), false, 0.0f);
    }

    // Analyzer bezel.
    panel({ 14.0f, 172.0f, 1390.0f, 352.0f });
    drawRecess(g, { 28.0f, 182.0f, 1362.0f, 332.0f }, 5.0f, juce::Colour(0xff0d0e0f));
    drawRecess(g, { 86.0f, 235.0f, 1184.0f, 247.0f }, 2.0f, blackFace);
    for (auto p : { juce::Point<float>(36, 194), { 1384, 194 }, { 36, 509 }, { 1384, 509 } }) drawScrew(g, p.x, p.y, 9.0f, 0.6f + p.x * 0.01f);

    // Equalizer / tone panel.
    panel({ 10.0f, 535.0f, 1398.0f, 277.0f });
    sectionTitle("EQUALIZER / TONE", 557.0f, 36.0f, 1384.0f);
    for (int i = 0; i < 6; ++i)
        drawTextCentred(g, labels[i], juce::Rectangle<float>(190.0f, 30.0f).withCentre({ 155.0f + (float)i * 221.2f, 590.0f }), 26.0f, cream);

    if (showAdvanced)
    {
        panel({ 10.0f, 818.0f, 1398.0f, E - 6.0f });
        sectionTitle(showDynamics ? "DYNAMIC PROCESSING" : "FREQUENCY / Q", 841.0f, 36.0f, 1384.0f);
        if (!showDynamics)
            for (int i = 0; i < 6; ++i)
            {
                const auto cx = 155.0f + (float)i * 221.2f;
                drawTextCentred(g, labels[i], juce::Rectangle<float>(160.0f, 22.0f).withCentre({ cx, 882.0f }), 20.0f, cream);
                drawTextCentred(g, "FREQ", juce::Rectangle<float>(90.0f, 18.0f).withCentre({ cx - 56.0f, 1050.0f }), 14.0f, creamDim);
                drawTextCentred(g, "Q", juce::Rectangle<float>(90.0f, 18.0f).withCentre({ cx + 56.0f, 1050.0f }), 14.0f, creamDim);
            }
        else
        {
            const juce::StringArray cap { "THRESHOLD", "ATTACK", "RELEASE", "MAX GR" };
            for (int i = 0; i < 8; ++i)
                drawTextCentred(g, juce::String(i < 4 ? "TIGHT " : "SMOOTH ") + cap[i % 4], juce::Rectangle<float>(170.0f, 22.0f).withCentre({ 100.0f + (float)i * 174.0f, 920.0f }), 15.0f, cream);
        }
    }

    // Input / dynamics / output panel.
    panel({ 10.0f, 818.0f + E, 1398.0f, 230.0f });
    const juce::String bottomNames[] { "INPUT", "OUTPUT", "MIX", "COMPRESSION", "DRIVE" };
    const float bottomX[] { 115, 296, 484, 709, 911 };
    for (int i = 0; i < 5; ++i)
        drawTextCentred(g, bottomNames[i], juce::Rectangle<float>(190.0f, 30.0f).withCentre({ bottomX[i], 852.0f + E }), 26.0f, cream);
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    for (const auto x : { 592.0f, 1017.0f })
    {
        g.fillRect(x, 846.0f + E, 2.0f, 176.0f);
        g.setColour(juce::Colours::white.withAlpha(0.07f)); g.fillRect(x + 2.0f, 846.0f + E, 1.0f, 176.0f);
        g.setColour(juce::Colours::black.withAlpha(0.8f));
    }

    // Footer.
    const auto fy = 1068.0f + E;
    const juce::String footer = "MODEL: DRM-01  /  MINIMUM PHASE  /  ZERO LATENCY  /  STEREO";
    const auto fw = textWidth(font(17.0f, false).withExtraKerningFactor(0.08f), footer);
    g.setColour(creamDim.withAlpha(0.5f));
    g.drawHorizontalLine((int)fy, 400.0f, kRefWidth * 0.5f - fw * 0.5f - 24.0f);
    g.drawHorizontalLine((int)fy, kRefWidth * 0.5f + fw * 0.5f + 24.0f, 1018.0f);
    g.setColour(cream.withAlpha(0.85f)); g.setFont(font(17.0f, false).withExtraKerningFactor(0.08f));
    g.drawText(footer, juce::Rectangle<float>(0.0f, fy - 12.0f, kRefWidth, 24.0f), juce::Justification::centred, false);
    for (auto p : { juce::Point<float>(32, 1088 + E), { 1386, 1088 + E }, { 32, 22 }, { 1386, 22 } }) drawScrew(g, p.x, p.y, 10.0f, 0.3f + p.y * 0.004f);

    // Analyzer scales (static).
    const double hzLabels[] { 20, 50, 100, 500, 1000, 5000, 10000, 20000 };
    const juce::StringArray hzText { "20", "50", "100", "500", "1k", "5k", "10k", "20k" };
    for (int i = 0; i < hzText.size(); ++i)
    {
        const auto x = 86.0f + (float)(std::log(hzLabels[i] / 20.0) / std::log(1000.0)) * 1184.0f;
        drawTextCentred(g, hzText[i], juce::Rectangle<float>(60.0f, 22.0f).withCentre({ juce::jlimit(112.0f, 1255.0f, x), 503.0f }), 18.0f, cream, true, 0.0f);
    }
    drawTextCentred(g, "IN", juce::Rectangle<float>(40.0f, 22.0f).withCentre({ 1297.0f, 226.0f }), 16.0f, amber);
    drawTextCentred(g, "OUT", juce::Rectangle<float>(50.0f, 22.0f).withCentre({ 1338.0f, 226.0f }), 16.0f, ember);
    for (int i = 0; i <= 5; ++i)
        drawTextCentred(g, juce::String(-12 * i), juce::Rectangle<float>(50.0f, 20.0f).withCentre({ 1378.0f, 250.0f + (float)i * 46.0f }), 17.0f, cream, true, 0.0f);
    const juce::StringArray eqScale { "+18", "0", "-18" };
    for (int i = 0; i < 3; ++i)
        drawTextCentred(g, eqScale[i], juce::Rectangle<float>(40.0f, 18.0f).withCentre({ 1247.0f, 235.0f + (float)i * 123.5f + (i == 0 ? 10.0f : i == 2 ? -10.0f : 0.0f) }), 15.0f, amber.withAlpha(0.9f), true, 0.0f);
    for (int i = 0; i <= 5; ++i)
        drawTextCentred(g, juce::String(-12 * i), juce::Rectangle<float>(60.0f, 20.0f).withCentre({ 54.0f, 235.0f + (float)i * 49.4f }), 17.0f, cream, true, 0.0f);
}

void DRMRESHAPEAudioProcessorEditor::drawMeters(juce::Graphics& g) const
{
    const auto inputDb = juce::Decibels::gainToDecibels(audioProcessor.getInputPeak(), -60.0f);
    const auto outputDb = juce::Decibels::gainToDecibels(audioProcessor.getOutputPeak(), -60.0f);
    const auto gr = audioProcessor.getCompressionReductionDb();
    const juce::Rectangle<float> meters[] { { 883, 46, 179, 116 }, { 1068, 46, 175, 116 }, { 1248, 46, 142, 116 } };
    const float readings[] { inputDb, outputDb, -gr };
    const float shown[] { heldReadings[0], heldReadings[1], -heldReadings[2] };
    for (int i = 0; i < 3; ++i)
    {
        const auto m = meters[i];
        const auto window = juce::Rectangle<float>(m.getX() + 12.0f, m.getY() + 36.0f, m.getWidth() - 24.0f, 46.0f);
        g.setColour(blackFace); g.fillRoundedRectangle(window, 3.0f);
        drawSevenSegmentReadout(g, window.reduced(6.0f, 3.0f), shown[i], redLed);

        // LED ladder: input/output fill from the left, gain reduction fills from the right.
        const auto bar = juce::Rectangle<float>(m.getX() + 16.0f, m.getY() + 87.0f, m.getWidth() - 32.0f, 9.0f);
        const auto frac = i == 2 ? juce::jlimit(0.0f, 1.0f, gr / 24.0f) : juce::jlimit(0.0f, 1.0f, (readings[i] + 48.0f) / 48.0f);
        constexpr int segments = 24;
        const auto segW = bar.getWidth() / (float)segments;
        for (int k = 0; k < segments; ++k)
        {
            const auto pos = (float)(k + 1) / (float)segments;
            const auto lit = i == 2 ? (1.0f - (float)k / (float)segments) <= frac + 1.0e-4f : pos <= frac + 1.0e-4f;
            g.setColour(lit ? (k > segments * 5 / 6 && i != 2 ? redLed : ember) : juce::Colour(0xff1f1210));
            g.fillRect(bar.getX() + (float)k * segW + 0.8f, bar.getY(), segW - 1.6f, bar.getHeight());
        }
    }
}

void DRMRESHAPEAudioProcessorEditor::drawAnalyzer(juce::Graphics& g) const
{
    constexpr float left = 86.0f, right = 1270.0f, top = 235.0f, bottom = 482.0f, width = right - left;
    const auto freqToX = [&](double hz) { return left + (float)(std::log(std::clamp(hz, 20.0, 20000.0) / 20.0) / std::log(1000.0)) * width; };
    g.setColour(gridLine);
    for (const double hz : { 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0 }) g.drawVerticalLine((int)freqToX(hz), top, bottom);
    for (int i = 0; i <= 5; ++i) g.drawHorizontalLine((int)(top + (float)i * (bottom - top) / 5.0f), left, right);

    const auto sampleRate = audioProcessor.getSampleRate() > 0.0 ? audioProcessor.getSampleRate() : 44100.0;
    const auto dbToY = [&](float db) { return bottom - (juce::jlimit(-60.0f, 0.0f, db) + 60.0f) * ((bottom - top) / 60.0f); };
    // Reads the analyzer bins covering [f0, f1]; narrower than one bin falls back to interpolation.
    const auto bandDb = [&](bool post, double f0, double f1)
    {
        const auto b0 = f0 * 2048.0 / sampleRate, b1 = f1 * 2048.0 / sampleRate;
        float magnitude = 0.0f;
        if (b1 - b0 < 1.0)
        {
            const auto centre = 0.5 * (b0 + b1);
            const auto lo = juce::jlimit(1, 1023, (int)std::floor(centre));
            const auto t = (float)juce::jlimit(0.0, 1.0, centre - (double)lo);
            magnitude = audioProcessor.getSpectrum(post, lo) * (1.0f - t) + audioProcessor.getSpectrum(post, lo + 1) * t;
        }
        else
            for (int bin = juce::jlimit(1, 1024, (int)std::ceil(b0)); bin <= juce::jlimit(1, 1024, (int)std::floor(b1)); ++bin)
                magnitude = juce::jmax(magnitude, audioProcessor.getSpectrum(post, bin));
        return juce::Decibels::gainToDecibels(magnitude, -72.0f);
    };
    constexpr int bars = 160;
    const auto barW = width / (float)bars;
    if (showPost)
    {
        const auto gradient = juce::ColourGradient(juce::Colour(0xff5b1209), 0.0f, bottom, juce::Colour(0xffff5a1c), 0.0f, top, false);
        for (int pass = 0; pass < 2; ++pass)
        {
            g.setGradientFill(gradient);
            g.setOpacity(pass == 0 ? 0.22f : 0.95f);
            for (int k = 0; k < bars; ++k)
            {
                const auto f0 = 20.0 * std::pow(1000.0, (double)k / bars), f1 = 20.0 * std::pow(1000.0, (double)(k + 1) / bars);
                const auto y = dbToY(bandDb(true, f0, f1));
                if (y >= bottom - 0.5f) continue;
                const auto inset = pass == 0 ? -1.2f : 0.45f;
                g.fillRect(left + (float)k * barW + inset, y - (pass == 0 ? 1.0f : 0.0f), barW - 2.0f * inset, bottom - y + (pass == 0 ? 1.0f : 0.0f));
            }
        }
    }
    if (showPre)
    {
        juce::Path trace;
        for (int k = 0; k < bars; ++k)
        {
            const auto f0 = 20.0 * std::pow(1000.0, (double)k / bars), f1 = 20.0 * std::pow(1000.0, (double)(k + 1) / bars);
            const auto y = dbToY(bandDb(false, f0, f1));
            if (k == 0) trace.startNewSubPath(left, y);
            trace.lineTo(left + ((float)k + 0.5f) * barW, y);
        }
        g.setColour(creamBright.withAlpha(0.8f)); g.strokePath(trace, juce::PathStrokeType(1.4f));
    }

    // Input/output peak ladders share the dB scale printed beside them.
    const auto ladder = [&](float x, float peakDb, juce::Colour colour)
    {
        constexpr int segments = 30;
        const auto frac = juce::jlimit(0.0f, 1.0f, (peakDb + 60.0f) / 60.0f);
        const auto segH = (480.0f - 250.0f) / (float)segments;
        for (int k = 0; k < segments; ++k)
        {
            const auto lit = (float)(segments - k) / (float)segments <= frac + 1.0e-4f;
            g.setColour(lit ? (k < 3 ? redLed : colour) : juce::Colour(0xff17120f));
            g.fillRect(x, 250.0f + (float)k * segH + 0.7f, 17.0f, segH - 1.4f);
        }
    };
    ladder(1288.0f, juce::Decibels::gainToDecibels(audioProcessor.getInputPeak(), -60.0f), amber);
    ladder(1326.0f, juce::Decibels::gainToDecibels(audioProcessor.getOutputPeak(), -60.0f), ember);
    if (showResponse)
    {
        const auto settings = audioProcessor.getDspSettings();
        juce::Path response;
        for (int x = 0; x <= 700; ++x)
        {
            const auto norm = x / 700.0, hz = 20.0 * std::pow(1000.0, norm);
            const auto db = juce::jlimit(-18.0f, 18.0f, drumshaper::DrumShaperDSP::responseAt(settings, hz));
            const auto px = left + (float)norm * width, py = (top + bottom) * 0.5f - db * ((bottom - top) / 36.0f);
            if (x == 0) response.startNewSubPath(px, py); else response.lineTo(px, py);
        }
        g.setColour(amber.withAlpha(0.3f)); g.strokePath(response, juce::PathStrokeType(5.0f));
        g.setColour(amber); g.strokePath(response, juce::PathStrokeType(2.2f));
    }

    const auto tab = [&](const juce::String& t, juce::Rectangle<float> r, bool active)
    {
        drawTextCentred(g, t, r, 20.0f, active ? redLed : creamDim, true, 0.05f);
        if (active) { g.setColour(redLed); g.fillRect(r.getX() + 4.0f, r.getBottom() - 2.0f, r.getWidth() - 8.0f, 2.5f); }
    };
    tab("PRE", { 82.0f, 190.0f, 70.0f, 28.0f }, showPre);
    tab("POST", { 170.0f, 190.0f, 82.0f, 28.0f }, showPost);
    tab("FREQUENCY RESPONSE", { 272.0f, 190.0f, 280.0f, 28.0f }, showResponse);
}

void DRMRESHAPEAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto scale = juce::jmax(1.0f, juce::Component::getApproximateScaleFactorForComponent(this));
    if (chassis.isNull() || chassisWidth != getWidth() || chassisHeight != getHeight() || ! juce::approximatelyEqual(chassisScale, scale))
        rebuildChassis(scale);
    g.drawImage(chassis, getLocalBounds().toFloat());
    g.addTransform(juce::AffineTransform::scale((float)getWidth() / kRefWidth, (float)getHeight() / refHeight()));
    drawMeters(g);
    drawAnalyzer(g);
}
