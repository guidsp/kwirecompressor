/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "K_Kwire.h"
constexpr auto supportedChannels = 2;
constexpr auto osFactor = 1;

class KwireAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    KwireAudioProcessor();
    ~KwireAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioParameterFloat *compGain,
        *compRatio,
        *compThreshold,
        *compAttack,
        *compRelease,
        *mix,
        *outGain;

    std::vector<juce::Atomic<float>> inAudio,
        inAudioPeak,
        compAudio,
        compAudioPeak,
        preCompAudio;

    //treestate
    juce::AudioProcessorValueTreeState treestate;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout makeParams();

    float prevCompGain = 0.0f,
        prevOutGain = 0.0f;
   
    //Compressor
    K_Kwire<supportedChannels> kwire;

    juce::AudioBuffer<float> dryBuffer;

    juce::dsp::Oversampling<float> oversampler, //Oversampler
        dryOversampler; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KwireAudioProcessor)
};
