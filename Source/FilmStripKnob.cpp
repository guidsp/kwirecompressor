#include "FilmStripKnob.h"

void FilmStripKnob::paint(Graphics& g) {
    if (isHorizontal) {
        g.drawImage(filmStrip, 0, 0, getWidth(), getHeight(),
            static_cast<int>(stripPos * frameWidth), 0, static_cast<int>(frameWidth), static_cast<int>(frameHeight));
    }
    else {
        g.drawImage(filmStrip, 0, 0, getWidth(), getHeight(),
            0, static_cast<int>(stripPos * frameHeight), static_cast<int>(frameWidth), static_cast<int>(frameHeight));
    }

    if (isMouseOverOrDragging() && showText)
    {
        g.setFont(static_cast<float>(getLocalBounds().getHeight()) * textBoxSize);
        juce::Rectangle<float> textSpace = Rectangle<float>::Rectangle(0.15 * this->getWidth(), (1.0f - textBoxSize) * this->getHeight(), 0.70 * this->getWidth(), textBoxSize * this->getHeight());
        juce::Rectangle<int> textSpace_ = Rectangle<int>::Rectangle(0.15 * this->getWidth(), (1.0f - textBoxSize) * this->getHeight(), 0.70 * this->getWidth(), textBoxSize * this->getHeight());

        g.setColour(boxFillColour);
        g.setOpacity(transparency);
        g.fillRect(textSpace);

        g.setColour(boxRimColour);
        g.drawRect(textSpace);

        g.setColour(textColour);
        g.drawFittedText(valueString, textSpace_, juce::Justification::centred, 2, 0.5f);
    }
}

void FilmStripKnob::showValue(const bool& showValueOnHover, const float& textSize) {
    showText = showValueOnHover;
    textBoxSize = jlimit(0.0f, 1.0f, textSize);
}

void FilmStripKnob::showValue(const bool& showValueOnHover) {
    showText = showValueOnHover;
}

void FilmStripKnob::setColours(const juce::Colour& boxRim, const juce::Colour& boxFill, const juce::Colour& text) {
    boxRimColour = boxRim;
    boxFillColour = boxFill;
    textColour = text;
}

void FilmStripKnob::valueChanged() {
    //Change value string
    valueString = std::to_string(this->getValue()); //slider value as a string
    valueString = valueString.dropLastCharacters(stringLengthToRemove);
    valueString.append(suffix, 10);

    //Get new film strip position
    stripPos = roundDoubleToInt(valueToProportionOfLength(this->getValue()) * static_cast<double>(numFrames - 1));
}