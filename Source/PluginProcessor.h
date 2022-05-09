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
        Low_Dry,
        Mid_Dry,
        High_Dry,

        Low_Mid_Crossover,
        Mid_High_Crossover,

        Delay_Time,

        Low_Reverb_Size,
        Mid_Reverb_Size,
        High_Reverb_Size,        
    };

    inline const std::map<Names, juce::String>& GetParams() {
        static std::map<Names, juce::String> params = {
        {Low_Wet, "Low Wet"},
        {Mid_Wet, "Mid Wet"},
        {High_Wet, "High Wet"},
        {Low_Dry, "Low Dry"},
        {Mid_Dry, "Mid Dry"},
        {High_Dry, "High Dry"},
        {Low_Mid_Crossover, "Low Mid Crossover"},
        {Mid_High_Crossover, "Mid High Crossover"},
        {Delay_Time, "Delay Time"},
        {Low_Reverb_Size, "Low Reverb Size"},
        {Mid_Reverb_Size, "Mid Reverb Size"},
        {High_Reverb_Size, "High Reverb Size"},
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
    
    //Delay Variables
    void readFromBuffer(
        juce::AudioBuffer<float>& buffer, 
        juce::AudioBuffer<float>& delayBuffer, 
        int channel
    );
    void fillBuffer(
        juce::AudioBuffer<float>& buffer,
        juce::AudioBuffer<float>& delayBuffer,
        int channel
    );
    float ChangeDelayTime(
        int delayTimeIndex
    );
    std::array<juce::AudioBuffer<float>, 3> delayBuffers;
    int writePosition{ 0 };
    juce::AudioBuffer<float> lowDelayBuffer;
    juce::AudioBuffer<float> midDelayBuffer;
    juce::AudioBuffer<float> highDelayBuffer;
    juce::AudioParameterChoice* delayTime {nullptr};
    float denominator { 4 };
    //========     

    //Reverb Variables
    juce::dsp::Reverb highReverb, midReverb, lowReverb;
    juce::dsp::Reverb::Parameters highReverbParams, midReverbParams, lowReverbParams;
    //========

    
    //Filter variables
    using Filter = juce::dsp::LinkwitzRileyFilter<float>;

    void addFilterBand(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& bandBuffer);
    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
    Filter LP, HP, AP;
    Filter LP2, HP2, AP2;
    juce::AudioParameterFloat* lowMidCrossover{ nullptr };
    juce::AudioParameterFloat* midHighCrossover{ nullptr };

    juce::AudioParameterFloat* dryLowGain{ nullptr };
    juce::AudioParameterFloat* dryMidGain{ nullptr };
    juce::AudioParameterFloat* dryHighGain{ nullptr };

    juce::AudioParameterFloat* wetLowGain{ nullptr };
    juce::AudioParameterFloat* wetMidGain{ nullptr };
    juce::AudioParameterFloat* wetHighGain{ nullptr };

    //=====
    
    juce::AudioBuffer<float> wetBuffer;
    std::array<juce::AudioBuffer<float>, 3> dryBuffers;
    juce::AudioPlayHead* playHead{ nullptr };
    double bpm;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitDelayAudioProcessor)
};
