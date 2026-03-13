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

    if (button.getComponentID() == "uploadFileButton")
    {
        // Transparent background with dashed outline
        auto dashColour = AbletonColours::bgDark;
        if (isDown)
            dashColour = dashColour.brighter (0.15f);
        else if (isHighlighted)
            dashColour = dashColour.brighter (0.08f);

        juce::Path outline;
        outline.addRoundedRectangle (bounds, 8.0f);

        float dashLengths[] = { 6.0f, 4.0f };
        juce::PathStrokeType strokeType (1.5f);
        juce::Path dashedPath;
        strokeType.createDashedStroke (dashedPath, outline, dashLengths, 2);

        g.setColour (dashColour);
        g.fillPath (dashedPath);
        return;
    }

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

    if (button.getComponentID() == "uploadFileButton")
    {
        g.setColour (juce::Colours::black);

        auto bounds = button.getLocalBounds();
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText (customFont, button.getButtonText(), 0.0f, 0.0f);
        auto textWidth = static_cast<int> (std::ceil (glyphs.getBoundingBox (0, -1, false).getWidth()));
        float iconSize = customFont.getHeight();
        float gap = 6.0f;
        float totalWidth = static_cast<float> (textWidth) + gap + iconSize;
        float startX = (static_cast<float> (bounds.getWidth()) - totalWidth) * 0.5f;

        // Draw text
        g.drawText (button.getButtonText(),
                    static_cast<int> (startX), 0,
                    textWidth, bounds.getHeight(),
                    juce::Justification::centredLeft);

        // Draw down-arrow icon (tray with arrow pointing down)
        float iconX = startX + static_cast<float> (textWidth) + gap;
        float iconY = (static_cast<float> (bounds.getHeight()) - iconSize) * 0.5f;

        // Arrow pointing down
        float arrowCentreX = iconX + iconSize * 0.5f;
        float arrowTop = iconY + iconSize * 0.1f;
        float arrowBottom = iconY + iconSize * 0.6f;
        float arrowWidth = iconSize * 0.35f;

        // Vertical stem
        g.drawLine (arrowCentreX, arrowTop, arrowCentreX, arrowBottom, 1.5f);

        // Arrow head
        juce::Path arrowHead;
        arrowHead.startNewSubPath (arrowCentreX - arrowWidth, arrowBottom - arrowWidth * 0.7f);
        arrowHead.lineTo (arrowCentreX, arrowBottom);
        arrowHead.lineTo (arrowCentreX + arrowWidth, arrowBottom - arrowWidth * 0.7f);
        g.strokePath (arrowHead, juce::PathStrokeType (1.5f));

        // Tray (U shape at bottom)
        float trayTop = iconY + iconSize * 0.65f;
        float trayBottom = iconY + iconSize * 0.9f;
        float trayLeft = iconX + iconSize * 0.1f;
        float trayRight = iconX + iconSize * 0.9f;

        juce::Path tray;
        tray.startNewSubPath (trayLeft, trayTop);
        tray.lineTo (trayLeft, trayBottom);
        tray.lineTo (trayRight, trayBottom);
        tray.lineTo (trayRight, trayTop);
        g.strokePath (tray, juce::PathStrokeType (1.5f));

        return;
    }

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
// SampleListModel
//==============================================================================
int SampleListModel::getNumRows()
{
    return 10; // Always show 10 sample slots
}

void SampleListModel::paintListBoxItem (int rowNumber, juce::Graphics& g,
                                         int width, int height, bool rowIsSelected)
{
    // Original visual style: alternating row colours with fillAll
    g.fillAll (rowNumber % 2 == 0 ? AbletonColours::rowEven : AbletonColours::rowOdd);

    const int numGenerated = (processor != nullptr) ? processor->getNumGeneratedSamples() : 0;
    const bool hasContent = rowNumber < numGenerated;

    // Playing highlight overrides row colour
    if (hasContent && processor != nullptr && processor->isPlayingSample (rowNumber))
        g.fillAll (AbletonColours::accent);
    else if (rowIsSelected && hasContent)
        g.fillAll (AbletonColours::accent);

    g.setColour (AbletonColours::text);

    if (hasContent)
    {
        auto name = processor->getGeneratedSampleName (rowNumber);
        if (name.isEmpty())
            name = "Sample " + juce::String (rowNumber + 1);

        g.drawText (juce::String (rowNumber + 1) + "  " + name, 8, 0, width - 8, height,
                    juce::Justification::centredLeft);
    }
    else
    {
        g.drawText (juce::String (rowNumber + 1), 8, 0, width - 8, height,
                    juce::Justification::centredLeft);
    }
}

