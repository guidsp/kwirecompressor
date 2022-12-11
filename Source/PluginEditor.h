#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilmStripKnob.h"
#include "K_Meter.h"
using namespace juce;

class KwireAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public Timer
{
public:
    KwireAudioProcessorEditor (KwireAudioProcessor&);
    ~KwireAudioProcessorEditor() override;

    void paint (Graphics&) override;
    void resized() override;    
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

private:
    KwireAudioProcessor& audioProcessor;

    ImageComponent bgImageComponentUnder,
        bgImageComponentOver;

    FilmStripKnob compGainKnob,
        compRatioKnob,
        compThreshKnob,
        compAttackKnob,
        compReleaseKnob,
        mixKnob,
        outGainKnob;

    K_Meter<supportedChannels> inMeter,
        compMeter,
        compReductionMeter;

    SliderParameterAttachment compGainSliderAttach,
        compRatioSliderAttach,
        compThreshSliderAttach,
        compAttackSliderAttach,
        compReleaseSliderAttach,
        mixSliderAttach,
        outGainSliderAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KwireAudioProcessorEditor)
};
