cmake_minimum_required(VERSION 3.22)
project(DRM_RESHAPE VERSION 1.2.1 LANGUAGES C CXX)

option(DRMRESHAPE_BUILD_TESTS "Build DSP validation tests" ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DRMRESHAPE
    COMPANY_NAME "yeahwaah audio"
    BUNDLE_ID "audio.yeahwaah.drmreshape"
    PLUGIN_MANUFACTURER_CODE Ywaa
    PLUGIN_CODE DrSh
    PRODUCT_NAME "DRM-RESHAPE"
    DESCRIPTION "Drum-focused equalizer and tone shaper"
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(DRMRESHAPE)

target_sources(DRMRESHAPE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/DrumShaperDSP.cpp
    Source/DrumShaperDSP.h
    Source/DrumShaperCharacter.cpp
    Source/DrumShaperCharacter.h)

target_compile_features(DRMRESHAPE PRIVATE cxx_std_17)
target_compile_definitions(DRMRESHAPE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(DRMRESHAPE PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(DRMRESHAPE_BUILD_TESTS)
    enable_testing()
    add_executable(DRMRESHAPE_DSPTests Tests/DSPTests.cpp Source/DrumShaperDSP.cpp Source/DrumShaperCharacter.cpp)
    target_include_directories(DRMRESHAPE_DSPTests PRIVATE Source)
    target_compile_features(DRMRESHAPE_DSPTests PRIVATE cxx_std_17)
    target_link_libraries(DRMRESHAPE_DSPTests PRIVATE juce::juce_core juce::juce_audio_basics juce::juce_dsp)
    add_test(NAME DrumShaperDSP COMMAND DRMRESHAPE_DSPTests)
endif()
