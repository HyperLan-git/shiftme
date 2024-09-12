#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.hpp"
#include "KnobComponent.hpp"
#include "BoolParamListener.hpp"

class ShiftMeAudioProcessor;

class ShiftMeAudioProcessorEditor : public juce::AudioProcessorEditor {
   public:
    ShiftMeAudioProcessorEditor(ShiftMeAudioProcessor&);
    ~ShiftMeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

   private:
    ShiftMeAudioProcessor& audioProcessor;

    KnobComponent freq;
    juce::ToggleButton freqOrPhase, aa;
    BoolParamListener freqOrPhaseListener, aaListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShiftMeAudioProcessorEditor)
};
