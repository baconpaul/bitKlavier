/*
  ==============================================================================

    GeneralViewController.cpp
    Created: 30 Nov 2016 5:00:39pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "GeneralViewController.h"

CommentViewController::CommentViewController(BKAudioProcessor& p, BKItemGraph* theGraph):
BKViewController(p, theGraph, 1)
{
    setLookAndFeel(&buttonsAndMenusLAF);
    
    removeChildComponent(&hideOrShow);
    
    addAndMakeVisible(comment);
    comment.setName("comment");
    comment.setMultiLine(true);
    comment.setWantsKeyboardFocus(true);
    
    comment.setReturnKeyStartsNewLine(true);
    comment.addListener(this);
    comment.setText("Text here...");
#if JUCE_IOS
    comment.setScrollbarsShown(true);
#endif
    //comment.setColour(TextEditor::ColourIds::high, )
    
    comment.setColour(TextEditor::ColourIds::backgroundColourId, Colours::burlywood.withMultipliedBrightness(0.45f));
    comment.setColour(TextEditor::ColourIds::textColourId, Colours::antiquewhite);
    
    comment.setOpaque(false);
    
    addAndMakeVisible(ok);
    ok.setButtonText("Ok");
    ok.addListener(this);
    ok.setColour(TextEditor::ColourIds::backgroundColourId, Colours::black.withAlpha(0.5f));
    
    addAndMakeVisible(cancel);
    cancel.setButtonText("Cancel");
    cancel.addListener(this);
    cancel.setColour(TextEditor::ColourIds::backgroundColourId, Colours::red.withAlpha(0.2f));
}

CommentViewController::~CommentViewController()
{
    setLookAndFeel(nullptr);
}

void CommentViewController::update(void)
{
    comment.setText(processor.updateState->comment, dontSendNotification);
    comment.grabKeyboardFocus();
}

void CommentViewController::paint (Graphics& g)
{
    g.fillAll(Colours::transparentBlack);
}

void CommentViewController::resized(void)
{
    float heightUnit = (getHeight() - hideOrShow.getHeight()) * 0.1;
    
#if JUCE_IOS
    cancel.setBounds    (0,                 hideOrShow.getBottom(), getWidth()*0.5,         2*heightUnit);
    ok.setBounds        (getWidth()*0.5,    cancel.getY(),          getWidth()*0.5,     2*heightUnit);
    comment.setBounds   (0,                 cancel.getBottom(),     getWidth(),     8*heightUnit);
#else
    comment.setBounds   (0,                 hideOrShow.getBottom(), getWidth(),         9*heightUnit);
    cancel.setBounds    (comment.getX(),    comment.getBottom(),    getWidth()*0.5,     1*heightUnit);
    ok.setBounds        (cancel.getRight(), cancel.getY(),          getWidth()*0.5,     1*heightUnit);
#endif
}

void CommentViewController::bkTextFieldDidChange (TextEditor& tf)
{

}

void CommentViewController::bkButtonClicked (Button* b)
{
    if (b == &ok)
    {
        if (processor.updateState->comment != comment.getText())
            processor.updateState->editsMade = true;
        processor.updateState->comment = comment.getText();
        processor.updateState->commentDidChange = true;
        processor.updateState->setCurrentDisplay(DisplayNil);
    }
    else if (b == &cancel)
    {
        processor.updateState->comment = "";
        processor.updateState->setCurrentDisplay(DisplayNil);
    }
}

AboutViewController::AboutViewController(BKAudioProcessor& p, BKItemGraph* theGraph):
BKViewController(p, theGraph, 1)
{
    setLookAndFeel(&buttonsAndMenusLAF);
    
    addAndMakeVisible(about);
    about.setEnabled(false);
    about.setJustification(Justification::centredTop);
    about.setMultiLine(true);
    String astring = "Welcome to bitKlavier v";
    astring.append(JucePlugin_VersionString, 8);
    astring += "! www.bitKlavier.com\n--\nbitKlavier was created by\nDan Trueman\n Mike Mulshine\nMatt Wang";
    about.setText(astring);

    image = ImageCache::getFromMemory(BinaryData::icon_png, BinaryData::icon_pngSize);
    
    placement = RectanglePlacement::centred;
}

AboutViewController::~AboutViewController()
{
    setLookAndFeel(nullptr);
}
    
void AboutViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    
    g.setOpacity (1.0f);
    
    g.drawImage (image, imageRect, placement);
}

void AboutViewController::resized(void)
{
    hideOrShow.setBounds(10,10,gComponentComboBoxHeight,gComponentComboBoxHeight);
    
    float imageZ = getHeight() * 0.5;
    float imageX = getWidth() * 0.5 - imageZ * 0.5;
    float imageY = 50;
    
    imageRect.setBounds(imageX, imageY, imageZ, imageZ);
    
    about.setBounds(10, imageRect.getBottom() + 20, getWidth() - 20, getBottom() - (imageRect.getBottom() + 20));
    
    repaint();
}

void AboutViewController::bkButtonClicked (Button* b)
{
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(DisplayNil);
    }
}


//==============================================================================
GeneralViewController::GeneralViewController(BKAudioProcessor& p, BKItemGraph* theGraph):
BKViewController(p, theGraph, 2),
processor(p)
{
    
    setLookAndFeel(&buttonsAndMenusLAF);
    
    // Settings UI components setup
    A4tuningReferenceFrequencySlider = std::make_unique<BKSingleSlider>("A4 reference frequency", "A4 reference frequency", 415., 450., 440., 0.1);
    A4tuningReferenceFrequencySlider->setJustifyRight(false);
    A4tuningReferenceFrequencySlider->addMyListener(this);
    addAndMakeVisible(*A4tuningReferenceFrequencySlider);
    
    tempoMultiplierSlider = std::make_unique<BKSingleSlider>("tempo multiplier", "tempo multiplier", 0.25, 4., 1., 0.01);
    tempoMultiplierSlider->setSkewFactorFromMidPoint(1.);
    tempoMultiplierSlider->setJustifyRight(false);
    tempoMultiplierSlider->addMyListener(this);
    addAndMakeVisible(*tempoMultiplierSlider);
    
    GeneralSettings::Ptr gen = processor.gallery->getGeneralSettings();
    
    invertSustainB.addListener(this);
    invertSustainB.setButtonText("invert sustain");
    invertSustainB.setToggleState(gen->getInvertSustain(), dontSendNotification);
    processor.setSustainInversion(gen->getInvertSustain());
    addAndMakeVisible(invertSustainB);
    
    noteOnSetsNoteOffVelocityB.addListener(this);
    noteOnSetsNoteOffVelocityB.setButtonText("noteOn velocity sets noteOff velocity");
    noteOnSetsNoteOffVelocityB.setToggleState(gen->getNoteOnSetsNoteOffVelocity(), dontSendNotification);
    addAndMakeVisible(noteOnSetsNoteOffVelocityB);

#if JUCE_IOS
    tempoMultiplierSlider->addWantsBigOneListener(this);
    A4tuningReferenceFrequencySlider->addWantsBigOneListener(this);
#endif
    
    // Equalizer UI components setup
    // Filter Selection Buttons - icons must be added first to be "behind" the buttons
    lowCutIcon.setImage(ImageCache::getFromMemory(BinaryData::lo_cut_png, BinaryData::lo_cut_pngSize));
    addAndMakeVisible(lowCutIcon);
    lowCutButton.setName("Low Cut");
    lowCutButton.setTooltip("Select the low cut filter to alter its parameters");
    lowCutButton.addListener(this);
    lowCutButton.setAlpha(0.5);
    addAndMakeVisible(lowCutButton);
    
    peak1Icon.setImage(ImageCache::getFromMemory(BinaryData::peak_png, BinaryData::peak_pngSize));
    addAndMakeVisible(peak1Icon);
    peak1Button.setName("Peak 1");
    peak1Button.setTooltip("Select the first peak filter to alter its parameters");
    peak1Button.addListener(this);
    peak1Button.setAlpha(0.5);
    addAndMakeVisible(peak1Button);
    
    peak2Icon.setImage(ImageCache::getFromMemory(BinaryData::peak_png, BinaryData::peak_pngSize));
    addAndMakeVisible(peak2Icon);
    peak2Button.setName("Peak 2");
    peak2Button.setTooltip("Select the second peak filter to alter its parameters");
    peak2Button.addListener(this);
    peak2Button.setAlpha(0.5);
    addAndMakeVisible(peak2Button);
    
    peak3Icon.setImage(ImageCache::getFromMemory(BinaryData::peak_png, BinaryData::peak_pngSize));
    addAndMakeVisible(peak3Icon);
    peak3Button.setName("Peak 3");
    peak3Button.setTooltip("Select the third peak filter to alter its parameters");
    peak3Button.addListener(this);
    peak3Button.setAlpha(0.5);
    addAndMakeVisible(peak3Button);
    
    highCutIcon.setImage(ImageCache::getFromMemory(BinaryData::hi_cut_png, BinaryData::hi_cut_pngSize));
    addAndMakeVisible(highCutIcon);
    highCutButton.setName("High Cut");
    highCutButton.setTooltip("Select the high cut filter to alter its parameters");
    highCutButton.addListener(this);
    highCutButton.setAlpha(0.5);
    addAndMakeVisible(highCutButton);
    
    BKEqualizer* eq = processor.getBKEqualizer();

    addAndMakeVisible(eqGraph);
    eqGraph.updateEQ(*eq);
    
    // EQ Parameter setup
    lowCutFreqSlider = std::make_unique<BKSingleSlider>("Frequency", "Low Cut Frequency", 20.f, 20000.f, eq->getLowCutFreq(), 1.f);
    lowCutFreqSlider->setToolTipString("Adjust the frequency of the low cut");
    lowCutFreqSlider->setSkewFactorFromMidPoint(1000.f);
    lowCutFreqSlider->addMyListener(this);
    addAndMakeVisible(*lowCutFreqSlider);
    
    lowCutSlopeBorder.setText("Slope (db/Oct)");
    lowCutSlopeBorder.setTextLabelPosition(juce::Justification::centred);
    lowCutSlopeBorder.setAlpha(0.75);
    addAndMakeVisible(lowCutSlopeBorder);
    
    lowCutSlope12.setButtonText("12");
    lowCutSlope12.setTooltip("Set the slope of the low cut filter to 12 db/Oct");
    lowCutSlope12.addListener(this);
    lowCutSlope12.setRadioGroupId(RadioButtonID::lowCutButtons);
    addAndMakeVisible(lowCutSlope12);
    
    lowCutSlope24.setButtonText("24");
    lowCutSlope24.setTooltip("Set the slope of the low cut filter to 24 db/Oct");
    lowCutSlope24.addListener(this);
    lowCutSlope24.setRadioGroupId(RadioButtonID::lowCutButtons);
    addAndMakeVisible(lowCutSlope24);
    
    lowCutSlope36.setButtonText("36");
    lowCutSlope36.setTooltip("Set the slope of the low cut filter to 36 db/Oct");
    lowCutSlope36.addListener(this);
    lowCutSlope36.setRadioGroupId(RadioButtonID::lowCutButtons);
    addAndMakeVisible(lowCutSlope36);
    
    lowCutSlope48.setButtonText("48");
    lowCutSlope48.setTooltip("Set the slope of the low cut filter to 48 db/Oct");
    lowCutSlope48.addListener(this);
    lowCutSlope48.setRadioGroupId(RadioButtonID::lowCutButtons);
    addAndMakeVisible(lowCutSlope48);
    
    int lowCutSlopeDefault = eq->getLowCutSlope();
    if (lowCutSlopeDefault == 12) lowCutSlope12.setToggleState(true, juce::dontSendNotification);
    else if (lowCutSlopeDefault == 24) lowCutSlope24.setToggleState(true, juce::dontSendNotification);
    else if (lowCutSlopeDefault == 36) lowCutSlope36.setToggleState(true, juce::dontSendNotification);
    else lowCutSlope48.setToggleState(true, juce::dontSendNotification);
    
    highCutFreqSlider = std::make_unique<BKSingleSlider>("Frequency", "High Cut Frequency", 20.f, 20000.f, eq->getHighCutFreq(), 1.f);
    highCutFreqSlider->setToolTipString("Adjust the frequency of the high cut");
    highCutFreqSlider->setSkewFactorFromMidPoint(1000.f);
    highCutFreqSlider->addMyListener(this);
    addAndMakeVisible(*highCutFreqSlider);
    
    highCutSlopeBorder.setText("Slope (db/Oct)");
    highCutSlopeBorder.setTextLabelPosition(juce::Justification::centred);
    highCutSlopeBorder.setAlpha(0.75);
    addAndMakeVisible(highCutSlopeBorder);
    
    highCutSlope12.setButtonText("12");
    highCutSlope12.setTooltip("Set the slope of the high cut filter to 12 db/Oct");
    highCutSlope12.addListener(this);
    highCutSlope12.setRadioGroupId(RadioButtonID::highCutButtons);
    addAndMakeVisible(highCutSlope12);
    
    highCutSlope24.setButtonText("24");
    highCutSlope24.setTooltip("Set the slope of the high cut filter to 24 db/Oct");
    highCutSlope24.addListener(this);
    highCutSlope24.setRadioGroupId(RadioButtonID::highCutButtons);
    addAndMakeVisible(highCutSlope24);
    
    highCutSlope36.setButtonText("36");
    highCutSlope36.setTooltip("Set the slope of the high cut filter to 36 db/Oct");
    highCutSlope36.addListener(this);
    highCutSlope36.setRadioGroupId(RadioButtonID::highCutButtons);
    addAndMakeVisible(highCutSlope36);
    
    highCutSlope48.setButtonText("48");
    highCutSlope48.setTooltip("Set the slope of the high cut filter to 48 db/Oct");
    highCutSlope48.addListener(this);
    highCutSlope48.setRadioGroupId(RadioButtonID::highCutButtons);
    addAndMakeVisible(highCutSlope48);
    
    int highCutSlopeDefault = eq->getHighCutSlope();
    if (highCutSlopeDefault == 12) highCutSlope12.setToggleState(true, juce::dontSendNotification);
    else if (highCutSlopeDefault == 24) highCutSlope24.setToggleState(true, juce::dontSendNotification);
    else if (highCutSlopeDefault == 36) highCutSlope36.setToggleState(true, juce::dontSendNotification);
    else highCutSlope48.setToggleState(true, juce::dontSendNotification);
    
    peak1FreqSlider = std::make_unique<BKSingleSlider>("Frequency", "Peak 1 Frequency", 20.f, 20000.f, eq->getPeak1Freq(), 1.f);
    peak1FreqSlider->setToolTipString("Adjust the frequency of the first peak filter");
    peak1FreqSlider->setSkewFactorFromMidPoint(1000.f);
    peak1FreqSlider->addMyListener(this);
    addAndMakeVisible(*peak1FreqSlider);
    
    peak1GainSlider = std::make_unique<BKSingleSlider>("Gain (db)", "Peak 1 Gain", -24.f, 24.f, eq->getPeak1Gain(), 0.5f);
    peak1GainSlider->setToolTipString("Adjust the cut or boost size of the first peak filter");
    peak1GainSlider->addMyListener(this);
    addAndMakeVisible(*peak1GainSlider);
    
    peak1QualitySlider = std::make_unique<BKSingleSlider>("Quality", "Peak 1 Quality", 0.1f, 10.f, eq->getPeak1Quality(), 0.05f);
    peak1QualitySlider->setToolTipString("Adjust the quality of the first peak filter's cut or boost");
    peak1QualitySlider->addMyListener(this);
    addAndMakeVisible(*peak1QualitySlider);
    
    peak2FreqSlider = std::make_unique<BKSingleSlider>("Frequency", "Peak 2 Frequency", 20.f, 20000.f, eq->getPeak2Freq(), 1.f);
    peak2FreqSlider->setToolTipString("Adjust the frequency of the second peak filter");
    peak2FreqSlider->setSkewFactorFromMidPoint(1000.f);
    peak2FreqSlider->addMyListener(this);
    addAndMakeVisible(*peak2FreqSlider);
    
    peak2GainSlider = std::make_unique<BKSingleSlider>("Gain (db)", "Peak 2 Gain", -24.f, 24.f, eq->getPeak2Gain(), 0.5f);
    peak2GainSlider->setToolTipString("Adjust the cut or boost size of the second peak filter");
    peak2GainSlider->addMyListener(this);
    addAndMakeVisible(*peak2GainSlider);
    
    peak2QualitySlider = std::make_unique<BKSingleSlider>("Quality", "Peak 2 Quality", 0.1f, 10.f, eq->getPeak2Quality(), 0.05f);
    peak2QualitySlider->setToolTipString("Adjust the quality of the second peak filter's cut or boost");
    peak2QualitySlider->addMyListener(this);
    addAndMakeVisible(*peak2QualitySlider);
    
    peak3FreqSlider = std::make_unique<BKSingleSlider>("Frequency", "Peak 3 Frequency", 20.f, 20000.f, eq->getPeak3Freq(), 1.f);
    peak3FreqSlider->setToolTipString("Adjust the frequency of the third peak filter");
    peak3FreqSlider->setSkewFactorFromMidPoint(1000.f);
    peak3FreqSlider->addMyListener(this);
    addAndMakeVisible(*peak3FreqSlider);
    
    peak3GainSlider = std::make_unique<BKSingleSlider>("Gain (db)", "Peak 3 Gain", -24.f, 24.f, eq->getPeak3Gain(), 0.5f);
    peak3GainSlider->setToolTipString("Adjust the cut or boost size of the third peak filter");
    peak3GainSlider->addMyListener(this);
    addAndMakeVisible(*peak3GainSlider);
    
    peak3QualitySlider = std::make_unique<BKSingleSlider>("Quality", "Peak 3 Quality", 0.1f, 10.f, eq->getPeak3Quality(), 0.05f);
    peak3QualitySlider->setToolTipString("Adjust the quality of the third peak filter's cut or boost");
    peak3QualitySlider->addMyListener(this);
    addAndMakeVisible(*peak3QualitySlider);
    
    
    update();
    
}

#if JUCE_IOS
void GeneralViewController::iWantTheBigOne(TextEditor* tf, String name)
{
    hideOrShow.setAlwaysOnTop(false);
    bigOne.display(tf, name, getBounds());
}
#endif

GeneralViewController::~GeneralViewController()
{
    setLookAndFeel(nullptr);
}

void GeneralViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void GeneralViewController::displayTab(int tab) {
    currentTab = tab;
    invisible();
    displayShared();
    
    if (tab == Tabs::settings) {
        A4tuningReferenceFrequencySlider->setVisible(true);
        tempoMultiplierSlider->setVisible(true);
        invertSustainB.setVisible(true);
        noteOnSetsNoteOffVelocityB.setVisible(true);
    }
    else if (tab == Tabs::equalizer) {
        lowCutButton.setVisible(true);
        lowCutIcon.setVisible(true);
        highCutButton.setVisible(true);
        highCutIcon.setVisible(true);
        peak1Button.setVisible(true);
        peak1Icon.setVisible(true);
        peak2Button.setVisible(true);
        peak2Icon.setVisible(true);
        peak3Button.setVisible(true);
        peak3Icon.setVisible(true);
        eqGraph.setVisible(true);
        displayFilter(currentFilter);
    }
    
}

void GeneralViewController::displayShared() {
    leftArrow.setVisible(true);
    rightArrow.setVisible(true);
}

void GeneralViewController::invisible() {
    A4tuningReferenceFrequencySlider->setVisible(false);
    tempoMultiplierSlider->setVisible(false);
    invertSustainB.setVisible(false);
    noteOnSetsNoteOffVelocityB.setVisible(false);
    lowCutButton.setVisible(false);
    lowCutIcon.setVisible(false);
    highCutButton.setVisible(false);
    highCutIcon.setVisible(false);
    peak1Button.setVisible(false);
    peak1Icon.setVisible(false);
    peak2Button.setVisible(false);
    peak2Icon.setVisible(false);
    peak3Button.setVisible(false);
    peak3Icon.setVisible(false);
    eqGraph.setVisible(false);
    invisibleFilters();
}

void GeneralViewController::resized()
{
    // Position shared UI components
    leftArrow.setBounds(0, getHeight() * 0.4, arrowSpace, arrowSpace);
    rightArrow.setBounds(getRight() - arrowSpace, getHeight() * 0.4, arrowSpace, arrowSpace);
    
    // Position settings UI components
    Rectangle<int> area (getLocalBounds());
    area.reduce(10 * processor.paddingScalarX + 4, 10 * processor.paddingScalarY + 4);
    
    Rectangle<int> leftColumn = area.removeFromLeft(area.getWidth() * 0.5);
    Rectangle<int> comboBoxSlice = leftColumn.removeFromTop(gComponentComboBoxHeight);
    comboBoxSlice.removeFromRight(4 + 2.*gPaddingConst * processor.paddingScalarX);
    comboBoxSlice.removeFromLeft(gXSpacing);
    hideOrShow.setBounds(comboBoxSlice.removeFromLeft(gComponentComboBoxHeight));
    
    Rectangle<int> settingsArea (getLocalBounds());
    settingsArea.removeFromLeft(arrowSpace);
    settingsArea.removeFromRight(arrowSpace);
    settingsArea.reduce(100, 100);
    int numSettings = 4;
    
    A4tuningReferenceFrequencySlider->setBounds(settingsArea.getX(), settingsArea.getY(),
                                                settingsArea.getWidth(),
                                                settingsArea.getHeight() / numSettings);
    tempoMultiplierSlider->setBounds(settingsArea.getX(), A4tuningReferenceFrequencySlider->getBottom(),
                                     settingsArea.getWidth(),
                                     settingsArea.getHeight() / numSettings);
    invertSustainB.setBounds(settingsArea.getX(), tempoMultiplierSlider->getBottom(),
                             settingsArea.getWidth(),
                             settingsArea.getHeight() / numSettings);
    noteOnSetsNoteOffVelocityB.setBounds(settingsArea.getX(), invertSustainB.getBottom(),
                               settingsArea.getWidth(),
                               settingsArea.getHeight() / numSettings);
    
    // Position equalizer UI components
    // Buttons get 1/8 space at top, graph gets 4/8 space in middle, sliders get 3/8 space at bottom
    // Do buttons first
    Rectangle<int> equalizerArea(getLocalBounds());
    equalizerArea.removeFromLeft(arrowSpace + 20);
    equalizerArea.removeFromRight(arrowSpace + 20);
    equalizerArea.removeFromTop(20);
    equalizerArea.removeFromBottom(10);
    Rectangle<int> equalizerButtonsArea(equalizerArea);
    equalizerButtonsArea.removeFromBottom(equalizerArea.getHeight() * 7 / 8);
    
    float spacing = equalizerButtonsArea.getWidth() / 5;
    int padding = spacing - 2; // change the 2 to change padding between buttons
    Rectangle<int> lowCutArea = equalizerButtonsArea.removeFromLeft(spacing).removeFromRight(padding);
    Rectangle<int> peak1Area = equalizerButtonsArea.removeFromLeft(spacing).removeFromRight(padding);
    Rectangle<int> peak2Area = equalizerButtonsArea.removeFromLeft(spacing).removeFromRight(padding);
    Rectangle<int> peak3Area = equalizerButtonsArea.removeFromLeft(spacing).removeFromRight(padding);
    Rectangle<int> highCutArea = equalizerButtonsArea.removeFromLeft(spacing).removeFromRight(padding);
    
    int rx = 20, ry = 10;
    lowCutButton.setBounds(lowCutArea);
    lowCutArea.reduce(rx, ry);
    lowCutIcon.setBounds(lowCutArea);
    peak1Button.setBounds(peak1Area);
    peak1Area.reduce(rx, ry);
    peak1Icon.setBounds(peak1Area);
    peak2Button.setBounds(peak2Area);
    peak2Area.reduce(rx, ry);
    peak2Icon.setBounds(peak2Area);
    peak3Button.setBounds(peak3Area);
    peak3Area.reduce(rx, ry);
    peak3Icon.setBounds(peak3Area);
    highCutButton.setBounds(highCutArea);
    highCutArea.reduce(rx, ry);
    highCutIcon.setBounds(highCutArea);
    
    // Graph
    Rectangle<int> equalizerGraphArea(equalizerArea);
    equalizerGraphArea.removeFromTop(equalizerArea.getHeight() * 1 / 8);
    equalizerGraphArea.removeFromBottom(equalizerArea.getHeight() * 3 / 8);
    equalizerGraphArea.reduce(0, 10); // some padding
    eqGraph.setBounds(equalizerGraphArea);
    
    // Peak parameters
    Rectangle<int> equalizerSlidersArea(equalizerArea);
    equalizerSlidersArea.removeFromTop(equalizerArea.getHeight() * 5 / 8);
    equalizerSlidersArea.removeFromTop(10); // needs a little extra space
    equalizerSlidersArea.reduce(100, 0); // sliders don't need to be so wide
    Rectangle<int> equalizerSlidersAreaCopy(equalizerSlidersArea);
    
    int spacing3 = equalizerSlidersArea.getHeight() / 3;
    Rectangle<int> firstOfThree(equalizerSlidersArea.removeFromTop(spacing3));
    Rectangle<int> secondOfThree(equalizerSlidersArea.removeFromTop(spacing3));
    Rectangle<int> thirdOfThree(equalizerSlidersArea.removeFromTop(spacing3));
    
    peak1FreqSlider->setBounds(firstOfThree);
    peak2FreqSlider->setBounds(firstOfThree);
    peak3FreqSlider->setBounds(firstOfThree);
    
    peak1GainSlider->setBounds(secondOfThree);
    peak2GainSlider->setBounds(secondOfThree);
    peak3GainSlider->setBounds(secondOfThree);
    
    peak1QualitySlider->setBounds(thirdOfThree);
    peak2QualitySlider->setBounds(thirdOfThree);
    peak3QualitySlider->setBounds(thirdOfThree);
    
    // Cut parameters
    int spacing2 = equalizerSlidersAreaCopy.getHeight() / 2;
    int trim = equalizerSlidersAreaCopy.getHeight() / 10;
    Rectangle<int> firstOfTwo(equalizerSlidersAreaCopy.removeFromTop(spacing2));
    firstOfTwo.removeFromTop(trim);
    Rectangle<int> secondOfTwo(equalizerSlidersAreaCopy.removeFromTop(spacing2));
    secondOfTwo.removeFromBottom(trim);
    
    lowCutSlopeBorder.setBounds(secondOfTwo);
    highCutSlopeBorder.setBounds(secondOfTwo);
    int spacing4 = secondOfTwo.getWidth() / 4;
    secondOfTwo.removeFromLeft(spacing4 / 3); // need to move everything over to the right
    Rectangle<int> slope12Area(secondOfTwo.removeFromLeft(spacing4));
    Rectangle<int> slope24Area(secondOfTwo.removeFromLeft(spacing4));
    Rectangle<int> slope36Area(secondOfTwo.removeFromLeft(spacing4));
    Rectangle<int> slope48Area(secondOfTwo.removeFromLeft(spacing4));
    
    lowCutFreqSlider->setBounds(firstOfTwo);
    highCutFreqSlider->setBounds(firstOfTwo);
    
    // Only draw the buttons when there is enough space
    if (spacing4 > 45 & secondOfTwo.getHeight() > 35) {
        lowCutSlope12.setBounds(slope12Area);
        lowCutSlope24.setBounds(slope24Area);
        lowCutSlope36.setBounds(slope36Area);
        lowCutSlope48.setBounds(slope48Area);
        
        highCutSlope12.setBounds(slope12Area);
        highCutSlope24.setBounds(slope24Area);
        highCutSlope36.setBounds(slope36Area);
        highCutSlope48.setBounds(slope48Area);
    }
    else {
        Rectangle<int> zero(0, 0, 0, 0);
        lowCutSlope12.setBounds(zero);
        lowCutSlope24.setBounds(zero);
        lowCutSlope36.setBounds(zero);
        lowCutSlope48.setBounds(zero);
        
        highCutSlope12.setBounds(zero);
        highCutSlope24.setBounds(zero);
        highCutSlope36.setBounds(zero);
        highCutSlope48.setBounds(zero);
    }
    
    // Update view
    displayShared();
    displayTab(currentTab);
    repaint();
}

void GeneralViewController::displayFilter(int filter) {
    invisibleFilters();
    
    currentFilter = filter;
    
    if (filter == Filters::lowCut) {
        lowCutFreqSlider->setVisible(true);
        lowCutSlopeBorder.setVisible(true);
        lowCutSlope12.setVisible(true);
        lowCutSlope24.setVisible(true);
        lowCutSlope36.setVisible(true);
        lowCutSlope48.setVisible(true);
    }
    else if (filter == Filters::peak1) {
        peak1FreqSlider->setVisible(true);
        peak1GainSlider->setVisible(true);
        peak1QualitySlider->setVisible(true);
    }
    else if (filter == Filters::peak2) {
        peak2FreqSlider->setVisible(true);
        peak2GainSlider->setVisible(true);
        peak2QualitySlider->setVisible(true);
    }
    else if (filter == Filters::peak3) {
        peak3FreqSlider->setVisible(true);
        peak3GainSlider->setVisible(true);
        peak3QualitySlider->setVisible(true);
    }
    else if (filter == Filters::highCut) {
        highCutFreqSlider->setVisible(true);
        highCutSlopeBorder.setVisible(true);
        highCutSlope12.setVisible(true);
        highCutSlope24.setVisible(true);
        highCutSlope36.setVisible(true);
        highCutSlope48.setVisible(true);
    }
}

void GeneralViewController::invisibleFilters() {
    lowCutFreqSlider->setVisible(false);
    lowCutSlopeBorder.setVisible(false);
    lowCutSlope12.setVisible(false);
    lowCutSlope24.setVisible(false);
    lowCutSlope36.setVisible(false);
    lowCutSlope48.setVisible(false);
    peak1FreqSlider->setVisible(false);
    peak1GainSlider->setVisible(false);
    peak1QualitySlider->setVisible(false);
    peak2FreqSlider->setVisible(false);
    peak2GainSlider->setVisible(false);
    peak2QualitySlider->setVisible(false);
    peak3FreqSlider->setVisible(false);
    peak3GainSlider->setVisible(false);
    peak3QualitySlider->setVisible(false);
    highCutFreqSlider->setVisible(false);
    highCutSlopeBorder.setVisible(false);
    highCutSlope12.setVisible(false);
    highCutSlope24.setVisible(false);
    highCutSlope36.setVisible(false);
    highCutSlope48.setVisible(false);
}


void GeneralViewController::bkTextFieldDidChange(TextEditor& tf)
{

}

void GeneralViewController::update(void)
{
    GeneralSettings::Ptr gen = processor.gallery->getGeneralSettings();
    
    A4tuningReferenceFrequencySlider->setValue(gen->getTuningFundamental(), dontSendNotification);
    tempoMultiplierSlider->setValue(gen->getTempoMultiplier(), dontSendNotification);
    invertSustainB.setToggleState(gen->getInvertSustain(), dontSendNotification);
}

void GeneralViewController::BKSingleSliderValueChanged(BKSingleSlider* slider, String name, double val)
{
    GeneralSettings::Ptr gen = processor.gallery->getGeneralSettings();
    BKEqualizer* eq = processor.getBKEqualizer();

    if(name == A4tuningReferenceFrequencySlider->getName())
    {
        DBG("general tuning fundamental " + String(val));
        gen->setTuningFundamental(val);
    }
    else if(name == tempoMultiplierSlider->getName())
    {
        DBG("general tempo multiplier " + String(val));
        gen->setTempoMultiplier(val);
    }
    else if (slider == lowCutFreqSlider.get()) {
        float lowCutFreq = lowCutFreqSlider->getValue();
        eq->setLowCutFreq(lowCutFreq);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Low Cut Freq: " + String(lowCutFreq));
    }
    else if (slider == peak1FreqSlider.get()) {
        float peak1Freq = peak1FreqSlider->getValue();
        eq->setPeak1Freq(peak1Freq);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 1 Frequency: " + String(peak1Freq));
    }
    else if (slider == peak1GainSlider.get()) {
        float peak1Gain = peak1GainSlider->getValue();
        eq->setPeak1Gain(peak1Gain);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 1 Gain: " + String(peak1Gain));
    }
    else if (slider == peak1QualitySlider.get()) {
        float peak1Quality = peak1QualitySlider->getValue();
        eq->setPeak1Quality(peak1Quality);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 1 Quality: " + String(peak1Quality));
    }
    else if (slider == peak2FreqSlider.get()) {
        float peak2Freq = peak2FreqSlider->getValue();
        eq->setPeak2Freq(peak2Freq);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 2 Frequency: " + String(peak2Freq));
    }
    else if (slider == peak2GainSlider.get()) {
        float peak2Gain = peak2GainSlider->getValue();
        eq->setPeak2Gain(peak2Gain);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 2 Gain: " + String(peak2Gain));
    }
    else if (slider == peak2QualitySlider.get()) {
        float peak2Quality = peak2QualitySlider->getValue();
        eq->setPeak2Quality(peak2Quality);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 2 Quality: " + String(peak2Quality));
    }
    else if (slider == peak3FreqSlider.get()) {
        float peak3Freq = peak3FreqSlider->getValue();
        eq->setPeak3Freq(peak3Freq);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 3 Frequency: " + String(peak3Freq));
    }
    else if (slider == peak3GainSlider.get()) {
        float peak3Gain = peak3GainSlider->getValue();
        eq->setPeak3Gain(peak3Gain);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 3 Gain: " + String(peak3Gain));
    }
    else if (slider == peak3QualitySlider.get()) {
        float peak3Quality = peak3QualitySlider->getValue();
        eq->setPeak3Quality(peak3Quality);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("Peak 3 Quality: " + String(peak3Quality));
    }
    else if (slider == highCutFreqSlider.get()) {
        float highCutFreq = highCutFreqSlider->getValue();
        eq->setHighCutFreq(highCutFreq);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
        DBG("High Cut Freq: " + String(highCutFreq));
    }
}

void GeneralViewController::bkButtonClicked (Button* b)
{
    GeneralSettings::Ptr gen = processor.gallery->getGeneralSettings();
    
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(DisplayNil);
        
    }
    else if (b == &invertSustainB)
    {
        //bool inversion =  (bool) b->getToggleStateValue().toString().getIntValue();
        bool inversion =  b->getToggleState();
        DBG("invert sustain button = " + String((int)inversion));
        processor.setSustainInversion(inversion);
        gen->setInvertSustain(inversion);
    }
    else if (b == &noteOnSetsNoteOffVelocityB)
    {
        bool newstate =  b->getToggleState();
        DBG("invert noteOnSetsNoteOffVelocity = " + String((int)newstate));
        gen->setNoteOnSetsNoteOffVelocity(newstate);
    }
    else if (b == &lowCutButton) {
        displayFilter(Filters::lowCut);
    }
    else if (b == &peak1Button) {
        displayFilter(Filters::peak1);
    }
    else if (b == &peak2Button) {
        displayFilter(Filters::peak2);
    }
    else if (b == &peak3Button) {
        displayFilter(Filters::peak3);
    }
    else if (b == &highCutButton) {
        displayFilter(Filters::highCut);
    }
    else if (b == &lowCutSlope12) {
        int lowCutSlope = 12;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setLowCutSlope(lowCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &lowCutSlope24) {
        int lowCutSlope = 24;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setLowCutSlope(lowCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &lowCutSlope36) {
        int lowCutSlope = 36;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setLowCutSlope(lowCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &lowCutSlope48) {
        int lowCutSlope = 48;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setLowCutSlope(lowCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &highCutSlope12) {
        int highCutSlope = 12;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setHighCutSlope(highCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &highCutSlope24) {
        int highCutSlope = 24;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setHighCutSlope(highCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &highCutSlope36) {
        int highCutSlope = 36;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setHighCutSlope(highCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &highCutSlope48) {
        int highCutSlope = 48;
        BKEqualizer* eq = processor.getBKEqualizer();
        eq->setHighCutSlope(highCutSlope);
        eqGraph.updateEQ(*eq);
        eqGraph.repaint();
    }
    else if (b == &leftArrow) {
        arrowPressed(LeftArrow);
        displayTab(currentTab);
    }
    else if (b == &rightArrow) {
        arrowPressed(RightArrow);
        displayTab(currentTab);
    }
}



//==============================================================================
ModdableViewController::ModdableViewController(BKAudioProcessor& p, BKItemGraph* theGraph):
BKViewController(p, theGraph, 1)
{
    
    setLookAndFeel(&buttonsAndMenusLAF);
    
    timeSlider = std::make_unique<BKSingleSlider>("mod time (ms)", "mod time (ms)", 0, 2000, 0, 1);
    timeSlider->setToolTipString("how long it will take to ramp to this mod value");
    timeSlider->setJustifyRight(false);
    timeSlider->addMyListener(this);
    addAndMakeVisible(*timeSlider);
    
    incSlider = std::make_unique<BKSingleSlider>("mod increment", "mod increment", -2.0f, 2.0f, 0.0, 0.001);
    incSlider->setToolTipString("how much to increment the mod value on each time the mod is triggered");
    incSlider->setJustifyRight(false);
    incSlider->addMyListener(this);
    addAndMakeVisible(*incSlider);
    
    maxIncSlider = std::make_unique<BKSingleSlider>("max times to increment", "max times to increment", 0, 10, 0, 1, "no max");
    maxIncSlider->setToolTipString("how many times maximum to increment the mod value");
    maxIncSlider->setJustifyRight(false);
    maxIncSlider->addMyListener(this);
    addAndMakeVisible(*maxIncSlider);

#if JUCE_IOS
    timeSlider->addWantsBigOneListener(this);
    incSlider->addWantsBigOneListener(this);
    maxIncSlider->addWantsBigOneListener(this);
#endif
    
    startTimerHz(10);
}

#if JUCE_IOS
void ModdableViewController::iWantTheBigOne(TextEditor* tf, String name)
{
    hideOrShow.setAlwaysOnTop(false);
    bigOne.display(tf, name, getBounds());
}
#endif

ModdableViewController::~ModdableViewController()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void ModdableViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black.withAlpha(0.6f));
    
    Rectangle<int> area (getLocalBounds());
    area.reduce(area.getWidth() * 0.2f, area.getHeight() * 0.2f);
    
    g.setColour(Colours::black);
    g.fillRect(area);
    
    g.setColour(Colours::antiquewhite.withAlpha(0.6f));
    g.drawRect(area, 1);
}

void ModdableViewController::resized()
{
    Rectangle<int> area (getLocalBounds());
    area.reduce(area.getWidth() * 0.2f, area.getHeight() * 0.2f);
    
    Rectangle<int> hideOrShowBounds = area.reduced(10 * processor.paddingScalarX + 4, 10 * processor.paddingScalarY + 4).removeFromLeft(area.getWidth() * 0.5);
    hideOrShowBounds = hideOrShowBounds.removeFromTop(gComponentComboBoxHeight);
    hideOrShowBounds.removeFromRight(4 + 2.0f * gPaddingConst * processor .paddingScalarX);
    hideOrShowBounds.removeFromLeft(gXSpacing);
    hideOrShow.setBounds(hideOrShowBounds.removeFromLeft(gComponentComboBoxHeight));
    
    area.reduce(area.getWidth() * 0.2f, area.getHeight() * 0.2f);
    
    int h = area.getHeight();
    
    timeSlider->setBounds(area.removeFromTop(h * 0.3f));
    incSlider->setBounds(area.removeFromTop(h * 0.3f));
    maxIncSlider->setBounds(area);
}


void ModdableViewController::bkTextFieldDidChange(TextEditor& tf)
{
    
}

void ModdableViewController::update(void)
{
    ModdableBase* mod = getCurrentModdable();
    if (mod == nullptr) return;
    
    timeSlider->setValue(mod->getTime(), dontSendNotification);
    incSlider->setValue(mod->getInc(), dontSendNotification);
    maxIncSlider->setValue(mod->getMaxNumberOfInc(), dontSendNotification);
}

void ModdableViewController::BKSingleSliderValueChanged(BKSingleSlider* slider, String name, double val)
{
    ModdableBase* mod = getCurrentModdable();
    if (mod == nullptr) return;
    
    if (name == timeSlider->getName())
    {
        mod->setTime(int(val));
    }
    if (name == incSlider->getName())
    {
        mod->setInc(val);
    }
    if (name == maxIncSlider->getName())
    {
        mod->setMaxNumberOfInc(val);
    }
}

void ModdableViewController::bkButtonClicked (Button* b)
{
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(processor.updateState->previousDisplay);
    }
}

ModdableBase* ModdableViewController::getCurrentModdable()
{
    // Might be a good idea to standardize the naming/organization of preparations parameters
    // and sliders, etc so this could be streamlined. Or at least use constants for component names
    if (processor.updateState->previousDisplay == DisplayDirectMod)
    {
        DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
        
        if (processor.updateState->currentModdableIdentifier == cDirectGain)
            return &mod->dGain;
        else if (processor.updateState->currentModdableIdentifier == cDirectResonanceGain)
            return &mod->dResonanceGain;
        else if (processor.updateState->currentModdableIdentifier == cDirectHammerGain)
            return &mod->dHammerGain;
        else if (processor.updateState->currentModdableIdentifier == cDirectBlendronicGain)
            return &mod->dBlendronicGain;
    }
    else if (processor.updateState->previousDisplay == DisplayNostalgicMod)
    {
        NostalgicModification::Ptr mod = processor.gallery->getNostalgicModification(processor.updateState->currentModNostalgicId);
        
        if (processor.updateState->currentModdableIdentifier == cNostalgicGain)
            return &mod->nGain;
        else if (processor.updateState->currentModdableIdentifier == cNostalgicBlendronicGain)
            return &mod->nBlendronicGain;
        else if (processor.updateState->currentModdableIdentifier == cNostalgicLengthMultiplier)
            return &mod->nLengthMultiplier;
        else if (processor.updateState->currentModdableIdentifier == cNostalgicBeatsToSkip)
            return &mod->nBeatsToSkip;
        else if (processor.updateState->currentModdableIdentifier == cNostalgicClusterMin)
            return &mod->clusterMin;
        else if (processor.updateState->currentModdableIdentifier == cNostalgicClusterThreshold)
            return &mod->clusterThreshold;
    }
    else if (processor.updateState->previousDisplay == DisplaySynchronicMod)
    {
        SynchronicModification::Ptr mod = processor.gallery->getSynchronicModification(processor.updateState->currentModSynchronicId);
        
        if (processor.updateState->currentModdableIdentifier == cSynchronicGain)
            return &mod->sGain;
        else if (processor.updateState->currentModdableIdentifier == cSynchronicBlendronicGain)
            return &mod->sBlendronicGain;
        else if (processor.updateState->currentModdableIdentifier == cSynchronicNumBeats)
            return &mod->sNumBeats;
        else if (processor.updateState->currentModdableIdentifier == cSynchronicClusterThresh)
            return &mod->sClusterThresh;
        else if (processor.updateState->currentModdableIdentifier == cSynchronicClusterCap)
            return &mod->sClusterCap;
        else if (processor.updateState->currentModdableIdentifier == cSynchronicNumClusters)
            return &mod->numClusters;
    }
    else if (processor.updateState->previousDisplay == DisplayBlendronicMod)
    {
        BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
        
        if (processor.updateState->currentModdableIdentifier == cBlendronicOutGain)
            return &mod->outGain;
    }
    else if (processor.updateState->previousDisplay == DisplayTuningMod)
    {
        TuningModification::Ptr mod = processor.gallery->getTuningModification(processor.updateState->currentModTuningId);
        
        if (processor.updateState->currentModdableIdentifier == cSpringTuningRate)
            return &mod->getSpringTuning()->rate;
        else if (processor.updateState->currentModdableIdentifier == cSpringTuningDrag)
            return &mod->getSpringTuning()->drag;
        else if (processor.updateState->currentModdableIdentifier == cSpringTuningStiffness)
            return &mod->getSpringTuning()->stiffness;
        else if (processor.updateState->currentModdableIdentifier == cSpringTuningIntervalStiffness)
            return &mod->getSpringTuning()->intervalStiffness;
        else if (processor.updateState->currentModdableIdentifier == cSpringTuningTetherWeight)
            return &mod->getSpringTuning()->tetherWeightGlobal;
        else if (processor.updateState->currentModdableIdentifier == cSpringTuningTetherWeightSecondary)
            return &mod->getSpringTuning()->tetherWeightSecondaryGlobal;
        else if (processor.updateState->currentModdableIdentifier == cTuningAdaptiveClusterThresh)
            return &mod->tAdaptiveClusterThresh;
        else if (processor.updateState->currentModdableIdentifier == cTuningAdaptiveHistory)
            return &mod->tAdaptiveHistory;
        else if (processor.updateState->currentModdableIdentifier == cTuningToneSemitoneWidth)
            return &mod->nToneSemitoneWidth;
        else if (processor.updateState->currentModdableIdentifier == cTuningFundamentalOffset)
            return &mod->tFundamentalOffset;
    }
    else if (processor.updateState->previousDisplay == DisplayTempoMod)
    {
        TempoModification::Ptr mod = processor.gallery->getTempoModification(processor.updateState->currentModTempoId);

        if (processor.updateState->currentModdableIdentifier == cTempoTempo)
            return &mod->sTempo;
        else if (processor.updateState->currentModdableIdentifier == cTempoSubdivisions)
            return &mod->subdivisions;
        else if (processor.updateState->currentModdableIdentifier == cTempoAT1History)
            return &mod->at1History;
        else if (processor.updateState->currentModdableIdentifier == cTempoAT1Subdivisions)
            return &mod->at1Subdivisions;
    }
    
    return nullptr;
}


void ModdableViewController::timerCallback()
{
//    ModdableBase* mod = getCurrentModdable();
}
