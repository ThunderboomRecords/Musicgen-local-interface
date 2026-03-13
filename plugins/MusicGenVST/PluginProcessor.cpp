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
MusicGenVSTProcessor::MusicGenVSTProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();
    userId = juce::Uuid().toString();
}

MusicGenVSTProcessor::~MusicGenVSTProcessor()
{
}

//==============================================================================
const juce::String MusicGenVSTProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MusicGenVSTProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MusicGenVSTProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MusicGenVSTProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MusicGenVSTProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MusicGenVSTProcessor::getNumPrograms()
{
    return 1;
}

int MusicGenVSTProcessor::getCurrentProgram()
{
    return 0;
}

void MusicGenVSTProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String MusicGenVSTProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void MusicGenVSTProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void MusicGenVSTProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;
}

void MusicGenVSTProcessor::releaseResources()
{
}

bool MusicGenVSTProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void MusicGenVSTProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Mix playing generated sample into output
    {
        std::lock_guard<std::mutex> lock (samplesMutex);
        if (playingSampleIndex >= 0 && playingSampleIndex < (int) generatedSamples.size())
        {
            auto& sample = generatedSamples[(size_t) playingSampleIndex];
            const int sampleLen = sample.buffer.getNumSamples();
            const int sampleCh = sample.buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();
            const int numChannels = buffer.getNumChannels();
            const int samplesToWrite = std::min (numSamples, sampleLen - playbackPosition);

            if (samplesToWrite > 0)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    int srcCh = std::min (ch, sampleCh - 1);
                    buffer.addFrom (ch, 0, sample.buffer, srcCh, playbackPosition, samplesToWrite);
                }
                playbackPosition += samplesToWrite;
            }

            if (playbackPosition >= sampleLen)
            {
                playingSampleIndex = -1;
                playbackPosition = 0;
            }
        }
    }
}

//==============================================================================
bool MusicGenVSTProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MusicGenVSTProcessor::createEditor()
{
    return new MusicGenVSTEditor (*this);
}

//==============================================================================
void MusicGenVSTProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("MusicGenVST");
    state.setProperty ("userId", userId, nullptr);
    juce::MemoryOutputStream stream (destData, true);
    state.writeToStream (stream);
}

void MusicGenVSTProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto state = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);
    if (state.isValid())
        userId = state.getProperty ("userId", userId).toString();
}

//==============================================================================
// Generation (acestep.cpp child processes)
//==============================================================================

juce::File MusicGenVSTProcessor::getAceStepDir() const
{
    // acestep.cpp is a sibling directory to the plugin source, but at runtime
    // we look relative to the plugin binary location or fall back to a
    // well-known path.
    auto pluginFile = juce::File::getSpecialLocation (juce::File::currentExecutableFile);

    // Walk up to find the acestep.cpp directory — handles build tree layouts
    auto dir = pluginFile.getParentDirectory();
    for (int i = 0; i < 8; ++i)
    {
        auto candidate = dir.getChildFile ("acestep.cpp");
        if (candidate.isDirectory() && candidate.getChildFile ("build").isDirectory())
            return candidate;
        dir = dir.getParentDirectory();
    }

    // Fallback: check common development paths
    auto home = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
    auto devPath = home.getChildFile ("projects/Other/MusicGenVST/acestep.cpp");
    if (devPath.isDirectory())
        return devPath;

    return {};
}

void MusicGenVSTProcessor::generateAsync (const AceStepParams& params)
{
    if (generating.load())
        return;

    generating.store (true);
    {
        std::lock_guard<std::mutex> lock (errorMutex);
        generationError.clear();
    }
    {
        std::lock_guard<std::mutex> lock (statusMutex);
        generationStatus = "Starting generation...";
    }

    std::thread ([this, params]()
    {
        performGeneration (params);
        generating.store (false);
    }).detach();
}

