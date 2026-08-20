cmake_minimum_required(VERSION 3.22)

project(SSAB VERSION 1.0.0
        DESCRIPTION "SSAB - Extreme Bass Synthesizer"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SSAB_ENABLE_VST2 "Enable VST2 (legacy) format build" ON)

# ------------------------------------------------------------------
# JUCE via FetchContent (auto-downloaded, no manual JUCE install)
# ------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)

# ------------------------------------------------------------------
# VST2 SDK stub headers (open-source replacement for Steinberg's
# discontinued VST2 SDK). Allows JUCE to build a VST2 .dll without
# needing a local copy of the proprietary SDK.
# Repo: https://github.com/robbert-vdh/juce6-vst2-headers
# ------------------------------------------------------------------
if(SSAB_ENABLE_VST2)
    FetchContent_Declare(
        vst2_headers
        GIT_REPOSITORY https://github.com/robbert-vdh/juce6-vst2-headers.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(vst2_headers)
    message(STATUS "SSAB: VST2 stub headers at: ${vst2_headers_SOURCE_DIR}")
endif()

FetchContent_MakeAvailable(JUCE)

# ------------------------------------------------------------------
# SSAB plugin target
# ------------------------------------------------------------------
if(SSAB_ENABLE_VST2)
    juce_set_vst2_sdk_path(${vst2_headers_SOURCE_DIR})
    set(SSAB_FORMATS VST VST3 Standalone)
else()
    set(SSAB_FORMATS VST3 Standalone)
endif()

juce_add_plugin(SSAB
    COMPANY_NAME           "SSAB NOISE LAB"
    BUNDLE_ID              "com.ssabnoiselab.ssab"
    PLUGIN_MANUFACTURER_CODE  SnLb
    PLUGIN_CODE               Ssab
    FORMATS                   ${SSAB_FORMATS}
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    PLUGIN_VST3_CATEGORY      "Instrument|Synth"
    PRODUCT_NAME              "SSAB"
)

# ------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------
target_sources(SSAB
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Parameters.cpp
        Source/Presets.cpp
        Source/DSP/Oscillator.cpp
        Source/DSP/Filter.cpp
        Source/DSP/Distortion.cpp
        Source/DSP/Envelope.cpp
        Source/DSP/LFO.cpp
        Source/DSP/Voice.cpp
        Source/DSP/DSPUtils.cpp
        Source/GUI/LookAndFeel.cpp
        Source/GUI/SSABKnob.cpp
        Source/GUI/SSABSwitch.cpp
        Source/GUI/SSABDisplay.cpp
)

target_include_directories(SSAB
    PRIVATE
        Source
        Source/DSP
        Source/GUI
)

# ------------------------------------------------------------------
# Binary resources (fonts embedded)
# ------------------------------------------------------------------
juce_add_binary_data(SSAB_Data
    SOURCES
        Source/Resources/HorrorNightDrip.ttf
        Source/Resources/ShockHorror.otf
)
set_target_properties(SSAB_Data PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(SSAB
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        SSAB_Data
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(SSAB
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=1
        JUCE_DISPLAY_SPLASH_SCREEN=0
)
