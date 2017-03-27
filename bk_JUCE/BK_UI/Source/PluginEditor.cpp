/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GraphicsConstants.h"

#include "PianoViewController.h"



//==============================================================================
BKAudioProcessorEditor::BKAudioProcessorEditor (BKAudioProcessor& p):
AudioProcessorEditor (&p),
processor (p),
mvc(p),
constrain(new ComponentBoundsConstrainer())
{
    viewPort.setViewedComponent(&mvc);
    viewPort.setViewPosition(0, 0);
    addAndMakeVisible(viewPort);
    
    addAndMakeVisible (resizer = new ResizableCornerComponent (this, constrain));
    
    resizer->setAlwaysOnTop(true);
    
    constrain->setSizeLimits(200, 150, 1500, 1000);
    
    mvc.setSize(gMainComponentWidth, gMainComponentHeight);
    setSize(gMainComponentWidth, gMainComponentHeight);

}

BKAudioProcessorEditor::~BKAudioProcessorEditor()
{
    
}

void BKAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::dimgrey);
}


void BKAudioProcessorEditor::resized()
{
    
    viewPort.setBoundsRelative(0.0f,0.0f,1.0f,1.0f);
    
    resizer->setBounds(getWidth()-16, getHeight()-16, 16, 16);
    
    
}


//==============================================================================

