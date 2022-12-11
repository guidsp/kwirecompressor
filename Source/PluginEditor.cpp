/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define pluginWidth 760
#define pluginHeight 420

//==============================================================================

KwireAudioProcessorEditor::KwireAudioProcessorEditor(KwireAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    compGainKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " db", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    compRatioKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " %", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    compThreshKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " db", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    compAttackKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " ms", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    compReleaseKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " ms", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    mixKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " %", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    outGainKnob(ImageCache::getFromMemory(BinaryData::KnobStrip_png, BinaryData::KnobStrip_pngSize), 128, true, " db", 5, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    
    inMeter(Colour(0xffac0000), Colour(0xff1b1b1b), 60, 5, true),
    compMeter(Colour(0xffac0000), 60, 5, true),
    compReductionMeter(Colour(0xffE2D6F3), Colour(0xff1b1b1b), 60, 5, false),

    compGainSliderAttach((*audioProcessor.treestate.getParameter("compGain")), compGainKnob, nullptr),
    compRatioSliderAttach((*audioProcessor.treestate.getParameter("compRatio")), compRatioKnob, nullptr),
    compThreshSliderAttach((*audioProcessor.treestate.getParameter("compThreshold")), compThreshKnob, nullptr),
    compAttackSliderAttach((*audioProcessor.treestate.getParameter("compAttack")), compAttackKnob, nullptr),
    compReleaseSliderAttach((*audioProcessor.treestate.getParameter("compRelease")), compReleaseKnob, nullptr),
    mixSliderAttach((*audioProcessor.treestate.getParameter("mix")), mixKnob, nullptr),
    outGainSliderAttach((*audioProcessor.treestate.getParameter("outGain")), outGainKnob, nullptr)
{
    //plugin window size
    setSize(pluginWidth, pluginHeight);

    //default font
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Avenir Next");

    //set fixed aspect ratio
    double ratio = (double)getWidth() / getHeight();

    //resize limits
    setResizeLimits(getWidth() * 0.8f, getHeight() * 0.8f, getWidth() * 1.25f, getHeight() * 1.25f);
    
    //fixed aspect ratio
    getConstrainer()->setFixedAspectRatio(ratio);

    //start timer
    Timer::startTimerHz(60);

    //retrieve images from resources
    juce::Image bgImageUnder = juce::ImageCache::getFromMemory(BinaryData::layoutunder_png, BinaryData::layoutunder_pngSize); 
    juce::Image bgImageOver = juce::ImageCache::getFromMemory(BinaryData::layoutover_png, BinaryData::layoutover_pngSize);

    //stretch image to window
    if (bgImageUnder.isValid())
        bgImageComponentUnder.setImage(bgImageUnder, RectanglePlacement::stretchToFit);
    if (bgImageOver.isValid())
        bgImageComponentOver.setImage(bgImageOver, RectanglePlacement::stretchToFit);

    //mouse ignores the overlay component
    bgImageComponentOver.setInterceptsMouseClicks(false, false);

    //describe sliders
    compGainKnob.setRange(
       audioProcessor.treestate.getParameter("compGain")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("compGain")->getNormalisableRange().end, 
        0.1f
    );

    compGainKnob.setValue(audioProcessor.compGain->get());
    compGainKnob.showValue(true, 0.16);
    compGainKnob.addListener(this);

    compRatioKnob.setRange(
        audioProcessor.treestate.getParameter("compRatio")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("compRatio")->getNormalisableRange().end, 
        0.1f
    );

    compRatioKnob.setValue(audioProcessor.compRatio->get());
    compRatioKnob.showValue(true, 0.16);
    compRatioKnob.addListener(this);

    compThreshKnob.setRange(
        audioProcessor.treestate.getParameter("compThreshold")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("compThreshold")->getNormalisableRange().end,
        0.1f
    );

    compThreshKnob.setValue(audioProcessor.compThreshold->get());
    compThreshKnob.showValue(true, 0.16);
    compThreshKnob.addListener(this);

    compAttackKnob.setRange(
        audioProcessor.treestate.getParameter("compAttack")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("compAttack")->getNormalisableRange().end,
        0.1f
    );

    compAttackKnob.setValue(audioProcessor.compAttack->get());
    compAttackKnob.showValue(true, 0.16);
    compAttackKnob.addListener(this);

    compReleaseKnob.setRange(
        audioProcessor.treestate.getParameter("compRelease")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("compRelease")->getNormalisableRange().end,
        0.1f
    );

    compReleaseKnob.setValue(audioProcessor.compRelease->get());
    compReleaseKnob.showValue(true, 0.16);
    compReleaseKnob.addListener(this);

    mixKnob.setRange(
        audioProcessor.treestate.getParameter("mix")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("mix")->getNormalisableRange().end,
        0.1f
    );

    mixKnob.setValue(audioProcessor.mix->get());
    mixKnob.showValue(true, 0.16);
    mixKnob.addListener(this);

    outGainKnob.setRange(
        audioProcessor.treestate.getParameter("outGain")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("outGain")->getNormalisableRange().end,
        0.1f
    );

    outGainKnob.setValue(audioProcessor.outGain->get());
    outGainKnob.showValue(true, 0.16);
    outGainKnob.addListener(this);
    
    //make BG image visible before the other components
    Component::addAndMakeVisible(bgImageComponentUnder); 

    Component::addAndMakeVisible(compGainKnob);
    Component::addAndMakeVisible(compRatioKnob);
    Component::addAndMakeVisible(compThreshKnob);
    Component::addAndMakeVisible(compAttackKnob);
    Component::addAndMakeVisible(compReleaseKnob);
    Component::addAndMakeVisible(mixKnob);
    Component::addAndMakeVisible(outGainKnob);

    Component::addAndMakeVisible(inMeter);
    Component::addAndMakeVisible(compReductionMeter);
    Component::addAndMakeVisible(compMeter);

    Component::addAndMakeVisible(bgImageComponentOver);

    compReductionMeter.setOpaque(false);
    compMeter.setOpaque(false);
}

KwireAudioProcessorEditor::~KwireAudioProcessorEditor()
{
    Timer::stopTimer();
}

//==============================================================================
void KwireAudioProcessorEditor::paint (Graphics& g)
{
    bgImageComponentUnder.setBoundsRelative(0, 0, 1, 1);

    compGainKnob.setBoundsRelative(0.158, 0.14, 0.15, 0.2576);
    compRatioKnob.setBoundsRelative(0.313, 0.14, 0.15, 0.2576);
    compThreshKnob.setBoundsRelative(0.465, 0.14, 0.15, 0.2576);
    compAttackKnob.setBoundsRelative(0.2265, 0.53, 0.15, 0.2576);
    compReleaseKnob.setBoundsRelative(0.4, 0.53, 0.15, 0.2576);
    mixKnob.setBoundsRelative(0.778, 0.14, 0.15, 0.2576);
    outGainKnob.setBoundsRelative(0.778, 0.53, 0.15, 0.2576);

    inMeter.setBoundsRelative(0.0503, 0.05, 0.066666, 0.8);
    compReductionMeter.setBoundsRelative(0.662, 0.05, 0.066666, 0.8);
    compMeter.setBoundsRelative(0.662, 0.05, 0.066666, 0.8);

    bgImageComponentOver.setBoundsRelative(0, 0, 1, 1);
}

void KwireAudioProcessorEditor::timerCallback()
{
    inMeter.getData(audioProcessor.inAudio, audioProcessor.inAudioPeak);
    compReductionMeter.getData(audioProcessor.preCompAudio);
    compMeter.getData(audioProcessor.compAudio, audioProcessor.compAudioPeak);
}

void KwireAudioProcessorEditor::resized()
{
    bgImageComponentUnder.setBoundsRelative(0, 0, 1, 1);

    compGainKnob.setBoundsRelative(0.158, 0.14, 0.15, 0.2576);
    compRatioKnob.setBoundsRelative(0.313, 0.14, 0.15, 0.2576);
    compThreshKnob.setBoundsRelative(0.465, 0.14, 0.15, 0.2576);
    compAttackKnob.setBoundsRelative(0.2265, 0.53, 0.15, 0.2576);
    compReleaseKnob.setBoundsRelative(0.4, 0.53, 0.15, 0.2576);
    mixKnob.setBoundsRelative(0.778, 0.14, 0.15, 0.2576);
    outGainKnob.setBoundsRelative(0.778, 0.53, 0.15, 0.2576);

    inMeter.setBoundsRelative(0.0503, 0.05, 0.066666, 0.8);
    compReductionMeter.setBoundsRelative(0.662, 0.05, 0.066666, 0.8);
    compMeter.setBoundsRelative(0.662, 0.05, 0.066666, 0.8);

    bgImageComponentOver.setBoundsRelative(0, 0, 1, 1);

    inMeter.update();
    compReductionMeter.update();
    compMeter.update();
}

void KwireAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) //this is where the value from the sliders gets passed to the variable
{
    /*
    if (slider == &compGainKnob)
        audioProcessor.compGain = compGainKnob.getValue();
    else if (slider == &compRatioKnob)
        audioProcessor.compRatio = compRatioKnob.getValue();
    else if (slider == &compThreshKnob)
        audioProcessor.compThreshold = compThreshKnob.getValue();
    else if (slider == &compAttackKnob)
        audioProcessor.compAttack = compAttackKnob.getValue();
    else if (slider == &compReleaseKnob)
        audioProcessor.compRelease = compReleaseKnob.getValue();
    else if (slider == &mixKnob)
        audioProcessor.mix = mixKnob.getValue();
    else if (slider == &outGainKnob)
        audioProcessor.outGain = outGainKnob.getValue();
        */
}

