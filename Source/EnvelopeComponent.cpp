//
//  EnvelopeComponent.cpp
//  RPCompressor - VST3
//
//  Created by 雷普唯怡 on 2023/7/4.
//

#include <JuceHeader.h>
#include "EnvelopeComponent.h"
#include "PluginProcessor.h"

EnvelopeComponent::EnvelopeComponent(RPCompressorAudioProcessor& p) : audioProcessor(p)
{
    setSize(600, 400);
    startTimerHz(30);
    displayRange = 120;
    bufferSize = 256; // 设置缓冲区大小
}

EnvelopeComponent::~EnvelopeComponent(){
    
}

void EnvelopeComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black); // 设置背景颜色

    g.setColour(juce::Colours::green); // 设置电平颜色
    g.strokePath(inputLevelPath, juce::PathStrokeType(2.0f));
    g.setColour(juce::Colours::red.withAlpha(0.5f)); // 设置电平颜色
    g.strokePath(outputLevelPath, juce::PathStrokeType(3.0f));
    g.setColour(juce::Colours::white);
    g.strokePath(indicator, juce::PathStrokeType(2.0f));
}

void EnvelopeComponent::resized()
    {
    inputLevelPath.clear(); // 清空电平路径
    outputLevelPath.clear();
    indicator.clear();

    const float startX = getWidth(); // 波形图起始点的 x 坐标
    const float startY = getHeight() * 0.13f; // 波形图起始点的 y 坐标

    // 添加波形图的起始点到路径中
    inputLevelPath.startNewSubPath(startX, startY);
    outputLevelPath.startNewSubPath(startX, startY);
}

void EnvelopeComponent::timerCallback()
{
    updateEnvelope(inputLevelPath,audioProcessor.currentInput);
    updateEnvelope(outputLevelPath,audioProcessor.currentOutput);
    updateIndicator();
}

void EnvelopeComponent::updateEnvelope(juce::Path& path, float audioData)
{
    const float stepX = getWidth() / static_cast<float>(bufferSize); // 计算每个采样点在 x 方向上的步进值
    

    path.applyTransform(juce::AffineTransform::translation(-stepX, 0.0f));
    
    const float startX = getWidth() - stepX; // 波形图绘制区域的起始 x 坐标
    const float startY = getHeight();
    
    const float amplitude = audioData; // 获取最新采样点的振幅值
    float db = juce::Decibels::gainToDecibels(std::abs(amplitude));
    float height = (db + displayRange) / displayRange;
    const float endY = startY - height * getHeight(); // 根据振幅计算采样点的 y 坐标
    path.lineTo(startX, endY);
    
    repaint();
}

void EnvelopeComponent::updateIndicator(){
    if (audioProcessor.lastThreshold == audioProcessor.threshold->get() && audioProcessor.lastRatio == audioProcessor.ratio->get() && audioProcessor.lastKneeWidth == audioProcessor.kneeWidth->get() && audioProcessor.lastSoftKneeFlag == audioProcessor.softKneeFlag->get()) return;
    audioProcessor.lastThreshold = audioProcessor.threshold->get();
    audioProcessor.lastRatio = audioProcessor.ratio->get();
    
    indicator.clear();
    
    if (!(bool) audioProcessor.lastSoftKneeFlag) {
        indicator.startNewSubPath(0, getHeight());
        float height = 1 - (audioProcessor.threshold->get() + displayRange) / displayRange;
        float lineEndX1 = getHeight() * (1 - height);
        float lineEndY1 = getHeight() * height;
        indicator.lineTo(lineEndX1, lineEndY1);
        
        indicator.startNewSubPath(lineEndX1, lineEndY1);
        float lineLength = getWidth() - lineEndX1;  // 直线长度
        float lineSlope = -1.0f / audioProcessor.lastRatio;  // 斜率
        float lineEndX2 = lineEndX1 + lineLength;  // 终点的X坐标
        float lineEndY2 = lineEndY1 + lineLength * lineSlope;  // 终点的Y坐标
        indicator.lineTo(lineEndX2, lineEndY2);
    } else {
        float kwidth = audioProcessor.kneeWidth->get() / displayRange * getHeight();
        indicator.startNewSubPath(0, getHeight());
        float height = 1 - (audioProcessor.threshold->get() + displayRange) / displayRange;
        float crossX = getHeight() * (1 - height);
        float croesY = getHeight() * height;
        float lineEndX1 = crossX - kwidth / 2;
        float lineEndY1 = croesY + kwidth / 2;
        indicator.lineTo(lineEndX1, lineEndY1);
        
        float lineSlope = -1.0f / audioProcessor.lastRatio;  // 斜率
        float lineStartX2 = lineEndX1 + kwidth;
        float lineStartY2 = lineEndY1 - kwidth * (0.5 - 0.5 * lineSlope);
        indicator.startNewSubPath(lineStartX2, lineStartY2);
        float lineLength = getWidth() - lineStartX2;  // 直线长度
        float lineEndX2 = lineStartX2 + lineLength;  // 终点的X坐标
        float lineEndY2 = lineStartY2 + lineLength * lineSlope;  // 终点的Y坐标
        indicator.lineTo(lineEndX2, lineEndY2);
        
        indicator.startNewSubPath(lineEndX1, lineEndY1);
        indicator.quadraticTo(crossX, croesY, lineStartX2, lineStartY2);
    }
    
}
