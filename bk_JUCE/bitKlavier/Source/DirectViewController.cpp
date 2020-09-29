/*
  ==============================================================================

    DirectViewController.cpp
    Created: 7 Jun 2017 1:42:55pm
    Author:  Daniel Trueman

  ==============================================================================
*/

#include "DirectViewController.h"

DirectViewController::DirectViewController(BKAudioProcessor& p, BKItemGraph* theGraph):
BKViewController(p, theGraph, 1)
{
    setLookAndFeel(&buttonsAndMenusLAF);
    
    iconImageComponent.setImage(ImageCache::getFromMemory(BinaryData::direct_icon_png, BinaryData::direct_icon_pngSize));
    iconImageComponent.setImagePlacement(RectanglePlacement(juce::RectanglePlacement::stretchToFit));
    iconImageComponent.setAlpha(0.095);
    //addAndMakeVisible(iconImageComponent);
    
    selectCB.setName("Direct");
    selectCB.addSeparator();
    selectCB.setSelectedItemIndex(0);
    selectCB.setTooltip("Select from available saved preparation settings");
    addAndMakeVisible(selectCB);
    
    transpositionSlider = std::make_unique<BKStackedSlider>("transpositions", -12, 12, -12, 12, 0, 0.01);
    transpositionSlider->setTooltip("Determines pitch (in semitones) of Direct notes; control-click to add another voice, double-click to edit all");
    addAndMakeVisible(*transpositionSlider);
    
    gainSlider = std::make_unique<BKSingleSlider>("gain", 0, 10, 1, 0.01);
    gainSlider->setSkewFactorFromMidPoint(1.);
    gainSlider->setJustifyRight(false);
    gainSlider->setToolTipString("Adjusts overall volume of keyboard");
    addAndMakeVisible(*gainSlider);
    
    resonanceGainSlider = std::make_unique<BKSingleSlider>("resonance gain", 0, 10, 0.2, 0.01);
    resonanceGainSlider->setSkewFactorFromMidPoint(1.);
    resonanceGainSlider->setJustifyRight(false);
    resonanceGainSlider->setToolTipString("Adjusts overall resonance/reverb based on keyOff velocity; change to keyOn velocity in Gallery>settings");
    addAndMakeVisible(*resonanceGainSlider);
    
    hammerGainSlider = std::make_unique<BKSingleSlider>("hammer gain", 0, 10, 1, 0.01);
    hammerGainSlider->setSkewFactorFromMidPoint(1.);
    hammerGainSlider->setJustifyRight(false);
    hammerGainSlider->setToolTipString("Adjusts mechanical noise sample based on keyOff velocity; change to keyOn velocity in Gallery>settings");
    addAndMakeVisible(*hammerGainSlider);
    
    velocityMinMaxSlider = std::make_unique<BKRangeSlider>("velocity min/max", 0, 127, 0, 127, 1);
    velocityMinMaxSlider->setToolTipString("Sets Min and Max velocity (0-127) to trigger this Direct; Min can be greater than Max");
    velocityMinMaxSlider->setJustifyRight(true);
    addAndMakeVisible(*velocityMinMaxSlider, ALL);
    
    ADSRSlider = std::make_unique<BKADSRSlider>("ADSR");
    ADSRSlider->setButtonText("edit envelope");
    ADSRSlider->setToolTip("adjust Attack, Decay, Sustain, and Release envelope parameters");
    ADSRSlider->setButtonMode(false);
    addAndMakeVisible(*ADSRSlider);
    setShowADSR(false);
    
    transpUsesTuning.setButtonText ("use Tuning?");
    transpUsesTuning.setTooltip("transposition will be tuned using attached Tuning");
    buttonsAndMenusLAF.setToggleBoxTextToRightBool(false);
    transpUsesTuning.setToggleState (false, dontSendNotification);
    addAndMakeVisible(&transpUsesTuning, ALL);
    
#if JUCE_IOS
    transpositionSlider->addWantsBigOneListener(this);
    gainSlider->addWantsBigOneListener(this);
    resonanceGainSlider->addWantsBigOneListener(this);
    hammerGainSlider->addWantsBigOneListener(this);
    velocityMinMaxSlider->addWantsBigOneListener(this);s
#endif
    
    addAndMakeVisible(actionButton);
    actionButton.setButtonText("Action");
    actionButton.setTooltip("Create, duplicate, rename, delete, or reset current settings");
    actionButton.addListener(this);
    
    currentTab = 0;
    displayTab(currentTab);

}

void DirectViewController::invisible(void)
{
    gainSlider->setVisible(false);
    resonanceGainSlider->setVisible(false);
    hammerGainSlider->setVisible(false);
    ADSRSlider->setVisible(false);
    
    transpositionSlider->setVisible(false);
    transpUsesTuning.setVisible(false);
    
    velocityMinMaxSlider->setVisible(false);
    //etc...
}

