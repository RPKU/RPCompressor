/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EnvelopeComponent.h"

//==============================================================================
RPCompressorAudioProcessorEditor::RPCompressorAudioProcessorEditor (RPCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), envelopeComponent(nullptr)
{
    envelopeComponent = new EnvelopeComponent(audioProcessor);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 700);
    addAndMakeVisible(envelopeComponent);
    
    thresholdSlider = new juce::Slider();
    ratioSlider = new juce::Slider();
    attackTimeSlider = new juce::Slider();
    releaseTimeSlider = new juce::Slider();
    kneeWidthSlider = new juce::Slider();
    makeUpGainSlider = new juce::Slider();
    
    softKneeButton = new juce::ToggleButton("soft knee");
    sideChainButton = new juce::ToggleButton("side chain");
    
    thresholdLabel = new juce::Label("threshold", "threshold");
    ratioLabel = new juce::Label("ratio", "ratio");
    attackTimeLabel = new juce::Label("attack time", "attack");
    releaseTimeLabel = new juce::Label("release time", "release");
    kneeWidthLabel = new juce::Label("knee width", "knee");
    makeUpGainLabel = new juce::Label("make up gain", "gain");
    softKneeLabel = new juce::Label("soft knee flag", "soft knee");
    sideChainLabel = new juce::Label("side chain flag", "side chain");
    
    initBaseSlider(*attackTimeSlider, *audioProcessor.attackTime, attackTimeAttachment);
    initBaseSlider(*releaseTimeSlider, *audioProcessor.releaseTime, releaseTimeAttachment);
    initBaseSlider(*thresholdSlider, *audioProcessor.threshold, thresholdAttachment);
    initBaseSlider(*ratioSlider, *audioProcessor.ratio, ratioAttachment);
    initBaseSlider(*kneeWidthSlider, *audioProcessor.kneeWidth, kneeWidthAttachment);
    initBaseSlider(*makeUpGainSlider, *audioProcessor.makeUpGain, makeUpGainAttachment);
    
    thresholdSlider->setBounds(0, 400, 100, 100);
    ratioSlider->setBounds(100, 400, 100, 100);
    attackTimeSlider->setBounds(200, 400, 100, 100);
    releaseTimeSlider->setBounds(300, 400, 100, 100);
    kneeWidthSlider->setBounds(400, 400, 100, 100);
    makeUpGainSlider->setBounds(500, 400, 100, 100);
    
    softKneeButton->setBounds(200, 550, 100, 20);
    sideChainButton->setBounds(400, 550, 100, 20);
    
    thresholdLabel->setBounds(30, 430, 100, 20);
    ratioLabel->setBounds(130, 430, 100, 20);
    attackTimeLabel->setBounds(230, 430, 100, 20);
    releaseTimeLabel->setBounds(330, 430, 100, 20);
    kneeWidthLabel->setBounds(430, 430, 100, 20);
    makeUpGainLabel->setBounds(530, 430, 100, 20);
    
    thresholdLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    ratioLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    attackTimeLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    releaseTimeLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    kneeWidthLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    makeUpGainLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    softKneeButton->setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    softKneeButton->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
    softKneeButton->setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
    sideChainButton->setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    sideChainButton->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
    sideChainButton->setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
    
    thresholdSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    thresholdSlider->setTitle("threshold");
    thresholdSlider->setTextValueSuffix("db");
    thresholdSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    thresholdSlider->setNumDecimalPlacesToDisplay(1);
    
    ratioSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    ratioSlider->setTitle("ratio");
    ratioSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    ratioSlider->setNumDecimalPlacesToDisplay(0);
    
    attackTimeSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    attackTimeSlider->setTitle("attack time");
    attackTimeSlider->setTextValueSuffix("ms");
    attackTimeSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    attackTimeSlider->setNumDecimalPlacesToDisplay(1);
    
    releaseTimeSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    releaseTimeSlider->setTitle("release time");
    releaseTimeSlider->setTextValueSuffix("ms");
    releaseTimeSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    releaseTimeSlider->setNumDecimalPlacesToDisplay(1);
    
    kneeWidthSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    kneeWidthSlider->setTitle("knee width");
    kneeWidthSlider->setTextValueSuffix("db");
    kneeWidthSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    kneeWidthSlider->setNumDecimalPlacesToDisplay(1);
    
    makeUpGainSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    makeUpGainSlider->setTitle("make up gain");
    makeUpGainSlider->setTextValueSuffix("db");
    makeUpGainSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    makeUpGainSlider->setNumDecimalPlacesToDisplay(1);
    
    softKneeAttachment = new juce::AudioProcessorValueTreeState::ButtonAttachment(*audioProcessor.parameters, "softKneeFlag", *softKneeButton);
    sideChainAttachment = new juce::AudioProcessorValueTreeState::ButtonAttachment(*audioProcessor.parameters, "sideChainFlag", *sideChainButton);
    
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(attackTimeSlider);
    addAndMakeVisible(releaseTimeSlider);
    addAndMakeVisible(kneeWidthSlider);
    addAndMakeVisible(makeUpGainSlider);
    
    addAndMakeVisible(softKneeButton);
    addAndMakeVisible(sideChainButton);
    
    addAndMakeVisible(thresholdLabel);
    addAndMakeVisible(ratioLabel);
    addAndMakeVisible(attackTimeLabel);
    addAndMakeVisible(releaseTimeLabel);
    addAndMakeVisible(kneeWidthLabel);
    addAndMakeVisible(makeUpGainLabel);
    
    startTimer(200);
}

