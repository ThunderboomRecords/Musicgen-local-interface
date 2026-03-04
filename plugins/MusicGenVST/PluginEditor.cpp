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
MusicGenVSTEditor::MusicGenVSTEditor (MusicGenVSTProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Load Space Grotesk font
    auto typeface = juce::Typeface::createSystemTypefaceFor (
        SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
    auto fontOptions = juce::FontOptions (typeface).withHeight (14.0f);
    customFont = juce::Font (fontOptions);

    // Upload File button
    uploadFileButton.setButtonText ("Upload File");
    addAndMakeVisible (uploadFileButton);

    // Prompt
    promptLabel.setText ("Prompt", juce::dontSendNotification);
    promptLabel.setFont (customFont);
    addAndMakeVisible (promptLabel);
    promptInput.setFont (customFont);
    addAndMakeVisible (promptInput);

    // Instrumentation
    instrumentationLabel.setText ("Instrumentation", juce::dontSendNotification);
    instrumentationLabel.setFont (customFont);
    addAndMakeVisible (instrumentationLabel);
    instrumentationInput.setFont (customFont);
    addAndMakeVisible (instrumentationInput);

    // Length
    lengthLabel.setText ("Length", juce::dontSendNotification);
    lengthLabel.setFont (customFont);
    addAndMakeVisible (lengthLabel);
    lengthInput.setInputRestrictions (5, "0123456789.");
    lengthInput.setFont (customFont);
    addAndMakeVisible (lengthInput);

    // BPM
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setFont (customFont);
    addAndMakeVisible (bpmLabel);
    bpmInput.setInputRestrictions (6, "0123456789.");
    bpmInput.setFont (customFont);
    addAndMakeVisible (bpmInput);

    // Samples
    samplesLabel.setText ("Samples", juce::dontSendNotification);
    samplesLabel.setFont (customFont);
    addAndMakeVisible (samplesLabel);
    samplesInput.setInputRestrictions (2, "0123456789");
    samplesInput.setFont (customFont);
    addAndMakeVisible (samplesInput);

    // Generate button
    generateButton.setButtonText ("Generate");
    addAndMakeVisible (generateButton);

    // Sample list
    sampleList.setModel (&sampleListModel);
    addAndMakeVisible (sampleList);

    // Advanced toggle
    advancedToggle.setButtonText ("Advanced Settings");
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
}

//==============================================================================
void MusicGenVSTEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
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

    // Reserve space for advanced section if visible
    auto topArea = bounds.removeFromTop (advancedVisible ? bounds.getHeight() - scaledAdvHeight : bounds.getHeight());

    // Split into left (50%) and right (50%)
    auto leftPanel = topArea.removeFromLeft (topArea.getWidth() / 2);
    auto rightPanel = topArea;

    // --- Left panel ---
    auto left = leftPanel.withTrimmedRight (static_cast<int> (5 * scale));

    // Upload File button
    uploadFileButton.setBounds (left.removeFromTop (buttonH));
    left.removeFromTop (buttonMargin);

    // Prompt
    promptLabel.setBounds (left.removeFromTop (labelH));
    promptInput.setBounds (left.removeFromTop (inputH));
    left.removeFromTop (margin);

    // Instrumentation
    instrumentationLabel.setBounds (left.removeFromTop (labelH));
    instrumentationInput.setBounds (left.removeFromTop (inputH));
    left.removeFromTop (margin);

    // Length / BPM / Samples row
    auto numberRow = left.removeFromTop (labelH + inputH);
    const int fieldWidth = (numberRow.getWidth() - margin * 2) / 3;

    auto lengthArea = numberRow.removeFromLeft (fieldWidth);
    lengthLabel.setBounds (lengthArea.removeFromTop (labelH));
    lengthInput.setBounds (lengthArea);
    numberRow.removeFromLeft (margin);

    auto bpmArea = numberRow.removeFromLeft (fieldWidth);
    bpmLabel.setBounds (bpmArea.removeFromTop (labelH));
    bpmInput.setBounds (bpmArea);
    numberRow.removeFromLeft (margin);

    samplesLabel.setBounds (numberRow.removeFromTop (labelH));
    samplesInput.setBounds (numberRow);

    left.removeFromTop (buttonMargin);

    // Generate button
    generateButton.setBounds (left.removeFromTop (buttonH));
    left.removeFromTop (buttonMargin);

    // Advanced toggle
    advancedToggle.setBounds (left.removeFromTop (buttonH));

    // --- Right panel ---
    auto right = rightPanel.withTrimmedLeft (static_cast<int> (5 * scale));
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
