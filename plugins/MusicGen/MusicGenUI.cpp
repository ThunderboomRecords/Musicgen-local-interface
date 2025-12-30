#include "MusicGenUI.hpp"

// TODO :
// Add highlight to drumpad button
// Segmentation fault when closing app when it is loading
// Loading bug? Forever hanging in loading screen.

// Fix hovering bug (top-down lagging vs bottom-up) <- probably not going to be a thing

// Compile for:
// Mac 64
// Linux
// Windows

// TODO:
// Upload button to the new style

std::string getCurrentDateTime() {
    // Get the current time
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    // Format the date and time
    std::ostringstream oss;
    oss << (now->tm_year % 100)  // Year (last two digits)
        << (now->tm_mon + 1 < 10 ? "0" : "") << (now->tm_mon + 1)  // Month
        << (now->tm_mday < 10 ? "0" : "") << now->tm_mday          // Day
        << (now->tm_hour < 10 ? "0" : "") << now->tm_hour          // Hour
        << (now->tm_min < 10 ? "0" : "") << now->tm_min            // Minute
        << (now->tm_sec < 10 ? "0" : "") << now->tm_sec;           // Second

    return oss.str();
}

START_NAMESPACE_DISTRHO

MusicGenUI::MusicGenUI() : UI(UI_W, UI_H),
                           fScaleFactor(2.0f),
                           fScale(1.0f)
{
    // getScaleFactor();
    const char* homeDir = std::getenv("HOME"); // Works on Unix-like systems
    std::filesystem::path documentsPath = std::filesystem::path(homeDir) / "Documents" / "MusicGenVST";
    std::filesystem::create_directory(documentsPath);
    documentsPath = std::filesystem::path(homeDir) / "Documents" / "MusicGenVST" / "generated";
    std::filesystem::create_directory(documentsPath);

    plugin = static_cast<MusicGen *>(getPluginInstancePointer());

    float width = UI_W * fScaleFactor;
    float height = UI_H * fScaleFactor;

    float padding = 4.f * fScaleFactor;

    std::cout << getScaleFactor() << std::endl;

    if(getScaleFactor() == 2){
        fscaleMult = 1.0;
    } else{
        fscaleMult = 2.0;
    }

    fontsize = 7.f * fScaleFactor * fscaleMult;

    fScaleFactor = fscaleMult; // Might need to remove this scaler.

    float textBoxHeight = 50;
    float numBoxHeight = 30;
    float buttonHeight = 40;
    // Main input panel
    {   
        // Generate Panel
        {
            generatePanel = new Panel(this);
            generatePanel->setSize((width * 0.5f * 0.5f), (height * 0.5f), true);
            generatePanel->setAbsolutePos(0, 0);
            generatePanel->background_color = WaiveColors::bgGrey;
        }

        // Loading and clearing the audio prompt
        {
            {
                importPanel = new Panel(this);
                importPanel->background_color = WaiveColors::bgGrey;
                importPanel->setSize(generatePanel->getWidth() - (padding * 2.0f), 160, true);
                importPanel->onTop(generatePanel, START, START, padding * 2.0f);
            }

            {
                importButton = new Button(this);
                importButton->text_color = Color(255, 255, 255);
                importButton->background_color = WaiveColors::hiddenColor;
                importButton->highlight_color = WaiveColors::hiddenColor;
                importButton->setLabel("");
                importButton->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                importButton->setFontSize(fontsize);
                importButton->setSize((importPanel->getWidth() * 0.35f), buttonHeight, true);
                importButton->onTop(importPanel, CENTER, CENTER, 0);
                importButton->setCallback(this);
            }

            {
                loadedFile = new Label(this, "Upload Audio File");
                loadedFile->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                loadedFile->setFontSize(fontsize * 0.9);
                loadedFile->text_color = WaiveColors::text;
                loadedFile->resizeToFit();
                loadedFile->onTop(importButton, START, CENTER, 0);
            }

            {
                importButtonIcon = new Button(this);
                importButtonIcon->text_color = Color(255, 255, 255);
                importButtonIcon->background_color = WaiveColors::bgDark;
                importButtonIcon->highlight_color = WaiveColors::bgHighlight;
                importButtonIcon->setLabel("↓");
                importButtonIcon->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                importButtonIcon->setFontSize(fontsize);
                importButtonIcon->fEnabled = false;
                // importButton->setSize((generatePanel->getWidth() * 0.7f) - (padding * 1.0f), buttonHeight, true);
                importButtonIcon->resizeToFit();
                importButtonIcon->setSize(importButton->getHeight(), importButton->getHeight(), true);
                importButtonIcon->onTop(importButton, END, CENTER, padding);
            }

            {
                clearImportedSample = new Button(this);
                clearImportedSample->text_color = Color(255, 255, 255);
                clearImportedSample->background_color = WaiveColors::bgDark;
                clearImportedSample->highlight_color = WaiveColors::bgHighlight;
                clearImportedSample->setLabel("X");
                clearImportedSample->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                clearImportedSample->setFontSize(fontsize);
                // clearImportedSample->setSize((generatePanel->getWidth() * 0.3f) - (padding * 1.0f), buttonHeight, true);
                clearImportedSample->resizeToFit();
                clearImportedSample->setSize(importButtonIcon->getHeight(), importButtonIcon->getHeight(), true);
                clearImportedSample->onTop(importButtonIcon, CENTER, CENTER, 0);
                clearImportedSample->setCallback(this);
                clearImportedSample->hide();
            }
        }

        // Prompt
        {
            textPrompt = new TextInput(this);
            textPrompt->setPlaceholder("Sample Description");
            textPrompt->text_color_greyed = WaiveColors::grey2;
            textPrompt->setSize(generatePanel->getWidth() - (padding * 2.0f), textBoxHeight, true);
            textPrompt->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            textPrompt->setFontSize(fontsize);
            textPrompt->below(importPanel, START, padding);
            textPrompt->setCallback(this);
        }

        // hBox1
        {
            hBox1 = new Panel(this);
            hBox1->background_color = WaiveColors::bgGrey;
            hBox1->setSize(textPrompt->getWidth(), 25, true);
            hBox1->below(textPrompt, START, padding);
        }

        // Input box for instrumentation
        {
            promptInstrumentation = new TextInput(this);
            promptInstrumentation->setPlaceholder("Add Instrument (optional)");
            promptInstrumentation->text_color_greyed = WaiveColors::grey2;
            promptInstrumentation->setSize(generatePanel->getWidth() - (padding * 2.0f), textBoxHeight, true);
            promptInstrumentation->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            promptInstrumentation->setFontSize(fontsize);
            promptInstrumentation->below(hBox1, START, padding);
            promptInstrumentation->setCallback(this);
        }

        // hBox2
        {
            hBox2 = new Panel(this);
            hBox2->background_color = WaiveColors::bgGrey;
            hBox2->setSize(promptInstrumentation->getWidth(), 50, true);
            hBox2->below(promptInstrumentation, START, padding);
        }

        // length
        {
            sampleLengthPanel = new Panel(this);
            sampleLengthPanel->background_color = WaiveColors::bgGrey;

            sampleLengthLabel = new Label(this, "Length");
            sampleLengthLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            sampleLengthLabel->setFontSize(fontsize);
            sampleLengthLabel->text_color = WaiveColors::text;
            sampleLengthLabel->resizeToFit();

            sampleLength = new NumberInput(this);
            sampleLength->text_color_greyed = WaiveColors::grey2;
            sampleLength->background_color = WaiveColors::numBoxColor;
            sampleLength->precision = 2;
            sampleLength->min = 0.4;
            sampleLength->max = 30;
            sampleLength->setText("8");
            sampleLength->align = Align::ALIGN_CENTER;
            sampleLength->setSize((generatePanel->getWidth() * 0.15) - (padding * 2.0f), numBoxHeight, true);
            sampleLength->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            sampleLength->setFontSize(fontsize);
            sampleLength->setCallback(this);

            sampleLengthPanel->setSize(sampleLength->getWidth() + sampleLengthLabel->getWidth(), numBoxHeight, true);
            sampleLengthPanel->below(hBox2, START, padding);

            sampleLengthLabel->onTop(sampleLengthPanel, START, CENTER, padding);
            sampleLength->rightOf(sampleLengthLabel, CENTER, padding);
        }

        // BPM
        {
            promptTempoPanel = new Panel(this);
            promptTempoPanel->background_color = WaiveColors::bgGrey;

            promptTempoLabel = new Label(this, "BPM");
            promptTempoLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            promptTempoLabel->setFontSize(fontsize);
            promptTempoLabel->text_color = WaiveColors::text;
            promptTempoLabel->resizeToFit();

            promptTempo = new NumberInput(this);
            promptTempo->text_color_greyed = WaiveColors::grey2;
            promptTempo->background_color = WaiveColors::numBoxColor;
            promptTempo->precision = 2;
            promptTempo->min = 1;
            promptTempo->max = 999;
            promptTempo->align = Align::ALIGN_CENTER;
            promptTempo->setText("120");
            promptTempo->setSize((generatePanel->getWidth() * 0.15) - (padding * 2.0f), numBoxHeight, true);
            promptTempo->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            promptTempo->setFontSize(fontsize);
            promptTempo->setCallback(this);

            promptTempoPanel->setSize(promptTempo->getWidth() + promptTempoLabel->getWidth(), numBoxHeight, true);
            promptTempoPanel->below(hBox2, CENTER, padding);

            promptTempoLabel->onTop(promptTempoPanel, START, CENTER, 0);
            promptTempo->rightOf(promptTempoLabel, CENTER, padding);
        }

        // nSamples
        {
            nSamplesPanel = new Panel(this);
            nSamplesPanel->background_color = WaiveColors::bgGrey;

            nSamplesLabel = new Label(this, "Samples");
            nSamplesLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            nSamplesLabel->setFontSize(fontsize);
            nSamplesLabel->text_color = WaiveColors::text;
            nSamplesLabel->resizeToFit();

            nSamples = new NumberInput(this);
            nSamples->text_color_greyed = WaiveColors::grey2;
            nSamples->background_color = WaiveColors::numBoxColor;
            nSamples->precision = 0;
            nSamples->min = 1;
            nSamples->max = 10;
            nSamples->setText("1");
            nSamples->align = Align::ALIGN_CENTER;
            nSamples->setSize((generatePanel->getWidth() * 0.15) - (padding * 2.0f), numBoxHeight, true);
            nSamples->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            nSamples->setFontSize(fontsize);
            nSamples->setCallback(this);

            nSamplesPanel->setSize(promptTempo->getWidth() + promptTempoLabel->getWidth(), numBoxHeight, true);
            nSamplesPanel->below(hBox2, END, padding);
            nSamplesPanel->setAbsolutePos(nSamplesPanel->getAbsoluteX() - (nSamplesPanel->getWidth() * 0.5f) + padding, nSamplesPanel->getAbsoluteY());

            nSamplesLabel->onTop(nSamplesPanel, START, CENTER, 0);
            nSamples->rightOf(nSamplesLabel, CENTER, padding);
        }

        // hBox3
        {
            hBox3 = new Panel(this);
            hBox3->background_color = WaiveColors::bgGrey;
            hBox3->setSize(promptInstrumentation->getWidth(), 25, true);
            hBox3->below(sampleLengthPanel, START, padding);
        }

        // generate Button
        {
            generateButton = new Button(this);
            generateButton->text_color = Color(255, 255, 255);
            generateButton->radius = 10;
            generateButton->background_color = WaiveColors::bgDark;
            generateButton->highlight_color = WaiveColors::bgHighlight;
            generateButton->setLabel("Generate");
            generateButton->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            generateButton->setFontSize(fontsize);
            generateButton->setSize((hBox3->getWidth()), buttonHeight, true);
            generateButton->below(hBox3, START, padding);
            generateButton->setCallback(this);
        }

        // hBox4
        {
            hBox4 = new Panel(this);
            hBox4->background_color = WaiveColors::bgGrey;
            hBox4->setSize(promptInstrumentation->getWidth(), 25, true);
            hBox4->below(generateButton, START, padding);
        }

        // Advanced Options
        {
            // Open Folder Button
            {
                advancedSettingsButton = new Button(this);
                advancedSettingsButton->text_color = Color(255, 255, 255);
                advancedSettingsButton->background_color = WaiveColors::bgDark;
                advancedSettingsButton->highlight_color = WaiveColors::bgHighlight;
                advancedSettingsButton->accent_color = WaiveColors::highlight;
                advancedSettingsButton->isToggle = true;
                advancedSettingsButton->radius = 10;
                advancedSettingsButton->setLabel("Advanced Settings");
                advancedSettingsButton->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                advancedSettingsButton->setFontSize(fontsize);
                advancedSettingsButton->resizeToFit();
                advancedSettingsButton->setSize((advancedSettingsButton->getWidth()), buttonHeight, true);
                advancedSettingsButton->onTop(generatePanel, START, END, padding);
                advancedSettingsButton->setCallback(this);
            }

            // advancedSettingsPanel = new Panel(this);
            // advancedSettingsPanel->setSize(generatePanel->getWidth() - (padding), buttonHeight, true);
            // advancedSettingsPanel->onTop(generatePanel, START, END, padding);
            // advancedSettingsPanel->background_color = WaiveColors::bgGrey;
            
            // advancedSettings = new Checkbox(this);
            // advancedSettings->setSize(20, 20, true);
            // advancedSettings->background_color = WaiveColors::text;
            // advancedSettings->foreground_color = WaiveColors::text;
            // advancedSettings->setChecked(false, false);
            // advancedSettings->onTop(generatePanel, START, END, padding);
            // advancedSettings->setCallback(this);
            // advancedSettings->highlight_color = WaiveColors::hiddenColor;
            // advancedSettings->accent_color = WaiveColors::highlight;

            // advancedSettingsLabel = new Label(this, "Advanced Settings");
            // advancedSettingsLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            // advancedSettingsLabel->setFontSize(fontsize);
            // advancedSettingsLabel->text_color = WaiveColors::text;
            // advancedSettingsLabel->resizeToFit();
            // advancedSettingsLabel->rightOf(advancedSettings, CENTER, padding);

            // // Sample prompt Button
        }

        // Hidden knobs
        {
            int knobW = 150;
            int knobH = 150;
            // Panel
            {
                knobsPanel = new Panel(this);
                knobsPanel->background_color = WaiveColors::bgGrey;
                knobsPanel->setSize(generatePanel->getWidth(), knobH, true);
                knobsPanel->below(hBox4, START, padding);
            }

            // Temperature
            {
                temperaturePanel = new Panel(this);
                temperaturePanel->setSize(knobW, knobH, true);
                temperaturePanel->onTop(knobsPanel, START, START, padding);
                temperaturePanel->background_color = WaiveColors::bgGrey;
            
                temperatureKnob = new Knob(this);
                temperatureKnob->setName("Temperature");
                temperatureKnob->max = 2.0;
                temperatureKnob->min = 0.0;
                temperatureKnob->label = "";
                temperatureKnob->setFontSize(fontsize);
                temperatureKnob->setRadius(14.f * fScaleFactor);
                temperatureKnob->gauge_width = 4.0f;
                temperatureKnob->setValue(0.7);
                temperatureKnob->resizeToFit();
                temperatureKnob->onTop(temperaturePanel, CENTER, CENTER, padding);
                temperatureKnob->setCallback(this);
                temperatureKnob->highlight_color = WaiveColors::text;
                temperatureKnob->accent_color = WaiveColors::highlight;
                temperatureKnob->foreground_color = WaiveColors::bgGrey;

                temperatureUpLabel = new Label(this, "Temperature");
                temperatureUpLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                temperatureUpLabel->setFontSize(fontsize);
                temperatureUpLabel->text_color = WaiveColors::text;
                temperatureUpLabel->resizeToFit();
                temperatureUpLabel->above(temperatureKnob, CENTER, padding);

                temperatureLabel = new ValueIndicator(this);
                temperatureLabel->setSize(70, 20);
                temperatureLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                temperatureLabel->setFontSize(fontsize);
                temperatureLabel->text_color = WaiveColors::text;
                temperatureLabel->background_color = WaiveColors::bgGrey;
                temperatureLabel->below(temperatureKnob, CENTER, padding);
                temperatureLabel->setValue(temperatureKnob->getValue());
            }

            // topK
            {
                topKPanel = new Panel(this);
                topKPanel->setSize(knobW, knobH, true);
                topKPanel->rightOf(temperaturePanel, START, padding);
                topKPanel->background_color = WaiveColors::bgGrey;
            
                topKKnob = new Knob(this);
                topKKnob->setName("TopK");
                topKKnob->min = 1;
                topKKnob->max = 1000;
                topKKnob->integer = true;
                topKKnob->label = "";
                topKKnob->setFontSize(fontsize);
                topKKnob->setRadius(14.f * fScaleFactor);
                topKKnob->gauge_width = 4.0f;
                topKKnob->setValue(500);
                topKKnob->resizeToFit();
                topKKnob->onTop(topKPanel, CENTER, CENTER, padding);
                topKKnob->setCallback(this);
                topKKnob->highlight_color = WaiveColors::text;
                topKKnob->accent_color = WaiveColors::highlight;
                topKKnob->foreground_color = WaiveColors::bgGrey;

                topKUpLabel = new Label(this, "Top K");
                topKUpLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                topKUpLabel->setFontSize(fontsize);
                topKUpLabel->text_color = WaiveColors::text;
                topKUpLabel->resizeToFit();
                topKUpLabel->above(topKKnob, CENTER, padding);

                topKLabel = new ValueIndicator(this);
                topKLabel->setSize(70, 20);
                topKLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                topKLabel->setFontSize(fontsize);
                topKLabel->text_color = WaiveColors::text;
                topKLabel->background_color = WaiveColors::bgGrey;
                topKLabel->below(topKKnob, CENTER, padding);
                topKLabel->setValue(topKKnob->getValue());
            }

            // topP
            {
                topPPanel = new Panel(this);
                topPPanel->setSize(knobW, knobH, true);
                topPPanel->rightOf(topKPanel, START, padding);
                topPPanel->background_color = WaiveColors::bgGrey;
            
                topPKnob = new Knob(this);
                topPKnob->setName("TopP");
                topPKnob->max = 1.0;
                topPKnob->min = 0.0;
                topPKnob->label = "";
                topPKnob->setFontSize(fontsize);
                topPKnob->setRadius(14.f * fScaleFactor);
                topPKnob->gauge_width = 4.0f;
                topPKnob->setValue(0.0);
                topPKnob->resizeToFit();
                topPKnob->onTop(topPPanel, CENTER, CENTER, padding);
                topPKnob->setCallback(this);
                topPKnob->highlight_color = WaiveColors::text;
                topPKnob->accent_color = WaiveColors::highlight;
                topPKnob->foreground_color = WaiveColors::bgGrey;

                topPUpLabel = new Label(this, "Top P");
                topPUpLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                topPUpLabel->setFontSize(fontsize);
                topPUpLabel->text_color = WaiveColors::text;
                topPUpLabel->resizeToFit();
                topPUpLabel->above(topPKnob, CENTER, padding);

                topPLabel = new ValueIndicator(this);
                topPLabel->setSize(70, 20);
                topPLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                topPLabel->setFontSize(fontsize);
                topPLabel->text_color = WaiveColors::text;
                topPLabel->background_color = WaiveColors::bgGrey;
                topPLabel->below(topPKnob, CENTER, padding);
                topPLabel->setValue(topPKnob->getValue());
            }

            // CFG
            {
                CFGPanel = new Panel(this);
                CFGPanel->setSize(knobW, knobH, true);
                CFGPanel->rightOf(topPPanel, START, padding);
                CFGPanel->background_color = WaiveColors::bgGrey;
            
                CFGKnob = new Knob(this);
                CFGKnob->setName("TopP");
                CFGKnob->max = 10;
                CFGKnob->min = 1;
                CFGKnob->integer = true;
                CFGKnob->label = "";
                CFGKnob->setFontSize(fontsize);
                CFGKnob->setRadius(14.f * fScaleFactor);
                CFGKnob->gauge_width = 4.0f;
                CFGKnob->setValue(5);
                CFGKnob->resizeToFit();
                CFGKnob->onTop(CFGPanel, CENTER, CENTER, padding);
                CFGKnob->setCallback(this);
                CFGKnob->highlight_color = WaiveColors::text;
                CFGKnob->accent_color = WaiveColors::highlight;
                CFGKnob->foreground_color = WaiveColors::bgGrey;

                CFGUpLabel = new Label(this, "CFG");
                CFGUpLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                CFGUpLabel->setFontSize(fontsize);
                CFGUpLabel->text_color = WaiveColors::text;
                CFGUpLabel->resizeToFit();
                CFGUpLabel->above(CFGKnob, CENTER, padding);

                CFGLabel = new ValueIndicator(this);
                CFGLabel->setSize(70, 20);
                CFGLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
                CFGLabel->setFontSize(fontsize);
                CFGLabel->text_color = WaiveColors::text;
                CFGLabel->background_color = WaiveColors::bgGrey;
                CFGLabel->below(CFGKnob, CENTER, padding);
                CFGLabel->setValue(CFGKnob->getValue());
            }
        
            knobsPanel->hide();

            temperatureKnob->hide();
            temperatureLabel->hide();
            temperaturePanel->hide();
            temperatureUpLabel->hide();

            topKKnob->hide();
            topKLabel->hide();
            topKPanel->hide();
            topKUpLabel->hide();

            topPKnob->hide();
            topPLabel->hide();
            topPPanel->hide();
            topPUpLabel->hide();

            CFGKnob->hide();
            CFGLabel->hide();
            CFGPanel->hide();
            CFGUpLabel->hide();
        }
    }
    
    // Height of prev elements
    float sampleListHeight = generateButton->getAbsoluteY() + generateButton->getHeight() - padding;
    // Right side of the screen
    {
        // Main panel
        {
            samplesListPanel = new Panel(this);
            samplesListPanel->setSize((width * 0.5f * 0.5f), (height * 0.5f), true);
            samplesListPanel->setAbsolutePos(generatePanel->getWidth(), 0);
            samplesListPanel->background_color = WaiveColors::bgGrey;
        }

        // Inner panel
        {
            samplesListInner = new Panel(this);
            samplesListInner->setSize((width * 0.5f * 0.5f) - (padding * 4), sampleListHeight, true);
            samplesListInner->onTop(samplesListPanel, CENTER, START, generateButton->getHeight() * 0.5f);
            samplesListInner->background_color = WaiveColors::hiddenColor;
        }

        // hBox5
        {
            hBox5 = new Panel(this);
            hBox5->background_color = WaiveColors::bgGrey;
            hBox5->setSize(promptInstrumentation->getWidth(), 25, true);
            hBox5->below(samplesListInner, START, padding);
        }

        // Open Folder Button
        {
            openFolderButton = new Button(this);
            openFolderButton->text_color = Color(255, 255, 255);
            openFolderButton->background_color = WaiveColors::bgDark;
            openFolderButton->highlight_color = WaiveColors::bgHighlight;
            openFolderButton->accent_color = WaiveColors::highlight;
            openFolderButton->radius = 10;
            openFolderButton->setLabel("Local Files");
            openFolderButton->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            openFolderButton->setFontSize(fontsize);
            openFolderButton->resizeToFit();
            openFolderButton->setSize((openFolderButton->getWidth()), buttonHeight, true);
            openFolderButton->onTop(samplesListPanel, END, END, padding);
            openFolderButton->setCallback(this);
        }

        // Drumpad panel
        float panelHeight = samplesListPanel->getHeight() - (hBox5->getHeight() + hBox5->getAbsoluteY()) - padding;
        float drumpadSize = ((panelHeight - (padding * 4.0)) / 4.0);
        float panelWidth = ((drumpadSize * 4.0) + (padding * 4.0)) * 2.0;
        {
            drumpadPanel = new Panel(this);
            drumpadPanel->setSize(panelWidth, panelHeight, true);
            drumpadPanel->below(hBox5, START, 0);
            drumpadPanel->background_color = WaiveColors::sampleColor2;
            drumpadPanel->hide();
        }

        {
            // Loop over it change the positioning to take into account the scroll top etc.
            float prevRow = 0;
            for(int i = 0; i < 32; i++) {
                drumpadButtons.push_back(new Button(this));
                drumpadButtons.back()->setSize(drumpadSize,drumpadSize,true);
                drumpadButtons.back()->setLabel("");
                drumpadButtons.back()->sqrt = false;
                drumpadButtons.back()->hasContextFN = true;
                drumpadButtons.back()->radius = 5;

                float col = i % 8;
                float row = floor(i / 8);
                if(i == 0) {
                    drumpadButtons.back()->onTop(drumpadPanel, START, START, padding);
                } else if(row != prevRow) {
                    drumpadButtons.back()->below(drumpadButtons[drumpadButtons.size() - 9], START, padding);
                } else {
                    drumpadButtons.back()->rightOf(drumpadButtons[drumpadButtons.size() - 2], START, padding);
                }
                drumpadButtons.back()->setCallback(this);
                drumpadButtons.back()->hide();
            }
        }

        // Load samples
        {
            // Loop over it change the positioning to take into account the scroll top etc.
            float button_height = ((samplesListInner->getHeight() * 0.5f) / 10.0f);
            for(int i = 0; i < 10; i++) {
                addSampleToPanel(padding, std::string(""), button_height);
                // if(samplePanels.size() > 12){
                //     scrollBar->setSize((padding * 2), round(static_cast<float>(scrollBarHeight) * ((12.0) / static_cast<float>(samplePanels.size()))), true);
                // }
            }
        }

        {
            localOnlineSwitch = new Button(this);
            localOnlineSwitch->text_color = Color(255, 255, 255);
            localOnlineSwitch->background_color = WaiveColors::bgDark;
            localOnlineSwitch->highlight_color = WaiveColors::bgHighlight;
            localOnlineSwitch->accent_color = WaiveColors::highlight;
            localOnlineSwitch->isToggle = true;
            localOnlineSwitch->radius = 10;
            localOnlineSwitch->setLabel("Use local server");
            localOnlineSwitch->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
            localOnlineSwitch->setFontSize(fontsize);
            localOnlineSwitch->resizeToFit();
            localOnlineSwitch->setSize((advancedSettingsButton->getWidth()), buttonHeight, true);
            localOnlineSwitch->onTop(generatePanel, END, END, padding);
            localOnlineSwitch->setCallback(this);
            localOnlineSwitch->hide();
        }

        // {
        //     localOnlineSwitch = new Checkbox(this);
        //     localOnlineSwitch->setSize(20, 20, true);
        //     localOnlineSwitch->background_color = WaiveColors::text;
        //     localOnlineSwitch->foreground_color = WaiveColors::text;
        //     localOnlineSwitch->setChecked(false, false);
        //     localOnlineSwitch->onTop(samplesListPanel, END, END, padding);
        //     localOnlineSwitch->setCallback(this);
        //     localOnlineSwitch->highlight_color = WaiveColors::hiddenColor;
        //     localOnlineSwitch->accent_color = WaiveColors::highlight;

        //     localOnlineSwitchLabel = new Label(this, "Use local server");
        //     localOnlineSwitchLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
        //     localOnlineSwitchLabel->setFontSize(fontsize);
        //     localOnlineSwitchLabel->text_color = WaiveColors::text;
        //     localOnlineSwitchLabel->resizeToFit();
        //     localOnlineSwitchLabel->leftOf(localOnlineSwitch, CENTER, padding);
        //     localOnlineSwitch->hide();
        //     localOnlineSwitchLabel->hide();
        // }
    }

    {
        loaderPanel = new Panel(this);
        loaderPanel->setSize((width * 0.5f), (height * 0.5f), true);
        loaderPanel->setAbsolutePos(0, 0);
        loaderPanel->toFront();

        static const Color bg_col(35, 35, 37, 0.5);
        loaderPanel->background_color = bg_col;
        loaderPanel->hide();

        loaderSpinner = new Label(this, "Loading...");
        loaderSpinner->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
        loaderSpinner->setFontSize(fontsize * 4.0f);
        loaderSpinner->text_color = WaiveColors::text;
        loaderSpinner->resizeToFit();
        loaderSpinner->onTop(loaderPanel, CENTER, CENTER, padding);
        loaderSpinner->hide();
    }

    {
        popupPanel = new Panel(this);
        popupPanel->setSize((width * 0.25f) - (padding * 2.0), (height * 0.25f) - (padding * 2.0), true);
        popupPanel->onTop(loaderPanel, CENTER, CENTER, padding);
        popupPanel->toFront();

        static const Color bg_col(35, 35, 37, 0.5);
        popupPanel->background_color = bg_col;
        popupPanel->hide();

        popupLabel = new Label(this, "Oke");
        popupLabel->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
        popupLabel->setFontSize(fontsize);
        popupLabel->text_color = WaiveColors::text;
        popupLabel->resizeToFit();
        popupLabel->onTop(popupPanel, CENTER, CENTER, padding);
        popupLabel->hide();

        popupButton = new Button(this);
        popupButton->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
        popupButton->setLabel("Ok");
        popupButton->setFontSize(fontsize);
        popupButton->resizeToFit();
        popupButton->below(popupLabel, CENTER, padding);
        popupButton->setCallback(this);
        popupButton->hide();
    }

    // advancedSettings->setChecked(true, true);
    // advancedSettings->setChecked(false, true);
}

