/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

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
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	// Adds the threshold parameter to the AudioProcessor
	addParameter(threshold = new AudioParameterFloat("threshold",
		"Threshold (dBFS)", NormalisableRange<float>(-32.0f, -0.1f), -0.1f));

	// Adds the attack parameter to the AudioProcessor
	addParameter(attack = new AudioParameterFloat("attack",
		"Attack (s)", 0.0f, 0.5f, 0.02f));

	// Adds the release parameter to the AudioProcessor
	addParameter(release = new AudioParameterFloat("release",
		"Release (s)", 0.0f, 1.0f, 0.2f));

	// Adds a parameter for choosing algortihm to the AudioProcessor
	addParameter(ratio = new AudioParameterInt("ratio",
		"Ratio", 1, 20, 1));
}

CompressorAudioProcessor::~CompressorAudioProcessor()
{
}

//==============================================================================
const String CompressorAudioProcessor::getName() const
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

const String CompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressorAudioProcessor::changeProgramName (int index, const String& newName)
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
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void CompressorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	auto alphaA = expf(-log(9.0f) / ( (float)getSampleRate() * attack->get()));
	auto alphaR = expf(-log(9.0f) / ( (float)getSampleRate() * release->get()));

	auto currentThreshold = threshold->get();
	auto currentRatio = ratio->get();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);


		for (int i = 0; i < buffer.getNumSamples(); i++) {

			auto x = channelData[i];
			auto x_uni = fabs(x);
			auto x_dB = 0.f;

			if (x_uni < 0.000001)
				x_dB = -120;
			else
				x_dB = Decibels::gainToDecibels(x_uni);

			auto gainSC = 0.f;

			if (x_dB >= currentThreshold) {

				gainSC = currentThreshold + ((x_dB - currentThreshold) / (float)currentRatio);
			}
			else {
				gainSC = x_dB;
			}

			auto gainChange_dB = gainSC - x_dB;
			auto gainSmooth = 0.0f;

			if (gainChange_dB > gainSmoothPrev) {

				gainSmooth = ((1.0f - alphaA) * gainChange_dB) + (alphaA * gainSmoothPrev);
			}
			else {

				gainSmooth = ((1.0f - alphaR) * gainChange_dB) + (alphaR * gainSmoothPrev);
			}

			auto lin_A = Decibels::decibelsToGain(gainSmooth);
			channelData[i] = lin_A * x;
			gainSmoothPrev = gainSmooth;
		}
    }
}

//==============================================================================
bool CompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor(this);
}

//==============================================================================
void CompressorAudioProcessor::getStateInformation (MemoryBlock& destData)
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
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}
