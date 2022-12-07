#pragma once
#include <JuceHeader.h>
#include "PluginEditor.h"
using namespace juce;

template <int chNum>
class K_Meter : public juce::AnimatedAppComponent
{
public:
    //Make meters with background
    K_Meter(Colour metercolour, Colour bgmetercolour, int framerate, int interpolationSteps, bool peakOn) {
        this->setOpaque(false);
        setFramesPerSecond(framerate);
        barColour = metercolour;
        backgroundOn = true;
        bgBarColour = bgmetercolour;
        showPeakOn = peakOn;
        interpSteps = interpolationSteps;

        inData.resize(chNum, 0.f);
        inPeakData.resize(chNum, 0.f);
        dbValue.resize(chNum, -70.f);
        peak.resize(chNum, -70.f);
        rawValue.resize(chNum, 0.f);
        maxRawValue.resize(chNum, 0.f);

        std::vector<float> interpolationStepChannel; //Vector with channels

        interpolationStepChannel.clear();
        interpolationStepChannel.resize(interpSteps, 0.f);
        interpolationStep.clear();
        interpolationStep.resize(chNum, interpolationStepChannel);
    }

    //Make meters with no background
    K_Meter(Colour metercolour, int framerate, int interpolationSteps, bool peakOn)
    {
        this->setOpaque(false);
        setFramesPerSecond(framerate);
        barColour = metercolour; //save colour
        backgroundOn = false;
        showPeakOn = peakOn;
        interpSteps = interpolationSteps;

        inData.resize(chNum, 0.f);
        inPeakData.resize(chNum, 0.f);
        dbValue.resize(chNum, -70.f);
        peak.resize(chNum, -70.f);
        rawValue.resize(chNum, 0.f);
        maxRawValue.resize(chNum, 0.f);

        std::vector<float> interpolationStepChannel;

        interpolationStepChannel.clear();
        interpolationStepChannel.resize(interpSteps, 0.f);
        interpolationStep.clear();
        interpolationStep.resize(chNum, interpolationStepChannel);
    }

    //Use this in timerCallback() to pull amplitude data from the processor
    inline void getData(const std::vector<juce::Atomic<float>> &rmsData)
    {
       for (int i = 0; i < chNum; i++)
        {
            inData[i] = abs(rmsData[i].get());
            inData[i] = inData[i] > 1.f ? 1.f : inData[i];
        }
    }

    //Use this in timerCallback() to pull amplitude data from the processor
    inline void getData(const std::vector<juce::Atomic<float>> &rmsData, const std::vector<juce::Atomic<float>> &peakLevelData)
    {
        for (int i = 0; i < chNum; i++)
        {
            inData[i] = abs(rmsData[i].get());
            inData[i] = inData[i] > 1.f ? 1.f : inData[i];

            inPeakData[i] = abs(peakLevelData[i].get());
            inPeakData[i] = inPeakData[i] > 1.f ? 1.f : inPeakData[i];
        }
    }

    //Calculate peak value
    void getPeak() {
        //peak value calculation and logic
        if (!isMouseOverOrDragging())
        {
            //show peak normally
            for (int i = 0; i < chNum; i++)
            {
                if (20.0f * log10(inPeakData[i]) > peak[i])
                    peak[i] = 20 * log10(inPeakData[i]);
            }
        }
        else
        {
            //reset the peak
            for (int i = 0; i < chNum; i++)
                peak[i] = 20 * log10(inPeakData[i]);
        }

        //Show peak value
        //peakStr = std::to_string(peak); 
        //peakStr = peakStr.dropLastCharacters(4);
    }

    //Paint the meters
    void paint(Graphics& g) override {

        for (int i = 0; i < chNum; i++)
        {
            if (backgroundOn)
            {
                g.setColour(bgBarColour);
                g.fillRect(BGbarSpace[i]);
            }

            g.setColour(barColour);
            g.fillRect(barSpace[i]);

            if (showPeakOn)
            {
                g.setColour(barColour);
                g.fillRect(peakSpace[i]);
            }
        }

        //Text space
        //g.setColour(juce::Colours::ghostwhite);
        //g.drawFittedText(peakStr, textSpace.toNearestInt(), juce::Justification::centred, false);

    }

