/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider 
{
    CustomRotarySlider() : juce::Slider(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::TextBoxBelow)
    {

    }
};


//==============================================================================
/**
*/
class BandSplitDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BandSplitDelayAudioProcessorEditor (BandSplitDelayAudioProcessor&);
    ~BandSplitDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BandSplitDelayAudioProcessor& audioProcessor;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    juce::Label
        lowLabel,
        midLabel,
        highLabel,
        lowDryLabel,
        midDryLabel,
        highDryLabel,
        lowWetLabel,
        midWetLabel,
        highWetLabel,
        lowMidCrosLabel,
        midHighCrosLabel;

    CustomRotarySlider
        lowDrySlider,
        lowWetSlider,
        midDrySlider,
        midWetSlider,
        highDrySlider,
        highWetSlider,

        lowMidCrosSlider,
        midHighCrosSlider;

    Attachment 
        lowDrySliderAttachment,
        lowWetSliderAttachment,
        midDrySliderAttachment,
        midWetSliderAttachment,
        highDrySliderAttachment,
        highWetSliderAttachment,

        lowMidCrosSliderAttachment,
        midHighCrosSliderAttachment;


    std::vector<juce::Component*> getComps();
    std::vector<juce::Label*> getLabels();
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitDelayAudioProcessorEditor)
};
