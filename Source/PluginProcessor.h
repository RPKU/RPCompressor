/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PluginProcessor_h
#define PluginProcessor_h

#include <JuceHeader.h>
#include "PluginEditor.h"

//==============================================================================
/**
*/
class RPCompressorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RPCompressorAudioProcessor();
    ~RPCompressorAudioProcessor() override;
    
    juce::Value attackTime;
    juce::Value releaseTime;
    float lastAttackTime;
    float lastReleaseTime;
    float attackTimeRatio;
    float releaseTimeRatio;
    float gainReduction;
    juce::Value threshold;
    juce::Value ratio;
    float lastThreshold;
    float lastRatio;
    float envelope;
    float* lastEnvelope;
    int numSamples;
    float currentInput;
    float currentOutput;
    float* gainDB;
    float timeInterval;
    float currentRatio;
    int* processStep;
    int* processFlag;
    juce::Value kneeWidth;
    juce::Value makeUpGain;
    juce::Value softKneeFlag;
    juce::Value sideChainFlag;
    float lastKneeWidth;
    bool lastSoftKneeFlag;
    bool lastSideChainFlag;
    

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void setEditor (RPCompressorAudioProcessorEditor* editor);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RPCompressorAudioProcessor)
    
    float calculateAttackCoeff(int sampleNum);
    float calculateReleaseCoeff(int sampleNum);
    float calDetectDb(float xn, int channel);
    float calGain(float detectDb);
};


#endif