    //Update
    void update() override {
        for (int i = 0; i < chNum; i++)
        {
            //=====
            //Interpolate the last (interpSteps) values for smooth meter movement.

            //Replace interpolation buffer step n with n+1, n+2..
            interpolationStep[i].erase(interpolationStep[i].begin());

            //Push incoming data to LAST spot in the array
            interpolationStep[i].push_back(inData[i]);

            //Reset rawValue for calculation of new buffer
            rawValue[i] = 0.f;

            //Sum interpolation steps and make value
            //sum the values in the arrays
            rawValue[i] = std::accumulate(interpolationStep[i].begin(), interpolationStep[i].end(), 0.f);

            rawValue[i] /= interpSteps; //normalise
            rawValue[i] = jlimit(0.f, 1.f, rawValue[i]); //clip
            rawValue[i] = 20 * log10(rawValue[i]);

            //calculate dbValue if the new value is larger
            if (rawValue[i] >= maxRawValue[i])
            {
                dbValue[i] = rawValue[i];

                maxRawValue[i] = rawValue[i];
            }
            //Shrink meter size if new value isn't larger
            else
            {
                dbValue[i] -= linearShrinkRate + exponentialShrinkRate * Decibels::decibelsToGain(dbValue[i]);
                maxRawValue[i] -= linearShrinkRate + exponentialShrinkRate * Decibels::decibelsToGain(maxRawValue[i]);
            }

            //Clip between 0.0 db and minDb
            dbValue[i] = jlimit(minDb, 0.f, dbValue[i]);


            //Calculate meter position / upper limit
            auto size = 1 - (dbValue[i] / minDb);
            //Exponential
            size = size * size;

            //Calculate rectangles for meters.
            //Is not static because component might be resized (also doesn't work in the constructor because component size hasn't been set at that point).
            float meterHeight = this->getHeight() * 0.89f; //Meter height within component
            float meterYstart = this->getHeight() * 0.1f; //Meter Y start position within component

            float barHeight = meterHeight * size;
            float barY = (float)this->getHeight() * 0.1f + meterHeight * (1.f - size);

            textSpace = Rectangle<float>::Rectangle(0.05f * this->getWidth(), 0, 0.9f * this->getWidth(), 0.08f * this->getHeight());

            //Set rectangle for meter BG
            for (int i = 0; i < chNum; i++)
                BGbarSpace[i] = Rectangle<float>::Rectangle(meterXpos(i), meterYstart, meterWidth(), meterHeight);

            //Set rectangle for meter
            barSpace[i] = Rectangle<float>::Rectangle(meterXpos(i), barY, meterWidth(), barHeight);

            if (showPeakOn)
            {
                //Calculate peak value
                getPeak();

                //Calculate position along Y. Limit to avoid signals below minDb making negative peakPos values.
                auto peakPos = jlimit(0.f, 1.f, 1 - peak[i] / minDb);
                //Exponential
                peakPos = peakPos * peakPos;

                auto peakY = (float)this->getHeight() * 0.1f + meterHeight * (1 - peakPos);

                //Set rectangle for peak
                peakSpace[i] = Rectangle<float>::Rectangle(meterXpos(i), peakY, meterWidth(), peakBarThickness);
            }
        }
    }

private:
    constexpr static float linearShrinkRate = 0.15f, //meter shrinking rates
        exponentialShrinkRate = 0.6f,
        minDb = -70.f, //lowest db value to display; bottom of the rectangle.
        peakBarThickness = 3.f;

    bool backgroundOn = false, //background colour display toggle
        showPeakOn; //peak level marker display toggle

    Colour barColour, //meter colours
        bgBarColour;

    int interpSteps; //number of meter interpolation steps / ballistics smoothness
        //channels;  //number of channels

    std::vector<float> inData, //Absolute value of incoming RMS amplitude
        inPeakData, //Absolute value of incoming peak amplitude
        dbValue, //Meter value in dB
        peak, //Peak value in dB
        rawValue, //meter data in amplitude
        maxRawValue; //Used to compare to next value

    //space for elements
    Rectangle<float> BGbarSpace[chNum],
        barSpace[chNum],
        peakSpace[chNum],
        textSpace;

    //Peak value text
    String peakStr;

    std::vector<std::vector<float>> interpolationStep;
    //dsp::AudioBlock<float> inBlock[chNum];

    ///Methods
    //Returns the meters' width within the component rectangle
    float meterWidth() {
        float width;

        if (chNum == 1)
            width = 1.f;
        else
            width = (float)this->getWidth() * (0.8f / chNum);

        return width;
    }

    //Returns the x position of a meter within the component rectangle for the specified channel 
    float meterXpos(const int &channel) {
        float meterWidth, meterGap;

        if (chNum == 1) {
            meterWidth = 1.f;
            meterGap = 0.f;
        }
        else {
            meterWidth = (float)this->getWidth() * (0.8f / chNum);
            meterGap = (float)this->getWidth() * 0.2f / (chNum - 1);
        }

        float x = channel * (meterWidth + meterGap);

        return x;
    }
};