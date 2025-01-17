/*
  ==============================================================================

    SynchronicView.h
    Created: 15 Nov 2016 4:02:15pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef SYNCHRONICVIEWCONTROLLER_H_INCLUDED
#define SYNCHRONICVIEWCONTROLLER_H_INCLUDED

#include "BKUtilities.h"

#include "PluginProcessor.h"

#include "BKListener.h"
#include "BKComponent.h"

//==============================================================================
/*
*/
class SynchronicViewController    : public BKComponent, public BKListener
{
public:
    SynchronicViewController(BKAudioProcessor&);
    ~SynchronicViewController();

    void paint (Graphics&) override;
    void resized() override;
    
    void updateModFields(void);
    void updateFields(void);
    
private:
    BKAudioProcessor& processor;
    int currentSynchronicId,currentModSynchronicId;
    
    OwnedArray<BKLabel> synchronicL;
    OwnedArray<BKTextField> synchronicTF;
    OwnedArray<BKTextField> modSynchronicTF;
    
    void bkTextFieldDidChange       (TextEditor&)           override;
    void bkMessageReceived          (const String& message) override;
    
    void bkComboBoxDidChange        (ComboBox* box)         override { };
    void bkButtonClicked            (Button* b)             override { };
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynchronicViewController)
};


#endif  // SYNCHRONICVIEWCONTROLLER_H_INCLUDED
