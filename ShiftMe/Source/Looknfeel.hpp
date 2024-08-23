#pragma once

#include "JuceHeader.h"

class Looknfeel : public juce::LookAndFeel_V4 {
   public:
    Looknfeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

   private:
};