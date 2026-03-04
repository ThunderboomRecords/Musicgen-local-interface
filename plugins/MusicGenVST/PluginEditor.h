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
class SampleListModel final : public juce::ListBoxModel
{
public:
    int getNumRows() override { return 10; }

    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (juce::Colours::lightblue);

        g.setColour (juce::Colours::black);
        g.drawText (juce::String (rowNumber + 1), 0, 0, width, height,
                    juce::Justification::centredLeft);
    }
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
    juce::Font customFont;

    // Left panel
    juce::TextButton uploadFileButton;
    juce::Label promptLabel;
    juce::TextEditor promptInput;
    juce::Label instrumentationLabel;
    juce::TextEditor instrumentationInput;
    juce::Label lengthLabel;
    juce::TextEditor lengthInput;
    juce::Label bpmLabel;
    juce::TextEditor bpmInput;
    juce::Label samplesLabel;
    juce::TextEditor samplesInput;
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
    static constexpr int baseHeight = 330;
    static constexpr int advancedHeight = 169;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MusicGenVSTEditor)
};
