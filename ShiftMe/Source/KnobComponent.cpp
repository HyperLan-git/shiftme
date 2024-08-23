#include "KnobComponent.hpp"

#define SET_PARAM_NORMALIZED(param, value) \
    param->setValueNotifyingHost(param->convertTo0to1(value))

ParamListener::ParamListener(juce::RangedAudioParameter* param,
                             juce::Slider& slider)
    : param(param), slider(slider) {
    slider.addListener(this);
    param->addListener(this);
}

ParamListener::~ParamListener() {
    slider.removeListener(this);
    param->removeListener(this);
}

void ParamListener::sliderValueChanged(juce::Slider* s) {
    SET_PARAM_NORMALIZED(param, (float)s->getValue());
}

void ParamListener::sliderDragStarted(juce::Slider* s) {
    (void)s;
    param->beginChangeGesture();
}

void ParamListener::sliderDragEnded(juce::Slider* s) {
    (void)s;
    param->endChangeGesture();
}

void ParamListener::parameterValueChanged(int parameterIndex, float newValue) {
    (void)parameterIndex;
    (void)newValue;
    slider.setValue(param->convertFrom0to1(param->getValue()));
}

void ParamListener::parameterGestureChanged(int parameterIndex,
                                            bool gestureIsStarting) {
    (void)parameterIndex;
    (void)gestureIsStarting;
}

KnobComponent::KnobComponent(juce::RangedAudioParameter* param, double step)
    : paramListener(param, knob),
      label(param->getName(128), param->getName(128)) {
    addAndMakeVisible(knob);
    addAndMakeVisible(label);

    label.attachToComponent(&knob, false);
    label.setJustificationType(juce::Justification::centred);

    knob.setLookAndFeel(&lf);
    knob.setColour(juce::Slider::thumbColourId, juce::Colours::grey.brighter());
    knob.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    knob.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 20);

    auto range = param->getNormalisableRange();
    knob.setRange(range.start, range.end, step);
    knob.setValue(param->convertFrom0to1(param->getValue()));

    setSize(100, 100);
}

KnobComponent::~KnobComponent() { knob.setLookAndFeel(nullptr); }

void KnobComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
}

double KnobComponent::getValue() const { return knob.getValue(); }

void KnobComponent::resized() {
    label.setBounds(0, 0, 100, 20);
    knob.setBounds(0, 20, 100, 80);
}
