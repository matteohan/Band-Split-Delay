/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BandSplitDelayAudioProcessor::BandSplitDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;
    const auto& params = GetParams();

    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)  {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(lowMidCrossover, Names::Low_Mid_Crossover);
    floatHelper(midHighCrossover, Names::Mid_High_Crossover);
    
    floatHelper(dryLowGain, Names::Low_Dry);
    floatHelper(dryMidGain, Names::Mid_Dry);
    floatHelper(dryHighGain, Names::High_Dry);
    
    floatHelper(wetLowGain, Names::Low_Wet);
    floatHelper(wetMidGain, Names::Mid_Wet);
    floatHelper(wetHighGain, Names::High_Wet);



    delayTime = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(Delay_Time)));
    jassert(delayTime != nullptr);

    LP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    AP.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);


}

BandSplitDelayAudioProcessor::~BandSplitDelayAudioProcessor()
{
}

//==============================================================================
const juce::String BandSplitDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BandSplitDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BandSplitDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BandSplitDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BandSplitDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BandSplitDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BandSplitDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BandSplitDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BandSplitDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BandSplitDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BandSplitDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..



    wetBuffer.setSize(getTotalNumOutputChannels(), sampleRate);

    auto delayBufferSize = sampleRate * 2;
    for (auto& buffer : delayBuffers) {
        buffer.setSize(getTotalNumOutputChannels(), (int)delayBufferSize);
    }

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    LP.prepare(spec);
    HP.prepare(spec);
    AP.prepare(spec);

    LP2.prepare(spec);
    HP2.prepare(spec);
    AP2.prepare(spec);

    lowReverb.prepare(spec);
    midReverb.prepare(spec);
    highReverb.prepare(spec);

    /*lowReverb.setSampleRate(sampleRate);
    midReverb.setSampleRate(sampleRate);
    highReverb.setSampleRate(sampleRate);*/

    for (auto& buffer : filterBuffers) 
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);

    }


}

void BandSplitDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BandSplitDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BandSplitDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo info;
        playHead->getCurrentPosition(info);
        bpm = info.bpm;
    }

    


    for ( auto& fb : filterBuffers)
    {
        fb = buffer;
    }
    // Setting cutoffs for filters
    auto lowCutoff = lowMidCrossover->get();
    auto highCutoff = midHighCrossover->get();
    auto cutoff = lowMidCrossover->get();

    //auto curDelayTime = delayTime->getCurrentChoiceName();

    LP.setCutoffFrequency(lowCutoff);
    HP.setCutoffFrequency(lowCutoff);
    AP.setCutoffFrequency(highCutoff);

    LP2.setCutoffFrequency(highCutoff);
    HP2.setCutoffFrequency(highCutoff);

    

    //Block of audio to be processed(split) by bands
    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    auto fb0Context = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Context = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Context = juce::dsp::ProcessContextReplacing<float>(fb2Block);


    //denominator = ChangeDelayTime(delayTimeIndex);
    

    //Processing the audio
    LP.process(fb0Context);
    AP2.process(fb1Context);

    HP.process(fb1Context);
    filterBuffers[2] = filterBuffers[1];

    LP2.process(fb1Context);
    HP2.process(fb2Context);
    //===
    
    auto i = 0;
    for (auto& buffer : filterBuffers) {
        dryBuffers[i] = buffer;
        i++;
    }



    buffer.clear();

    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {

        for (size_t i = 0; i < 3; i++)
        {
            fillBuffer(filterBuffers[i], delayBuffers[i], channel);
            readFromBuffer(filterBuffers[i], delayBuffers[i], channel);
            fillBuffer(filterBuffers[i], delayBuffers[i], channel);

        }

        /*fillBuffer(filterBuffers[2], delayBuffers[2], channel);
        readFromBuffer(filterBuffers[2], delayBuffers[2], channel);
        fillBuffer(filterBuffers[2], delayBuffers[2], channel);*/

    }

    //for (auto& buffer : filterBuffers) {
    //    auto revBlock = juce::dsp::AudioBlock<float>(buffer);
    //    auto revContext = juce::dsp::ProcessContextReplacing<float>(revBlock);
    //    lowReverb.process(revContext);
    //}
     
    //Controlling volume of bands
    dryBuffers[0].applyGain(*dryLowGain);
    dryBuffers[1].applyGain(*dryMidGain);
    dryBuffers[2].applyGain(*dryHighGain);
    
    filterBuffers[0].applyGain(*wetLowGain);
    filterBuffers[1].applyGain(*wetMidGain);
    filterBuffers[2].applyGain(*wetHighGain);
    //=====


    for (auto& bandBuffer : dryBuffers) {
        addFilterBand(buffer, bandBuffer);
    }
    for (auto& bandBuffer : filterBuffers) {
        addFilterBand(buffer, bandBuffer);
    }


    //auto revBlock = juce::dsp::AudioBlock<float>(buffer);
    //auto revContext = juce::dsp::ProcessContextReplacing<float>(revBlock);

    //lowReverb.process(revContext);


    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffers[2].getNumSamples();

    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

