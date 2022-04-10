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
    floatHelper(lowGain, Names::Low_Wet);
    floatHelper(midGain, Names::Mid_Wet);
    floatHelper(highGain, Names::High_Wet);

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

    // In case we have more outputs than inputs, this code clears any output
   // channels that didn't contain input data, (because these aren't
   // guaranteed to be empty - they may contain garbage).
   // This is here to avoid people getting screaming feedback
   // when they first compile a plugin, but obviously you don't need to keep
   // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    for ( auto& fb : filterBuffers)
    {
        fb = buffer;
    }

    auto lowCutoff = lowMidCrossover->get();
    auto highCutoff = midHighCrossover->get();
    auto cutoff = lowMidCrossover->get();

    LP.setCutoffFrequency(lowCutoff);
    HP.setCutoffFrequency(lowCutoff);
    AP.setCutoffFrequency(highCutoff);

    LP2.setCutoffFrequency(highCutoff);
    HP2.setCutoffFrequency(highCutoff);

    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);

    auto fb0Context = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Context = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Context = juce::dsp::ProcessContextReplacing<float>(fb2Block);

    LP.process(fb0Context);
    AP2.process(fb1Context);

    HP.process(fb1Context);
    filterBuffers[2] = filterBuffers[1];

    LP2.process(fb1Context);
    HP2.process(fb2Context);
     

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    buffer.clear();

    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source) {
        for (auto i = 0; i < nc ; i++)
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
            

        }
    };




    filterBuffers[0].applyGain(*lowGain);
    filterBuffers[1].applyGain(*midGain);
    filterBuffers[2].applyGain(*highGain);
    
    



    addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);
    addFilterBand(buffer, filterBuffers[2]);

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


    layout.add(std::make_unique<AudioParameterFloat>(   params.at(Names::Low_Mid_Crossover),
                                                        params.at(Names::Low_Mid_Crossover),
                                                        NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                        500.f));
    layout.add(std::make_unique<AudioParameterFloat>(   params.at(Names::Mid_High_Crossover),
                                                        params.at(Names::Mid_High_Crossover),
                                                        NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                        7000.f));


    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BandSplitDelayAudioProcessor();
}
