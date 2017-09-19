/*
  ==============================================================================

    BKSlider.h
    Created: 6 Apr 2017 9:50:44pm
    Author:  Daniel Trueman

  ==============================================================================
*/

/* TODO
 
 1. need version of SingleSlider that can handle multiple values (for Nostalgic transposition)
 
 */

#ifndef BKSLIDER_H_INCLUDED
#define BKSLIDER_H_INCLUDED

#include "BKUtilities.h"
#include "BKComponent.h"
#include "BKLookAndFeel.h"

typedef enum BKMultiSliderType {
    HorizontalMultiSlider = 0,
    VerticalMultiSlider,
    HorizontalMultiBarSlider,
    VerticalMultiBarSlider,
    BKMultiSliderTypeNil
} BKMultiSliderType;



// ******************************************************************************************************************** //
// **************************************************  BKSubSlider **************************************************** //
// ******************************************************************************************************************** //

class BKSubSlider : public Slider
{
public:
    
    BKSubSlider (SliderStyle sstyle, double min, double max, double def, double increment, int width, int height);
    ~BKSubSlider();

    double getValueFromText	(const String & text ) override;
    bool isActive() { return active; }
    void isActive(bool newactive) {active = newactive; }
    void setMinMaxDefaultInc(std::vector<float> newvals);
    void setSkewFromMidpoint(bool sfm);
        
    
private:
    
    double sliderMin, sliderMax;
    double sliderDefault;
    double sliderIncrement;
    
    int sliderWidth, sliderHeight;
    
    bool sliderIsVertical;
    bool sliderIsBar;
    bool skewFromMidpoint;
    
    bool active;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKSubSlider)
};




// ******************************************************************************************************************** //
// **************************************************  BKMultiSlider ************************************************** //
// ******************************************************************************************************************** //


class BKMultiSliderListener
{
    
public:
    
    //BKMultiSliderListener() {}
    virtual ~BKMultiSliderListener() {};
    
    virtual void multiSliderValueChanged(String name, int whichSlider, Array<float> values) = 0;
    virtual void multiSliderAllValuesChanged(String name, Array<Array<float>> values) = 0;
};


class BKMultiSlider :
public Component,
public Slider::Listener,
public TextEditor::Listener
{
    
public:
    
    BKMultiSlider(BKMultiSliderType which);
    ~BKMultiSlider();

    void addSlider(int where, bool active, NotificationType newnotify);
    void addSubSlider(int where, bool active, NotificationType newnotify);

    void deactivateSlider(int where, NotificationType notify);
    void deactivateAll(NotificationType notify);
    void deactivateAllAfter(int where, NotificationType notify);
    void deactivateAllBefore(int where, NotificationType notify);
    
    int whichSlider (const MouseEvent &e);
    int whichSubSlider (int which);
    int whichSubSlider (int which, const MouseEvent &e);
    int whichActiveSlider (int which);
    
    void mouseDrag(const MouseEvent &e) override;
    void mouseMove(const MouseEvent& e) override;
    void mouseDoubleClick (const MouseEvent &e) override;
    void mouseDown (const MouseEvent &event) override;
    void mouseUp (const MouseEvent &event) override;
    void textEditorEscapeKeyPressed (TextEditor& textEditor) override;
    
    void setTo(Array<float> newvals, NotificationType newnotify);
    void setTo(Array<Array<float>> newvals, NotificationType newnotify);
    void setMinMaxDefaultInc(std::vector<float> newvals);
    void setCurrentSlider(int activeSliderNum);
    
    void setAllowSubSlider(bool ss) { allowSubSliders = ss; }
    void setSubSliderName(String ssname) { subSliderName = ssname; }
    void setSkewFromMidpoint(bool sfm);
    
    void cleanupSliderArray();
    void resetRanges();
    
    inline int getNumActive(void) const noexcept { return numActiveSliders;}
    inline int getNumVisible(void) const noexcept { return numVisibleSliders;}
    
    void resized() override;
    
    ListenerList<BKMultiSliderListener> listeners;
    void addMyListener(BKMultiSliderListener* listener)     { listeners.add(listener);      }
    void removeMyListener(BKMultiSliderListener* listener)  { listeners.remove(listener);   }
    
    void setName(String newName)                            { sliderName = newName; showName.setText(sliderName, dontSendNotification);        }
    String getName()                                        { return sliderName; }
    
    Array<Array<float>> getAllValues();
    Array<Array<float>> getAllActiveValues();
    Array<float> getOneSliderBank(int which);
    
private:
    
    BKMultiSliderLookAndFeel activeSliderLookAndFeel;
    BKMultiSliderLookAndFeel passiveSliderLookAndFeel;
    BKMultiSliderLookAndFeel highlightedSliderLookAndFeel;
    BKMultiSliderLookAndFeel displaySliderLookAndFeel;
    
    String sliderName;
    BKLabel showName;

    bool dragging;
    bool arrangedHorizontally;
    bool sliderIsVertical;
    bool sliderIsBar;
    int currentSubSlider;
    int lastHighlightedSlider;
    bool focusLostByEscapeKey;
    bool skewFromMidpoint;
    
    Slider::SliderStyle subsliderStyle;
    
    double currentInvisibleSliderValue;
    
    OwnedArray<OwnedArray<BKSubSlider>> sliders;
    ScopedPointer<BKSubSlider> displaySlider;
    ScopedPointer<BKSubSlider> bigInvisibleSlider;
    ScopedPointer<TextEditor> editValsTextField;
    
    double sliderMin, sliderMax, sliderMinDefault, sliderMaxDefault;
    double sliderDefault;
    double sliderIncrement;
    
    int totalWidth;
    int sliderHeight;
    float sliderWidth;
    int displaySliderWidth;
    
    bool allowSubSliders;
    String subSliderName;
    
    int numActiveSliders;
    int numDefaultSliders;
    int numVisibleSliders;
    
    float clickedHeight;
    
    void sliderValueChanged (Slider *slider) override;
    void textEditorReturnKeyPressed(TextEditor& textEditor) override;
    void textEditorFocusLost(TextEditor& textEditor) override;
    
    void showModifyPopupMenu(int which);
    static void sliderModifyMenuCallback (const int result, BKMultiSlider* slider, int which);
    
    void highlight(int activeSliderNum);
    void deHighlight(int sliderNum);
    int getActiveSlider(int sliderNum);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKMultiSlider)
};