void DirectViewController::displayShared(void)
{
    Rectangle<int> area (getBounds());
    
    iconImageComponent.setBounds(area);
    
    area.reduce(10 * processor.paddingScalarX + 4, 10 * processor.paddingScalarY + 4);
    
    Rectangle<int> leftColumn = area.removeFromLeft(area.getWidth() * 0.5);
    Rectangle<int> comboBoxSlice = leftColumn.removeFromTop(gComponentComboBoxHeight);
    comboBoxSlice.removeFromRight(4 + 2.0f * gPaddingConst * processor .paddingScalarX);
    comboBoxSlice.removeFromLeft(gXSpacing);
    hideOrShow.setBounds(comboBoxSlice.removeFromLeft(gComponentComboBoxHeight));
    comboBoxSlice.removeFromLeft(gXSpacing);
    selectCB.setBounds(comboBoxSlice.removeFromLeft(comboBoxSlice.getWidth() / 2.));
    
    actionButton.setBounds(selectCB.getRight()+gXSpacing,
                           selectCB.getY(),
                           selectCB.getWidth() * 0.5,
                           selectCB.getHeight());
    
    //leftArrow.setBounds (0, getHeight() * 0.4, 50, 50);
    //rightArrow.setBounds (getRight() - 50, getHeight() * 0.4, 50, 50);
    leftArrow.setVisible(false); //use one tab for now
    rightArrow.setVisible(false);
    
}

void DirectViewController::displayTab(int tab)
{
    DBG(String(processor.updateState->currentDirectId));
    currentTab = tab;
    
    invisible();
    displayShared();
    
    if (tab == 0)
    {
        gainSlider->setVisible(true);
        resonanceGainSlider->setVisible(true);
        hammerGainSlider->setVisible(true);
        ADSRSlider->setVisible(true);
        transpositionSlider->setVisible(true);
        transpUsesTuning.setVisible(true);
        velocityMinMaxSlider->setVisible(true);
        
        Rectangle<int> area (getBounds());
        area.removeFromTop(selectCB.getHeight() + 70 * processor.paddingScalarY + 4 + gYSpacing);
        area.removeFromBottom(gYSpacing + 30 * processor.paddingScalarY);
        area.removeFromLeft(processor.paddingScalarX * 20);
        area.removeFromRight(processor.paddingScalarX * 20);
        
        ADSRSlider->setBounds(area.removeFromBottom(area.getHeight() * 0.35));

        area.removeFromBottom(processor.paddingScalarY * 50);
        Rectangle<int> leftColumn (area.removeFromLeft(area.getWidth()* 0.5)); //area is now right column
        leftColumn.removeFromRight(processor.paddingScalarX * 20);
        area.removeFromLeft(processor.paddingScalarX * 20);
        
        int columnHeight = leftColumn.getHeight();
        
        gainSlider->setBounds(leftColumn.removeFromBottom(columnHeight / 3));
        resonanceGainSlider->setBounds(leftColumn.removeFromBottom(columnHeight / 3));
        hammerGainSlider->setBounds(leftColumn.removeFromBottom(columnHeight / 3));
    
        velocityMinMaxSlider->setBounds(area.removeFromBottom(gainSlider->getHeight() + 9 * processor.paddingScalarY));
        area.removeFromBottom(21 * processor.paddingScalarY);
        transpUsesTuning.setBounds(area.removeFromBottom(gComponentToggleBoxHeight));
        transpositionSlider->setBounds(area.removeFromBottom(gComponentStackedSliderHeight + processor.paddingScalarY * 30));
    }
    else if (tab == 1)
    {
        // SET VISIBILITY
        transpositionSlider->setVisible(true);
        
        Rectangle<int> area (getBounds());
        area.removeFromTop(selectCB.getHeight() + 100 * processor.paddingScalarY + 4 + gYSpacing);
        area.removeFromRight(rightArrow.getWidth());
        area.removeFromLeft(leftArrow.getWidth());
        area.removeFromBottom(area.getHeight() * 0.3);
        
        transpositionSlider->setBounds(area.removeFromBottom(gComponentStackedSliderHeight + processor.paddingScalarY * 30));
        
        area.removeFromLeft(processor.paddingScalarX * 100);
        area.removeFromRight(processor.paddingScalarX * 100);
    }
}


void DirectViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void DirectViewController::setShowADSR(bool newval)
{
    showADSR = newval;
    
    if(showADSR)
    {
        resonanceGainSlider->setVisible(false);
        hammerGainSlider->setVisible(false);
        gainSlider->setVisible(false);
        transpositionSlider->setVisible(false);
        
        ADSRSlider->setButtonText("close envelope");

        
    }
    else
    {
        resonanceGainSlider->setVisible(true);
        hammerGainSlider->setVisible(true);
        gainSlider->setVisible(true);
        transpositionSlider->setVisible(true);

        ADSRSlider->setButtonText("edit envelope");
    }
    
    resized();
    
}

void DirectViewController::resized()
{
    
    displayShared();
    displayTab(currentTab);
    
#if 0
    Rectangle<int> area (getLocalBounds());

    iconImageComponent.setBounds(area);
    area.reduce(10 * processor.paddingScalarX + 4, 10 * processor.paddingScalarY + 4);
    
    Rectangle<int> areaSave = area;

    Rectangle<int> leftColumn = area.removeFromLeft(area.getWidth() * 0.5);
    Rectangle<int> comboBoxSlice = leftColumn.removeFromTop(gComponentComboBoxHeight);
    comboBoxSlice.removeFromRight(4 + 2.*gPaddingConst * processor.paddingScalarX);
    comboBoxSlice.removeFromLeft(gXSpacing);
    hideOrShow.setBounds(comboBoxSlice.removeFromLeft(gComponentComboBoxHeight));
    comboBoxSlice.removeFromLeft(gXSpacing);
    selectCB.setBounds(comboBoxSlice.removeFromLeft(comboBoxSlice.getWidth() / 2.));

    comboBoxSlice.removeFromLeft(gXSpacing);
    
#if JUCE_IOS
    actionButton.setBounds(selectCB.getRight()+gXSpacing,
                           selectCB.getY(),
                           selectCB.getWidth() * 0.5,
                           selectCB.getHeight());
#else
    actionButton.setBounds(selectCB.getRight()+gXSpacing,
                           selectCB.getY(),
                           selectCB.getWidth() * 0.5,
                           selectCB.getHeight());
#endif
    
    /* *** above here should be generic to all prep layouts *** */
    /* ***    below here will be specific to each prep      *** */
    
    if(!showADSR)
    {
        Rectangle<int> sliderSlice = leftColumn;
        sliderSlice.removeFromRight(gXSpacing + 2.*gPaddingConst * processor.paddingScalarX);
        //sliderSlice.removeFromLeft(gXSpacing);
        /*
         sliderSlice.reduce(4 + 2.*gPaddingConst * processor.paddingScalarX,
         4 + 2.*gPaddingConst * processor.paddingScalarY);
         */
        
        int nextCenter = sliderSlice.getY() + sliderSlice.getHeight() / 5.;
        resonanceGainSlider->setBounds(sliderSlice.getX(),
                                       nextCenter - gComponentSingleSliderHeight/2 + 8,
                                       sliderSlice.getWidth(),
                                       gComponentSingleSliderHeight);
        
        nextCenter = sliderSlice.getY() + sliderSlice.getHeight() / 2.;
        hammerGainSlider->setBounds(sliderSlice.getX(),
                                    nextCenter - gComponentSingleSliderHeight/2 + 8,
                                    sliderSlice.getWidth(),
                                    gComponentSingleSliderHeight);
        
        nextCenter = sliderSlice.getY() + 4. * sliderSlice.getHeight() / 5.;
        gainSlider->setBounds(sliderSlice.getX(),
                              nextCenter - gComponentSingleSliderHeight/2 + 4,
                              sliderSlice.getWidth(),
                              gComponentSingleSliderHeight);
        
        
        
        //leftColumn.reduce(4, 0);
        area.removeFromLeft(gXSpacing + 2.*gPaddingConst * processor.paddingScalarX);
        area.removeFromRight(gXSpacing);
        
        
        transpositionSlider->setBounds(area.getX(),
                                       resonanceGainSlider->getY(),
                                       area.getWidth(),
                                       gComponentStackedSliderHeight + processor.paddingScalarY * 30);
        
        
        //area.removeFromTop(gComponentComboBoxHeight);
        //transpositionSlider->setBounds(area.removeFromTop(gComponentStackedSliderHeight + processor.paddingScalarY * 40));
        
        //area.removeFromTop(gYSpacing + 6.*gPaddingConst * processor.paddingScalarY);
        //ADSRSlider->setBounds(area.removeFromTop(5*gComponentSingleSliderHeight));
        
        //area.removeFromBottom(gYSpacing + 6.*gPaddingConst * processor.paddingScalarY);
        //ADSRSlider->setBounds(area.removeFromBottom(gComponentComboBoxHeight));
        
        ADSRSlider->setBounds(area.getX(),
                              gainSlider->getY() + gComponentComboBoxHeight * 0.5,
                              area.getWidth(),
                              gComponentComboBoxHeight);
    }
    else
    {
        areaSave.removeFromTop(gYSpacing * 2 + 8.*gPaddingConst * processor.paddingScalarY);
        Rectangle<int> adsrSliderSlice = areaSave.removeFromTop(gComponentComboBoxHeight * 2 + gComponentSingleSliderHeight * 2 + gYSpacing * 3);
        ADSRSlider->setBounds(adsrSliderSlice);
        
        //areaSave.removeFromTop(gComponentComboBoxHeight * 2 + gYSpacing + 8.*gPaddingConst * processor.paddingScalarY);
        //ADSRSlider->setBounds(areaSave);
        
        selectCB.toFront(false);
    }
#endif
}

