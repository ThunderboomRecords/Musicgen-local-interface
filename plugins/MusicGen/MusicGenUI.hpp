#ifndef MUSICGEN_UI_HPP
#define MUSICGEN_UI_HPP
#include <string>
#include <iostream>
#include <ctime>
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"
#include "Color.hpp"

#include "MusicGen.hpp"

#include "./src/SimpleButton.hpp"
#include "./src/Knob.hpp"
#include "./src/Panel.hpp"
#include "./src/TextInput.hpp"
#include "./src/NumberInput.hpp"
#include "./src/WAIVEColors.hpp"
#include "./src/Label.hpp"
#include "./src/ValueIndicator.hpp"
#include "./src/Checkbox.hpp"
#include "tinyfiledialogs.h"
#include "PluginParams.h"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 1300;
const unsigned int UI_H = 600;

class MusicGenUI : public UI,
                   public Button::Callback,
                   public Knob::Callback,
                   public Checkbox::Callback,
                   public TextInput::Callback,
                   public NumberInput::Callback
{
public:
    MusicGenUI();
    ~MusicGenUI();

protected:
    // Plugin callbacks
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void uiScaleFactorChanged(const double scaleFactor) override;
    
    // Callback handler
    void buttonClicked(Button *button) override;
    void contextClicked(Button *button) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;

    void textEntered(TextInput *textInput, std::string text) override;
    void textInputChanged(TextInput *textInput, std::string text) override;

    void textEntered(NumberInput *numberInput, std::string text) override;
    void textInputChanged(NumberInput *numberInput, std::string text) override;

    void checkboxUpdated(Checkbox *checkbox, bool value) override;

    void addSampleToPanel(float padding, std::string name, float button_height);

    void generateFn(std::atomic<bool>& done);
    void loaderAnim(std::atomic<bool>& done);

    void startPollingForCompletion(std::atomic<bool>* done);
    void addTimer(std::function<bool()> callback, int interval);

private:
    MusicGen *plugin;

    // Drum Pad
    // Panel Drum pad
    Panel *drumpadPanel;
    std::vector<Button*> drumpadButtons;

    // Still in Use
    // Lose Panels
    Panel *importPanel;
    Panel *generatePanel;
    Panel *knobsPanel;

    // Buttons
    Button *importButton;
    Button *importButtonIcon;
    Button *clearImportedSample;
    Button *generateButton;
    Button *openFolderButton;
    Button *dragAndDrop;
    Button *advancedSettingsButton;
    Button *localOnlineSwitch;

    // text inputs
    TextInput *textPrompt;
    TextInput *promptInstrumentation;

    Label *loadedFile;
    Label *loaderSpinner;

    // HBoxes
    Panel *hBox1;
    Panel *hBox2;
    Panel *hBox3;
    Panel *hBox4;
    Panel *hBox5;

    Panel *sampleLengthPanel;
    Label *sampleLengthLabel;
    NumberInput *sampleLength;

    Panel *promptTempoPanel;
    Label *promptTempoLabel;
    NumberInput *promptTempo;

    Panel *nSamplesPanel;
    Label *nSamplesLabel;
    NumberInput *nSamples;

    // Knobs
    Panel *temperaturePanel;
    Knob *temperatureKnob;
    Label *temperatureUpLabel;
    ValueIndicator *temperatureLabel;

    Panel *topKPanel;
    Knob *topKKnob;
    Label *topPUpLabel;
    ValueIndicator *topKLabel;

    Panel *topPPanel;
    Knob *topPKnob;
    Label *topKUpLabel;
    ValueIndicator *topPLabel;

    Panel *CFGPanel;
    Knob *CFGKnob;
    Label *CFGUpLabel;
    ValueIndicator *CFGLabel;

    std::vector<Panel*> samplePanels;
    std::vector<Button*> sampleButtons;
    std::vector<Label*> sampleLabels;
    std::vector<Label*> sampleNames;
    std::vector<Panel*> sampleLabelWrappers;
    std::vector<Button*> samplesRemove;

    Panel *samplesListPanel;
    Panel *samplesListInner;
    Panel *loaderPanel;

    Panel *popupPanel;
    Button *popupButton;
    Label *popupLabel;

    std::string userid = "undefined";

    std::string playBtn = "▶";

    double fScaleFactor;
    float fScale;
    float fscaleMult;
    int yOffset = 0;
    int butt_down = -1;
    bool startedDragging = false;
    float fontsize = 0.0;
    std::string current_dragging_path = "";
    std::condition_variable cv;
    std::string selectedFile = "";
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicGenUI);
};

UI *createUI()
{
    return new MusicGenUI();
}

END_NAMESPACE_DISTRHO

#endif