#ifndef MUSICGEN_HPP
#define MUSICGEN_HPP

#include <string>
#include <sndfile.h>
#include <iostream>
#include <vector>

#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "PluginParams.h"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

/**
  Plugin to show how to get some basic information sent to the UI.
 */
class MusicGen : public Plugin
{
public:
    MusicGen();
    ~MusicGen();

protected:
    const char* getLabel() const noexcept override { return "MusicGen VST"; }
    const char* getDescription() const override{ return "VST Impl of MusicGen"; }
    const char* getMaker() const noexcept override { return "Thunderboom Records"; }
    const char* getHomePage() const override { return "https://github.com/ThunderboomRecords"; }
    const char* getLicense() const noexcept override { return "GPL V3"; }
    uint32_t getVersion() const noexcept override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const noexcept override { return d_cconst('t', 'b', 'W', 'S'); }

    void initParameter(uint32_t index, Parameter &parameter) override;
    void initAudioPort(bool input, uint32_t index, AudioPort& port) override;
    void setState(const char *key, const char *value) override;
    String getState(const char *key) const override;
    void initState(unsigned int, String &, String &) override;

    // --- Internal data ----------
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    // --- Process ----------------
    void run(const float **, float **outputs, uint32_t numFrames, const MidiEvent *midiEvents, uint32_t midiEventCount) override;
    void readFile(std::string selectedFile);

private:
    friend class MusicGenUI;
    bool startOfFile = false;
    std::string readFilePath = "";
    std::vector<float> generatedBuffer;
    size_t generatedBufferPosition;

    std::vector<std::vector<float>> drumpadBuffers;
    std::vector<size_t> drumpadBufferPositions;

};

END_NAMESPACE_DISTRHO

#endif