MusicGenUI::~MusicGenUI()
{
}

void MusicGenUI::parameterChanged(uint32_t index, float value)
{

}

void MusicGenUI::stateChanged(const char *key, const char *value)
{

}

void MusicGenUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(WaiveColors::dark);
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();
}

void MusicGenUI::uiScaleFactorChanged(const double scaleFactor)
{

}

// Callback function to capture the response from the server
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t WriteCallbackStream (void* contents, size_t size, size_t nmemb, void* userp)
{
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to extract the basename from a URL
std::string getBasename(const std::string& url)
{
    size_t lastSlash = url.find_last_of("/");
    if (lastSlash == std::string::npos)
    {
        return url; // Return the full URL if no slash is found
    }
    return url.substr(lastSlash + 1);
}

void MusicGenUI::generateFn(std::atomic<bool>& done)
{
    bool crashed = false;
    // TODO: add way for audio prompt
    // Make if else statment here to update the IP to localhost if in offline mode.
    std::string ip = "";
    if(!localOnlineSwitch->getToggled()){
        ip = "http://82.217.111.120/";
    } else {
        ip = "http://127.0.0.1:55000/";
    }

    float duration = std::stof(sampleLength->getText());
    float temperature = temperatureKnob->getValue();
    float topp = topPKnob->getValue();
    int samples = std::stoi(nSamples->getText());
    int topk = topKKnob->getValue();
    int CFG = CFGKnob->getValue();
    std::string prompt = textPrompt->getText();
    prompt.append(" ");
    prompt.append(promptTempo->getText());
    prompt.append(" BPM ");
    prompt.append(promptInstrumentation->getText());
    std::string datetime = getCurrentDateTime();
    std::string dir_name = textPrompt->getText();
    dir_name.append("_");
    dir_name.append(promptTempo->getText());
    dir_name.append("_BPM_");
    dir_name.append(promptInstrumentation->getText());
    dir_name.append("_");
    dir_name.append(datetime);

    // Make a request
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    // Parse and print the response using a JSON library
    Json::Value jsonData;
    Json::CharReaderBuilder readerBuilder;

    // Do a generate request
    if(curl) {
        std::string readBuffer;

        // Prepare curl form data
        struct curl_httppost* form = NULL;
        struct curl_httppost* last = NULL;

        // Add the audio file
        if(selectedFile.size() > 0){
            curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "audioInput",
                    CURLFORM_FILE, selectedFile.c_str(),
                    CURLFORM_CONTENTTYPE, "audio/wav",
                    CURLFORM_END);
        }
        
        // Add other form data
        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "prompt",
                    CURLFORM_COPYCONTENTS, prompt.c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "userid",
                    CURLFORM_COPYCONTENTS, userid.c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "Temperature",
                    CURLFORM_COPYCONTENTS, std::to_string(temperature).c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "Top K",
                    CURLFORM_COPYCONTENTS, std::to_string(topk).c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "Top P",
                    CURLFORM_COPYCONTENTS, std::to_string(topp).c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "Samples",
                    CURLFORM_COPYCONTENTS, std::to_string(samples).c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "Classifier Free Guidance",
                    CURLFORM_COPYCONTENTS, std::to_string(CFG).c_str(),
                    CURLFORM_END);

        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "Duration",
                    CURLFORM_COPYCONTENTS, std::to_string(duration).c_str(),
                    CURLFORM_END);

        // Set URL and form data
        curl_easy_setopt(curl, CURLOPT_URL, ip.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request and store the result
        res = curl_easy_perform(curl);
        // Check for errors
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            if(ip == "http://127.0.0.1:55000/"){
                popupLabel->setLabel("Local server not found, try starting local server.");
            } else {
                popupLabel->setLabel("Could not reach server, try with local server.");
            }
            float padding = 4.f * fScaleFactor;
            popupLabel->resizeToFit();

            popupLabel->onTop(popupPanel, CENTER, CENTER, padding);

            popupPanel->show();
            popupButton->show();
            popupLabel->show();
            repaint();
            crashed = true;
        } else {
            std::string errs;
            std::istringstream ss(readBuffer);
            // std::cout << "Raw Response: " << readBuffer << std::endl;

            if (Json::parseFromStream(readerBuilder, ss, &jsonData, &errs)) {
                // Access the values from the JSON response
                bool success = jsonData["success"].asBool();
                std::string userId = jsonData["userid"].asString();
                const Json::Value downloadLinks = jsonData["download_links"];

                userid = userId;
                std::cout << "Status: " << (success ? "ok" : "error") << std::endl;
                std::cout << "User ID: " << userId << std::endl;
                std::cout << "Download Links:" << std::endl;

                for (const auto &link : downloadLinks) {
                    std::cout << " - " << link.asString() << std::endl;
                }
            } else {
                std::cerr << "Failed to parse JSON: " << errs << std::endl;
            }

            const char* homeDir = std::getenv("HOME"); // Works on Unix-like systems
            std::filesystem::path documentsPath = std::filesystem::path(homeDir) / "Documents" / "MusicGenVST" / "generated" / dir_name;
            std::filesystem::create_directory(documentsPath);

        }

        // Cleanup
        curl_easy_cleanup(curl);
        curl_formfree(form);
    }

    const char* homeDir = std::getenv("HOME"); // Works on Unix-like systems

    // Download the files into the generated folder   
    if(curl && !crashed) {
        for(std::size_t i = 0; i < samplePanels.size(); i++){
            sampleButtons[i]->hide();
        }
        std::string readBuffer;
        const Json::Value downloadLinks = jsonData["download_links"];
        int i = 0;
        for (const auto &link : downloadLinks) {
            const std::string url = std::string(ip) + link.asString();
            std::filesystem::path outputFilename = std::filesystem::path(homeDir) / "Documents" / "MusicGenVST" / "generated" / dir_name / (std::to_string(i) + ".wav");

            curl = curl_easy_init();

            std::ofstream outFile(outputFilename, std::ios::binary);
            if (!outFile)
            {
                std::cerr << "Failed to open file for writing" << std::endl;
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackStream);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);


            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }

            curl_easy_cleanup(curl);

            outFile.close();

            std::string fileName = textPrompt->getText();
            std::cout << fileName << std::endl;

            if(fileName.size() > 40) {
                fileName = fileName.substr(0, 40-3) + "...";
            } 

            std::cout << fileName << std::endl;

            sampleNames[i]->setLabel(fileName);
            sampleButtons[i]->filename = outputFilename;
            sampleNames[i]->show();
            sampleButtons[i]->show();
            i++;
        }
    }
    done = true;
}

