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

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <mutex>

struct GeneratedSample
{
    juce::String name;
    juce::AudioBuffer<float> buffer;
    double sampleRate = 0.0;
};

class MusicGenVSTProcessor final : public juce::AudioProcessor
{
public:
    MusicGenVSTProcessor();
    ~MusicGenVSTProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- Generation (HTTP to localhost:55000) ---
    void generateAsync (const juce::String& prompt, const juce::String& instrumentation,
                        float tempo, float duration, int numSamples,
                        float temperature, int topK, float topP, int cfg,
                        const juce::File& audioInput);

    bool isGenerating() const { return generating.load(); }
    juce::String getGenerationError() const;

    int getNumGeneratedSamples() const;
    juce::String getGeneratedSampleName (int index) const;
    void playGeneratedSample (int index);
    void stopPlayback();

    juce::String getUserId() const { return userId; }
    juce::File getGeneratedDir() const;

private:
    void performGeneration (juce::String prompt, juce::String instrumentation,
                            float tempo, float duration, int numSamples,
                            float temperature, int topK, float topP, int cfg,
                            juce::File audioInput);

    bool loadAudioFile (const juce::File& file, juce::AudioBuffer<float>& buffer, double& fileSampleRate);

    juce::AudioFormatManager formatManager;
    double currentSampleRate = 44100.0;

    // Generated samples
    mutable std::mutex samplesMutex;
    std::vector<GeneratedSample> generatedSamples;
    int playingSampleIndex = -1;
    int playbackPosition = 0;

    // Generation state
    std::atomic<bool> generating { false };
    juce::String generationError;
    mutable std::mutex errorMutex;

    // User identity
    juce::String userId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MusicGenVSTProcessor)
};
