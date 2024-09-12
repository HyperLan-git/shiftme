#pragma once

#include <JuceHeader.h>

class BoolParamListener : public juce::Button::Listener,
                          public juce::AudioProcessorParameter::Listener {
   public:
    BoolParamListener(juce::AudioParameterBool* param,
                      juce::ToggleButton& button);

    ~BoolParamListener() override;

    void buttonClicked(juce::Button* b) override;

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex,
                                 bool gestureIsStarting) override;

   private:
    juce::AudioParameterBool* param;
    juce::ToggleButton& button;
};
