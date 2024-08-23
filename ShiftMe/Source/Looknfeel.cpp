#include "Looknfeel.hpp"

Looknfeel::Looknfeel() {}

constexpr int SIDES = 10;

void Looknfeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width,
                                 int height, float sliderPos,
                                 float rotaryStartAngle, float rotaryEndAngle,
                                 juce::Slider& slider) {
    juce::Colour outline =
        slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    juce::Colour fill =
        slider.findColour(juce::Slider::rotarySliderFillColourId);

    juce::Rectangle<float> bounds =
        juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    int radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    float toAngle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    float lineW = juce::jmin(6.0f, radius * 0.5f);
    float arcRadius = radius - lineW * 0.5f;

    if (slider.isEnabled()) {
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                               arcRadius + lineW / 4, arcRadius + lineW / 4,
                               0.0f, rotaryStartAngle, toAngle, true);

        g.setColour(fill);
        g.strokePath(valueArc,
                     juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    juce::Path knob;

    float ang = toAngle - juce::MathConstants<float>::pi / SIDES;
    knob.preallocateSpace(1 + 3 * (SIDES + 1));
    knob.startNewSubPath(bounds.getCentre() +
                         juce::Point<float>(arcRadius * std::cos(ang),
                                            arcRadius * std::sin(ang)));
    for (int i = 0; i < SIDES; i++) {
        float angle = i * juce::MathConstants<float>::twoPi / SIDES + ang;
        juce::Point<float> pos(std::cos(angle) * arcRadius,
                               std::sin(angle) * arcRadius);
        pos += bounds.getCentre();
        knob.lineTo(pos);
    }
    knob.closeSubPath();

    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    g.fillPath(knob);
    g.setColour(slider.findColour(juce::Slider::thumbColourId).darker());
    g.strokePath(knob, juce::PathStrokeType(1.5f));

    float thumbWidth = lineW / 2;
    arcRadius *= .75;
    juce::Point<float> thumbPoint(
        bounds.getCentreX() +
            arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
        bounds.getCentreY() +
            arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));

    g.setColour(
        slider.findColour(juce::Slider::thumbColourId).darker().contrasting());
    g.fillEllipse(thumbPoint.x - thumbWidth / 2, thumbPoint.y - thumbWidth / 2,
                  thumbWidth, thumbWidth);
}