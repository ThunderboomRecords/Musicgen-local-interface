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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// AbletonLookAndFeel
//==============================================================================
AbletonLookAndFeel::AbletonLookAndFeel()
{
    auto typeface = juce::Typeface::createSystemTypefaceFor (
        SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
    customFont = juce::Font (juce::FontOptions (typeface).withHeight (14.0f));

    // Window / general
    setColour (juce::ResizableWindow::backgroundColourId, AbletonColours::background);

    // TextButton
    setColour (juce::TextButton::buttonColourId,      AbletonColours::bgDark);
    setColour (juce::TextButton::buttonOnColourId,     AbletonColours::accent);
    setColour (juce::TextButton::textColourOffId,      juce::Colours::white);
    setColour (juce::TextButton::textColourOnId,       AbletonColours::text);

    // TextEditor
    setColour (juce::TextEditor::backgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::TextEditor::textColourId,         AbletonColours::text);
    setColour (juce::TextEditor::outlineColourId,      AbletonColours::border);
    setColour (juce::TextEditor::focusedOutlineColourId, AbletonColours::accent);
    setColour (juce::TextEditor::highlightColourId,    AbletonColours::accent);
    setColour (juce::TextEditor::highlightedTextColourId, AbletonColours::text);
    setColour (juce::CaretComponent::caretColourId,    AbletonColours::text);

    // Label
    setColour (juce::Label::textColourId,              AbletonColours::textDim);
    setColour (juce::Label::textWhenEditingColourId,   AbletonColours::text);
    setColour (juce::Label::outlineWhenEditingColourId, AbletonColours::accent);

    // ListBox
    setColour (juce::ListBox::backgroundColourId,      AbletonColours::background);
    setColour (juce::ListBox::outlineColourId,         juce::Colours::transparentBlack);

    // Slider (rotary text box)
    setColour (juce::Slider::textBoxTextColourId,      AbletonColours::text);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,   juce::Colours::transparentBlack);
}

void AbletonLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y,
                                            int width, int height,
                                            float sliderPos,
                                            float rotaryStartAngle,
                                            float rotaryEndAngle,
                                            juce::Slider& slider)
{
    juce::ignoreUnused (slider);

    const float radius   = static_cast<float> (juce::jmin (width, height)) * 0.4f;
    const float centreX  = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
    const float centreY  = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float arcThickness = radius * 0.18f;

    // Background arc (full range)
    {
        juce::Path bgArc;
        bgArc.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (AbletonColours::border);
        g.strokePath (bgArc, juce::PathStrokeType (arcThickness,
                      juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    // Value arc (filled portion)
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    if (sliderPos > 0.0f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                                rotaryStartAngle, angle, true);
        g.setColour (AbletonColours::accent);
        g.strokePath (valueArc, juce::PathStrokeType (arcThickness,
                      juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    // Tick mark pointing inward at current position
    const float tickLength = radius * 0.5f;
    const float angleRad = angle - juce::MathConstants<float>::halfPi;
    const float outerX = centreX + (radius + arcThickness * 0.5f) * std::cos (angleRad);
    const float outerY = centreY + (radius + arcThickness * 0.5f) * std::sin (angleRad);
    const float innerX = centreX + (radius - tickLength) * std::cos (angleRad);
    const float innerY = centreY + (radius - tickLength) * std::sin (angleRad);
    g.setColour (AbletonColours::accent);
    g.drawLine (outerX, outerY, innerX, innerY, arcThickness * 0.4f);
}

void AbletonLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour&,
                                                 bool isHighlighted, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto baseColour = button.getToggleState() ? AbletonColours::accent : AbletonColours::bgDark;

    if (isDown)
        baseColour = baseColour.brighter (0.1f);
    else if (isHighlighted)
        baseColour = baseColour.brighter (0.05f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 8.0f);
}

void AbletonLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                          bool, bool)
{
    g.setFont (customFont);
    auto colour = button.getToggleState()
                    ? button.findColour (juce::TextButton::textColourOnId)
                    : button.findColour (juce::TextButton::textColourOffId);
    g.setColour (colour);
    g.drawText (button.getButtonText(), button.getLocalBounds(),
                juce::Justification::centred);
}

void AbletonLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height,
                                                     juce::TextEditor& editor)
{
    if (editor.getName() == "underline")
        return; // No filled background — underline style

    g.setColour (AbletonColours::inputBg);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (width),
                            static_cast<float> (height), 3.0f);
}

void AbletonLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height,
                                                  juce::TextEditor& editor)
{
    if (editor.getName() == "underline")
    {
        g.setColour (AbletonColours::border);
        g.drawLine (0.0f, static_cast<float> (height) - 0.5f,
                    static_cast<float> (width), static_cast<float> (height) - 0.5f, 1.0f);
        return;
    }

    g.setColour (AbletonColours::border);
    g.drawRoundedRectangle (0.5f, 0.5f, static_cast<float> (width) - 1.0f,
                            static_cast<float> (height) - 1.0f, 3.0f, 1.0f);
}

void AbletonLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.setColour (label.findColour (juce::Label::textColourId));
    g.setFont (customFont);
    g.drawFittedText (label.getText(), label.getLocalBounds(),
                      label.getJustificationType(), 1);
}

juce::Label* AbletonLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox (slider);
    label->setColour (juce::Label::textColourId, AbletonColours::text);
    label->setColour (juce::Label::textWhenEditingColourId, AbletonColours::text);
    label->setColour (juce::TextEditor::textColourId, AbletonColours::text);
    label->setColour (juce::TextEditor::backgroundColourId, AbletonColours::inputBg);
    label->setColour (juce::TextEditor::highlightColourId, AbletonColours::accent);
    label->setColour (juce::TextEditor::highlightedTextColourId, AbletonColours::text);
    label->setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    label->setColour (juce::TextEditor::focusedOutlineColourId, AbletonColours::accent);
    label->setColour (juce::Label::outlineWhenEditingColourId, AbletonColours::accent);
    label->onEditorShow = [label]
    {
        if (auto* editor = label->getCurrentTextEditor())
        {
            editor->setColour (juce::TextEditor::textColourId, AbletonColours::text);
            editor->setColour (juce::TextEditor::highlightedTextColourId, AbletonColours::text);
            editor->setColour (juce::TextEditor::highlightColourId, AbletonColours::accent);
            editor->applyColourToAllText (AbletonColours::text, true);
        }
    };
    return label;
}