#if JUCE_IOS
void DirectViewController::iWantTheBigOne(TextEditor* tf, String name)
{
    hideOrShow.setAlwaysOnTop(false);
    bigOne.display(tf, name, getBounds());
}
#endif

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ DirectPreparationEditor ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~//
DirectPreparationEditor::DirectPreparationEditor(BKAudioProcessor& p, BKItemGraph* theGraph):
DirectViewController(p, theGraph)
{
    selectCB.addMyListener(this);
    selectCB.addListener(this);
    
    fillSelectCB(-1,-1);
    
    transpositionSlider->addMyListener(this);
    gainSlider->addMyListener(this);
    resonanceGainSlider->addMyListener(this);
    hammerGainSlider->addMyListener(this);
    ADSRSlider->addMyListener(this);
    transpUsesTuning.addListener(this);
    velocityMinMaxSlider->addMyListener(this);
    
    startTimer(30);
}

void DirectPreparationEditor::update(void)
{
    if (processor.updateState->currentDirectId < 0) return;
    setShowADSR(false);
    setSubWindowInFront(false);
    ADSRSlider->setIsButtonOnly(true);
    
    DirectPreparation::Ptr prep = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);

    if (prep != nullptr)
    {
        selectCB.setSelectedId(processor.updateState->currentDirectId, dontSendNotification);
        
        transpositionSlider->setValue(prep->getTransposition(), dontSendNotification);
        resonanceGainSlider->setValue(prep->getResonanceGain(), dontSendNotification);
        hammerGainSlider->setValue(prep->getHammerGain(), dontSendNotification);
        gainSlider->setValue(prep->getGain(), dontSendNotification);
        ADSRSlider->setAttackValue(prep->getAttack(), dontSendNotification);
        ADSRSlider->setDecayValue(prep->getDecay(), dontSendNotification);
        ADSRSlider->setSustainValue(prep->getSustain(), dontSendNotification);
        ADSRSlider->setReleaseValue(prep->getRelease(), dontSendNotification);
        transpUsesTuning.setToggleState(prep->getTranspUsesTuning(), dontSendNotification);
        
        velocityMinMaxSlider->setMinValue(prep->getVelocityMin(), dontSendNotification);
        velocityMinMaxSlider->setMaxValue(prep->getVelocityMax(), dontSendNotification);
    }
}

void DirectPreparationEditor::bkMessageReceived (const String& message)
{
    if (message == "direct/update")
    {
        update();
    }
}

int DirectPreparationEditor::addPreparation(void)
{
    processor.gallery->add(PreparationTypeDirect);
    
    return processor.gallery->getAllDirect().getLast()->getId();
}

int DirectPreparationEditor::duplicatePreparation(void)
{
    processor.gallery->duplicate(PreparationTypeDirect, processor.updateState->currentDirectId);
    
    return processor.gallery->getAllDirect().getLast()->getId();
}

void DirectPreparationEditor::deleteCurrent(void)
{
    int directId = selectCB.getSelectedId();
    int index = selectCB.getSelectedItemIndex();
    
    if ((index == 0) && (selectCB.getItemId(index+1) == -1)) return;
    
    processor.gallery->remove(PreparationTypeDirect, directId);
    
    fillSelectCB(0, 0);
    
    selectCB.setSelectedItemIndex(0, dontSendNotification);
    int newId = selectCB.getSelectedId();
    
    setCurrentId(newId);
}

void DirectPreparationEditor::setCurrentId(int Id)
{
    processor.updateState->currentDirectId = Id;
    
    processor.updateState->idDidChange = true;
    
    update();
    
    fillSelectCB(lastId, Id);
    
    lastId = Id;
}