void MusicGenVSTProcessor::performGeneration (AceStepParams params)
{
    auto aceDir = getAceStepDir();
    if (! aceDir.isDirectory())
    {
        std::lock_guard<std::mutex> lock (errorMutex);
        generationError = "acestep.cpp directory not found. Ensure it exists alongside the plugin.";
        return;
    }

    auto buildDir = aceDir.getChildFile ("build");
    auto modelsDir = aceDir.getChildFile ("models");

    // Verify binaries exist
    auto aceQwen3 = buildDir.getChildFile ("ace-qwen3");
    auto ditVae = buildDir.getChildFile ("dit-vae");
    if (! aceQwen3.existsAsFile() || ! ditVae.existsAsFile())
    {
        std::lock_guard<std::mutex> lock (errorMutex);
        generationError = "acestep.cpp binaries not found. Run the build script first.";
        return;
    }

    // Verify models exist
    auto lmModel = modelsDir.getChildFile ("acestep-5Hz-lm-4B-Q8_0.gguf");
    auto textEncoder = modelsDir.getChildFile ("Qwen3-Embedding-0.6B-Q8_0.gguf");
    auto ditModel = modelsDir.getChildFile ("acestep-v15-sft-Q8_0.gguf");
    auto vaeModel = modelsDir.getChildFile ("vae-BF16.gguf");

    if (! lmModel.existsAsFile() || ! textEncoder.existsAsFile()
        || ! ditModel.existsAsFile() || ! vaeModel.existsAsFile())
    {
        std::lock_guard<std::mutex> lock (errorMutex);
        generationError = "Model files not found. Run the models download script first.";
        return;
    }

    // Create output directory (flat — every generation overwrites the same folder)
    auto outputDir = getGeneratedDir();
    outputDir.createDirectory();

    std::vector<GeneratedSample> newSamples;

    for (int sampleIdx = 0; sampleIdx < params.numSamples; ++sampleIdx)
    {
        {
            std::lock_guard<std::mutex> lock (statusMutex);
            generationStatus = "Generating sample " + juce::String (sampleIdx + 1)
                             + " of " + juce::String (params.numSamples) + "...";
        }

        // Write request.json in the output directory
        auto requestFile = outputDir.getChildFile ("request_" + juce::String (sampleIdx) + ".json");

        auto* jsonObj = new juce::DynamicObject();
        jsonObj->setProperty ("caption", params.caption);
        jsonObj->setProperty ("lyrics", params.lyrics);

        if (params.negativePrompt.isNotEmpty())
            jsonObj->setProperty ("lm_negative_prompt", params.negativePrompt);
        if (params.keyscale.isNotEmpty())
            jsonObj->setProperty ("keyscale", params.keyscale);
        if (params.timesignature.isNotEmpty())
            jsonObj->setProperty ("timesignature", params.timesignature);
        if (params.vocalLanguage.isNotEmpty())
            jsonObj->setProperty ("vocal_language", params.vocalLanguage);

        jsonObj->setProperty ("bpm", params.bpm);
        jsonObj->setProperty ("duration", params.duration);
        jsonObj->setProperty ("seed", params.seed);
        jsonObj->setProperty ("lm_temperature", (double) params.lmTemperature);
        jsonObj->setProperty ("lm_cfg_scale", (double) params.lmCfgScale);
        jsonObj->setProperty ("lm_top_p", (double) params.lmTopP);
        jsonObj->setProperty ("lm_top_k", params.lmTopK);
        jsonObj->setProperty ("inference_steps", params.inferenceSteps);
        jsonObj->setProperty ("guidance_scale", (double) params.guidanceScale);
        jsonObj->setProperty ("shift", (double) params.shift);

        juce::var jsonVar (jsonObj);
        auto jsonStr = juce::JSON::toString (jsonVar);

        requestFile.replaceWithText (jsonStr);

        // Step 1: Run ace-qwen3 (LLM — generates audio codes)
        {
            std::lock_guard<std::mutex> lock (statusMutex);
            generationStatus = "Sample " + juce::String (sampleIdx + 1)
                             + ": Running language model...";
        }

        juce::StringArray lmArgs;
        lmArgs.add ("--request");
        lmArgs.add (requestFile.getFullPathName());
        lmArgs.add ("--model");
        lmArgs.add (lmModel.getFullPathName());

        if (! runChildProcess (aceQwen3.getFullPathName(), lmArgs))
        {
            std::lock_guard<std::mutex> lock (errorMutex);
            generationError = "ace-qwen3 (language model) failed for sample "
                            + juce::String (sampleIdx + 1) + ".";
            return;
        }

        // ace-qwen3 produces request_N0.json (appends "0" before extension)
        auto intermediateFile = outputDir.getChildFile ("request_" + juce::String (sampleIdx) + "0.json");
        if (! intermediateFile.existsAsFile())
        {
            std::lock_guard<std::mutex> lock (errorMutex);
            generationError = "Language model did not produce intermediate file for sample "
                            + juce::String (sampleIdx + 1) + ".";
            return;
        }

        // Step 2: Run dit-vae (synthesize audio)
        {
            std::lock_guard<std::mutex> lock (statusMutex);
            generationStatus = "Sample " + juce::String (sampleIdx + 1)
                             + ": Synthesizing audio...";
        }

        juce::StringArray ditArgs;
        ditArgs.add ("--request");
        ditArgs.add (intermediateFile.getFullPathName());
        ditArgs.add ("--text-encoder");
        ditArgs.add (textEncoder.getFullPathName());
        ditArgs.add ("--dit");
        ditArgs.add (ditModel.getFullPathName());
        ditArgs.add ("--vae");
        ditArgs.add (vaeModel.getFullPathName());

        if (! runChildProcess (ditVae.getFullPathName(), ditArgs))
        {
            std::lock_guard<std::mutex> lock (errorMutex);
            generationError = "dit-vae (audio synthesis) failed for sample "
                            + juce::String (sampleIdx + 1) + ".";
            return;
        }

        // dit-vae produces request_N00.wav (appends another "0" to the intermediate name)
        auto wavFile = outputDir.getChildFile ("request_" + juce::String (sampleIdx) + "00.wav");
        if (! wavFile.existsAsFile())
        {
            std::lock_guard<std::mutex> lock (errorMutex);
            generationError = "Audio synthesis did not produce WAV file for sample "
                            + juce::String (sampleIdx + 1) + ".";
            return;
        }

        // Rename to something meaningful
        auto destFile = outputDir.getChildFile ("sample_" + juce::String (sampleIdx + 1) + ".wav");
        wavFile.moveFileTo (destFile);

        GeneratedSample sample;
        sample.name = destFile.getFileName();
        sample.file = destFile;
        if (loadAudioFile (destFile, sample.buffer, sample.sampleRate))
            newSamples.push_back (std::move (sample));
    }

    {
        std::lock_guard<std::mutex> lock (samplesMutex);
        generatedSamples = std::move (newSamples);
        playingSampleIndex = -1;
        playbackPosition = 0;
    }

    {
        std::lock_guard<std::mutex> lock (statusMutex);
        generationStatus = "Done!";
    }
}

