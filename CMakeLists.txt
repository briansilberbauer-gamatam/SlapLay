cmake_minimum_required(VERSION 3.22)

project(SlapLay VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Never bake the local dependency sysroot into distributed binaries.
set(CMAKE_SKIP_RPATH TRUE)

option(SLAPLAY_BUILD_TESTS "Build the offline DSP test executable" ON)
option(SLAPLAY_BUILD_SCREENSHOT "Build the headless editor renderer" ON)
option(SLAPLAY_BUILD_HOST_TEST "Build the VST3 host smoke test" ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_binary_data(SlapLayAssets
    SOURCES
        Resources/LatoRegular.ttf
        Resources/LatoSemibold.ttf
        Resources/LatoBlack.ttf
        Resources/Lato-OFL.txt)

set(SLAPLAY_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Parameters.h
    Source/Presets.h
    Source/dsp/FractionalDelayLine.h
    Source/dsp/FeedbackEQ.h
    Source/dsp/TapeSaturation.h
    Source/dsp/WowFlutter.h
    Source/dsp/SlapbackEngine.h
    Source/ui/Theme.cpp
    Source/ui/Theme.h
    Source/ui/StudioLookAndFeel.cpp
    Source/ui/StudioLookAndFeel.h
    Source/ui/Controls.cpp
    Source/ui/Controls.h
    Source/ui/WaveformDisplay.cpp
    Source/ui/WaveformDisplay.h)

set(SLAPLAY_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1)

juce_add_plugin(SlapLay
    COMPANY_NAME "BLS Audio"
    BUNDLE_ID "com.blsaudio.slaplay"
    PLUGIN_MANUFACTURER_CODE Blsa
    PLUGIN_CODE Slay
    FORMATS VST3 Standalone
    PRODUCT_NAME "SlapLay"
    DESCRIPTION "Tape-style slapback delay with a degraded, modulated feedback loop"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx")

target_sources(SlapLay PRIVATE ${SLAPLAY_SOURCES})
target_include_directories(SlapLay PRIVATE Source)
target_compile_definitions(SlapLay PUBLIC ${SLAPLAY_DEFINITIONS})
target_link_libraries(SlapLay
    PRIVATE
        SlapLayAssets
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

if(SLAPLAY_BUILD_TESTS)
    enable_testing()

    juce_add_console_app(SlapLayTests PRODUCT_NAME "SlapLayTests")
    target_sources(SlapLayTests PRIVATE Tests/DspTests.cpp ${SLAPLAY_SOURCES})
    target_include_directories(SlapLayTests PRIVATE Source)
    target_compile_definitions(SlapLayTests PRIVATE ${SLAPLAY_DEFINITIONS})
    target_link_libraries(SlapLayTests
        PRIVATE
            SlapLayAssets
            juce::juce_audio_utils
            juce::juce_dsp
            juce::juce_gui_extra
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    add_test(NAME slaplay_dsp COMMAND $<TARGET_FILE:SlapLayTests>)
endif()

if(SLAPLAY_BUILD_HOST_TEST)
    juce_add_console_app(SlapLayHostSmokeTest PRODUCT_NAME "SlapLayHostSmokeTest")

    target_sources(SlapLayHostSmokeTest PRIVATE Tools/HostSmokeTest.cpp)
    target_compile_definitions(SlapLayHostSmokeTest PRIVATE
        ${SLAPLAY_DEFINITIONS}
        JUCE_PLUGINHOST_VST3=1)
    target_link_libraries(SlapLayHostSmokeTest
        PRIVATE
            juce::juce_audio_utils
            juce::juce_gui_extra
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    add_dependencies(SlapLayHostSmokeTest SlapLay_VST3)
endif()

if(SLAPLAY_BUILD_SCREENSHOT)
    juce_add_gui_app(SlapLayScreenshot
        PRODUCT_NAME "SlapLay Screenshot"
        COMPANY_NAME "BLS Audio"
        BUNDLE_ID "com.blsaudio.slaplay.screenshot")

    target_sources(SlapLayScreenshot PRIVATE ${SLAPLAY_SOURCES} Tools/RenderEditor.cpp)
    target_include_directories(SlapLayScreenshot PRIVATE Source)
    target_compile_definitions(SlapLayScreenshot PRIVATE ${SLAPLAY_DEFINITIONS})
    target_link_libraries(SlapLayScreenshot
        PRIVATE
            SlapLayAssets
            juce::juce_audio_utils
            juce::juce_dsp
            juce::juce_gui_extra
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
endif()
