/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BandSplitDelayAudioProcessorEditor::BandSplitDelayAudioProcessorEditor (BandSplitDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto& component : getComps()) {
        addAndMakeVisible(component);
    }

    lowDrySlider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
    midDrySlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    highDrySlider.setColour(juce::Slider::thumbColourId, juce::Colours::yellow);



    //addAndMakeVisible(lowDrySlider);
    //addAndMakeVisible(midDrySlider);
    //addAndMakeVisible(highDrySlider);

    setSize (600, 500);
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

    g.drawLine(getWidth() * 0.33, 10, getWidth() * 0.33, getHeight() - 10);
    g.drawLine(getWidth() * 0.66, 10, getWidth() * 0.66, getHeight() - 10);
}

void BandSplitDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..


    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

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