//==============================================================================
// MusicGenVSTEditor
//==============================================================================
MusicGenVSTEditor::MusicGenVSTEditor (MusicGenVSTProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&abletonLnF);
    customFont = abletonLnF.customFont;

    // Upload File button
    uploadFileButton.setButtonText ("Upload File");
    addAndMakeVisible (uploadFileButton);

    // Prompt
    promptInput.setName ("underline");
    promptInput.setTextToShowWhenEmpty ("A hip hop style beat", AbletonColours::light1);
    promptInput.setFont (customFont);
    promptInput.setJustification (juce::Justification::centredLeft);
    addAndMakeVisible (promptInput);

    // Instrumentation
    instrumentationInput.setName ("underline");
    instrumentationInput.setTextToShowWhenEmpty ("Drums, adds a bassline", AbletonColours::light1);
    instrumentationInput.setFont (customFont);
    instrumentationInput.setJustification (juce::Justification::centredLeft);
    addAndMakeVisible (instrumentationInput);

    // Length
    lengthLabel.setText ("Length", juce::dontSendNotification);
    lengthLabel.setFont (customFont);
    addAndMakeVisible (lengthLabel);
    lengthInput.setFont (customFont);
    addAndMakeVisible (lengthInput);

    // BPM
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setFont (customFont);
    addAndMakeVisible (bpmLabel);
    bpmInput.setFont (customFont);
    addAndMakeVisible (bpmInput);

    // Samples
    samplesLabel.setText ("Samples", juce::dontSendNotification);
    samplesLabel.setFont (customFont);
    addAndMakeVisible (samplesLabel);
    samplesInput.setFont (customFont);
    addAndMakeVisible (samplesInput);

    // Generate button
    generateButton.setButtonText ("Generate");
    addAndMakeVisible (generateButton);

    // Sample list
    sampleList.setModel (&sampleListModel);
    sampleList.setOutlineThickness (0);
    sampleList.getViewport()->setScrollBarsShown (false, false);
    addAndMakeVisible (sampleList);

    // Advanced toggle
    advancedToggle.setButtonText ("Advanced Settings");
    advancedToggle.setClickingTogglesState (true);
    advancedToggle.onClick = [this]
    {
        advancedVisible = ! advancedVisible;
        double scale = static_cast<double> (getWidth()) / static_cast<double> (baseWidth);
        int targetH = advancedVisible ? baseHeight + advancedHeight : baseHeight;
        constrainer.setFixedAspectRatio (static_cast<double> (baseWidth) / static_cast<double> (targetH));
        constrainer.setMinimumSize (baseWidth / 2, targetH / 2);
        constrainer.setMaximumSize (static_cast<int> (baseWidth * 2.5), static_cast<int> (targetH * 2.5));
        setSize (static_cast<int> (baseWidth * scale), static_cast<int> (targetH * scale));
    };
    addAndMakeVisible (advancedToggle);

    // Temperature dial
    temperatureDial.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    temperatureDial.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    temperatureDial.setRange (0.0, 2.0, 0.01);
    temperatureDial.setValue (1.0);
    temperatureDial.setColour (juce::Slider::textBoxTextColourId, AbletonColours::text);
    temperatureDial.setColour (juce::Slider::textBoxBackgroundColourId, AbletonColours::inputBg);
    temperatureLabel.setText ("Temperature", juce::dontSendNotification);
    temperatureLabel.setFont (customFont);
    temperatureLabel.setJustificationType (juce::Justification::centred);
    addChildComponent (temperatureDial);
    addChildComponent (temperatureLabel);

    // Top K dial
    topKDial.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    topKDial.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    topKDial.setRange (1.0, 1000.0, 1.0);
    topKDial.setValue (250.0);
    topKDial.setColour (juce::Slider::textBoxTextColourId, AbletonColours::text);
    topKDial.setColour (juce::Slider::textBoxBackgroundColourId, AbletonColours::inputBg);
    topKLabel.setText ("Top K", juce::dontSendNotification);
    topKLabel.setFont (customFont);
    topKLabel.setJustificationType (juce::Justification::centred);
    addChildComponent (topKDial);
    addChildComponent (topKLabel);

    // Top P dial
    topPDial.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    topPDial.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    topPDial.setRange (0.0, 1.0, 0.01);
    topPDial.setValue (0.0);
    topPDial.setColour (juce::Slider::textBoxTextColourId, AbletonColours::text);
    topPDial.setColour (juce::Slider::textBoxBackgroundColourId, AbletonColours::inputBg);
    topPLabel.setText ("Top P", juce::dontSendNotification);
    topPLabel.setFont (customFont);
    topPLabel.setJustificationType (juce::Justification::centred);
    addChildComponent (topPDial);
    addChildComponent (topPLabel);

    // CFG dial
    cfgDial.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    cfgDial.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    cfgDial.setRange (1.0, 10.0, 1.0);
    cfgDial.setValue (3.0);
    cfgDial.setColour (juce::Slider::textBoxTextColourId, AbletonColours::text);
    cfgDial.setColour (juce::Slider::textBoxBackgroundColourId, AbletonColours::inputBg);
    cfgLabel.setText ("CFG", juce::dontSendNotification);
    cfgLabel.setFont (customFont);
    cfgLabel.setJustificationType (juce::Justification::centred);
    addChildComponent (cfgDial);
    addChildComponent (cfgLabel);

    // Resizable from 50% to 250% with list-height snapping
    constrainer.advancedVisiblePtr = &advancedVisible;
    constrainer.bw = baseWidth;
    constrainer.bh = baseHeight;
    constrainer.advH = advancedHeight;
    const int currentH = advancedVisible ? baseHeight + advancedHeight : baseHeight;
    constrainer.setFixedAspectRatio (static_cast<double> (baseWidth) / static_cast<double> (currentH));
    constrainer.setMinimumSize (baseWidth / 2, currentH / 2);
    constrainer.setMaximumSize (static_cast<int> (baseWidth * 2.5), static_cast<int> (currentH * 2.5));
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (baseWidth, baseHeight);
}