void SampleListModel::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (processor == nullptr)
        return;

    // Ignore clicks on empty slots
    if (row >= processor->getNumGeneratedSamples())
        return;

    // Toggle: click same row again to stop
    if (processor->isPlayingSample (row))
        processor->stopPlayback();
    else
        processor->playGeneratedSample (row);

    if (listBox != nullptr)
    {
        listBox->repaint();

        // Start a timer on the editor to update the playing highlight
        if (auto* editor = dynamic_cast<MusicGenVSTEditor*> (listBox->getParentComponent()))
            editor->startPlaybackTimer();
    }
}

juce::var SampleListModel::getDragSourceDescription (const juce::SparseSet<int>& selectedRows)
{
    if (processor == nullptr || selectedRows.isEmpty())
        return {};

    int row = selectedRows[0];
    if (row >= processor->getNumGeneratedSamples())
        return {};
    auto file = processor->getGeneratedSampleFile (row);
    if (file.existsAsFile())
    {
        // Trigger an OS-level file drag so DAWs can accept the drop
        juce::StringArray files;
        files.add (file.getFullPathName());
        juce::DragAndDropContainer::performExternalDragDropOfFiles (files, false);
        return {};  // cancel the internal drag — external drag takes over
    }

    return {};
}

//==============================================================================
// Helper to set up a rotary dial
//==============================================================================
static void setupDial (juce::Slider& dial, juce::Label& label, const juce::String& name,
                       double min, double max, double interval, double defaultVal,
                       const juce::Font& font, juce::Component* parent)
{
    dial.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    dial.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    dial.setRange (min, max, interval);
    dial.setValue (defaultVal);
    dial.setColour (juce::Slider::textBoxTextColourId, AbletonColours::text);
    dial.setColour (juce::Slider::textBoxBackgroundColourId, AbletonColours::inputBg);
    label.setText (name, juce::dontSendNotification);
    label.setFont (font);
    label.setJustificationType (juce::Justification::centred);
    parent->addChildComponent (dial);
    parent->addChildComponent (label);
}

//==============================================================================
// Prompt parsing helpers — extract keyscale, time signature, vocal language
//==============================================================================
static juce::String extractKeyscale (const juce::String& text)
{
    auto lower = text.toLowerCase();
    const char* notes[] = { "a", "b", "c", "d", "e", "f", "g" };
    const char* accs[]  = { "#", "b", "" };
    const char* modes[] = {
        "harmonic minor", "melodic minor",  // check longer modes first
        "major", "minor", "dorian", "phrygian", "lydian", "mixolydian",
        "aeolian", "locrian", "chromatic", "blues", "pentatonic"
    };

    for (auto n : notes)
        for (auto a : accs)
            for (auto m : modes)
            {
                auto key = juce::String (n) + a + " " + m;
                if (lower.contains (key))
                {
                    // Capitalise note for the request: "C minor" not "c minor"
                    auto result = key;
                    result = result.substring (0, 1).toUpperCase() + result.substring (1);
                    return result;
                }
            }

    return {};
}

static juce::String extractTimeSignature (const juce::String& text)
{
    auto lower = text.toLowerCase();

    // Match patterns like "3/4", "6/8", "4/4", "2/4" etc.
    const char* sigs[] = { "6/", "3/", "2/", "4/" };
    const char* vals[] = { "6",  "3",  "2",  "4"  };
    for (int i = 0; i < 4; ++i)
        if (lower.contains (sigs[i]))
            return vals[i];

    // Also match "waltz" → 3
    if (lower.contains ("waltz"))
        return "3";

    return {};
}

static juce::String buildNegativePrompt (const juce::String& text)
{
    auto lower = text.toLowerCase();
    juce::StringArray negatives;

    // If instrumental (no vocals requested), suppress vocal-related content
    bool hasVocals = lower.contains ("vocal") || lower.contains ("sing")
                  || lower.contains ("voice") || lower.contains ("rap")
                  || lower.contains ("choir");
    if (! hasVocals)
        negatives.addArray ({ "vocals", "singing", "voice" });

    // Suppress instruments/qualities not mentioned — contrast what the user wants
    struct NegRule { const char* keyword; std::initializer_list<const char*> suppress; };
    static const NegRule rules[] = {
        { "techno",    { "soft", "ambient", "slow", "acoustic", "guitar" } },
        { "ambient",   { "drums", "percussion", "loud", "distortion" } },
        { "acoustic",  { "synth", "electronic", "distortion", "808" } },
        { "electronic",{ "acoustic", "guitar", "strings" } },
        { "metal",     { "soft", "ambient", "piano", "flute" } },
        { "jazz",      { "distortion", "808", "heavy" } },
        { "classical", { "drums", "808", "distortion", "synth" } },
        { "hip hop",   { "guitar", "strings", "flute", "classical" } },
        { "rock",      { "soft", "ambient", "flute", "harp" } },
        { "piano",     { "guitar", "drums", "bass", "synth" } },
        { "guitar",    { "piano", "synth", "electronic" } },
    };

    for (auto& rule : rules)
        if (lower.contains (rule.keyword))
            for (auto s : rule.suppress)
                if (! lower.contains (s) && ! negatives.contains (s))
                    negatives.add (s);

    // Always add general quality negatives
    if (! negatives.contains ("noise"))
        negatives.add ("noise");
    if (! negatives.contains ("distorted"))
        negatives.add ("distorted");

    return negatives.joinIntoString (", ");
}

