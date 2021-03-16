/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressorAudioProcessorEditor::CompressorAudioProcessorEditor (CompressorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), treeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);

    // Threshold
    thresholdValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "threshold", thresholdSlider);
    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.setRange(-32.0f, 0.0f, 1.0f);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&thresholdSlider);

    addAndMakeVisible(thresholdLabel);
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.attachToComponent(&thresholdSlider, false);

    // Ratio
    ratioValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "ratio", ratioSlider);
    ratioSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    ratioSlider.setRange(1.0f, 20.0f, 1.0f);
    ratioSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&ratioSlider);

    addAndMakeVisible(ratioLabel);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.attachToComponent(&ratioSlider, false);

    // Attack
    attackValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "attack", attackSlider);
    attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    attackSlider.setRange(0.0f, 500.0f, 1.0f);
    attackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&attackSlider);

    addAndMakeVisible(attackLabel);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackSlider, false);

    // Attack
    releaseValue = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(treeState, "release", releaseSlider);
    releaseSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    releaseSlider.setRange(0.0f, 500.0f, 1.0f);
    releaseSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 75, 25);
    addAndMakeVisible(&releaseSlider);

    addAndMakeVisible(releaseLabel);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.attachToComponent(&releaseSlider, false);
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
}

//==============================================================================
void CompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Compressor", 150, 20,130, 30, juce::Justification::centred, 1, 0.0f);
}

void CompressorAudioProcessorEditor::resized()
{
    thresholdSlider.setBounds(50, 110, 320, 50);
    ratioSlider.setBounds(50, 180, 320, 50);
    attackSlider.setBounds(50, 250, 320, 50);
    releaseSlider.setBounds(50, 320, 320, 50);
}