void DirectPreparationEditor::actionButtonCallback(int action, DirectPreparationEditor* vc)
{
    if (vc == nullptr)
    {
        PopupMenu::dismissAllActiveMenus();
        return;
    }
    
    BKAudioProcessor& processor = vc->processor;
    
    if (action == 1)
    {
        int Id = vc->addPreparation();
        vc->setCurrentId(Id);
        processor.saveGalleryToHistory("New Direct Preparation");
    }
    else if (action == 2)
    {
        int Id = vc->duplicatePreparation();
        vc->setCurrentId(Id);
        processor.saveGalleryToHistory("Duplicate Direct Preparation");
    }
    else if (action == 3)
    {
        vc->deleteCurrent();
        processor.saveGalleryToHistory("Delete Direct Preparation");
    }
    else if (action == 4)
    {
        processor.reset(PreparationTypeDirect, processor.updateState->currentDirectId);
        vc->update();
    }
    else if (action == 5)
    {
        processor.clear(PreparationTypeDirect, processor.updateState->currentDirectId);
        vc->update();
        processor.saveGalleryToHistory("Clear Direct Preparation");
    }
    else if (action == 6)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentDirectId;
        Direct::Ptr prep = processor.gallery->getDirect(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            prep->setName(name);
            vc->fillSelectCB(Id, Id);
            processor.saveGalleryToHistory("Rename Direct Preparation");
        }
        
        vc->update();
    }
    else if (action == 7)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentDirectId;
        Direct::Ptr prep = processor.gallery->getDirect(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        DBG("name: " + String(name));
        
        if (result == 1)
        {
            processor.exportPreparation(PreparationTypeDirect, Id, name);
        }
    }
    else if (action >= 100)
    {
        int which = action - 100;
        processor.importPreparation(PreparationTypeDirect, processor.updateState->currentDirectId, which);
        vc->update();
        processor.saveGalleryToHistory("Import Direct Preparation");
    }
}

void DirectPreparationEditor::bkComboBoxDidChange (ComboBox* box)
{
    String name = box->getName();
    int Id = box->getSelectedId();

    if (name == "Direct")
    {
        setCurrentId(Id);
    }
    
    processor.updateState->editsMade = true;
}

void DirectPreparationEditor::BKEditableComboBoxChanged(String name, BKEditableComboBox* cb)
{
    Direct::Ptr direct = processor.gallery->getDirect(processor.updateState->currentDirectId);
    direct->setName(name);
    
    fillSelectCB(0, processor.updateState->currentDirectId);
    
    processor.updateState->editsMade = true;
}

void DirectPreparationEditor::BKSingleSliderValueChanged(BKSingleSlider* slider, String name, double val)
{
    DirectPreparation::Ptr prep = processor.gallery->getStaticDirectPreparation(processor.updateState->currentDirectId);
    DirectPreparation::Ptr active = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);
    
    if (prep == nullptr || active == nullptr) return;
    
    if(name == "resonance gain")
    {
        //DBG("note length multiplier " + String(val));
        prep->setResonanceGain(val);
        active->setResonanceGain(val);
    }
    else if(name == "hammer gain")
    {
        //DBG("beats to skip " + String(val));
        prep->setHammerGain(val);
        active->setHammerGain(val);
    }
    else if(name == "gain")
    {
        //DBG("gain " + String(val));
        prep->setGain(val);
        active->setGain(val);
    }
    
    processor.updateState->editsMade = true;
}

void DirectPreparationEditor::BKRangeSliderValueChanged(String name, double minval, double maxval)
{
    DirectPreparation::Ptr prep = processor.gallery->getStaticDirectPreparation(processor.updateState->currentDirectId);
    DirectPreparation::Ptr active = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);
    
    if(name == "velocity min/max") {
        DBG("got new velocity min/max " + String(minval) + " " + String(maxval));
        prep->setVelocityMin(minval);
        prep->setVelocityMax(maxval);
        active->setVelocityMin(minval);
        active->setVelocityMax(maxval);
    }
    
    processor.updateState->editsMade = true;
}

void DirectPreparationEditor::BKStackedSliderValueChanged(String name, Array<float> val)
{
    DirectPreparation::Ptr prep = processor.gallery->getStaticDirectPreparation(processor.updateState->currentDirectId);
    DirectPreparation::Ptr active = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);
    
    prep->setTransposition(val);
    active->setTransposition(val);
    
    processor.updateState->editsMade = true;
}

void DirectPreparationEditor::BKADSRSliderValueChanged(String name, int attack, int decay, float sustain, int release)
{
    DBG("BKADSRSliderValueChanged received");
    
    DirectPreparation::Ptr prep = processor.gallery->getStaticDirectPreparation(processor.updateState->currentDirectId);
    DirectPreparation::Ptr active = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);
    
    prep->setAttack(attack);
    active->setAttack(attack);
    prep->setDecay(decay);
    active->setDecay(decay);
    prep->setSustain(sustain);
    active->setSustain(sustain);
    prep->setRelease(release);
    active->setRelease(release);
    
    processor.updateState->editsMade = true;
}

