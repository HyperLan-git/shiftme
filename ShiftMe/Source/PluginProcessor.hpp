#pragma once

#include <JuceHeader.h>

#include "PluginEditor.hpp"
#include "BiquadFilter.hpp"

// This must be odd (it seems)
constexpr int MAX_WINDOW = 15;

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
    inline juce::AudioParameterBool* getAliasingParam() { return antialiasing; }

   private:
    juce::AudioParameterFloat* frequency;
    juce::AudioParameterFloat* phase;
    juce::AudioParameterBool* antialiasing;

    double theta = 0;

    float hWindow[MAX_WINDOW] = {};

    BiquadFilter l_aa, r_aa;
    struct SOState l_s = {}, r_s = {};

    float block_l[MAX_WINDOW * 2] = {};
    float block_r[MAX_WINDOW * 2] = {};

    float block_l_aa[MAX_WINDOW * 4] = {};
    float block_r_aa[MAX_WINDOW * 4] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShiftMeAudioProcessor)
};
