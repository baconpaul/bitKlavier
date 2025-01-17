/*
  ==============================================================================

    BKMenu.h
    Created: 9 Dec 2016 3:31:21pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef BKMENU_H_INCLUDED
#define BKMENU_H_INCLUDED

#include "BKLookAndFeel.h"
#include "BKTextField.h"

//==============================================================================
/*
*/
class BKPopupMenu : public PopupMenu
{
public:
    BKPopupMenu()
    {
        setLookAndFeel(new BKButtonAndMenuLAF());
    }
    
    ~BKPopupMenu()
    {
        setLookAndFeel(nullptr);
    }
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKPopupMenu)
};

class BKComboBox : public ComboBox
{
public:
    BKComboBox() :
    thisJustification (Justification::centredLeft)
    {
        setSize(200,20);

        //lookAndFeelChanged();
        //setLookAndFeel(&thisLAF);
        
    }
    
    ~BKComboBox()
    {
        setLookAndFeel(nullptr);
    }
    
    void BKSetJustificationType (Justification justification)
    {
        thisJustification = justification;
        //setJustificationType(justification);
        //thisLAF.setComboBoxJustificationType(justification);
    }
    
    void mouseDown(const MouseEvent& e) override
    {
        if (beforeOpen != nullptr) beforeOpen();
        ComboBox::mouseDown(e);
        if (afterOpen != nullptr) afterOpen();
    }
    
    std::function<void()> beforeOpen;
    std::function<void()> afterOpen;
    
private:
    
    //BKButtonAndMenuLAF thisLAF;
    Justification thisJustification;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKComboBox)
};


class BKEditableComboBoxListener;

class BKEditableComboBox :
public ComboBox,
public TextEditor::Listener
{
public:
    
    BKEditableComboBox() :
    thisJustification (Justification::centredLeft)
    {
        setSize(200,20);
        
        //lookAndFeelChanged();
        //setLookAndFeel(&thisLAF);

        nameEditor.setName("NAMETXTEDIT");
        addAndMakeVisible(nameEditor);
        nameEditor.toBack();
        nameEditor.setAlpha(0.);
        nameEditor.setOpaque(true);
        nameEditor.addListener(this);
        
        lastItemId = 0;

    }
    
    ~BKEditableComboBox()
    {
        setLookAndFeel(nullptr);
    }
    
    void textEditorReturnKeyPressed(TextEditor& textEditor) override;
    void textEditorFocusLost (TextEditor& textEditor) override;
    void mouseDoubleClick(const MouseEvent& e) override;
    void textEditorEscapeKeyPressed (TextEditor &) override;
    
    ListenerList<BKEditableComboBoxListener> listeners;
    void addMyListener(BKEditableComboBoxListener* listener)     { listeners.add(listener);      }
    void removeMyListener(BKEditableComboBoxListener* listener)  { listeners.remove(listener);   }
    
    void BKSetJustificationType (Justification justification)
    {
        thisJustification = justification;
        //thisLAF.setComboBoxJustificationType(justification);
        setJustificationType(justification);
    }
    
private:
        
    BKTextField nameEditor;
    bool focusLostByEscapeKey;
    int lastItemId;
    
    //BKButtonAndMenuLAF thisLAF;
    Justification thisJustification;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKEditableComboBox)
};

class BKEditableComboBoxListener
{
    
public:
    
    virtual ~BKEditableComboBoxListener() {};
    
    virtual void BKEditableComboBoxChanged(String text, BKEditableComboBox*) = 0;
};



#endif  // BKMENU_H_INCLUDED
