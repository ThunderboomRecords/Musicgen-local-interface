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

MusicGenVSTProcessor::MusicGenVSTProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

MusicGenVSTProcessor::~MusicGenVSTProcessor() {}

const juce::String MusicGenVSTProcessor::getName() const { return JucePlugin_Name; }

bool MusicGenVSTProcessor::acceptsMidi() const { return true; }
bool MusicGenVSTProcessor::producesMidi() const { return true; }
bool MusicGenVSTProcessor::isMidiEffect() const { return true; }
double MusicGenVSTProcessor::getTailLengthSeconds() const { return 0.0; }

int MusicGenVSTProcessor::getNumPrograms() { return 1; }
int MusicGenVSTProcessor::getCurrentProgram() { return 0; }
void MusicGenVSTProcessor::setCurrentProgram(int) {}
const juce::String MusicGenVSTProcessor::getProgramName(int) { return {}; }
void MusicGenVSTProcessor::changeProgramName(int, const juce::String&) {}

void MusicGenVSTProcessor::prepareToPlay(double, int) {}
void MusicGenVSTProcessor::releaseResources() {}

void MusicGenVSTProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
}

bool MusicGenVSTProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* MusicGenVSTProcessor::createEditor()
{
    return new MusicGenVSTEditor(*this);
}

void MusicGenVSTProcessor::getStateInformation(juce::MemoryBlock&) {}
void MusicGenVSTProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MusicGenVSTProcessor();
}
