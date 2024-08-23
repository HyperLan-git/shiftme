#pragma once

#include <JuceHeader.h>

#include "PluginEditor.hpp"

constexpr int MAX_WINDOW = 63;

constexpr float pi = juce::MathConstants<float>::pi,
                tpi = juce::MathConstants<float>::twoPi;

class ShiftMeAudioProcessor : public juce::AudioProcessor {
   public:
    ShiftMeAudioProcessor();
    ~ShiftMeAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    inline juce::AudioParameterFloat* getFrequencyParam() { return frequency; }

   private:
    juce::AudioParameterFloat* frequency;

    float hWindow[MAX_WINDOW] = {};

    float block[MAX_WINDOW * 2] = {};
    float block2[MAX_WINDOW] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShiftMeAudioProcessor)
};