MusicGenVSTEditor::~MusicGenVSTEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void MusicGenVSTEditor::paint (juce::Graphics& g)
{
    g.fillAll (AbletonColours::background);

    const double scale = static_cast<double> (getWidth()) / static_cast<double> (baseWidth);
    const int pad = static_cast<int> (10 * scale);
    auto bounds = getLocalBounds().reduced (pad);
    const int innerPad = static_cast<int> (8 * scale);
    const int buttonH = static_cast<int> (31 * scale);
    const int labelH = static_cast<int> (23 * scale);
    const int inputH = static_cast<int> (31 * scale);
    const int margin = static_cast<int> (8 * scale);
    const int buttonMargin = static_cast<int> (12 * scale);

    const int leftContentH = buttonH + buttonMargin
                           + inputH + margin
                           + inputH + margin
                           + inputH
                           + buttonMargin + buttonH
                           + buttonMargin + buttonH;
    const int panelH = leftContentH + innerPad * 2;
    const int verticalGap = (bounds.getHeight() - panelH) / 2;
    bounds.removeFromTop (verticalGap);

    auto topArea = bounds.removeFromTop (panelH);

    // Left panel background
    auto leftBg = topArea.removeFromLeft (topArea.getWidth() / 2)
                         .withTrimmedRight (static_cast<int> (5 * scale));
    g.setColour (AbletonColours::surface);
    g.fillRoundedRectangle (leftBg.toFloat(), 4.0f);

    // Right panel background
    auto rightBg = topArea.withTrimmedLeft (static_cast<int> (5 * scale));
    g.setColour (AbletonColours::surface);
    g.fillRoundedRectangle (rightBg.toFloat(), 4.0f);

    // Advanced section background
    if (advancedVisible)
    {
        auto advBg = bounds.reduced (0, static_cast<int> (3 * scale));
        g.setColour (AbletonColours::surface);
        g.fillRoundedRectangle (advBg.toFloat(), 4.0f);
    }
}

