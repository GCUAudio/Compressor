/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressorAudioProcessor::CompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, juce::Identifier("PARAMETERS"),
                           { std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", -32.f, 0.f, 0.f),
                             std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", 0.f, 20.0f, 1.0f),
                             std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.f, 500.f, 20.0f),
                             std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.f, 500.f, 20.f) })
#endif
{
    treeState.addParameterListener("threshold", this);
    treeState.addParameterListener("ratio", this);
    treeState.addParameterListener("attack", this);
    treeState.addParameterListener("release", this);
}

CompressorAudioProcessor::~CompressorAudioProcessor()
{
}

//==============================================================================
const juce::String CompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void CompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float alphaA = expf(-log(9.0f) / (getSampleRate() * mstosec(mAttack)));
    float alphaR = expf(-log(9.0f) / (getSampleRate() * mstosec(mRelease)));

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {

            auto in = channelData[i];
            float x_uni = fabs(in); 
            float x_dB = juce::Decibels::gainToDecibels(x_uni); 

            float gainSC = 0.f;

            // Implement static characteristics
            if (x_dB > mThreshold)
                gainSC = mThreshold + ((x_dB - mThreshold) / mRatio);
            else
                gainSC = x_dB;

            float gainChange_dB = gainSC - x_dB;
            float gainSmooth = 0.0f;

            // Smooth gain changed for attack or release. Without this would be a clipping distortion
            if (gainChange_dB < gainSmoothPrev[channel])
                gainSmooth = ((1.0f - alphaA) * gainChange_dB) + (alphaA * gainSmoothPrev[channel]);
            else
                gainSmooth = ((1.0f - alphaR) * gainChange_dB) + (alphaR * gainSmoothPrev[channel]);

            // Apply linear amplitude to input sample
            float lin_A = juce::Decibels::decibelsToGain(gainSmooth);
            float out = lin_A * in;
            channelData[i] = out;

            gainSmoothPrev[channel] = gainSmooth;
        }
    }
}

//==============================================================================
bool CompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new CompressorAudioProcessorEditor (*this, treeState);
}

//==============================================================================
void CompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}

// Function called when parameter is changed
void CompressorAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "threshold")
    {
        mThreshold = newValue;
    }

    else if (parameterID == "ratio")
    {
        mRatio = newValue;
    }

    else if (parameterID == "attack")
    {
        mAttack = newValue;
    }

    else if (parameterID == "release")
    {
        mRelease = newValue;
    }
}

// Time conversion function
float CompressorAudioProcessor::mstosec(float ms) 
{
    return ms * 0.001;
}