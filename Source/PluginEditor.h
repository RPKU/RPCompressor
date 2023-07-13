/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EnvelopeComponent.h"

class RPCompressorAudioProcessor;

//==============================================================================
/**
*/
class RPCompressorAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    RPCompressorAudioProcessorEditor (RPCompressorAudioProcessor&);
    ~RPCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    juce::Slider* thresholdSlider;
    juce::Slider* ratioSlider;
    juce::Slider* attackTimeSlider;
    juce::Slider* releaseTimeSlider;
    juce::Slider* kneeWidthSlider;
    juce::Slider* makeUpGainSlider;
    
    juce::ToggleButton* softKneeButton;
    juce::ToggleButton* sideChainButton;
    
    juce::Label* thresholdLabel;
    juce::Label* ratioLabel;
    juce::Label* attackTimeLabel;
    juce::Label* releaseTimeLabel;
    juce::Label* kneeWidthLabel;
    juce::Label* makeUpGainLabel;
    
    juce::Label* softKneeLabel;
    juce::Label* sideChainLabel;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RPCompressorAudioProcessor& audioProcessor;
    EnvelopeComponent* envelopeComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RPCompressorAudioProcessorEditor)
};
