#pragma once

#include <JuceHeader.h>
#include "Looknfeel.hpp"

class ParamListener : public juce::Slider::Listener,
                      public juce::AudioProcessorParameter::Listener {
   public:
    ParamListener(juce::RangedAudioParameter* param, juce::Slider& slider);

    ~ParamListener() override;

    void sliderValueChanged(juce::Slider* slider) override;

    void sliderDragStarted(juce::Slider* slider) override;

    void sliderDragEnded(juce::Slider* slider) override;

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex,
                                 bool gestureIsStarting) override;

   private:
    juce::RangedAudioParameter* param;
    juce::Slider& slider;
};

class KnobComponent : public juce::Component {
   public:
    KnobComponent(juce::RangedAudioParameter* param, double step = .001);
    ~KnobComponent() override;

    void paint(juce::Graphics& g) override;

    void resized() override;

    double getValue() const;

   private:
    juce::Slider knob;
    juce::Label label;
    Looknfeel lf;

    ParamListener paramListener;
};