static juce::String extractVocalLanguage (const juce::String& text)
{
    auto lower = text.toLowerCase();

    struct LangMap { const char* keyword; const char* code; };
    static const LangMap langs[] = {
        { "english",    "en" }, { "chinese",    "zh" }, { "japanese",   "ja" },
        { "korean",     "ko" }, { "spanish",    "es" }, { "french",     "fr" },
        { "german",     "de" }, { "ukrainian",  "uk" }, { "russian",    "ru" },
        { "portuguese", "pt" }, { "italian",    "it" }, { "arabic",     "ar" },
        { "turkish",    "tr" }, { "polish",     "pl" }, { "swedish",    "sv" },
        { "dutch",      "nl" },
    };

    for (auto& l : langs)
        if (lower.contains (l.keyword))
            return l.code;

    return {};
}

//==============================================================================
// MusicGenVSTEditor
//==============================================================================
MusicGenVSTEditor::MusicGenVSTEditor (MusicGenVSTProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel (&abletonLnF);
    customFont = abletonLnF.customFont;

    // Upload Audio File button
    uploadFileButton.setButtonText ("Upload Audio File");
    uploadFileButton.setComponentID ("uploadFileButton");
    addAndMakeVisible (uploadFileButton);

    // Prompt (caption)
    promptInput.setName ("underline");
    promptInput.setTextToShowWhenEmpty ("A hip hop style beat", AbletonColours::light1);
    promptInput.setFont (customFont);
    promptInput.setJustification (juce::Justification::centredLeft);
    addAndMakeVisible (promptInput);

    // Instrumentation
    instrumentInput.setName ("underline");
    instrumentInput.setTextToShowWhenEmpty ("drums, guitar, bass, vocals", AbletonColours::light1);
    instrumentInput.setFont (customFont);
    instrumentInput.setJustification (juce::Justification::centredLeft);
    instrumentInput.setMultiLine (false);
    addAndMakeVisible (instrumentInput);

    // Length (duration in seconds)
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
    generateButton.onClick = [this]
    {
        if (processorRef.isGenerating())
            return;

        AceStepParams params;
        auto instruments = instrumentInput.getText();
        if (instruments.isNotEmpty())
            params.caption = promptInput.getText() + ", " + instruments;
        else
            params.caption = promptInput.getText();

        auto instrumentsLower = instruments.toLowerCase();
        bool hasVocals = instrumentsLower.contains ("vocal")
                      || instrumentsLower.contains ("sing")
                      || instrumentsLower.contains ("voice")
                      || instrumentsLower.contains ("rap")
                      || instrumentsLower.contains ("choir");
        params.lyrics = hasVocals ? "" : "[Instrumental]";
        params.bpm = static_cast<int> (bpmInput.getValue());
        params.duration = static_cast<int> (lengthInput.getValue());
        params.numSamples = static_cast<int> (samplesInput.getValue());

        // Extract keyscale, time signature, vocal language from prompt/instrumentation
        auto fullText = promptInput.getText() + " " + instruments;
        auto ks = extractKeyscale (fullText);
        if (ks.isNotEmpty())
            params.keyscale = ks;
        auto ts = extractTimeSignature (fullText);
        if (ts.isNotEmpty())
            params.timesignature = ts;
        auto vl = extractVocalLanguage (fullText);
        if (vl.isNotEmpty())
            params.vocalLanguage = vl;

        // Auto-generate negative prompt based on what user asked for
        params.negativePrompt = buildNegativePrompt (fullText);

        // Advanced params
        params.lmTemperature = static_cast<float> (temperatureDial.getValue());
        params.lmCfgScale = static_cast<float> (cfgScaleDial.getValue());
        params.lmTopP = static_cast<float> (topPDial.getValue());
        params.lmTopK = static_cast<int> (topKDial.getValue());

        processorRef.generateAsync (params);
        startTimerHz (10);
    };
    addAndMakeVisible (generateButton);

    // Sample list
    sampleListModel.processor = &processorRef;
    sampleListModel.listBox = &sampleList;
    sampleList.setModel (&sampleListModel);
    sampleList.setOutlineThickness (0);
    sampleList.getViewport()->setScrollBarsShown (false, false);
    sampleList.setMultipleSelectionEnabled (false);
    addAndMakeVisible (sampleList);

    // Status label
    statusLabel.setText ("", juce::dontSendNotification);
    statusLabel.setFont (customFont);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setColour (juce::Label::textColourId, AbletonColours::text);
    addAndMakeVisible (statusLabel);

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

    // --- Advanced section components (4 dials) ---
    setupDial (temperatureDial, temperatureLabel, "Temperature", 0.0, 2.0, 0.01, 0.8, customFont, this);
    setupDial (cfgScaleDial, cfgScaleLabel, "CFG Scale", 0.0, 10.0, 0.1, 7.0, customFont, this);
    setupDial (topPDial, topPLabel, "Top P", 0.0, 1.0, 0.01, 0.9, customFont, this);
    setupDial (topKDial, topKLabel, "Top K", 0.0, 1000.0, 1.0, 0.0, customFont, this);

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
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void MusicGenVSTEditor::timerCallback()
{
    // Repaint sample list to update playing highlight
    sampleList.repaint();

    if (processorRef.isGenerating())
    {
        auto status = processorRef.getGenerationStatus();
        statusLabel.setText (status, juce::dontSendNotification);
        statusLabel.setColour (juce::Label::textColourId, AbletonColours::accent);
        generateButton.setButtonText ("Generating...");
        return;
    }

    // Not generating — check if we just finished
    auto error = processorRef.getGenerationError();
    if (error.isNotEmpty())
    {
        statusLabel.setText (error, juce::dontSendNotification);
        statusLabel.setColour (juce::Label::textColourId, juce::Colours::red);
    }
    else if (generateButton.getButtonText() == "Generating...")
    {
        statusLabel.setText ("Done!", juce::dontSendNotification);
        statusLabel.setColour (juce::Label::textColourId, AbletonColours::text);
    }

    generateButton.setButtonText ("Generate");
    sampleList.updateContent();

    // Stop timer if nothing is playing either
    bool anyPlaying = false;
    for (int i = 0; i < processorRef.getNumGeneratedSamples(); ++i)
    {
        if (processorRef.isPlayingSample (i))
        {
            anyPlaying = true;
            break;
        }
    }
    if (! anyPlaying)
        stopTimer();
}

void MusicGenVSTEditor::startPlaybackTimer()
{
    if (! isTimerRunning())
        startTimerHz (10);
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
    juce::ignoreUnused (labelH);
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
                           + inputH                           // Number row
                           + buttonMargin + buttonH           // Generate
                           + buttonMargin + buttonH;          // Advanced toggle
    const int panelH = leftContentH + innerPad * 2;
    const int availableH = advancedVisible ? bounds.getHeight() - scaledAdvHeight : bounds.getHeight();
    const int verticalGap = (availableH - panelH) / 2;
    bounds.removeFromTop (verticalGap);

    // Reserve space for main panels
    auto topArea = bounds.removeFromTop (panelH);

    // Split into left (50%) and right (50%)
    auto leftPanel = topArea.removeFromLeft (topArea.getWidth() / 2);
    auto rightPanel = topArea;

    // --- Left panel ---
    auto left = leftPanel.withTrimmedRight (static_cast<int> (5 * scale)).reduced (innerPad);

    int bottomContentH = buttonH                              // Advanced toggle
                       + buttonMargin + buttonH               // Generate
                       + buttonMargin + inputH                // Number row
                       + margin + inputH                      // Instrumentation
                       + margin + inputH                      // Prompt
                       + buttonMargin;                        // gap after upload button

    int uploadH = left.getHeight() - bottomContentH;
    if (uploadH < buttonH)
        uploadH = buttonH;

    // Upload Audio File button
    uploadFileButton.setBounds (left.removeFromTop (uploadH));
    left.removeFromTop (buttonMargin);

    // Prompt (caption)
    promptInput.setBounds (left.removeFromTop (inputH));
    left.removeFromTop (margin);

    // Instrumentation
    instrumentInput.setBounds (left.removeFromTop (inputH));
    left.removeFromTop (margin);

    // Length / BPM / Samples row
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

    // Status label between panels and advanced section
    statusLabel.setBounds (bounds.removeFromTop (static_cast<int> (16 * scale)));

    // --- Advanced section ---
    auto setAdvVisible = [](bool vis, auto&... components)
    {
        (components.setVisible (vis), ...);
    };

    if (advancedVisible)
    {
        setAdvVisible (true,
            temperatureDial, temperatureLabel, topKDial, topKLabel,
            topPDial, topPLabel, cfgScaleDial, cfgScaleLabel);

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

        cfgScaleLabel.setBounds (advArea.removeFromTop (advLabelH));
        cfgScaleDial.setBounds (advArea);
    }
    else
    {
        setAdvVisible (false,
            temperatureDial, temperatureLabel, topKDial, topKLabel,
            topPDial, topPLabel, cfgScaleDial, cfgScaleLabel);
    }
}