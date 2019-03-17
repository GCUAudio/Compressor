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
		"Attack (ms)", NormalisableRange<float>(0.0f, 500.0f), 20.0f));

	// Adds the release parameter to the AudioProcessor
	addParameter(release = new AudioParameterFloat("release",
		"Release (ms)", NormalisableRange<float>(0.0f, 1000.f), 200.0f));

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

	float alphaA = 0.0f;
	float alphaR = 0.0f;

	// Only calculate if control has been moved
	if(currentAttack != attack->get())
		alphaA = expf(-log(9.0f) / (getSampleRate() * mstosec(attack->get())));

	// Only calculate if control has been moved
	if (currentRelease != release->get())
		alphaR = expf(-log(9.0f) / (getSampleRate() * mstosec(release->get())));

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
		auto currentThreshold = threshold->get();
		auto currentRatio = ratio->get();

		for (int i = 0; i < buffer.getNumSamples(); ++i) {

			auto in = channelData[i];
			float y_uni = fabs(y_prev[channel]); // change to unipolar signal
			float y_dB = Decibels::gainToDecibels(y_uni); // convert to dB

			// Ensure no negative infinity values
			if (y_dB < -120.0f)
				y_dB = -120.0f; 
				
			float gainSC = 0.f;

			// Implement static characteristics
			if (y_dB > currentThreshold) 
				gainSC = currentThreshold + ((y_dB - currentThreshold) / currentRatio);
			else 
				gainSC = y_dB;

			float gainChange_dB = gainSC - y_dB;
			float gainSmooth = 0.0f;

			// Smooth gain changed for attack or release
			if (gainChange_dB < gainSmoothPrev[channel]) 
				gainSmooth = ((1.0f - alphaA) * gainChange_dB) + (alphaA * gainSmoothPrev[channel]);
			else 
				gainSmooth = ((1.0f - alphaR) * gainChange_dB) + (alphaR * gainSmoothPrev[channel]);

			// Apply linear amplitude to input sample
			float lin_A = Decibels::decibelsToGain(gainSmooth);
			float out = lin_A * in;
			channelData[i] = out;

			// update for next cycle as used in next sample of the loop
			y_prev[channel] = out;
			gainSmoothPrev[channel] = gainSmooth;
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
	MemoryOutputStream stream(destData, true);
	stream.writeFloat(*threshold);
	stream.writeFloat(*attack);
	stream.writeFloat(*release);
	stream.writeInt(*ratio);
}

void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	MemoryInputStream stream(data, static_cast<size_t> (sizeInBytes), false);
	threshold->setValueNotifyingHost(threshold->getNormalisableRange().convertTo0to1(stream.readFloat()));
	attack->setValueNotifyingHost(attack->getNormalisableRange().convertTo0to1(stream.readFloat()));
	release->setValueNotifyingHost(release->getNormalisableRange().convertTo0to1(stream.readFloat()));
	ratio->setValueNotifyingHost(ratio->getNormalisableRange().convertTo0to1(stream.readInt()));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}

float CompressorAudioProcessor::mstosec(float ms) {
	return ms * 0.001;
}