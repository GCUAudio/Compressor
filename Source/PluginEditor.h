/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CompressorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CompressorAudioProcessorEditor (CompressorAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~CompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CompressorAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& treeState;

    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;

    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;

    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> thresholdValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> ratioValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> attackValue;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> releaseValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessorEditor)
};
