#pragma once

#include <JuceHeader.h>

#include "PluginEditor.hpp"
#include "BiquadFilter.hpp"

// This must be odd (it seems)
constexpr int MAX_WINDOW = 255;

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
    inline juce::AudioParameterBool* getPhaseParam() { return phaseMode; }

   private:
    juce::AudioParameterFloat* frequency;
    juce::AudioParameterBool* antialiasing;
    juce::AudioParameterBool* phaseMode;

    double theta = 0;

    float hWindow[MAX_WINDOW] = {};

    BiquadFilter aa;
    struct SOState l_s_aa = {}, r_s_aa = {}, l_s2_aa = {}, r_s2_aa = {};

    float block_l[MAX_WINDOW * 2] = {};
    float block_r[MAX_WINDOW * 2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShiftMeAudioProcessor)
};
