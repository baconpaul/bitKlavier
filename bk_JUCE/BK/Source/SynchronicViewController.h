/*
  ==============================================================================

    SynchronicView.h
    Created: 15 Nov 2016 4:02:15pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef SYNCHRONICVIEWCONTROLLER_H_INCLUDED
#define SYNCHRONICVIEWCONTROLLER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "PluginProcessor.h"

#include "PreparationViewController.h"

#include "BKTextField.h"

#include "BKLabel.h"

//==============================================================================
/*
*/
class SynchronicViewController    : public PreparationViewController
{
public:
    SynchronicViewController(BKAudioProcessor&);
    ~SynchronicViewController();

    void paint (Graphics&) override;
    void resized() override;
    
    
    
private:
    BKAudioProcessor& processor;
    int currentSynchronicLayer;
    
    // BKLabels
    BKLabel sNumLayersL;
    BKLabel sCurrentLayerL;
    BKLabel sKeymapL;
    OwnedArray<BKLabel> synchronicL;
    
    // Text Fields
    BKTextField sNumLayersTF;
    BKTextField sCurrentLayerTF;
    BKTextField sKeymapTF;
    OwnedArray<BKTextField> synchronicTF;
    
    void textFieldDidChange(TextEditor&) override;
    
    void updateFieldsToLayer(int numLayer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynchronicViewController)
};


#endif  // SYNCHRONICVIEWCONTROLLER_H_INCLUDED