// ******************************************************************************************************************** //
// **************************************************  BKSingleSlider ************************************************* //
// ******************************************************************************************************************** //


class BKSingleSliderListener
{
    
public:
    
    //BKSingleSliderListener() {}
    virtual ~BKSingleSliderListener() {};
    
    virtual void BKSingleSliderValueChanged(String name, double val) = 0;
};

//basic horizontal slider with its own text box and label
//entering values in the text box will reset the range as needed
class BKSingleSlider :
public Component,
public Slider::Listener,
public TextEditor::Listener
{
public:
    BKSingleSlider(String sliderName, double min, double max, double def, double increment);
    ~BKSingleSlider() {};
    
    Slider thisSlider;
    
    String sliderName;
    BKLabel showName;
    bool textIsAbove;
    
    TextEditor valueTF;
    
    void setName(String newName)    { sliderName = newName; showName.setText(sliderName, dontSendNotification); }
    String getName()                { return sliderName; }
    void setTextIsAbove(bool nt)    { textIsAbove = nt; }
    void setJustifyRight(bool jr)
    {
        justifyRight = jr;
        if (justifyRight) showName.setJustificationType(Justification::bottomRight);
        else showName.setJustificationType(Justification::bottomLeft);
    }
    
    void setValue(double newval, NotificationType notify);
    void checkValue(double newval);
    double getValue() {return thisSlider.getValue();}
    
    void sliderValueChanged (Slider *slider) override;
    void textEditorReturnKeyPressed(TextEditor& textEditor) override;
    void mouseUp(const MouseEvent &event) override;
    void mouseDrag(const MouseEvent &e) override;
    void textEditorEscapeKeyPressed (TextEditor& textEditor) override;
    void textEditorFocusLost(TextEditor& textEditor) override;
    void textEditorTextChanged(TextEditor& textEditor) override;
    void resized() override;
    
    void setSkewFactor (double factor, bool symmetricSkew) { thisSlider.setSkewFactor(factor, symmetricSkew); }
    void setSkewFactorFromMidPoint (double sliderValueToShowAtMidPoint	) { thisSlider.setSkewFactorFromMidPoint(sliderValueToShowAtMidPoint); }
    
    ListenerList<BKSingleSliderListener> listeners;
    void addMyListener(BKSingleSliderListener* listener)     { listeners.add(listener);      }
    void removeMyListener(BKSingleSliderListener* listener)  { listeners.remove(listener);   }
    
    void setDim(float newAlpha);
    void setBright();
    
private:
    
    double sliderMin, sliderMax;
    double sliderDefault;
    double sliderIncrement;
    
    bool focusLostByEscapeKey;
    
    bool justifyRight;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKSingleSlider)

};



