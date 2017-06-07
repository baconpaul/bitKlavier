/*
  ==============================================================================

    NostalgicViewController2.h
    Created: 24 May 2017 7:13:23pm
    Author:  Daniel Trueman

  ==============================================================================
*/

/* TODO
 
 Transposition slider to handle multiple values
 
 */

#pragma once

#include "BKUtilities.h"
#include "PluginProcessor.h"
#include "BKListener.h"
#include "BKComponent.h"
#include "BKSlider.h"
#include "BKGraph.h"

class NostalgicViewController2 :
public BKComponent,
public BKListener,
public BKWaveDistanceUndertowSliderListener,
public BKEditableComboBoxListener,
public BKSingleSliderListener,
//public SliderListener,
public Timer
{
public:
    NostalgicViewController2(BKAudioProcessor&, BKItemGraph* theGraph);
    ~NostalgicViewController2() {};
    
    BKWaveDistanceUndertowSlider nDisplaySlider;
    

    BKEditableComboBox selectCB;
    BKComboBox lengthModeSelectCB;
    
    BKSingleSlider* transpositionSlider;
    BKSingleSlider* lengthMultiplierSlider;
    BKSingleSlider* beatsToSkipSlider;
    BKSingleSlider* gainSlider;
    
    
    void paint (Graphics&) override;
    void resized() override;
    
    void updateFields();
    //void updateFields(NotificationType notify);
    
    void bkMessageReceived (const String& message) override;
    void bkComboBoxDidChange (ComboBox* box) override;
    void bkTextFieldDidChange (TextEditor&) override {};
    void bkButtonClicked (Button* b) override { };
    void BKEditableComboBoxChanged(String name, int index) override;
    void BKSingleSliderValueChanged(String name, double val) override;
    void fillSelectCB(void);
    void fillModeSelectCB(void);
    
    void timerCallback() override;
    void BKWaveDistanceUndertowSliderValueChanged(String name, double wavedist, double undertow) override;
    

private:
    BKAudioProcessor& processor;
    BKItemGraph* theGraph;

};