void MusicGenVSTEditor::resized()
{
    const double scale = static_cast<double> (getWidth()) / static_cast<double> (baseWidth);
    const int pad = static_cast<int> (10 * scale);
    auto bounds = getLocalBounds().reduced (pad);
    const int margin = static_cast<int> (8 * scale);
    const int buttonMargin = static_cast<int> (12 * scale);
    const int buttonH = static_cast<int> (31 * scale);
    const int labelH = static_cast<int> (23 * scale);
    const int inputH = static_cast<int> (31 * scale);
    const int scaledAdvHeight = static_cast<int> (advancedHeight * scale);
    const int innerPad = static_cast<int> (8 * scale);

    // Calculate total left-panel content height so both panels match
    const int leftContentH = buttonH + buttonMargin          // Upload File + gap
                           + labelH + inputH + margin         // Prompt
                           + labelH + inputH + margin         // Instrumentation
                           + inputH                             // Number row
                           + buttonMargin + buttonH           // Generate
                           + buttonMargin + buttonH;          // Advanced toggle
    const int panelH = leftContentH + innerPad * 2;
    const int availableH = advancedVisible ? bounds.getHeight() - scaledAdvHeight : bounds.getHeight();
    const int verticalGap = (availableH - panelH) / 2;
    bounds.removeFromTop (verticalGap);

    // Reserve space for advanced section if visible
    auto topArea = bounds.removeFromTop (panelH);

    // Split into left (50%) and right (50%)
    auto leftPanel = topArea.removeFromLeft (topArea.getWidth() / 2);
    auto rightPanel = topArea;

    // --- Left panel ---
    auto left = leftPanel.withTrimmedRight (static_cast<int> (5 * scale)).reduced (innerPad);

    // Upload File button
    uploadFileButton.setBounds (left.removeFromTop (buttonH));
    left.removeFromTop (buttonMargin);

    // Prompt
    promptInput.setBounds (left.removeFromTop (inputH));
    left.removeFromTop (margin);

    // Instrumentation
    instrumentationInput.setBounds (left.removeFromTop (inputH));
    left.removeFromTop (margin);

    // Length / BPM / Samples row (label + input side-by-side)
    auto numberRow = left.removeFromTop (inputH);
    const int fieldWidth = (numberRow.getWidth() - margin * 2) / 3;

    auto lengthArea = numberRow.removeFromLeft (fieldWidth);
    lengthLabel.setBounds (lengthArea.removeFromLeft (lengthArea.getWidth() / 2));
    lengthInput.setBounds (lengthArea);
    numberRow.removeFromLeft (margin);

    auto bpmArea = numberRow.removeFromLeft (fieldWidth);
    bpmLabel.setBounds (bpmArea.removeFromLeft (bpmArea.getWidth() / 2));
    bpmInput.setBounds (bpmArea);
    numberRow.removeFromLeft (margin);

    samplesLabel.setBounds (numberRow.removeFromLeft (numberRow.getWidth() / 2));
    samplesInput.setBounds (numberRow);

    left.removeFromTop (buttonMargin);

    // Generate button
    generateButton.setBounds (left.removeFromTop (buttonH));
    left.removeFromTop (buttonMargin);

    // Advanced toggle
    advancedToggle.setBounds (left.removeFromTop (buttonH));

    // --- Right panel ---
    auto right = rightPanel.withTrimmedLeft (static_cast<int> (5 * scale)).reduced (innerPad);
    sampleList.setBounds (right);
    sampleList.setRowHeight (right.getHeight() / 10);

    // --- Advanced section (below both panels) ---
    if (advancedVisible)
    {
        temperatureDial.setVisible (true);
        temperatureLabel.setVisible (true);
        topKDial.setVisible (true);
        topKLabel.setVisible (true);
        topPDial.setVisible (true);
        topPLabel.setVisible (true);
        cfgDial.setVisible (true);
        cfgLabel.setVisible (true);

        auto advArea = bounds.reduced (static_cast<int> (5 * scale));
        const int dialWidth = advArea.getWidth() / 4;
        const int advLabelH = static_cast<int> (23 * scale);

        auto tempArea = advArea.removeFromLeft (dialWidth);
        temperatureLabel.setBounds (tempArea.removeFromTop (advLabelH));
        temperatureDial.setBounds (tempArea);

        auto topKArea = advArea.removeFromLeft (dialWidth);
        topKLabel.setBounds (topKArea.removeFromTop (advLabelH));
        topKDial.setBounds (topKArea);

        auto topPArea = advArea.removeFromLeft (dialWidth);
        topPLabel.setBounds (topPArea.removeFromTop (advLabelH));
        topPDial.setBounds (topPArea);

        cfgLabel.setBounds (advArea.removeFromTop (advLabelH));
        cfgDial.setBounds (advArea);
    }
    else
    {
        temperatureDial.setVisible (false);
        temperatureLabel.setVisible (false);
        topKDial.setVisible (false);
        topKLabel.setVisible (false);
        topPDial.setVisible (false);
        topPLabel.setVisible (false);
        cfgDial.setVisible (false);
        cfgLabel.setVisible (false);
    }
}