// ******************************************************************************************************************** //
// **************************************************  BKRangeSlider ************************************************** //
// ******************************************************************************************************************** //


class BKRangeSliderListener
{
    
public:
    
    //BKRangeSliderListener() {}
    virtual ~BKRangeSliderListener() {};
    
    virtual void BKRangeSliderValueChanged(String name, double min, double max) = 0;
};


class BKRangeSlider :
public Component,
public Slider::Listener,
public TextEditor::Listener
{
public:
    BKRangeSlider(String sliderName, double min, double max, double defmin, double defmax, double increment);
    ~BKRangeSlider() {};
    
    Slider minSlider;
    Slider maxSlider;
    String minSliderName;
    String maxSliderName;
    
    Slider invisibleSlider;

    String sliderName;
    BKLabel showName;
    
    TextEditor minValueTF;
    TextEditor maxValueTF;
    
    void setName(String newName)    { sliderName = newName; showName.setText(sliderName, dontSendNotification); }
    String getName()                { return sliderName; }
    void setMinValue(double newval, NotificationType notify);
    void setMaxValue(double newval, NotificationType notify);
    void setIsMinAlwaysLessThanMax(bool im) { isMinAlwaysLessThanMax = im; }
    void setJustifyRight(bool jr)
    {
        justifyRight = jr;
        if (justifyRight) showName.setJustificationType(Justification::bottomRight);
        else showName.setJustificationType(Justification::bottomLeft);
    }
    
    void checkValue(double newval);
    void rescaleMinSlider();
    void rescaleMaxSlider();
    
    void sliderValueChanged (Slider *slider) override;
    void textEditorReturnKeyPressed(TextEditor& textEditor) override;
    void textEditorFocusLost(TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (TextEditor& textEditor) override;
    void textEditorTextChanged(TextEditor& textEditor) override;
    void resized() override;
    void sliderDragEnded(Slider *slider) override;
    void mouseDown (const MouseEvent &event) override;
    
    void setDim(float newAlpha);
    void setBright();
    
    ListenerList<BKRangeSliderListener> listeners;
    void addMyListener(BKRangeSliderListener* listener)     { listeners.add(listener);      }
    void removeMyListener(BKRangeSliderListener* listener)  { listeners.remove(listener);   }
    
private:
    
    double sliderMin, sliderMax;
    double sliderDefaultMin, sliderDefaultMax;
    double sliderIncrement;
    
    bool newDrag;
    bool clickedOnMinSlider;
    bool isMinAlwaysLessThanMax;
    bool focusLostByEscapeKey;
    bool justifyRight;
    
    BKRangeMinSliderLookAndFeel minSliderLookAndFeel;
    BKRangeMaxSliderLookAndFeel maxSliderLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKRangeSlider)
    
};



// ******************************************************************************************************************** //
// *******************************************  BKWaveDistanceUndertowSlider ****************************************** //
// ******************************************************************************************************************** //


class BKWaveDistanceUndertowSliderListener
{
    
public:
    
    //BKRangeSliderListener() {}
    virtual ~BKWaveDistanceUndertowSliderListener() {};
    
    virtual void BKWaveDistanceUndertowSliderValueChanged(String name, double wavedist, double undertow) = 0;
};



