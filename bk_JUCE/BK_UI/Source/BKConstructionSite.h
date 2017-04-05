/*
  ==============================================================================

    BKConstructionSite.h
    Created: 4 Apr 2017 5:46:38pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef BKCONSTRUCTIONSITE_H_INCLUDED
#define BKCONSTRUCTIONSITE_H_INCLUDED

#include "BKUtilities.h"

#include "BKListener.h"
#include "BKComponent.h"

#include "PluginProcessor.h"

class BKConstructionSite : public BKDraggableComponent
{
public:
    BKConstructionSite(BKAudioProcessor& p):
    BKDraggableComponent(false,true),
    processor(p)
    {
        
    }
    
    ~BKConstructionSite(void)
    {
        
    }

    void paint(Graphics& g) override
    {
        g.setColour(Colours::lightgrey);
        g.fillAll();
        
        
        g.setColour(Colours::white);
        g.drawRect(getLocalBounds(), 1);
        
        if (itemIsHovering)
        {
            g.setColour(Colours::black);
            g.drawRect(getLocalBounds(), 3);
        }
    }
    
    void resized(void) override
    {
        for (int i = 0; i < poppers.size(); i++)
        {
            
        }
    }
    
private:
    BKAudioProcessor& processor;
    
    OwnedArray<Label> poppers;
    
    // Drag interface
    void itemWasDropped(BKPreparationType type, Array<int> data, int x, int y) override
    {
        // Now need to make custom objects for preparations/keymaps (draggable, connectable, selectable, deletable)
        for (int i = 0; i < data.size(); i++)
        {
            String name = cPreparationTypes[type]+String(data[i]);
            
            int index = poppers.size();
            
            poppers.add(new Label(String(index), name));
            
            poppers[index]->setJustificationType(Justification::centred);
            poppers[index]->setBorderSize(BorderSize<int>(2));
            
            poppers[index]->setCentrePosition(x, (i-1)*25 + y);
            poppers[index]->setSize(150,20);
            
            addAndMakeVisible(poppers[index]);
            
            DBG("added: " + name);
            
        }
        
        
        

        
        
    }
    
    JUCE_LEAK_DETECTOR(BKConstructionSite)
};



#endif  // BKCONSTRUCTIONSITE_H_INCLUDED
