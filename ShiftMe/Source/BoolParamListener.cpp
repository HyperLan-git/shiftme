#include "BoolParamListener.hpp"

BoolParamListener::BoolParamListener(juce::AudioParameterBool* param,
                                     juce::ToggleButton& button)
    : param(param), button(button) {
    if (*param)
        button.setToggleState(true, juce::NotificationType::sendNotification);

    param->addListener(this);
    button.addListener(this);
}

BoolParamListener::~BoolParamListener() {
    param->removeListener(this);
    button.removeListener(this);
}

void BoolParamListener::buttonClicked(juce::Button* b) {
    param->setValueNotifyingHost(b->getToggleState() ? 1 : 0);
}

void BoolParamListener::parameterValueChanged(int parameterIndex,
                                              float newValue) {
    (void)parameterIndex;
    (void)newValue;
    button.setToggleState(*param, juce::NotificationType::dontSendNotification);
}

void BoolParamListener::parameterGestureChanged(int parameterIndex,
                                                bool gestureIsStarting) {
    (void)parameterIndex;
    (void)gestureIsStarting;
}