class BKWaveDistanceUndertowSlider :
public Component,
public Slider::Listener
//public BKSingleSliderListener,
//public TextEditor::Listener
{
public:
    BKWaveDistanceUndertowSlider();
    ~BKWaveDistanceUndertowSlider() {};
    
    ScopedPointer<Slider> wavedistanceSlider;
    ScopedPointer<Slider> undertowSlider;
    OwnedArray<Slider> displaySliders;
    
    String wavedistanceSliderName;
    String undertowSliderName;
    
    String sliderName;
    BKLabel wavedistanceName;
    BKLabel undertowName;
    
    TextEditor wavedistanceValueTF;
    TextEditor undertowValueTF;
    
    void setName(String newName)    { sliderName = newName; }
    String getName()                { return sliderName; }
    
    void updateSliderPositions(Array<int> newpositions);
    
    void sliderValueChanged (Slider *slider) override {};
    void resized() override;
    void sliderDragEnded(Slider *slider) override;
    void mouseDown (const MouseEvent &event) override {};
    
    void setWaveDistance(int newwavedist, NotificationType notify);
    void setUndertow(int newundertow, NotificationType notify);
    
    void setDim(float newAlpha);
    void setBright();
    
    ListenerList<BKWaveDistanceUndertowSliderListener> listeners;
    void addMyListener(BKWaveDistanceUndertowSliderListener* listener)     { listeners.add(listener);      }
    void removeMyListener(BKWaveDistanceUndertowSliderListener* listener)  { listeners.remove(listener);   }
    
private:
    double sliderMin, sliderMax;
    double sliderIncrement;
    
    int maxSliders;
    
    bool newDrag;
    bool clickedOnMinSlider;

    ImageComponent sampleImageComponent;
    
    //BKRangeMinSliderLookAndFeel minSliderLookAndFeel; //possibly need to remake for this
    //BKRangeMaxSliderLookAndFeel maxSliderLookAndFeel;
    BKMultiSliderLookAndFeel displaySliderLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKWaveDistanceUndertowSlider)
    
};



// ******************************************************************************************************************** //
// ************************************************** BKStackedSlider ************************************************* //
// ******************************************************************************************************************** //



class BKStackedSliderListener
{
    
public:
    
    //BKStackedSliderListener() {}
    virtual ~BKStackedSliderListener() {};
    
    virtual void BKStackedSliderValueChanged(String name, Array<float> val) = 0; //rewrite all this to pass "this" and check by slider ref instead of name?
};

class BKStackedSlider :
public Component,
public Slider::Listener,
public TextEditor::Listener
{
public:
    
    BKStackedSlider(String sliderName, double min, double max, double defmin, double defmax, double def, double increment);
    ~BKStackedSlider() {};
    
    void sliderValueChanged (Slider *slider) override;
    void textEditorReturnKeyPressed(TextEditor& textEditor) override;
    void textEditorFocusLost(TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (TextEditor& textEditor) override;
    void mouseDown (const MouseEvent &event) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseMove(const MouseEvent& e) override;
    void mouseDoubleClick (const MouseEvent &e) override;
    
    void setTo(Array<float> newvals, NotificationType newnotify);
    void setValue(Array<float> newvals, NotificationType newnotify) { setTo(newvals, newnotify); }
    void resetRanges();
    int whichSlider();
    int whichSlider(const MouseEvent& e);
    void addSlider(NotificationType newnotify);
    
    void setName(String newName)    { sliderName = newName; showName.setText(sliderName, dontSendNotification); }
    String getName()                { return sliderName; }
    
    void resized() override;
    
    void setDim(float newAlpha);
    void setBright();
    
    ListenerList<BKStackedSliderListener> listeners;
    void addMyListener(BKStackedSliderListener* listener)     { listeners.add(listener);      }
    void removeMyListener(BKStackedSliderListener* listener)  { listeners.remove(listener);   }
    
private:
    
    ScopedPointer<Slider> topSlider; //user interacts with this
    OwnedArray<Slider> dataSliders;  //displays data, user controls with topSlider
    Array<bool> activeSliders;
    
    ScopedPointer<TextEditor> editValsTextField;
    
    int numSliders;
    int numActiveSliders;
    int clickedSlider;
    float clickedPosition;
    
    String sliderName;
    BKLabel showName;
    bool justifyRight;
    
    BKMultiSliderLookAndFeel stackedSliderLookAndFeel;
    BKMultiSliderLookAndFeel topSliderLookAndFeel;

    double sliderMin, sliderMax, sliderMinDefault, sliderMaxDefault;
    double sliderDefault;
    double sliderIncrement;
    double currentDisplaySliderValue;
    
    bool focusLostByEscapeKey;
    bool mouseJustDown;
    
    Array<float> getAllActiveValues();
    
    void showModifyPopupMenu();
    static void sliderModifyMenuCallback (const int result, BKStackedSlider* ss);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKStackedSlider)
};


#endif  // BKSLIDER_H_INCLUDED