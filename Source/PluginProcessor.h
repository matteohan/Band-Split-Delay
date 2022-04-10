/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Params {

    enum Names {

        Low_Wet,
        Mid_Wet,
        High_Wet,

        Low_Mid_Crossover,
        Mid_High_Crossover,
    };

    inline const std::map<Names, juce::String>& GetParams() {
        static std::map<Names, juce::String> params = {
        {Low_Wet, "Low Wet"},
        {Mid_Wet, "Mid Wet"},
        {High_Wet, "High Wet"},
        {Low_Mid_Crossover, "Low Mid Crossover"},
        {Mid_High_Crossover, "Mid High Crossover"},
        };

        return params;
    }
}

//==============================================================================
/**
*/
class BandSplitDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BandSplitDelayAudioProcessor();
    ~BandSplitDelayAudioProcessor() override;

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

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };
    

private:   

    using Filter = juce::dsp::LinkwitzRileyFilter<float>;
    Filter LP, HP, AP;
    Filter LP2, HP2, AP2;
    juce::AudioParameterFloat* lowMidCrossover{ nullptr };
    juce::AudioParameterFloat* midHighCrossover{ nullptr };
    juce::AudioParameterFloat* lowGain{ nullptr };
    juce::AudioParameterFloat* midGain{ nullptr };
    juce::AudioParameterFloat* highGain{ nullptr };


    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
        
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitDelayAudioProcessor)
};