void DirectPreparationEditor::BKADSRButtonStateChanged(String name, bool mod, bool state)
{
    setShowADSR(!state);
    setSubWindowInFront(!state);
}

void DirectPreparationEditor::timerCallback()
{
    if (processor.updateState->currentDisplay == DisplayDirect)
    {
        DirectProcessor::Ptr dProcessor = processor.currentPiano->getDirectProcessor(processor.updateState->currentDirectId);
        DirectPreparation::Ptr active = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);
        velocityMinMaxSlider->setDisplayValue(dProcessor->getLastVelocity() * 127.);
        DBG("sProcessor->getLastVelocity() = " + String(dProcessor->getLastVelocity()));
    }
}

void DirectPreparationEditor::closeSubWindow()
{
    ADSRSlider->setIsButtonOnly(true);
    setShowADSR(false);
    setSubWindowInFront(false);
}


void DirectPreparationEditor::fillSelectCB(int last, int current)
{
    selectCB.clear(dontSendNotification);
    
    for (auto prep : processor.gallery->getAllDirect())
    {
        int Id = prep->getId();
        
        if (Id == -1) continue;
        
        String name = prep->getName();
        
        if (name != String())  selectCB.addItem(name, Id);
        else                        selectCB.addItem("Direct"+String(Id), Id);
        
        selectCB.setItemEnabled(Id, true);
        if (processor.currentPiano->isActive(PreparationTypeDirect, Id))
            selectCB.setItemEnabled(Id, false);
    }
    
    if (last != 0)      selectCB.setItemEnabled(last, true);
    if (current != 0)   selectCB.setItemEnabled(current, false);
    
    int selectedId = processor.updateState->currentDirectId;
    
    selectCB.setSelectedId(selectedId, NotificationType::dontSendNotification);
    
    selectCB.setItemEnabled(selectedId, false);
    
    
    lastId = selectedId;
}

void DirectPreparationEditor::buttonClicked (Button* b)
{
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(DisplayNil);
        
        ADSRSlider->setIsButtonOnly(true);
        setShowADSR(false);
        setSubWindowInFront(false);
        
    }
    else if (b == &actionButton)
    {
        bool single = processor.gallery->getAllDirect().size() == 2;
        getPrepOptionMenu(PreparationTypeDirect, single).showMenuAsync (PopupMenu::Options().withTargetComponent (&actionButton), ModalCallbackFunction::forComponent (actionButtonCallback, this) );
    }
    else if (b == &rightArrow)
    {
        arrowPressed(RightArrow);
        
        DBG("currentTab: " + String(currentTab));
        
        displayTab(currentTab);
    }
    else if (b == &leftArrow)
    {
        arrowPressed(LeftArrow);
        
        DBG("currentTab: " + String(currentTab));
        
        displayTab(currentTab);
    }
    else if (b == &transpUsesTuning)
    {
        DirectPreparation::Ptr prep = processor.gallery->getStaticDirectPreparation(processor.updateState->currentDirectId);
        DirectPreparation::Ptr active = processor.gallery->getActiveDirectPreparation(processor.updateState->currentDirectId);
        DBG("Direct transpUsesTuning = " + String((int)b->getToggleState()));
        prep->setTranspUsesTuning(b->getToggleState());
        active->setTranspUsesTuning(b->getToggleState());
        
    }
}


// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ DirectModificationEditor ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~//
DirectModificationEditor::DirectModificationEditor(BKAudioProcessor& p, BKItemGraph* theGraph):
DirectViewController(p, theGraph)
{
    
    selectCB.addListener(this);
    selectCB.addMyListener(this);
    
    fillSelectCB(-1,-1);
    
    transpositionSlider->addMyListener(this);
    gainSlider->addMyListener(this);
    resonanceGainSlider->addMyListener(this);
    hammerGainSlider->addMyListener(this);
    ADSRSlider->addMyListener(this);
    transpUsesTuning.addListener(this);
    velocityMinMaxSlider->addMyListener(this);
    
    velocityMinMaxSlider->displaySliderVisible(false);
}

void DirectModificationEditor::greyOutAllComponents()
{
    hammerGainSlider->setDim(gModAlpha);
    resonanceGainSlider->setDim(gModAlpha);
    transpositionSlider->setDim(gModAlpha);
    gainSlider->setDim(gModAlpha);
    ADSRSlider->setDim(gModAlpha);
    transpUsesTuning.setAlpha(gModAlpha);
    velocityMinMaxSlider->setDim(gModAlpha);
}

