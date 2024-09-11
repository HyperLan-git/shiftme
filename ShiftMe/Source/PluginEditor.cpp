#include "PluginEditor.hpp"

ShiftMeAudioProcessorEditor::ShiftMeAudioProcessorEditor(
    ShiftMeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), freq(p.getFrequencyParam()) {
    setSize(400, 300);

    this->addAndMakeVisible(freq);
}

ShiftMeAudioProcessorEditor::~ShiftMeAudioProcessorEditor() {}

void ShiftMeAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20);
}

void ShiftMeAudioProcessorEditor::resized() { freq.setBounds(0, 0, 100, 100); }