void MusicGenUI::startPollingForCompletion(std::atomic<bool>* done) {
    // Use a timer or periodic task in your framework (DPF doesn’t have a built-in timer)
    int i = 0;
    float padding = 4.f * 2.f;
    const std::string spinner = "Loading";
    auto pollCompletion = [this, done, i, spinner, padding]() mutable {
        if (done->load()) {
            loaderPanel->hide();
            loaderSpinner->hide();
            repaint(); // Update the UI to hide the loader

            delete done; // Free the memory allocated for `done`
            return true; // Stop polling
        }

        // loaderSpinner->setLabel(spinner);
        // loaderSpinner->onTop(loaderPanel, CENTER, CENTER, padding);
        // loaderSpinner->resizeToFit();
        std::cout << "\rLoading " << spinner << std::flush;

        repaint(); // Keep the UI responsive while polling
        return false; // Continue polling
    };

    // Replace this with your framework's timer or loop mechanism
    addTimer(pollCompletion, 50); // Call `pollCompletion` every 50ms
}

void MusicGenUI::addTimer(std::function<bool()> callback, int interval) {
    std::thread([callback, interval]() {
        while (true) {
            if (callback()) break; // Stop the timer if the callback returns true
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }).detach();
}

void MusicGenUI::buttonClicked(Button *button)
{
    float padding = 4.f * fScaleFactor;
    if(!loaderPanel->isVisible()){
        // Start making the request
        if(button == generateButton){
            textPrompt->hasKeyFocus = false;
            promptTempo->hasKeyFocus = false;

            promptInstrumentation->hasKeyFocus = false;
            sampleLength->hasKeyFocus = false;
            nSamples->hasKeyFocus = false;

            loaderPanel->show();
            loaderSpinner->show();
            repaint();
            std::atomic<bool>* done = new std::atomic<bool>(false);

            // Start worker threads
            std::thread(&MusicGenUI::generateFn, this, std::ref(*done)).detach();

            // Set up a timer to poll for the `done` flag and update the UI
            startPollingForCompletion(done);
        } else if(button == importButton) {
            const char* filters[] = { "*.wav" };
            const char* filePath = tinyfd_openFileDialog(
                "Select a File",    // Title of the dialog
                "",                 // Default path
                1,                  // Number of filters
                filters,            // File filters
                "WAV Files",        // Filter description
                0                   // Do not allow multiple selections
            );

            if (filePath) {
                std::cout << "Selected file: " << filePath << std::endl;
                selectedFile = static_cast<std::string>(filePath);
                loadedFile->setLabel(getBasename(selectedFile));
                loadedFile->resizeToFit();
                importButton->setSize(loadedFile->getWidth() + importButtonIcon->getWidth() + padding, importButton->getHeight());
                importButton->onTop(importPanel, CENTER, CENTER, 0);
                loadedFile->onTop(importButton, START, CENTER, 0);
                importButtonIcon->onTop(importButton, END, CENTER, 0);
                clearImportedSample->onTop(importButtonIcon, CENTER, CENTER, 0);
                importButtonIcon->hide();
                clearImportedSample->show();
            } else {
                std::cout << "No file selected." << std::endl;
                selectedFile = "";
            }

        } else if(button == clearImportedSample) {
            selectedFile = "";
            loadedFile->setLabel("Upload Audio File");
            loadedFile->resizeToFit();
            importButton->setSize(loadedFile->getWidth() + importButtonIcon->getWidth() + padding, importButton->getHeight());
            importButton->onTop(importPanel, CENTER, CENTER, 0);
            loadedFile->onTop(importButton, START, CENTER, 0);
            importButtonIcon->onTop(importButton, END, CENTER, 0);
            clearImportedSample->onTop(importButtonIcon, CENTER, CENTER, 0);
            importButtonIcon->show();
            clearImportedSample->hide();
        } else if(button == openFolderButton) {
            #ifdef _WIN32
                // Windows
                std::system("explorer .");
            #elif __APPLE__
                // macOS
                std::system("open ~/Documents/MusicGenVST/generated");
                std::cerr << "open ~/Documents/MusicGenVST/generated" << std::endl;
            #elif __linux__
                // Linux
                std::system("xdg-open ~/Documents/MusicGenVST/generated");
            #else
                std::cerr << "Unsupported operating system." << std::endl;
            #endif
        } else if(button == popupButton){
            popupPanel->hide();
            popupButton->hide();
            popupLabel->hide();
        } else if(button == advancedSettingsButton){
            if(advancedSettingsButton->getToggled()){
                setSize(UI_W, UI_H + 150);
    
                generatePanel->setSize(generatePanel->getWidth(), (generatePanel->getHeight()) + 150, true);
                samplesListPanel->setSize(samplesListPanel->getWidth(), (samplesListPanel->getHeight()) + 150, true);
                loaderPanel->setSize(loaderPanel->getWidth(), (loaderPanel->getHeight()) + 150, true);
                localOnlineSwitch->onTop(samplesListPanel, END, END, padding);
    
                advancedSettingsButton->onTop(generatePanel, START, END, padding*2.0f);

                knobsPanel->show();
    
                temperatureKnob->show();
                temperatureLabel->show();
                temperaturePanel->show();
                temperatureUpLabel->show();
    
                topKKnob->show();
                topKLabel->show();
                topKPanel->show();
                topKUpLabel->show();
    
                topPKnob->show();
                topPLabel->show();
                topPPanel->show();
                topPUpLabel->show();
    
                CFGKnob->show();
                CFGLabel->show();
                CFGPanel->show();
                CFGUpLabel->show();
    
    
                localOnlineSwitch->show();
            } else {
                setSize(UI_W, UI_H);
    
                generatePanel->setSize(generatePanel->getWidth(), (generatePanel->getHeight()) - 150, true);
                samplesListPanel->setSize(samplesListPanel->getWidth(), (samplesListPanel->getHeight()) - 150, true);
                loaderPanel->setSize(loaderPanel->getWidth(), (loaderPanel->getHeight()) - 150, true);
                localOnlineSwitch->onTop(samplesListPanel, END, END, padding);

                advancedSettingsButton->onTop(generatePanel, START, END, padding*2.0f);
    
                knobsPanel->hide();
    
                temperatureKnob->hide();
                temperatureLabel->hide();
                temperaturePanel->hide();
                temperatureUpLabel->hide();
    
                topKKnob->hide();
                topKLabel->hide();
                topKPanel->hide();
                topKUpLabel->hide();
    
                topPKnob->hide();
                topPLabel->hide();
                topPPanel->hide();
                topPUpLabel->hide();
    
                CFGKnob->hide();
                CFGLabel->hide();
                CFGPanel->hide();
                CFGUpLabel->hide();
    
                localOnlineSwitch->hide();
            }
        } else{
            for(size_t i = 0; i < drumpadButtons.size(); i++){
                if (button == drumpadButtons[i] && drumpadButtons[i]->filename.size() != 0){
                    plugin->setParameterValue(33, i); // Sample trigger
                    break;
                }
            }

            for(std::size_t i = 0; i < sampleButtons.size(); i++){
                if (button == sampleButtons[i]){
                    std::filesystem::path outputFilename = sampleButtons[i]->filename;
                    std::string selectedFile = static_cast<std::string>(outputFilename);
                    plugin->setParameterValue(0, -1.0f);
                    for(std::size_t j = 0; j < selectedFile.size(); j++){
                        plugin->setParameterValue(0, static_cast<float>(selectedFile[j]));
                        // std::cout << selectedFile[i] << std::endl;
                    }
                    plugin->setParameterValue(0, -2.0f);
                    break;
                }
            }   
        }
    }
}

void MusicGenUI::contextClicked(Button *button)
{
    for(size_t i = 0; i < drumpadButtons.size(); i++){
        if(drumpadButtons[i] == button){
            const char* homeDir = std::getenv("HOME"); // Works on Unix-like systems
            std::filesystem::path outputFilename = std::filesystem::path(homeDir) / "Documents" / "MusicGenVST" / "generated" / "";
            const char* filters[] = { "*.wav" };
            const char* filePath = tinyfd_openFileDialog(
                "Select a File",    // Title of the dialog
                outputFilename.c_str(),                 // Default path
                1,                  // Number of filters
                filters,            // File filters
                "WAV Files",        // Filter description
                1                   // Do not allow multiple selections
            );

            if (filePath) {
                std::cout << "Selected file: " << filePath << std::endl;
                selectedFile = static_cast<std::string>(filePath);
                drumpadButtons[i]->filename = filePath;
            } else {
                std::cout << "No file selected." << std::endl;
                selectedFile = "";
                break;
            }
            std::cout << filePath << std::endl;
            std::string selectedFile = static_cast<std::string>(filePath);
            std::cout << selectedFile << std::endl;
            plugin->setParameterValue(i+1, -1.0f);
            for(std::size_t j = 0; j < selectedFile.size(); j++){
                plugin->setParameterValue(i+1, static_cast<float>(selectedFile[j]));
                // std::cout << selectedFile[i] << std::endl;
            }
            plugin->setParameterValue(i+1, -2.0f);
            break;
        }
    }
}

void MusicGenUI::knobDragStarted(Knob *knob)
{
    if(!loaderPanel->isVisible()){
        if (knob == temperatureKnob){
            temperatureLabel->setValue(knob->getValue());
        } else if (knob == topKKnob){
            topKLabel->setValue(knob->getValue());
        } else if (knob == topPKnob){
            topPLabel->setValue(knob->getValue());
        } else if (knob == CFGKnob){
            CFGLabel->setValue(knob->getValue());
        }
    }
}

void MusicGenUI::knobDragFinished(Knob *knob, float value)
{
    // plugin->triggerPreview();
}

void MusicGenUI::knobValueChanged(Knob *knob, float value)
{
    if(!loaderPanel->isVisible()){
        if (knob == temperatureKnob){
            temperatureLabel->setValue(knob->getValue());
        } else if (knob == topKKnob){
            topKLabel->setValue(knob->getValue());
        } else if (knob == topPKnob){
            topPLabel->setValue(knob->getValue());
        } else if (knob == CFGKnob){
            CFGLabel->setValue(knob->getValue());
        }
    }
}

void MusicGenUI::textEntered(TextInput *textInput, std::string text){

}

void MusicGenUI::textInputChanged(TextInput *textInput, std::string text)
{
    
}

void MusicGenUI::textEntered(NumberInput *numberInput, std::string text){

}

void MusicGenUI::textInputChanged(NumberInput *numberInput, std::string text)
{
    
}

void MusicGenUI::checkboxUpdated(Checkbox *checkbox, bool value)
{
    float padding = 4.f * 2.0f;
    // if(checkbox == advancedSettings){
    //     if(value == true){
    //         setSize(UI_W, UI_H + 150);

    //         generatePanel->setSize(generatePanel->getWidth(), (generatePanel->getHeight()) + 150, true);
    //         samplesListPanel->setSize(samplesListPanel->getWidth(), (samplesListPanel->getHeight()) + 150, true);
    //         loaderPanel->setSize(loaderPanel->getWidth(), (loaderPanel->getHeight()) + 150, true);
    //         localOnlineSwitch->onTop(samplesListPanel, END, END, padding);
    //         localOnlineSwitchLabel->leftOf(localOnlineSwitch, CENTER, padding);

    //         advancedSettings->onTop(generatePanel, START, END, padding);
    //         advancedSettingsLabel->rightOf(advancedSettings, CENTER, padding);

    //         knobsPanel->show();

    //         temperatureKnob->show();
    //         temperatureLabel->show();
    //         temperaturePanel->show();
    //         temperatureUpLabel->show();

    //         topKKnob->show();
    //         topKLabel->show();
    //         topKPanel->show();
    //         topKUpLabel->show();

    //         topPKnob->show();
    //         topPLabel->show();
    //         topPPanel->show();
    //         topPUpLabel->show();

    //         CFGKnob->show();
    //         CFGLabel->show();
    //         CFGPanel->show();
    //         CFGUpLabel->show();


    //         localOnlineSwitch->show();
    //         localOnlineSwitchLabel->show();
    //     } else {
    //         setSize(UI_W, UI_H);

    //         generatePanel->setSize(generatePanel->getWidth(), (generatePanel->getHeight()) - 150, true);
    //         samplesListPanel->setSize(samplesListPanel->getWidth(), (samplesListPanel->getHeight()) - 150, true);
    //         loaderPanel->setSize(loaderPanel->getWidth(), (loaderPanel->getHeight()) - 150, true);
    //         localOnlineSwitch->onTop(samplesListPanel, END, END, padding);
    //         localOnlineSwitchLabel->leftOf(localOnlineSwitch, CENTER, padding);

    //         advancedSettings->onTop(generatePanel, START, END, padding);
    //         advancedSettingsLabel->rightOf(advancedSettings, CENTER, padding);

    //         knobsPanel->hide();

    //         temperatureKnob->hide();
    //         temperatureLabel->hide();
    //         temperaturePanel->hide();
    //         temperatureUpLabel->hide();

    //         topKKnob->hide();
    //         topKLabel->hide();
    //         topKPanel->hide();
    //         topKUpLabel->hide();

    //         topPKnob->hide();
    //         topPLabel->hide();
    //         topPPanel->hide();
    //         topPUpLabel->hide();

    //         CFGKnob->hide();
    //         CFGLabel->hide();
    //         CFGPanel->hide();
    //         CFGUpLabel->hide();

    //         localOnlineSwitch->hide();
    //         localOnlineSwitchLabel->hide();
    //     }
    // } else if(checkbox == localOnlineSwitch){
    //     if(value == true){

    //     } else {

    //     }
    // }
    repaint();
}

void MusicGenUI::addSampleToPanel(float padding, std::string name, float button_height)
{
    float g = 2.f * fscaleMult;
    float h = std::ceil(button_height) * fscaleMult - g;
    // Init the holding panel
    samplePanels.push_back(new Panel(this));
    samplePanels.back()->setSize((samplesListInner->getWidth() * 0.5f * fscaleMult), h);
    samplePanels.back()->roundCorner = true;
    samplePanels.back()->radius = 2;
    if(samplePanels.size() % 2 == 1){
        samplePanels.back()->background_color = WaiveColors::sampleColor1;
    } else {
        samplePanels.back()->background_color = WaiveColors::sampleColor2;
    }
    if(samplePanels.size() == 1){
        samplePanels.back()->onTop(samplesListInner, START, START, g/2);
    } else {
        samplePanels.back()->below(samplePanels[samplePanels.size()-2], START, g/2);
    }
    
    sampleButtons.push_back(new Button(this));
    sampleButtons.back()->sqrt = false;
    sampleButtons.back()->radius = 2;
    sampleButtons.back()->setSize((samplePanels.back()->getWidth() * 0.5 * fscaleMult), h);
    sampleButtons.back()->setLabel(name);
    sampleButtons.back()->background_color = WaiveColors::hiddenColor;
    sampleButtons.back()->highlight_color = WaiveColors::highlight;
    sampleButtons.back()->onTop(samplePanels.back(), START, START, 0);
    sampleButtons.back()->setCallback(this);

    sampleLabelWrappers.push_back(new Panel(this));
    sampleLabelWrappers.back()->background_color = WaiveColors::hiddenColor;
    sampleLabelWrappers.back()->setSize(samplePanels.back()->getWidth() * 0.5f * 0.12f - (padding * 2.0f), h);
    sampleLabelWrappers.back()->onTop(samplePanels.back(), START, CENTER, 0);

    sampleLabels.push_back(new Label(this, std::to_string(sampleLabels.size() + 1)));
    sampleLabels.back()->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
    sampleLabels.back()->setFontSize(fontsize);
    sampleLabels.back()->text_color = WaiveColors::text;
    sampleLabels.back()->background_color = WaiveColors::hiddenColor;
    sampleLabels.back()->resizeToFit();
    sampleLabels.back()->onTop(sampleLabelWrappers.back(), START, CENTER, padding);

    sampleNames.push_back(new Label(this, std::to_string(sampleNames.size() + 1)));
    sampleNames.back()->setFont("Space-Grotesk", SpaceGrotesk_Regular_ttf, SpaceGrotesk_Regular_ttf_len);
    sampleNames.back()->setFontSize(fontsize);
    sampleNames.back()->text_color = WaiveColors::text;
    sampleNames.back()->background_color = WaiveColors::hiddenColor;
    sampleNames.back()->setSize(sampleButtons.back()->getWidth() * 0.5f - sampleLabelWrappers.back()->getWidth() * 0.5f - (padding * 2.0f), sampleLabels.back()->getHeight() - (padding * 2.0f), false);
    // sampleNames.back()->setSize(samplePanels.back()->getWidth() - sampleLabelWrappers.back()->getWidth(), sampleLabelWrappers.back()->getHeight(), false);
    sampleNames.back()->rightOf(sampleLabelWrappers.back(), CENTER, padding);
    sampleNames.back()->hide();

    sampleButtons.back()->hide();

    // samplesRemove.push_back(new Button(this));
    // samplesRemove.back()->setSize((samplePanels.back()->getWidth() * 0.5) * 0.25, 25);
    // samplesRemove.back()->setLabel("⌫");
    // samplesRemove.back()->background_color = WaiveColors::grey2;
    // samplesRemove.back()->onTop(samplePanels.back(), END, END, 0);
}

END_NAMESPACE_DISTRHO
