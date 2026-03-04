cmake_minimum_required(VERSION 3.22)

project(MusicGenVST VERSION ${MUSICGENVST_VERSION})

juce_add_plugin(MusicGenVST
    COMPANY_NAME "Thunderboom Records"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Tbmr
    PLUGIN_CODE Mgvs
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "MusicGenVST")

target_sources(MusicGenVST
    PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp)

target_compile_definitions(MusicGenVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=1
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MusicGenVST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(APPLE)
    target_link_libraries(MusicGenVST PRIVATE "-framework IOKit")
endif()
