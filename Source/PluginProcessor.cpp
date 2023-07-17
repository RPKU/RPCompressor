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
                       .withInput  ("Input",  juce::AudioChannelSet::stereo())
                       .withOutput ("Output", juce::AudioChannelSet::stereo())
                       )
#endif
{
    parameters = new juce::AudioProcessorValueTreeState(*this, nullptr, "PARAMETERS", {
        std::make_unique<juce::AudioParameterFloat>(*(new juce::ParameterID("attackTime", 1)), "Attack Time", *(new juce::NormalisableRange<float>(0.1f, 200.0f, 0.1f)), 10.0f),
        std::make_unique<juce::AudioParameterFloat>(*(new juce::ParameterID("releaseTime", 1)), "Release Time", *(new juce::NormalisableRange<float>(10.0f, 500.0f, 0.1f)), 200.0f),
        std::make_unique<juce::AudioParameterFloat>(*(new juce::ParameterID("threshold", 1)), "Threshold", *(new juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f)), -12.0f),
        std::make_unique<juce::AudioParameterFloat>(*(new juce::ParameterID("ratio", 1)), "Ratio", *(new juce::NormalisableRange<float>(1.0f, 20.0f, 1.0f)), 4.0f),
        std::make_unique<juce::AudioParameterFloat>(*(new juce::ParameterID("kneeWidth", 1)), "Knee Width", *(new juce::NormalisableRange<float>(1.0f, 80.0f, 0.1f)), 10.0f),
        std::make_unique<juce::AudioParameterFloat>(*(new juce::ParameterID("makeUpGain", 1)), "Make Up Gain", *(new juce::NormalisableRange<float>(-20.0f, 12.0f, 0.1f)), 0.0f),
        std::make_unique<juce::AudioParameterBool>(*(new juce::ParameterID("softKneeFlag", 1)), "Soft Knee Flag", false),
        std::make_unique<juce::AudioParameterBool>(*(new juce::ParameterID("sideChainFlag", 1)), "Side Chain Flag", false)
    });
    
    attackTime = (juce::AudioParameterFloat*) parameters->getParameter("attackTime");
    releaseTime = (juce::AudioParameterFloat*) parameters->getParameter("releaseTime");
    threshold = (juce::AudioParameterFloat*) parameters->getParameter("threshold");
    ratio = (juce::AudioParameterFloat*) parameters->getParameter("ratio");
    kneeWidth = (juce::AudioParameterFloat*) parameters->getParameter("kneeWidth");
    makeUpGain = (juce::AudioParameterFloat*) parameters->getParameter("makeUpGain");
    softKneeFlag = (juce::AudioParameterBool*) parameters->getParameter("softKneeFlag");
    sideChainFlag = (juce::AudioParameterBool*) parameters->getParameter("sideChainFlag");
}

RPCompressorAudioProcessor::~RPCompressorAudioProcessor()
{
    delete parameters;
    delete attackTime;
    delete releaseTime;
    delete threshold;
    delete ratio;
    delete kneeWidth;
    delete makeUpGain;
    delete softKneeFlag;
    delete sideChainFlag;
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
    
    lastKneeWidth = false;
    lastSoftKneeFlag = false;
    lastSideChainFlag = false;
    lastAttackTime = 0.0;
    lastReleaseTime = 0.0;
    
    attackTimeRatio = 0.0;
    releaseTimeRatio = 0.0;
    gainReduction = 1.0;
    currentRatio = 1.0;
    numSamples = 0;
    envelope = 0.0;
    
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
}

void RPCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


bool RPCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
    && ! layouts.getMainInputChannelSet().isDisabled();
}

void RPCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto inputBuffer = getBusBuffer (buffer, true, 0);
    auto outputBuffer = getBusBuffer (buffer, true, 0);
    auto sideChainInput = getBusBuffer (buffer, true, 0);
    
    if (sideChainFlag->get() && !lastSideChainFlag) {
        if (addBus(true)) {
            sideChainInput = getBusBuffer (buffer, true, 1);
            lastSideChainFlag = true;
        } else {
            *sideChainFlag = false;
        }
    }
    
    if (!sideChainFlag->get() && lastSideChainFlag) {
        if (removeBus(true)) {
            lastSideChainFlag = false;
        } else {
            *sideChainFlag = true;
            sideChainInput = getBusBuffer (buffer, true, 1);
        }
    }
    
    releaseTimeRatio = calculateReleaseCoeff(0);
    attackTimeRatio = calculateAttackCoeff(0);
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    currentInput = inputBuffer.getReadPointer(0)[0];

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
    
    if (!(bool)sideChainFlag->get()){
        for (int channel = 0; channel < inputBuffer.getNumChannels(); ++channel)
        {
            const float* inputChannelData = inputBuffer.getReadPointer(channel);
            float* outputChannelData = outputBuffer.getWritePointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                // 获取当前样本的振幅
                float detectDb = 0.0f;
                detectDb = calDetectDb(inputChannelData[sample], channel);
                gainReduction = calGain(detectDb);
                float makeupGainLinear = pow(10.0, makeUpGain->get() / 20.0);
                
                outputChannelData[sample] = inputChannelData[sample] * gainReduction * makeupGainLinear;
            }
        }
    }else {
        for (int channel = 0; channel < sideChainInput.getNumChannels(); ++channel)
        {
            const float* inputChannelData = sideChainInput.getReadPointer(channel);
            float* outputChannelData = outputBuffer.getWritePointer(channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                // 获取当前样本的振幅
                float detectDb = 0.0f;
                detectDb = calDetectDb(inputChannelData[sample], channel);
                gainReduction = calGain(detectDb);
                float makeupGainLinear = pow(10.0, makeUpGain->get() / 20.0);
                
                outputChannelData[sample] = inputChannelData[sample] * gainReduction * makeupGainLinear;
            }
        }
    }
    
    currentOutput = outputBuffer.getWritePointer(0)[0];
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
    if (*attackTime == lastAttackTime) return attackTimeRatio;
    return std::exp(-0.99967234081 / (getSampleRate() * attackTime->get() * 0.001));
}

float RPCompressorAudioProcessor::calculateReleaseCoeff(int sampleNum)
{
    if (*releaseTime == lastReleaseTime) return releaseTimeRatio;
    return std::exp(-0.99967234081 / (getSampleRate() * releaseTime->get() * 0.001));
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
    
    if (!softKneeFlag->get()) {
        outputDb = threshold->get() + (detectDb - threshold->get()) / ratio->get();
    } else {
        if (2.0*(detectDb - threshold->get()) < - kneeWidth->get())
            outputDb = detectDb;
        else if (2.0*(fabs(detectDb - threshold->get())) <= kneeWidth->get())
        {
            outputDb = detectDb + (((1.0 / ratio->get()) - 1.0) * pow((detectDb - threshold->get() + (kneeWidth->get() / 2.0)), 2.0)) / (2.0 * kneeWidth->get());
        }
        // --- right of knee, compression zone
        else if (2.0*(detectDb - threshold->get()) > kneeWidth->get())
        {
            outputDb = threshold->get() + (detectDb - threshold->get()) / ratio->get();
        }
    }
    
    return std::pow(10, (outputDb - detectDb) / 20.0);
}
