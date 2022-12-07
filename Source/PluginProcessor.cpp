/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KwireAudioProcessor::KwireAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    treestate(*this, nullptr, "PARAMETERS", {  // Add parameters to the treestate
        std::make_unique<juce::AudioParameterFloat>("compGain", "Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>("compRatio", "Ratio", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.f),
        std::make_unique<juce::AudioParameterFloat>("compThreshold", "Threshold", juce::NormalisableRange<float>(-24.0f, 0.0f, 0.1f), -12.0f),
        std::make_unique<juce::AudioParameterFloat>("compAttack", "Attack", juce::NormalisableRange<float>(1.0f, 200.0f, 0.1f, 0.5f), 30.f),
        std::make_unique<juce::AudioParameterFloat>("compRelease", "Release", juce::NormalisableRange<float>(1.0f, 800.f, 1.f, 0.5f), 15.f),
        std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.f),
        std::make_unique<juce::AudioParameterFloat>("outGain", "Output Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.0f)
        }),
    oversampler(supportedChannels, osFactor, juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, true, true),
    dryOversampler(supportedChannels, osFactor, juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, true, true)
#endif
{
    compGain = *treestate.getRawParameterValue("compGain");
    compRatio = *treestate.getRawParameterValue("compRatio");
    compThreshold = *treestate.getRawParameterValue("compThreshold");
    compAttack = *treestate.getRawParameterValue("compAttack");
    compRelease = *treestate.getRawParameterValue("compRelease");
}

KwireAudioProcessor::~KwireAudioProcessor()
{
}

//==============================================================================
const juce::String KwireAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KwireAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KwireAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KwireAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KwireAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KwireAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KwireAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KwireAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KwireAudioProcessor::getProgramName (int index)
{
    return {};
}

void KwireAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KwireAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    kwire.setupParams(compRatio, compThreshold, compAttack, compRelease, sampleRate);

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    inAudio.resize(totalNumInputChannels);
    inAudioPeak.resize(totalNumInputChannels);
    compAudio.resize(totalNumInputChannels);
    compAudioPeak.resize(totalNumInputChannels);
    preCompAudio.resize(totalNumInputChannels);

    lastSamplingRate = sampleRate;

    oversampler.numChannels = getTotalNumInputChannels(); //set os numchannels
    oversampler.setUsingIntegerLatency(true);
    oversampler.clearOversamplingStages();

    oversampler.addOversamplingStage(juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, 0.15f, -90.0f, 0.15f, -90.0f);

    oversampler.reset();
    oversampler.initProcessing(samplesPerBlock);

    dryOversampler.numChannels = getTotalNumInputChannels(); //set os numchannels
    dryOversampler.setUsingIntegerLatency(true);
    dryOversampler.clearOversamplingStages();

    dryOversampler.addOversamplingStage(juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, 0.15f, -90.0f, 0.15f, -90.0f);

    dryOversampler.reset();
    dryOversampler.initProcessing(samplesPerBlock);
    
    setLatencySamples(oversampler.getLatencyInSamples());
}

void KwireAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KwireAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void KwireAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
     //   buffer.clear (i, 0, buffer.getNumSamples());

    //send input to in meter
    for (int channel = 0; channel < supportedChannels; ++channel) {
        inAudio[channel].set(buffer.getRMSLevel(channel, 0, buffer.getNumSamples()));
        inAudioPeak[channel].set(buffer.getMagnitude(channel, 0, buffer.getNumSamples()));
    }

    //Make a copy of the buffer at this point
    dryBuffer.makeCopyOf(buffer);

    juce::dsp::AudioBlock<float> block(buffer); //point block to the buffer
    juce::dsp::AudioBlock<float> dryBlock(dryBuffer); //Points dry block at buffer copy before processing

    auto compGain_ = pow(10, compGain / 20);
    buffer.applyGainRamp(0, buffer.getNumSamples(), prevCompGain, compGain_);
    prevCompGain = compGain_;

    for (int channel = 0; channel < supportedChannels; ++channel) {
        preCompAudio[channel].set(buffer.getRMSLevel(channel, 0, buffer.getNumSamples()));
    }

    auto& osBlock = oversampler.processSamplesUp(block); //make oversampled block
    auto& dryOsBlock = dryOversampler.processSamplesUp(dryBlock);

    //update params
    //Ratio range (1 - 2)
    kwire.updateParams(1.f + compRatio * 0.01f, compThreshold, compAttack, compRelease);

    //compress
    kwire.compress(osBlock);

    //overdrive
    kwire.overdrive(osBlock);

    //downsampling
    oversampler.processSamplesDown(block);
    dryOversampler.processSamplesDown(dryBlock);

    for (int channel = 0; channel < supportedChannels; ++channel) {
        compAudio[channel].set(buffer.getRMSLevel(channel, 0, buffer.getNumSamples()));
        compAudioPeak[channel].set(buffer.getMagnitude(channel, 0, buffer.getNumSamples()));
    }

    //Mix
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);
        auto* dryData = dryBlock.getChannelPointer(channel);

        auto scaledMix = jlimit(0.f, 100.f, mix) * 0.01f;
        
       for (int sample = 0; sample < block.getNumSamples(); ++sample)
            channelData[sample] = channelData[sample] * scaledMix + dryData[sample] * (1.f - scaledMix);
    }

    //Out gain
    auto outGain_ = pow(10, outGain / 20);
    buffer.applyGainRamp(0, buffer.getNumSamples(), prevOutGain, outGain_);
    prevOutGain = outGain_;
}

//==============================================================================
bool KwireAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KwireAudioProcessor::createEditor()
{
    return new KwireAudioProcessorEditor (*this);
}

//==============================================================================
void KwireAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treestate.copyState(); 
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KwireAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes)); 

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treestate.state.getType()))
            treestate.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KwireAudioProcessor();
}