void DirectModificationEditor::highlightModedComponents()
{
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);

    if(mod->getDirty(DirectTransposition))    transpositionSlider->setBright();
    if(mod->getDirty(DirectGain))             gainSlider->setBright();
    if(mod->getDirty(DirectResGain))          resonanceGainSlider->setBright();
    if(mod->getDirty(DirectHammerGain))       hammerGainSlider->setBright();
    if(mod->getDirty(DirectADSR))             ADSRSlider->setBright();
    if(mod->getDirty(DirectTranspUsesTuning)) transpUsesTuning.setAlpha(1.);
    if(mod->getDirty(DirectVelocityMin))      velocityMinMaxSlider->setBright();
    if(mod->getDirty(DirectVelocityMax))      velocityMinMaxSlider->setBright();
}

void DirectModificationEditor::update(void)
{
    if (processor.updateState->currentModDirectId < 0) return;
    
    selectCB.setSelectedId(processor.updateState->currentModDirectId, dontSendNotification);
    
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
    
    if (mod != nullptr)
    {
        greyOutAllComponents();
        highlightModedComponents();
        
        transpositionSlider->setValue(mod->getTransposition(), dontSendNotification);
        resonanceGainSlider->setValue(mod->getResonanceGain(), dontSendNotification);
        hammerGainSlider->setValue(mod->getHammerGain(), dontSendNotification);
        gainSlider->setValue(mod->getGain(), dontSendNotification);
        ADSRSlider->setValue(mod->getADSRvals(), dontSendNotification);
        transpUsesTuning.setToggleState(mod->getTranspUsesTuning(), dontSendNotification);
        velocityMinMaxSlider->setMinValue(mod->getVelocityMin(), dontSendNotification);
        velocityMinMaxSlider->setMaxValue(mod->getVelocityMax(), dontSendNotification);
    }
}

void DirectModificationEditor::fillSelectCB(int last, int current)
{
    selectCB.clear(dontSendNotification);
    
    for (auto prep : processor.gallery->getDirectModifications())
    {
        int Id = prep->getId();;
        String name = prep->getName();
        
        if (name != String())  selectCB.addItem(name, Id);
        else                        selectCB.addItem("DirectMod"+String(Id), Id);
        
        selectCB.setItemEnabled(Id, true);
        if (processor.currentPiano->isActive(PreparationTypeDirect, Id))
            selectCB.setItemEnabled(Id, false);
    }
    
    if (last != 0)      selectCB.setItemEnabled(last, true);
    if (current != 0)   selectCB.setItemEnabled(current, false);
    
    int selectedId = processor.updateState->currentDirectId;
    
    selectCB.setSelectedId(selectedId, NotificationType::dontSendNotification);
    
    selectCB.setItemEnabled(selectedId, false);
    
    lastId = selectedId;
    
}

void DirectModificationEditor::bkMessageReceived (const String& message)
{
    if (message == "direct/update")
    {
        update();
    }
}

int DirectModificationEditor::addPreparation(void)
{
    processor.gallery->add(PreparationTypeDirectMod);
    
    return processor.gallery->getDirectModifications().getLast()->getId();
}

int DirectModificationEditor::duplicatePreparation(void)
{
    processor.gallery->duplicate(PreparationTypeDirectMod, processor.updateState->currentModDirectId);
    
    return processor.gallery->getDirectModifications().getLast()->getId();
}

void DirectModificationEditor::deleteCurrent(void)
{
    int directId = selectCB.getSelectedId();
    int index = selectCB.getSelectedItemIndex();
    
    if ((index == 0) && (selectCB.getItemId(index+1) == -1)) return;
    
    processor.gallery->remove(PreparationTypeDirect, directId);
    
    fillSelectCB(0, 0);
    
    selectCB.setSelectedItemIndex(0, dontSendNotification);
    int newId = selectCB.getSelectedId();
    
    setCurrentId(newId);
}

void DirectModificationEditor::setCurrentId(int Id)
{
    processor.updateState->currentDirectId = Id;
    
    processor.updateState->idDidChange = true;
    
    update();
    
    fillSelectCB(lastId, Id);
    
    lastId = Id;
}

