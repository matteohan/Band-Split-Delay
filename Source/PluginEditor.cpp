/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace Params;
const auto& params = GetParams();


//==============================================================================
BandSplitDelayAudioProcessorEditor::BandSplitDelayAudioProcessorEditor(BandSplitDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    lowDrySliderAttachment(audioProcessor.apvts, params.at(Names::Low_Dry), lowDrySlider),
    lowWetSliderAttachment(audioProcessor.apvts, params.at(Names::Low_Wet), lowWetSlider),
    midDrySliderAttachment(audioProcessor.apvts, params.at(Names::Mid_Dry), midDrySlider),
    midWetSliderAttachment(audioProcessor.apvts, params.at(Names::Mid_Wet), midWetSlider),
    highDrySliderAttachment(audioProcessor.apvts, params.at(Names::High_Dry), highDrySlider),
    highWetSliderAttachment(audioProcessor.apvts, params.at(Names::High_Wet), highWetSlider),
    lowMidCrosSliderAttachment(audioProcessor.apvts, params.at(Names::Low_Mid_Crossover), lowMidCrosSlider),
    midHighCrosSliderAttachment(audioProcessor.apvts, params.at(Names::Mid_High_Crossover), midHighCrosSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* component : getComps()) {
        addAndMakeVisible(component);
    }

    lowDrySlider.setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    midDrySlider.setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    highDrySlider.setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    lowWetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::yellow);
    midWetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::yellow);
    highWetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::yellow);

    lowLabel.setText("LOW", juce::dontSendNotification);
    midLabel.setText("MID", juce::dontSendNotification);
    highLabel.setText("HIGH", juce::dontSendNotification);

    lowMidCrosLabel.setText("Low->Mid \n Crossover", juce::dontSendNotification);
    midHighCrosLabel.setText("Mid->High \n Crossover", juce::dontSendNotification);
    
    lowMidCrosLabel.attachToComponent(&lowMidCrosSlider, false);
    midHighCrosLabel.attachToComponent(&midHighCrosSlider, false);

    lowDryLabel.setText("DRY", juce::dontSendNotification);
    midDryLabel.setText("DRY", juce::dontSendNotification);
    highDryLabel.setText("DRY", juce::dontSendNotification);
    lowWetLabel.setText("WET", juce::dontSendNotification);
    midWetLabel.setText("WET", juce::dontSendNotification);
    highWetLabel.setText("WET", juce::dontSendNotification);

    for (auto* label : getLabels()) {
        label->setJustificationType(juce::Justification::centred);
    }


    
   

    setSize(600, 500);
    setResizable(false, false);
    isResizable();
}

BandSplitDelayAudioProcessorEditor::~BandSplitDelayAudioProcessorEditor()
{
}

//==============================================================================
void BandSplitDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void BandSplitDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    

    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    auto labelArea = bounds.removeFromTop(bounds.getHeight() * 0.1);

    auto bandLabelsArea = labelArea.removeFromTop(labelArea.getHeight() * 0.75);

    auto lowLabelArea = bandLabelsArea.removeFromLeft(bandLabelsArea.getWidth() * 0.33);
    auto midLabelArea = bandLabelsArea.removeFromLeft(bandLabelsArea.getWidth() * 0.5);
    auto highLabelArea = bandLabelsArea;

    lowLabel.setBounds(lowLabelArea);
    midLabel.setBounds(midLabelArea);
    highLabel.setBounds(highLabelArea);

    auto lowDryLabelArea = labelArea.removeFromLeft(labelArea.getWidth() * 0.33);
    auto midDryLabelArea = labelArea.removeFromLeft(labelArea.getWidth() * 0.5);
    auto highDryLabelArea = labelArea;

    auto lowWetLabelArea = lowDryLabelArea.removeFromRight(lowDryLabelArea.getWidth() * 0.5);
    auto midWetLabelArea = midDryLabelArea.removeFromRight(midDryLabelArea.getWidth() * 0.5);
    auto highWetLabelArea = highDryLabelArea.removeFromRight(highDryLabelArea.getWidth() * 0.5);

    lowDryLabel.setBounds(lowDryLabelArea);
    midDryLabel.setBounds(midDryLabelArea);
    highDryLabel.setBounds(highDryLabelArea);
    
    lowWetLabel.setBounds(lowWetLabelArea);
    midWetLabel.setBounds(midWetLabelArea);
    highWetLabel.setBounds(highWetLabelArea);



    auto wetDryArea = bounds.removeFromBottom(bounds.getHeight() * 0.75);

    auto lowDryArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto midDryArea = bounds.removeFromLeft(bounds.getWidth() * 0.5);
    auto highDryArea = bounds;

    auto lowWetArea = lowDryArea.removeFromRight(lowDryArea.getWidth() * 0.5);
    auto midWetArea = midDryArea.removeFromRight(midDryArea.getWidth() * 0.5);
    auto highWetArea = highDryArea.removeFromRight(highDryArea.getWidth() * 0.5);

    lowDrySlider.setBounds(lowDryArea);
    midDrySlider.setBounds(midDryArea);
    highDrySlider.setBounds(highDryArea);
    lowWetSlider.setBounds(lowWetArea);
    midWetSlider.setBounds(midWetArea);
    highWetSlider.setBounds(highWetArea);


    lowMidCrosSlider.setBounds(width * 0.33 - 75, height * 0.6 - 75, 150, 150);
    midHighCrosSlider.setBounds(width * 0.66 - 75, height * 0.6 - 75, 150, 150);




    


    



}

std::vector<juce::Component*> BandSplitDelayAudioProcessorEditor::getComps()
{
    return
    {
        &lowLabel,
        &midLabel,
        &highLabel,


        &lowDryLabel,
        &midDryLabel,
        &highDryLabel,

        &lowWetLabel,
        &midWetLabel,
        &highWetLabel,

        &lowMidCrosLabel,
        &midHighCrosLabel,

        &lowDrySlider,
        &lowWetSlider,
        &midDrySlider,
        &midWetSlider,
        &highDrySlider,
        &highWetSlider,

        &lowMidCrosSlider,
        &midHighCrosSlider
    };
         
}

std::vector<juce::Label*>  BandSplitDelayAudioProcessorEditor::getLabels()
{
    return {

        &lowLabel,
        &midLabel,
        &highLabel,


        &lowDryLabel,
        &midDryLabel,
        &highDryLabel,

        & lowMidCrosLabel,
        & midHighCrosLabel,

        &lowWetLabel,
        &midWetLabel,
        &highWetLabel
    };
   
}