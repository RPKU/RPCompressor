/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/


#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
RPCompressorAudioProcessor::RPCompressorAudioProcessor()
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
}

RPCompressorAudioProcessor::~RPCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String RPCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RPCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RPCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RPCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RPCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RPCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RPCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RPCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RPCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void RPCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RPCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    attackTime = 10;
    releaseTime = 200;
    threshold = -12;
    ratio = 4;
    envelope = 0.0;
    lastAttackTime = 0.0;
    lastReleaseTime = 0.0;
    attackTimeRatio = 0.0;
    releaseTimeRatio = 0.0;
    gainReduction = 1.0;
    currentRatio = 1.0;
    numSamples = 0;
    processStep = new int[getTotalNumInputChannels()];
    processFlag = new int[getTotalNumInputChannels()];
    gainDB = new float[getTotalNumInputChannels()];
    lastEnvelope = new float[getTotalNumInputChannels()];
    for (int i = 0; i < getTotalNumInputChannels(); i++) {
        processStep[i] = 0;
        processFlag[i] = 0;
        gainDB[i] = 0.0f;
        lastEnvelope[i] = 0.0f;
    }
    timeInterval = 1000 / getSampleRate();
    
    kneeWidth = 10.0;
    makeUpGain = 0.0;
    lastKneeWidth = (float) kneeWidth.getValue();
    lastSoftKneeFlag = false;
}

void RPCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RPCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void RPCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::AudioBuffer<float>& inputBuffer = buffer;
    juce::AudioBuffer<float>& outputBuffer = buffer;
    
    releaseTimeRatio = calculateReleaseCoeff(0);
    attackTimeRatio = calculateAttackCoeff(0);
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    currentInput = buffer.getReadPointer(0)[0];

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    numSamples = inputBuffer.getNumSamples();
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        const float* inputChannelData = inputBuffer.getReadPointer(channel);
        float* outputChannelData = outputBuffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // 获取当前样本的振幅
            float detectDb = 0.0f;
            detectDb = calDetectDb(inputChannelData[sample], channel);
            gainReduction = calGain(detectDb);
            float makeupGainLinear = pow(10.0, (float) makeUpGain.getValue() / 20.0);
            
            outputChannelData[sample] = inputChannelData[sample] * gainReduction * makeupGainLinear;
        }
    }
    currentOutput = buffer.getWritePointer(0)[0];
}

//==============================================================================
bool RPCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RPCompressorAudioProcessor::createEditor()
{
    return new RPCompressorAudioProcessorEditor (*this);
}

//==============================================================================
void RPCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RPCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RPCompressorAudioProcessor();
}

float RPCompressorAudioProcessor::calculateAttackCoeff(int sampleNum)
{
    if (attackTime == lastAttackTime) return attackTimeRatio;
    return std::exp(-0.99967234081 / (getSampleRate() * (float) attackTime.getValue() * 0.001));
}

float RPCompressorAudioProcessor::calculateReleaseCoeff(int sampleNum)
{
    if (releaseTime == lastReleaseTime) return releaseTimeRatio;
    return std::exp(-0.99967234081 / (getSampleRate() * (float) releaseTime.getValue() * 0.001));
}

float RPCompressorAudioProcessor::calDetectDb(float xn, int channel)
{
    float input = std::abs(xn);
    float currEnvelope = 0.0;
    if (input > lastEnvelope[channel])
        currEnvelope = calculateAttackCoeff(0) * (lastEnvelope[channel] - input) + input;
    else
        currEnvelope = calculateReleaseCoeff(0) * (lastEnvelope[channel] - input) + input;
    lastEnvelope[channel] = currEnvelope;
    
    currEnvelope = std::max(std::min(currEnvelope, 1.0f), 0.0f);
    
    if (currEnvelope <= 0)
    {
        return -96.0;
    }
    
    return 20.0 * std::log10(currEnvelope);
}

float RPCompressorAudioProcessor::calGain(float detectDb){
    if (detectDb <= -96.0f) return 1.0f;
    float outputDb = 0.0f;
    
    if (!(bool) softKneeFlag.getValue()) {
        outputDb = (float) threshold.getValue() + (detectDb - (float) threshold.getValue()) / (float) ratio.getValue();
    } else {
        if (2.0*(detectDb - (float) threshold.getValue()) < - (float) kneeWidth.getValue())
            outputDb = detectDb;
        else if (2.0*(fabs(detectDb - (float) threshold.getValue())) <= (float) kneeWidth.getValue())
        {
            outputDb = detectDb + (((1.0 / (float) ratio.getValue()) - 1.0) * pow((detectDb - (float) threshold.getValue() + ((float) kneeWidth.getValue() / 2.0)), 2.0)) / (2.0*(float) kneeWidth.getValue());
        }
        // --- right of knee, compression zone
        else if (2.0*(detectDb - (float) threshold.getValue()) > (float) kneeWidth.getValue())
        {
            outputDb = (float) threshold.getValue() + (detectDb - (float) threshold.getValue()) / (float) ratio.getValue();
        }
    }
    
    return std::pow(10, (outputDb - detectDb) / 20.0);
}
