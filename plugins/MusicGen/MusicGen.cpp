#include "MusicGen.hpp"

START_NAMESPACE_DISTRHO

MusicGen::MusicGen() : Plugin(kParameterCount, 0, 0)
{
    for(size_t i = 0; i < 32; i++) {
        drumpadBuffers.push_back(std::vector<float>());
        drumpadBufferPositions.push_back(0);
    }
}

MusicGen::~MusicGen() { 

}

void MusicGen::initParameter(uint32_t index, Parameter &parameter)
{
    switch (index)
    {
        case kCharFloat:
            parameter.hints = kParameterIsAutomatable|kParameterIsBoolean;
            parameter.name = "char_float_val";
            parameter.ranges.min = -1.0f;
            parameter.ranges.max = 1000000.0f;
            parameter.ranges.def = 0.0f;
            parameter.groupId = kCharFloat;
            parameter.symbol = "char_float_val";
            break;
        default:
            break;
    }
}

void MusicGen::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
    // treat meter audio ports as stereo
    port.groupId = kPortGroupStereo;

    // everything else is as default
    Plugin::initAudioPort(input, index, port);
}

float MusicGen::getParameterValue(uint32_t index) const 
{
    return 0.5;
}

void MusicGen::setParameterValue(uint32_t index, float value)
{
    if(index == 0){
        if(value > 0){
            readFilePath.push_back(static_cast<char>(value));
        } else if(value == -1) { // Start of list
            startOfFile = true;
            readFilePath = "";
            generatedBufferPosition = 0;
            generatedBuffer.clear();
        } else if(value == -2) { // End of list
            startOfFile = false;
            SF_INFO sfInfo;
            SNDFILE* sndFile = sf_open(readFilePath.c_str(), SFM_READ, &sfInfo);
            if (!sndFile) {
                std::cerr << "Error: Could not open file: " << sf_strerror(sndFile) << std::endl;
                generatedBuffer.clear();
            } else {
                // Display audio file information
                std::cout << "Sample Rate: " << sfInfo.samplerate << std::endl;
                std::cout << "Channels: " << sfInfo.channels << std::endl;
                std::cout << "Frames: " << sfInfo.frames << std::endl;

                // Read the audio data into a buffer
                generatedBuffer.resize(sfInfo.frames * sfInfo.channels);
                sf_count_t numFrames = sf_readf_float(sndFile, generatedBuffer.data(), sfInfo.frames);

                // Close the audio file
                sf_close(sndFile);
            }
        } 
    } else if(index >= 1 && index <= 32){
        if(value > 0){
            readFilePath.push_back(static_cast<char>(value));
        } else if(value == -1) { // Start of list
            startOfFile = true;
            readFilePath = "";
            drumpadBufferPositions[index-1] = 0;
            drumpadBuffers[index-1].clear();
        } else if(value == -2) { // End of list
            startOfFile = false; 
            SF_INFO sfInfo;
            SNDFILE* sndFile = sf_open(readFilePath.c_str(), SFM_READ, &sfInfo);
            if (!sndFile) { 
                std::cerr << "Error: Could not open file: " << sf_strerror(sndFile) << std::endl;
                drumpadBuffers[index-1].clear();
            } else {
                // Display audio file information
                std::cout << "Sample Rate: " << sfInfo.samplerate << std::endl;
                std::cout << "Channels: " << sfInfo.channels << std::endl;
                std::cout << "Frames: " << sfInfo.frames << std::endl;

                // Read the audio data into a buffer
                drumpadBuffers[index-1].resize(sfInfo.frames * sfInfo.channels);
                sf_count_t numFrames = sf_readf_float(sndFile, drumpadBuffers[index-1].data(), sfInfo.frames);

                // Close the audio file
                sf_close(sndFile);
            }
            drumpadBufferPositions[index-1] = drumpadBuffers[index-1].size() + 1;
        } 
    } else if(index == 33){
        drumpadBufferPositions[value] = 0;
    }
}

void MusicGen::setState(const char* key, const char* value)
{

}

String MusicGen::getState(const char* key) const
{
    String retString = String("undefined state");
    return retString;
}

void MusicGen::initState(unsigned int index, String &stateKey, String &defaultStateValue)
{

}

void MusicGen::run(
    const float **,               // incoming audio
    float **outputs,              // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{
    for (uint32_t i = 0; i < midiEventCount; ++i) {
        uint8_t status = midiEvents[i].data[0];
        uint8_t data1 = midiEvents[i].data[1];
        uint8_t type = status >> 4;

        if (static_cast<int>(type) == 9 && static_cast<int>(data1) < drumpadBufferPositions.size())
        {
            /* NoteOn */
            drumpadBufferPositions[static_cast<int>(data1)] = 0;
        }

    }

    // std::cout << midiEvents->data << std::endl;
    float* outLeft = outputs[0];
    float* outRight = outputs[1];

    // Process each frame
    float num_sums = 0;
    if (generatedBufferPosition < generatedBuffer.size() / 2) { // from generate button
        num_sums += 1;
    } 
    for(size_t j = 0; j < drumpadBufferPositions.size(); j++) {
        if(drumpadBufferPositions[j] < drumpadBuffers[j].size() / 2) {
            num_sums += 1;
        }
    }
    for (uint32_t i = 0; i < numFrames; ++i) {
        // Sampling
        outLeft[i] = 0.0f;
        outRight[i] = 0.0f;
        if (generatedBufferPosition < generatedBuffer.size() / 2) { // from generate button
            // Copy samples from the buffer to the outputs
            outLeft[i] += generatedBuffer[2 * generatedBufferPosition];     // Left channel
            outRight[i] += generatedBuffer[2 * generatedBufferPosition + 1]; // Right channel
            generatedBufferPosition++;
        } 

        for(size_t j = 0; j < drumpadBufferPositions.size(); j++) {
            if(drumpadBufferPositions[j] < drumpadBuffers[j].size() / 2) {
                outLeft[i] += drumpadBuffers[j][2 * drumpadBufferPositions[j]];     // Left channel
                outRight[i] += drumpadBuffers[j][2 * drumpadBufferPositions[j] + 1]; // Right channel
                drumpadBufferPositions[j]++;
            }
        }
        
        if(num_sums == 0){
            num_sums = 1;
        }
        outLeft[i] /= num_sums;
        outRight[i] /= num_sums;
    }

}

void MusicGen::readFile(std::string selectedFile)
{
    
}

Plugin *createPlugin()
{
    return new MusicGen();
}

END_NAMESPACE_DISTRHO