bool MusicGenVSTProcessor::runChildProcess (const juce::String& executable,
                                             const juce::StringArray& args)
{
    juce::ChildProcess process;
    juce::StringArray fullArgs;
    fullArgs.add (executable);
    fullArgs.addArray (args);

    if (! process.start (fullArgs))
        return false;

    // Wait for the process to finish (blocking — we're on a background thread)
    return process.waitForProcessToFinish (600000); // 10 minute timeout
}

juce::String MusicGenVSTProcessor::getGenerationError() const
{
    std::lock_guard<std::mutex> lock (errorMutex);
    return generationError;
}

juce::String MusicGenVSTProcessor::getGenerationStatus() const
{
    std::lock_guard<std::mutex> lock (statusMutex);
    return generationStatus;
}

int MusicGenVSTProcessor::getNumGeneratedSamples() const
{
    std::lock_guard<std::mutex> lock (samplesMutex);
    return (int) generatedSamples.size();
}

juce::String MusicGenVSTProcessor::getGeneratedSampleName (int index) const
{
    std::lock_guard<std::mutex> lock (samplesMutex);
    if (index >= 0 && index < (int) generatedSamples.size())
        return generatedSamples[(size_t) index].name;
    return {};
}

juce::File MusicGenVSTProcessor::getGeneratedSampleFile (int index) const
{
    std::lock_guard<std::mutex> lock (samplesMutex);
    if (index >= 0 && index < (int) generatedSamples.size())
        return generatedSamples[(size_t) index].file;
    return {};
}

bool MusicGenVSTProcessor::isPlayingSample (int index) const
{
    std::lock_guard<std::mutex> lock (samplesMutex);
    return playingSampleIndex == index;
}

void MusicGenVSTProcessor::playGeneratedSample (int index)
{
    std::lock_guard<std::mutex> lock (samplesMutex);
    if (index >= 0 && index < (int) generatedSamples.size())
    {
        playingSampleIndex = index;
        playbackPosition = 0;
    }
}

void MusicGenVSTProcessor::stopPlayback()
{
    std::lock_guard<std::mutex> lock (samplesMutex);
    playingSampleIndex = -1;
    playbackPosition = 0;
}

//==============================================================================
bool MusicGenVSTProcessor::loadAudioFile (const juce::File& file, juce::AudioBuffer<float>& buffer, double& fileSampleRate)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;

    fileSampleRate = reader->sampleRate;
    buffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&buffer, 0, (int) reader->lengthInSamples, 0, true, true);
    return true;
}

juce::File MusicGenVSTProcessor::getGeneratedDir() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("MusicGenVST")
               .getChildFile ("generated");
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MusicGenVSTProcessor();
}