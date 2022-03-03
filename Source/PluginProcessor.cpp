/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OverdriveAudioProcessor::OverdriveAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Setup and add the parameter to the processor class
    inputGain = new juce::AudioParameterFloat("inputGain", "Input Gain", 0.0f, 10.0f, 1.0f);
    addParameter(inputGain);
}

OverdriveAudioProcessor::~OverdriveAudioProcessor()
{
}

//==============================================================================
const juce::String OverdriveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OverdriveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OverdriveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OverdriveAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OverdriveAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OverdriveAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OverdriveAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OverdriveAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OverdriveAudioProcessor::getProgramName (int index)
{
    return {};
}

void OverdriveAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OverdriveAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void OverdriveAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OverdriveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void OverdriveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float thresh1 = 1.0f / 3.0f; // 0.33
    float thresh2 = 2.0f / 3.0f; // 0.66

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        float out = 0.0f;

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float in = channelData[i] * inputGain->get();
            float absIn = fabs(in);
            int signIn = signum(in);

            if (absIn > thresh2)
            {
                out = 0.99f * signIn;
            }
            else if (absIn > thresh1)
            {
                out = ((3.0f - (2.0f - 3.0f * absIn) * (2.0f - 3.0f * absIn)) / 3.0f) * signIn;
            }
            else
            {
                out = 2.0f * in;
            }

            channelData[i] = out;
        }
    }
}

int OverdriveAudioProcessor::signum(float x)
{
    if (x > 0)
        return 1;
    else if (x < 0)
        return -1;
    else
        return 0;
}

//==============================================================================
bool OverdriveAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OverdriveAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void OverdriveAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeFloat(*inputGain);
}

void OverdriveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t> (sizeInBytes), false);
    *inputGain = stream.readFloat();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OverdriveAudioProcessor();
}
