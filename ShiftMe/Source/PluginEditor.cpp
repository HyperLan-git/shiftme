#include "PluginEditor.hpp"

ShiftMeAudioProcessorEditor::ShiftMeAudioProcessorEditor(
    ShiftMeAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      freq(p.getFrequencyParam()),
      freqOrPhase("Phase shift mode"),
      aa("Antialiasing"),
      freqOrPhaseListener(p.getPhaseParam(), freqOrPhase),
      aaListener(p.getAliasingParam(), aa) {
    setSize(400, 300);

    this->addAndMakeVisible(freq);
    this->addAndMakeVisible(freqOrPhase);
    this->addAndMakeVisible(aa);
}

ShiftMeAudioProcessorEditor::~ShiftMeAudioProcessorEditor() {}

void ShiftMeAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20);
}

void ShiftMeAudioProcessorEditor::resized() {
    freq.setBounds(0, 0, 100, 100);
    freqOrPhase.setBounds(100, 0, 100, 50);
    aa.setBounds(200, 0, 100, 50);
}