RPCompressorAudioProcessorEditor::~RPCompressorAudioProcessorEditor()
{
    delete thresholdSlider;
    delete ratioSlider;
    delete attackTimeSlider;
    delete releaseTimeSlider;
    delete kneeWidthSlider;
    delete makeUpGainSlider;
    
    delete softKneeButton;
    delete sideChainButton;
    
    delete thresholdAttachment;
    delete ratioAttachment;
    delete attackTimeAttachment;
    delete releaseTimeAttachment;
    delete kneeWidthAttachment;
    delete makeUpGainAttachment;
    
    delete softKneeAttachment;
    delete sideChainAttachment;
    
    delete thresholdLabel;
    delete ratioLabel;
    delete attackTimeLabel;
    delete releaseTimeLabel;
    delete kneeWidthLabel;
    delete makeUpGainLabel;
    
    delete softKneeLabel;
    delete sideChainLabel;
}

//==============================================================================
void RPCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);
//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    std::ostringstream oss;
//    oss << "Input: " << audioProcessor.currentInput[0] << ", output:" << audioProcessor.currentOutput[0];
//    g.drawFittedText (oss.str(), getLocalBounds(), juce::Justification::centred, 1);
}

void RPCompressorAudioProcessorEditor::timerCallback()
{
    audioProcessor.lastAttackTime = audioProcessor.attackTime->get();
    audioProcessor.lastReleaseTime = audioProcessor.releaseTime->get();
    audioProcessor.lastThreshold = audioProcessor.threshold->get();
    audioProcessor.lastRatio = audioProcessor.ratio->get();
    audioProcessor.lastKneeWidth = audioProcessor.kneeWidth->get();
    audioProcessor.lastSoftKneeFlag = audioProcessor.softKneeFlag->get();
}
    
void RPCompressorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void RPCompressorAudioProcessorEditor::initBaseSlider(juce::Slider& slider, juce::AudioParameterFloat& param, juce::AudioProcessorValueTreeState::SliderAttachment*& attach) {
    slider.setRange({param.range.start, param.range.end}, param.range.interval);
    slider.setSliderStyle(juce::Slider::Rotary);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, false);
    attach = new juce::AudioProcessorValueTreeState::SliderAttachment(*audioProcessor.parameters, param.getParameterID(), slider);
}