void DirectModificationEditor::actionButtonCallback(int action, DirectModificationEditor* vc)
{
    if (vc == nullptr)
    {
        PopupMenu::dismissAllActiveMenus();
        return;
    }
    
    BKAudioProcessor& processor = vc->processor;
    
    if (action == 1)
    {
        int Id = vc->addPreparation();
        vc->setCurrentId(Id);
        processor.saveGalleryToHistory("New Direct Modification");
    }
    else if (action == 2)
    {
        int Id = vc->duplicatePreparation();
        vc->setCurrentId(Id);
        processor.saveGalleryToHistory("Duplicate Direct Modification");
    }
    else if (action == 3)
    {
        vc->deleteCurrent();
        processor.saveGalleryToHistory("Delete Direct Modification");
    }
    else if (action == 5)
    {
        processor.clear(PreparationTypeDirectMod, processor.updateState->currentModDirectId);
        vc->update();
        vc->updateModification();
        processor.saveGalleryToHistory("Clear Direct Modification");
    }
    else if (action == 6)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentModDirectId;
        DirectModification::Ptr prep = processor.gallery->getDirectModification(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            prep->setName(name);
            vc->fillSelectCB(Id, Id);
            processor.saveGalleryToHistory("Rename Direct Modification");
        }
        
        vc->update();
    }
    else if (action == 7)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentModDirectId;
        DirectModification::Ptr prep = processor.gallery->getDirectModification(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        DBG("name: " + String(name));
        
        if (result == 1)
        {
            processor.exportPreparation(PreparationTypeDirectMod, Id, name);
        }
    }
    else if (action >= 100)
    {
        int which = action - 100;
        processor.importPreparation(PreparationTypeDirectMod, processor.updateState->currentModDirectId, which);
        vc->update();
        processor.saveGalleryToHistory("Import Direct Modification");
    }
    
}

void DirectModificationEditor::bkComboBoxDidChange (ComboBox* box)
{
    String name = box->getName();
    int Id = box->getSelectedId();
    
    if (name == "Direct")
    {
        setCurrentId(Id);
    }
    
    processor.updateState->editsMade = true;
}

void DirectModificationEditor::BKEditableComboBoxChanged(String name, BKEditableComboBox* cb)
{
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
    
    mod->setName(name);
    
    updateModification();
    
    processor.updateState->editsMade = true;
}


void DirectModificationEditor::BKSingleSliderValueChanged(BKSingleSlider* slider, String name, double val)
{
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
    
    if(name == "resonance gain")
    {
        mod->setResonanceGain(val);
        mod->setDirty(DirectResGain);
        
        resonanceGainSlider->setBright();
    }
    else if(name == "hammer gain")
    {
        mod->setHammerGain(val);
        mod->setDirty(DirectHammerGain);
        
        hammerGainSlider->setBright();
    }
    else if(name == "gain")
    {
        mod->setGain(val);
        mod->setDirty(DirectGain);
        
        gainSlider->setBright();
    }
    
    updateModification();
}

void DirectModificationEditor::BKRangeSliderValueChanged(String name, double minval, double maxval)
{
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
    
    if (name == "velocity min/max")
    {
        mod->setVelocityMin(minval);
        mod->setDirty(DirectVelocityMin);
        
        mod->setVelocityMax(maxval);
        mod->setDirty(DirectVelocityMax);
        
        velocityMinMaxSlider->setBright();
    }
    
    updateModification();
}


void DirectModificationEditor::BKStackedSliderValueChanged(String name, Array<float> val)
{
    
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
    
    mod->setTransposition(val);
    mod->setDirty(DirectTransposition);
    
    transpositionSlider->setBright();
    
    updateModification();
}

void DirectModificationEditor::BKADSRSliderValueChanged(String name, int attack, int decay, float sustain, int release)
{
    DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
    
    Array<float> newvals = {(float)attack, (float)decay, sustain, (float)release};
    
    mod->setADSRvals(newvals);
    mod->setDirty(DirectADSR);
    
    ADSRSlider->setBright();
    
    updateModification();
}

void DirectModificationEditor::BKADSRButtonStateChanged(String name, bool mod, bool state)
{
    setShowADSR(!state);
    setSubWindowInFront(!state);
}

void DirectModificationEditor::updateModification(void)
{
    processor.updateState->modificationDidChange = true;
    
    processor.updateState->editsMade = true;
}

void DirectModificationEditor::buttonClicked (Button* b)
{
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(DisplayNil);
        
    }
    else if (b == &actionButton)
    {
        bool single = processor.gallery->getDirectModifications().size() == 2;
        getModOptionMenu(PreparationTypeDirectMod, single).showMenuAsync (PopupMenu::Options().withTargetComponent (&actionButton), ModalCallbackFunction::forComponent (actionButtonCallback, this) );
    }
    else if (b == &rightArrow)
    {
        arrowPressed(RightArrow);
        
        DBG("currentTab: " + String(currentTab));
        
        displayTab(currentTab);
    }
    else if (b == &leftArrow)
    {
        arrowPressed(LeftArrow);
        
        DBG("currentTab: " + String(currentTab));
        
        displayTab(currentTab);
    }
    else if (b == &transpUsesTuning)
    {
        DirectModification::Ptr mod = processor.gallery->getDirectModification(processor.updateState->currentModDirectId);
        mod->setTranspUsesTuning(transpUsesTuning.getToggleState());
        mod->setDirty(DirectTranspUsesTuning);
        transpUsesTuning.setAlpha(1.);
    }
}

void DirectModificationEditor::timerCallback()
{
    
}
