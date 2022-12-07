#pragma once
#include <JuceHeader.h>
using namespace juce;

class FilmStripKnob : public Slider
{
public:
    FilmStripKnob(Image image, const int numFrames_, const bool stripIsHorizontal_, String append, int strLength, float transparency, juce::Slider::SliderStyle style)
        : Slider(),
        numFrames(numFrames_),
        isHorizontal(stripIsHorizontal_),
        filmStrip(image),
        suffix(append),
        stringLengthToRemove(strLength),
        transparency(transparency)
    {
        this->setTextBoxStyle(NoTextBox, 0, 0, 0);
        this->setSliderStyle(style);
        this->showValue(false, 0.3f);
        this->setColours(juce::Colours::darkgrey, juce::Colours::grey, juce::Colours::ghostwhite);

        if (isHorizontal) {
            frameHeight = filmStrip.getHeight();
            frameWidth = filmStrip.getWidth() / numFrames_;
        }
        else {
            frameHeight = filmStrip.getHeight() / numFrames_;
            frameWidth = filmStrip.getWidth();
        }

        valueString = std::to_string(this->getValue());
    }

    void paint(Graphics& g);

    /// Set whether to show value on mouse hover. textSize (0 - 1) sets how large the value's text box is relative to the knob.
    void showValue(const bool& showValueOnHover, const float& textSize);

    /// Set whether to show textbox with value on mouse hover.
    void showValue(const bool& showValueOnHover);

    /// Set text box colours. Default colours are grey and white.
    void setColours(const juce::Colour& boxRim, const juce::Colour& boxFill, const juce::Colour& text);

    void valueChanged() override;
    int getFrameWidth() const { return frameWidth; }
    int getFrameHeight() const { return frameHeight; }

private:
    int stripPos;
    bool showText;
    String valueString;
    float textBoxSize;
    float frameWidth,
        frameHeight;

    const int numFrames;
    const bool isHorizontal;
    const Image filmStrip;
    const int stringLengthToRemove;
    const String suffix;
    const float transparency;

    juce::Colour boxRimColour,
        boxFillColour,
        textColour;
};