//
//  EnvelopeComponent.hpp
//  RPCompressor - VST3
//
//  Created by 雷普唯怡 on 2023/7/4.
//

#pragma once

#include <JuceHeader.h>

class RPCompressorAudioProcessor;

class EnvelopeComponent : public juce::Component, public juce::Timer
{
public:
    EnvelopeComponent(RPCompressorAudioProcessor&);
    ~EnvelopeComponent();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void updateEnvelope(juce::Path& path,float audioData);
    void updateIndicator();
    
private:
    RPCompressorAudioProcessor& audioProcessor;
    float envelopeValue;
    float displayRange;
    juce::Path inputLevelPath;
    juce::Path outputLevelPath;
    juce::Path indicator;
    std::queue<float> bufferDest;
    int bufferSize;
};
