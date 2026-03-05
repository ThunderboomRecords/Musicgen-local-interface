/*
 * Copyright (C) 2026 Thunderboom Records
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "PluginProcessor.h"
#include "../MusicGen/src/space-grotesk.h"

//==============================================================================
// Ableton-style colour palette
namespace AbletonColours
{
    const juce::Colour dark         { 18, 18, 18 };        // #121212
    const juce::Colour grey1        { 35, 35, 37 };        // #232325
    const juce::Colour grey2        { 51, 51, 52 };        // #333334
    const juce::Colour light1       { 113, 113, 113 };     // #717171
    const juce::Colour light2       { 162, 162, 162 };     // #A2A2A2
    const juce::Colour bgGrey       { 153, 153, 153 };     // #999999
    const juce::Colour bgDark       { 44, 44, 44 };        // #2C2C2C
    const juce::Colour bgHighlight  { 54, 54, 54 };        // #363636
    const juce::Colour highlight    { 237, 254, 88 };      // #EDFE58
    const juce::Colour highlightDim { 100, 107, 37 };      // dimmed highlight for selection
    const juce::Colour numBoxColor  { 160, 160, 160 };     // #A0A0A0
    const juce::Colour sampleColor1 { 140, 140, 140 };     // #8C8C8C
    const juce::Colour sampleColor2 { 160, 160, 160 };     // #A0A0A0
    const juce::Colour text         { 0, 0, 0 };           // #000000

    // Semantic aliases
    const juce::Colour background   = bgGrey;
    const juce::Colour surface      = bgGrey;
    const juce::Colour surfaceLight = bgGrey;
    const juce::Colour border       = bgHighlight;
    const juce::Colour textPrimary  = text;
    const juce::Colour textDim      = text;
    const juce::Colour inputBg      = numBoxColor;
    const juce::Colour accent       = highlight;
    const juce::Colour accentDim    = highlightDim;
    const juce::Colour rowEven      = sampleColor1;
    const juce::Colour rowOdd       = sampleColor2;
}

//==============================================================================
class AbletonLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    AbletonLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    void drawTextEditorOutline (juce::Graphics&, int width, int height,
                                juce::TextEditor&) override;

    void fillTextEditorBackground (juce::Graphics&, int width, int height,
                                   juce::TextEditor&) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;

    juce::Label* createSliderTextBox (juce::Slider&) override;

    juce::Font customFont { juce::FontOptions {} };
};

//==============================================================================
class SampleListModel final : public juce::ListBoxModel
{
public:
    int getNumRows() override { return 10; }

    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override
    {
        g.fillAll (rowNumber % 2 == 0 ? AbletonColours::rowEven : AbletonColours::rowOdd);

        if (rowIsSelected)
            g.fillAll (AbletonColours::accent);

        g.setColour (AbletonColours::text);
        g.drawText (juce::String (rowNumber + 1), 8, 0, width - 8, height,
                    juce::Justification::centredLeft);
    }
};

//==============================================================================
class ListSnapConstrainer final : public juce::ComponentBoundsConstrainer
{
public:
    bool* advancedVisiblePtr = nullptr;
    int bw = 728, bh = 330, advH = 169;

    void checkBounds (juce::Rectangle<int>& bounds,
                      const juce::Rectangle<int>& old,
                      const juce::Rectangle<int>& limits,
                      bool isStretchingTop, bool isStretchingLeft,
                      bool isStretchingBottom, bool isStretchingRight) override
    {
        ComponentBoundsConstrainer::checkBounds (bounds, old, limits,
            isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);

        bool adv = (advancedVisiblePtr != nullptr) && *advancedVisiblePtr;
        int w = bounds.getWidth();
        int h = bounds.getHeight();
        double scale = static_cast<double> (w) / static_cast<double> (bw);
        int pad = static_cast<int> (10.0 * scale);
        int scaledAdvH = adv ? static_cast<int> (advH * scale) : 0;
        int listH = h - 2 * pad - scaledAdvH;
        int remainder = listH % 10;

        if (remainder != 0)
        {
            h -= remainder;
            int targetBaseH = adv ? bh + advH : bh;
            double ar = static_cast<double> (bw) / static_cast<double> (targetBaseH);
            w = static_cast<int> (h * ar);
            bounds.setSize (w, h);
        }
    }
};

//==============================================================================
class NumberBox final : public juce::Component
{
public:
    NumberBox (double minVal, double maxVal, double defaultVal, int decimals)
        : minimum (minVal), maximum (maxVal), value (defaultVal), numDecimals (decimals)
    {
        editor.setFont (juce::FontOptions (14.0f));
        editor.setJustification (juce::Justification::centred);
        editor.setMultiLine (false);
        editor.setScrollbarsShown (false);

        if (decimals > 0)
            editor.setInputRestrictions (10, "0123456789.");
        else
            editor.setInputRestrictions (10, "0123456789");

        editor.setColour (juce::TextEditor::backgroundColourId, AbletonColours::inputBg);
        editor.setColour (juce::TextEditor::textColourId, AbletonColours::text);
        editor.setColour (juce::TextEditor::outlineColourId, AbletonColours::border);
        editor.setColour (juce::TextEditor::focusedOutlineColourId, AbletonColours::accent);
        editor.setColour (juce::TextEditor::highlightColourId, AbletonColours::accent);
        editor.setColour (juce::TextEditor::highlightedTextColourId, AbletonColours::text);
        editor.setColour (juce::CaretComponent::caretColourId, AbletonColours::text);

        updateText();
        addChildComponent (editor);

        editor.onReturnKey = [this] { commitEdit(); };
        editor.onEscapeKey = [this] { cancelEdit(); };
        editor.onFocusLost = [this] { commitEdit(); };
    }

    void setFont (const juce::Font& f) { font = f; editor.setFont (f); }

    double getValue() const { return value; }

    void setValue (double newValue)
    {
        value = juce::jlimit (minimum, maximum, newValue);
        updateText();
        repaint();
    }

    std::function<void()> onValueChange;

    void paint (juce::Graphics& g) override
    {
        if (editor.isVisible())
            return;

        auto bounds = getLocalBounds().toFloat();
        g.setColour (AbletonColours::inputBg);
        g.fillRoundedRectangle (bounds, 3.0f);
        g.setColour (AbletonColours::border);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

        g.setColour (AbletonColours::text);
        g.setFont (font);
        g.drawText (formatValue(), getLocalBounds(), juce::Justification::centred);
    }

    void resized() override
    {
        editor.setBounds (getLocalBounds());
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.getNumberOfClicks() >= 2)
        {
            startEdit();
            return;
        }
        dragStartValue = value;
        dragStartY = e.getPosition().getY();
        dragging = false;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        dragging = true;
        double sensitivity = (numDecimals > 0) ? 0.01 : 1.0;
        double delta = static_cast<double> (dragStartY - e.getPosition().getY()) * sensitivity;
        double range = maximum - minimum;
        if (range > 100.0 && numDecimals == 0)
            sensitivity = juce::jmax (1.0, range / 200.0);
        else if (range > 10.0 && numDecimals > 0)
            sensitivity = range / 500.0;

        delta = static_cast<double> (dragStartY - e.getPosition().getY()) * sensitivity;
        setValue (dragStartValue + delta);
        if (onValueChange)
            onValueChange();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragging = false;
    }

    juce::MouseCursor getMouseCursor() override
    {
        return juce::MouseCursor::UpDownResizeCursor;
    }

private:
    double minimum, maximum, value;
    int numDecimals;
    juce::Font font { juce::FontOptions (14.0f) };
    juce::TextEditor editor;
    double dragStartValue = 0.0;
    int dragStartY = 0;
    bool dragging = false;

    juce::String formatValue() const
    {
        if (numDecimals > 0)
            return juce::String (value, numDecimals);
        return juce::String (static_cast<int> (value));
    }

    void updateText()
    {
        editor.setText (formatValue(), juce::dontSendNotification);
    }

    void startEdit()
    {
        editor.setVisible (true);
        editor.setText (formatValue(), juce::dontSendNotification);
        editor.selectAll();
        editor.grabKeyboardFocus();
        repaint();
    }

    void commitEdit()
    {
        double parsed = editor.getText().getDoubleValue();
        setValue (parsed);
        editor.setVisible (false);
        repaint();
        if (onValueChange)
            onValueChange();
    }

    void cancelEdit()
    {
        editor.setVisible (false);
        updateText();
        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumberBox)
};

//==============================================================================
class MusicGenVSTEditor final : public juce::AudioProcessorEditor
{
public:
    explicit MusicGenVSTEditor (MusicGenVSTProcessor&);
    ~MusicGenVSTEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MusicGenVSTProcessor& processorRef;
    AbletonLookAndFeel abletonLnF;
    juce::Font customFont { juce::FontOptions {} };

    // Left panel
    juce::TextButton uploadFileButton;
    juce::Label promptLabel;
    juce::TextEditor promptInput;
    juce::Label instrumentationLabel;
    juce::TextEditor instrumentationInput;
    juce::Label lengthLabel;
    NumberBox lengthInput { 0.25, 30.0, 8.0, 2 };
    juce::Label bpmLabel;
    NumberBox bpmInput { 1.0, 300.0, 120.0, 2 };
    juce::Label samplesLabel;
    NumberBox samplesInput { 1.0, 10.0, 1.0, 0 };
    juce::TextButton generateButton;

    // Right panel
    SampleListModel sampleListModel;
    juce::ListBox sampleList;

    // Advanced section (collapsible)
    juce::TextButton advancedToggle;
    bool advancedVisible = false;

    juce::Slider temperatureDial;
    juce::Label temperatureLabel;
    juce::Slider topKDial;
    juce::Label topKLabel;
    juce::Slider topPDial;
    juce::Label topPLabel;
    juce::Slider cfgDial;
    juce::Label cfgLabel;

    static constexpr int baseWidth = 728;
    static constexpr int baseHeight = 350;
    static constexpr int advancedHeight = 85;

    ListSnapConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MusicGenVSTEditor)
};