float ChangeDelayTime(int delayTimeIndex) {
    switch (delayTimeIndex)
    {

    //Default quarter note delay
    default:
        return 1;
        break;
    
    //16th note delay
    case 0:
        return 1 / 4;
        break;
    
    //8th note delay
    case 1:
        return 1 / 2;
        break;
    
    //6th note delay
    case 2:
        return 4 / 6;
        break;

    //Quarter note delay
    case 3:
        return 1;
        break;

    //Triplet delay
    case 4:
        return 4 / 3;
        break;

    //Half Note Delay
    case 5:
        return 2;
        break;

    //Whole note delay
    case 6:
        return 4;
        break;
    }
}

void BandSplitDelayAudioProcessor::readFromBuffer(
    juce::AudioBuffer<float>& buffer, 
    juce::AudioBuffer<float>& delayBuffer, 
    int channel
) {

    auto readPosition = writePosition - (getSampleRate() * (60/bpm));
    int delayBufferSize = delayBuffer.getNumSamples();
    int bufferSize = buffer.getNumSamples();

    if (readPosition < 0) {
        readPosition += delayBufferSize;
    }
    auto delayGain = 0.5f;
    if (readPosition + bufferSize < delayBufferSize)
    {
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, delayGain, delayGain);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, delayGain, delayGain);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, delayGain, delayGain);


    }
}

void BandSplitDelayAudioProcessor::fillBuffer(
    juce::AudioBuffer<float>& buffer,
    juce::AudioBuffer<float>& delayBuffer,
    int channel 
) {
    auto delayBufferSize = delayBuffer.getNumSamples();

    auto* channelData = buffer.getWritePointer(channel);
    auto bufferSize = buffer.getNumSamples();
    delayBufferSize = delayBuffer.getNumSamples();

    if (delayBufferSize > bufferSize + writePosition)
    {
        delayBuffer.copyFrom(channel, writePosition, channelData, bufferSize);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - writePosition;
        delayBuffer.copyFrom(channel, writePosition, channelData, numSamplesToEnd);

        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        delayBuffer.copyFrom(channel, 0, channelData + numSamplesToEnd, numSamplesAtStart);

    }
}


void BandSplitDelayAudioProcessor::addFilterBand(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& bandBuffer) {

    auto nc = buffer.getNumChannels();
    auto ns = buffer.getNumSamples();

        for (auto i = 0; i < nc; i++)
        {
            buffer.addFrom(i, 0, bandBuffer, i, 0, ns); 

        }
}
//==============================================================================
bool BandSplitDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BandSplitDelayAudioProcessor::createEditor()
{
    //return new BandSplitDelayAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BandSplitDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BandSplitDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout BandSplitDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();

    juce::StringArray delayTimes = { "1/16", "1/8", "1/6", "1/4", "1/3", "1/2", "1/1"};

    layout.add(std::make_unique<AudioParameterFloat>(params.at( Names::High_Wet),
                                                                params.at(Names::High_Wet),
                                                                NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                                0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(params.at( Names::Mid_Wet),
                                                                params.at(Names::Mid_Wet),
                                                                NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                                0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(params.at( Names::Low_Wet),
                                                                params.at(Names::Low_Wet),
                                                                NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                                0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(params.at( Names::High_Dry),
                                                                params.at(Names::High_Dry),
                                                                NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                                0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(params.at( Names::Mid_Dry),
                                                                params.at(Names::Mid_Dry),
                                                                NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                                0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(params.at( Names::Low_Dry),
                                                                params.at(Names::Low_Dry),
                                                                NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
                                                                0.5f));


    layout.add(std::make_unique<AudioParameterFloat>(   params.at(Names::Low_Mid_Crossover),
                                                        params.at(Names::Low_Mid_Crossover),
                                                        NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                        500.f));
    layout.add(std::make_unique<AudioParameterFloat>(   params.at(Names::Mid_High_Crossover),
                                                        params.at(Names::Mid_High_Crossover),
                                                        NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                        7000.f));
    layout.add(std::make_unique<AudioParameterChoice>(
        params.at(Names::Delay_Time),
        params.at(Names::Delay_Time),
        delayTimes,
        3
        ));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::Low_Reverb_Size),
        params.at(Names::Low_Reverb_Size),
        NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
        0.5f
        ));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::Mid_Reverb_Size),
        params.at(Names::Mid_Reverb_Size),
        NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
        0.5f
        ));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::High_Reverb_Size),
        params.at(Names::High_Reverb_Size),
        NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f),
        0.5f
        ));


    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BandSplitDelayAudioProcessor();
}
