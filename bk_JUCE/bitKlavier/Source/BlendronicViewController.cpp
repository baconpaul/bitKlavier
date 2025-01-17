/*
  ==============================================================================

    BlendronicViewController.cpp
    Created: 6 Sep 2019 3:15:24pm
    Author:  Matthew Wang

  ==============================================================================
*/

#include "BlendronicViewController.h"

BlendronicViewController::BlendronicViewController(BKAudioProcessor& p, BKItemGraph* theGraph):
BKViewController(p, theGraph, 3)
{
    
    setLookAndFeel(&buttonsAndMenusLAF);
    comboBoxRightJustifyLAF.setComboBoxJustificationType(juce::Justification::centredRight);
    
    iconImageComponent.setImage(ImageCache::getFromMemory(BinaryData::blendronic_icon_png, BinaryData::blendronic_icon_pngSize));
    iconImageComponent.setImagePlacement(RectanglePlacement(juce::RectanglePlacement::stretchToFit));
    iconImageComponent.setAlpha(0.095);
    //addAndMakeVisible(iconImageComponent);
    
    gainSlider = std::make_unique<BKSingleSlider>("volume (dB)", cBlendronicOutGain, -100, 24, 0, 0.01, "-inf");
    gainSlider->setSkewFactorFromMidPoint(1.);
    gainSlider->setJustifyRight(true);
    gainSlider->setToolTipString("Adjusts overall volume of blendronic");
    addAndMakeVisible(*gainSlider);
    
    // MultSliders
    paramSliders = OwnedArray<BKMultiSlider>();
    
    int idx = 0; int o = 0;
    for (int i = 0; i < cBlendronicParameterTypes.size(); i++)
    {
        if (cBlendronicDataTypes[i] == BKFloatArr || cBlendronicDataTypes[i] == BKArrFloatArr)
        {
            // skip over the alternate smoothing param, not using it for now
            if (cBlendronicParameterTypes[idx+BlendronicBeats] == "smoothing (?)") { o++; i++; }
            
            paramSliders.insert(idx, new BKMultiSlider());
            addAndMakeVisible(paramSliders[idx], ALL);
            paramSliders[idx]->setName(cBlendronicParameterTypes[o+idx+BlendronicBeats]);
            paramSliders[idx]->addMyListener(this);
#if JUCE_IOS
            paramSliders[idx]->addWantsBigOneListener(this);
#endif
            paramSliders[idx]->setMinMaxDefaultInc(cBlendronicDefaultRangeValuesAndInc[i]);
        
            if (paramSliders[idx]->getName() == "beat lengths") // shouldn't all these be referencing cBlendronicParameterTypes??
            {
                paramSliders[idx]->setToolTipString("Determines the beat pattern of changing sequenced parameters, as a multiplier of the pulse length set by Tempo; double-click to edit all or add additional sequence steps");
            }
            else if (paramSliders[idx]->getName() == "delay lengths")
            {
                paramSliders[idx]->setToolTipString("Determines the length of delay, as a multiplier of the pulse length set by Tempo; double-click to edit all or add additional sequence steps");
            }
            else if (paramSliders[idx]->getName() == "smoothing (ms)")
            {
                paramSliders[idx]->setToolTipString("Determines duration (ms) of smoothing between delay times; double-click to edit all or add additional sequence steps.");
                //paramSliders[idx]->setToolTipString("Determines duration of smoothing between delay times; double-click to edit all or add additional sequence steps\nUnits: Constant and Proportional multiply the beat length by the given value to set the smooth time; Constant uses just the first beat, Proportional changes the smooth time with every beat, based on the length of that beat; Constant and Proportional Rate do the same, except they divide the beat length by the given value to set the smooth time.");
            }
            else if (paramSliders[idx]->getName() == "feedback coefficients")
            {
                paramSliders[idx]->setToolTipString("Determines delay feedback; double-click to edit all or add additional sequence steps");
            }
            
            idx++;
        }
        
    }
    
    selectCB.setName("Blendronic");
    selectCB.setTooltip("Select from available saved preparation settings");
    selectCB.addSeparator();
    selectCB.addListener(this);
    selectCB.setSelectedItemIndex(0);
    addAndMakeVisible(selectCB);
    
    // Target Control CBs
    targetControlCBs = OwnedArray<BKComboBox>();
    for (int i=TargetTypeBlendronicPatternSync; i<=TargetTypeBlendronicOpenCloseOutput; i++)
    {
        targetControlCBs.add(new BKComboBox()); // insert at the end of the array
        targetControlCBs.getLast()->setName(cKeymapTargetTypes[i]);
        targetControlCBs.getLast()->addListener(this);
        targetControlCBs.getLast()->setLookAndFeel(&comboBoxRightJustifyLAF);
        targetControlCBs.getLast()->addItem(cTargetNoteModes[TargetNoteMode::NoteOn], TargetNoteMode::NoteOn + 1);
        targetControlCBs.getLast()->addItem(cTargetNoteModes[TargetNoteMode::NoteOff], TargetNoteMode::NoteOff + 1);
        targetControlCBs.getLast()->addItem(cTargetNoteModes[TargetNoteMode::Both], TargetNoteMode::Both + 1);
        targetControlCBs.getLast()->setSelectedItemIndex(0, dontSendNotification);
        addAndMakeVisible(targetControlCBs.getLast(), ALL);
        
        targetControlCBLabels.add(new BKLabel());
        targetControlCBLabels.getLast()->setText(cKeymapTargetTypes[i], dontSendNotification);
        addAndMakeVisible(targetControlCBLabels.getLast(), ALL);
    }
    
    // border for Target Triggers
    targetControlsGroup.setName("targetGroup");
    targetControlsGroup.setText("Blendronic Target Triggers");
    targetControlsGroup.setTextLabelPosition(Justification::centred);
    targetControlsGroup.setAlpha(0.65);
    addAndMakeVisible(targetControlsGroup);
    
    addAndMakeVisible(&actionButton, ALL);
    actionButton.setButtonText("Action");
    actionButton.setTooltip("Create, duplicate, rename, delete, or reset current settings");
    actionButton.addListener(this);
    
    alternateMod.setButtonText ("alternate mod");
    alternateMod.setTooltip("activating this mod will alternate between modding and reseting attached preparations");
    alternateMod.setToggleState (false, dontSendNotification);
    addChildComponent(&alternateMod, ALL);
    alternateMod.setLookAndFeel(&buttonsAndMenusLAF2);
    
    bufferSizeSlider = std::make_unique<BKSingleSlider>("buffer length (sec)", "buffer length (sec)", 1., 10., 4., 0.01);
    bufferSizeSlider->setChangeNotificationOnlyOnRelease(true);
    bufferSizeSlider->setJustifyRight(true);
    bufferSizeSlider->setToolTipString("Sets the size of the delay line in seconds");
    addAndMakeVisible(*bufferSizeSlider);
    
    delayLineDisplay.setColours(Colours::black, Colours::lightgrey);
    addAndMakeVisible(&delayLineDisplay);
    
#if JUCE_IOS
    gainSlider->addWantsBigOneListener(this);
    bufferSizeSlider->addWantsBigOneListener(this);
#endif

//    keyThreshSlider = std::make_unique< BKSingleSlider>("cluster threshold", 20, 2000, 200, 10);
//    keyThreshSlider->setToolTipString("successive notes spaced by less than this time (ms) are grouped for determining first note-on and first note-off");
//    keyThreshSlider->setJustifyRight(true);
//    addAndMakeVisible(*keyThreshSlider, ALL);
    
    currentTab = 0;
    displayTab(currentTab);
    
}

void BlendronicViewController::invisible(void)
{
    gainSlider->setVisible(false);
    
    for (int i = 0; i < paramSliders.size(); i++)
    {
        paramSliders[i]->setVisible(false);
    }
    
    for (int i=0; i<targetControlCBs.size(); i++)
    {
        targetControlCBs[i]->setVisible(false);
        targetControlCBLabels[i]->setVisible(false);
    }
    targetControlsGroup.setVisible(false);
    
    bufferSizeSlider->setVisible(false);
    
    delayLineDisplay.setVisible(false);
//    keyThreshSlider->setVisible(false);
}

void BlendronicViewController::displayShared(void)
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
    
    alternateMod.setBounds(actionButton.getRight()+gXSpacing,
                           actionButton.getY(),
                           selectCB.getWidth(),
                           actionButton.getHeight());
    
    actionButton.toFront(false);
    
    leftArrow.setBounds (0, getHeight() * 0.4, 50, 50);
    rightArrow.setBounds (getRight() - 50, getHeight() * 0.4, 50, 50);
    
}

void BlendronicViewController::displayTab(int tab)
{
    currentTab = tab;
    
    invisible();
    displayShared();
    
    int y0 = hideOrShow.getBottom() + gYSpacing;
    int height = getHeight() - y0;
    
    if (tab == 0)
    {
        gainSlider->setVisible(true);
        
        for (int i = 0; i < paramSliders.size(); i++)
        {
            paramSliders[i]->setVisible(true);
        }
        
        // SET BOUNDS
        int sliderHeight = height * 0.2f;
        
        Rectangle<int> area (getBounds());
        area.removeFromTop(hideOrShow.getBottom() + gYSpacing);
        area.removeFromRight(rightArrow.getWidth() + gXSpacing);
        area.removeFromLeft(leftArrow.getWidth() + gXSpacing);
        
        Rectangle<int> leftColumn (area);
        Rectangle<int> rightColumn (leftColumn.removeFromRight(leftColumn.getWidth()* 0.5));
        
        leftColumn.removeFromRight(processor.paddingScalarX * 20);
        rightColumn.removeFromLeft(processor.paddingScalarX * 20);

        //area.removeFromTop(gComponentComboBoxHeight*2);
        area.removeFromTop(gComponentRangeSliderHeight + 2 * gYSpacing);
        rightColumn.removeFromTop(gYSpacing);
        gainSlider->setBounds(rightColumn.removeFromTop(gComponentRangeSliderHeight));
        
        for (int i = 0; i < paramSliders.size(); i++)
        {
            paramSliders[i]->setBounds(area.removeFromTop(sliderHeight));
            area.removeFromTop(gYSpacing);
        }
    }
    else if (tab == 1)
    {
        bufferSizeSlider->setVisible(true);
        delayLineDisplay.setVisible(true);
        
        // SET BOUNDS
        int sliderHeight = height * 0.2f;
        
        Rectangle<int> area (getBounds());
        area.removeFromTop(hideOrShow.getBottom() + gYSpacing);
        area.removeFromRight(rightArrow.getWidth() + gXSpacing);
        area.removeFromLeft(leftArrow.getWidth() + gXSpacing);
        
        Rectangle<int> leftColumn (area);
        Rectangle<int> rightColumn (leftColumn.removeFromRight(leftColumn.getWidth()* 0.5));
        leftColumn.removeFromRight(processor.paddingScalarX * 20);
        rightColumn.removeFromLeft(processor.paddingScalarX * 20);
        
        area.removeFromTop(gComponentRangeSliderHeight + 2 * gYSpacing);
        rightColumn.removeFromTop(gYSpacing);
        bufferSizeSlider->setBounds(rightColumn.removeFromTop(gComponentRangeSliderHeight));
        
        Rectangle<int> delayLineDisplayRect (area.removeFromTop((sliderHeight + gYSpacing)*4));
        delayLineDisplay.setBounds(delayLineDisplayRect);
    }
    else if (tab == 2) // keymap target tab
    {
        // make the combo boxes visible
        for (int i=0; i<targetControlCBs.size(); i++)
        {
            targetControlCBs[i]->setVisible(true);
            targetControlCBLabels[i]->setVisible(true);
        }
        targetControlsGroup.setVisible(true);
        
        Rectangle<int> area (getBounds());
        area.removeFromTop(selectCB.getHeight() + 50 * processor.paddingScalarY + 4 + gYSpacing);
        area.removeFromRight(rightArrow.getWidth());
        area.removeFromLeft(leftArrow.getWidth());
        
        area.removeFromTop((area.getHeight() - (targetControlCBs.size() / 2) * (gComponentComboBoxHeight + gYSpacing)) / 3.);
        
        // make four columns
        // textlabel | combobox | combobox | textlabel
    
        int wideColumnWidth = area.getWidth() * 0.3;
        int narrowColumnWidth = area.getWidth() * 0.2;
        
        Rectangle<int> column1 (area.removeFromLeft(wideColumnWidth));
        column1.removeFromRight(processor.paddingScalarX * 5);
        column1.removeFromLeft(processor.paddingScalarX * 20);
        
        Rectangle<int> column2 (area.removeFromLeft(narrowColumnWidth));
        column2.removeFromRight(processor.paddingScalarX * 5);
        column2.removeFromLeft(processor.paddingScalarX * 20);
        
        Rectangle<int> column4 (area.removeFromRight(wideColumnWidth));
        column4.removeFromRight(processor.paddingScalarX * 20);
        column4.removeFromLeft(processor.paddingScalarX * 5);
        
        Rectangle<int> column3 (area.removeFromRight(narrowColumnWidth));
        column3.removeFromRight(processor.paddingScalarX * 20);
        column3.removeFromLeft(processor.paddingScalarX * 5);
        
        
        for (int i=0; i<targetControlCBs.size() / 2; i++)
        {
            targetControlCBs[i]->setLookAndFeel(&buttonsAndMenusLAF);
            
            targetControlCBs[i]->setBounds(column2.removeFromTop(gComponentComboBoxHeight));
            column2.removeFromTop(gYSpacing);
            
            targetControlCBLabels[i]->setBounds(column1.removeFromTop(gComponentComboBoxHeight));
            column1.removeFromTop(gYSpacing);
            
            targetControlCBLabels[i]->setJustificationType(juce::Justification::centredRight);
        }
        
        for (int i=targetControlCBs.size() / 2; i<targetControlCBs.size(); i++)
        {
            targetControlCBs[i]->setLookAndFeel(&comboBoxRightJustifyLAF);
            
            targetControlCBs[i]->setBounds(column3.removeFromTop(gComponentComboBoxHeight));
            column3.removeFromTop(gYSpacing);
            
            targetControlCBLabels[i]->setBounds(column4.removeFromTop(gComponentComboBoxHeight));
            column4.removeFromTop(gYSpacing);
        }
        
        targetControlsGroup.setBounds(targetControlCBLabels[0]->getX() - 4 * gXSpacing,
                                      targetControlCBLabels[0]->getY() - gComponentComboBoxHeight - 2 * gXSpacing,
                                      targetControlCBLabels[targetControlCBs.size() - 1]->getRight() - targetControlCBLabels[0]->getX() + 8 * gXSpacing,
                                      //column1.getWidth() + column2.getWidth() + column3.getWidth() + column4.getWidth() + 8 * gXSpacing,
                                      targetControlCBs[0]->getHeight() * (targetControlCBs.size() / 2) + 2 * gComponentComboBoxHeight + 4 * gYSpacing);
    }
}

void BlendronicViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void BlendronicViewController::resized()
{
    displayShared();
    displayTab(currentTab);
}

#if JUCE_IOS
void BlendronicViewController::iWantTheBigOne(TextEditor* tf, String name)
{
    hideOrShow.setAlwaysOnTop(false);
    rightArrow.setAlwaysOnTop(false);
    leftArrow.setAlwaysOnTop(false);
    bigOne.display(tf, name, getBounds());
}
#endif

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ BlendronicPreparationEditor ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

BlendronicPreparationEditor::BlendronicPreparationEditor(BKAudioProcessor& p, BKItemGraph* theGraph):
BlendronicViewController(p, theGraph)
{
    fillSelectCB(-1,-1);

    selectCB.addListener(this);
    selectCB.addMyListener(this);
    
    gainSlider->addMyListener(this);
    bufferSizeSlider->addMyListener(this);
    
    startTimer(10);
}

void BlendronicPreparationEditor::BKEditableComboBoxChanged(String name, BKEditableComboBox* cb)
{
    Blendronic::Ptr blendronic = processor.gallery->getBlendronic(processor.updateState->currentBlendronicId);
    
    blendronic->setName(name);
    
    processor.updateState->editsMade = true;
}

void BlendronicPreparationEditor::update(void)
{
    if (processor.updateState->currentBlendronicId < 0) return;

    setSubWindowInFront(false);
    
    BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
    
    if (prep != nullptr)
    {
        selectCB.setSelectedId(processor.updateState->currentBlendronicId, dontSendNotification);
        
        gainSlider->setValue(prep->outGain.value, dontSendNotification);
        bufferSizeSlider->setValue(prep->delayBufferSizeInSeconds.value, dontSendNotification);
        
        for (int i = TargetTypeBlendronicPatternSync; i <= TargetTypeBlendronicOpenCloseOutput; i++)
        {
            targetControlCBs[i - TargetTypeBlendronicPatternSync]->setSelectedItemIndex
            (prep->getTargetTypeBlendronic(KeymapTargetType(i)), dontSendNotification);
        }

        for(int i = 0; i < paramSliders.size(); i++)
        {
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicBeats]))
            {
                paramSliders[i]->setToOnlyActive(prep->bBeats.value, prep->bBeatsStates.value, dontSendNotification);
            }
            
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicDelayLengths]))
            {
                // paramSliders[i]->setTo(prep->bDelayLengths.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(prep->bDelayLengths.value, prep->bDelayLengthsStates.value, dontSendNotification);
            }

            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicSmoothLengths]))
            {
                // paramSliders[i]->setTo(prep->bSmoothLengths.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(prep->bSmoothLengths.value, prep->bSmoothLengthsStates.value, dontSendNotification);
            }

            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicFeedbackCoeffs]))
            {
                // paramSliders[i]->setTo(prep->bFeedbackCoefficients.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(prep->bFeedbackCoefficients.value, prep->bFeedbackCoefficientsStates.value, dontSendNotification);
            }
        }
    }
}

void BlendronicPreparationEditor::bkMessageReceived (const String& message)
{
    if (message == "blendronic/update")
    {
        update();
    }
}

int BlendronicPreparationEditor::addPreparation(void)
{
    processor.gallery->add(PreparationTypeBlendronic);
    
    return processor.gallery->getAllBlendronic().getLast()->getId();
}

int BlendronicPreparationEditor::duplicatePreparation(void)
{
    processor.gallery->duplicate(PreparationTypeBlendronic, processor.updateState->currentBlendronicId);
    
    return processor.gallery->getAllBlendronic().getLast()->getId();
}

void BlendronicPreparationEditor::deleteCurrent(void)
{
    int BlendronicId = selectCB.getSelectedId();
    int index = selectCB.getSelectedItemIndex();
    
    if ((index == 0) && (selectCB.getItemId(index+1) == -1)) return;
    
    processor.gallery->remove(PreparationTypeBlendronic, BlendronicId);
    
    fillSelectCB(0, 0);
    
    selectCB.setSelectedItemIndex(0, dontSendNotification);
    int newId = selectCB.getSelectedId();
    
    setCurrentId(newId);
}

void BlendronicPreparationEditor::setCurrentId(int Id)
{
    processor.updateState->currentBlendronicId = Id;
    
    processor.updateState->idDidChange = true;
    
    update();
    
    fillSelectCB(lastId, Id);
    
    lastId = Id;
}

void BlendronicPreparationEditor::actionButtonCallback(int action, BlendronicPreparationEditor* vc)
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
        processor.saveGalleryToHistory("New Blendronic Preparation");
    }
    else if (action == 2)
    {
        int Id = vc->duplicatePreparation();
        vc->setCurrentId(Id);
        processor.saveGalleryToHistory("Duplicate Blendronic Preparation");
    }
    else if (action == 3)
    {
        vc->deleteCurrent();
        processor.saveGalleryToHistory("Delete Blendronic Preparation");
    }
    else if (action == 4)
    {
        processor.reset(PreparationTypeBlendronic, processor.updateState->currentBlendronicId);
        vc->update();
    }
    else if (action == 5)
    {
        processor.clear(PreparationTypeBlendronic, processor.updateState->currentBlendronicId);
        vc->update();
        processor.saveGalleryToHistory("Clear Blendronic Preparation");
    }
    else if (action == 6)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentBlendronicId;
        Blendronic::Ptr prep = processor.gallery->getBlendronic(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            prep->setName(name);
            vc->fillSelectCB(Id, Id);
            processor.saveGalleryToHistory("Rename Blendronic Preparation");
        }
        
        vc->update();
    }
    else if (action == 7)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentBlendronicId;
        Blendronic::Ptr prep = processor.gallery->getBlendronic(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        DBG("name: " + String(name));
        
        if (result == 1)
        {
            processor.exportPreparation(PreparationTypeBlendronic, Id, name);
        }
    }
    else if (action >= 100)
    {
        int which = action - 100;
        processor.importPreparation(PreparationTypeBlendronic, processor.updateState->currentBlendronicId, which);
        vc->update();
        
        processor.saveGalleryToHistory("Import Blendronic Preparation");
    }
}

void BlendronicPreparationEditor::bkComboBoxDidChange (ComboBox* box)
{
    String name = box->getName();
    int Id = box->getSelectedId();
//    int index = box->getSelectedItemIndex();
    
    if (name == "Blendronic")
    {
        setCurrentId(Id);
    }
    else // target combo boxes from tab 3
    {
        BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
        
        for (int i=0; i<targetControlCBs.size(); i++)
        {
            if (box == targetControlCBs[i])
            {
                int selectedItem = targetControlCBs[i]->getSelectedId() - 1;
                DBG(targetControlCBs[i]->getName() + " " + cTargetNoteModes[selectedItem]);
                
                prep    ->setTargetTypeBlendronic(KeymapTargetType(i + TargetTypeBlendronicPatternSync), (TargetNoteMode)selectedItem);
            }
        }
        processor.updateState->editsMade = true;
    }
}

void BlendronicPreparationEditor::BKSingleSliderValueChanged(BKSingleSlider* slider, String name, double val)
{
    BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
    BlendronicProcessor::Ptr proc = processor.currentPiano->getBlendronicProcessor(processor.updateState->currentBlendronicId);
    
    if (slider->getName() == gainSlider->getName())
    {
        prep->outGain.set(val);
    }
    else if (slider->getName() == bufferSizeSlider->getName())
    {
        proc->setDelayBufferSizeInSeconds(val);
    }
    
    processor.updateState->editsMade = true;
}

void BlendronicPreparationEditor::fillSelectCB(int last, int current)
{
    selectCB.clear(dontSendNotification);
    
    for (auto prep : processor.gallery->getAllBlendronic())
    {
        int Id = prep->getId();
        
        if (Id == -1) continue;
        
        String name = prep->getName();
        
        if (name != String())  selectCB.addItem(name, Id);
        else                        selectCB.addItem("Blendronic"+String(Id), Id);
        
        selectCB.setItemEnabled(Id, true);
        if (processor.currentPiano->isActive(PreparationTypeBlendronic, Id))
            selectCB.setItemEnabled(Id, false);
    }
    
    if (last != 0)      selectCB.setItemEnabled(last, true);
    if (current != 0)   selectCB.setItemEnabled(current, false);
    
    int selectedId = processor.updateState->currentBlendronicId;
    
    selectCB.setSelectedId(selectedId, NotificationType::dontSendNotification);
    
    selectCB.setItemEnabled(selectedId, false);
    
    lastId = selectedId;
    
}

void BlendronicPreparationEditor::timerCallback()
{
    if (processor.updateState->currentDisplay == DisplayBlendronic)
    {
        BlendronicProcessor::Ptr proc = processor.currentPiano->getBlendronicProcessor(processor.updateState->currentBlendronicId);
        BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
        
        if (prep != nullptr && proc != nullptr)
        {
            delayLineDisplay.setAudio(proc->getAudioDisplayData());
            delayLineDisplay.setSmoothing(proc->getSmoothingDisplayData());
            delayLineDisplay.setLineSpacing(proc->getPulseLengthInSamples());
            float maxDelayLength = 0.0f;
            for (auto d : prep->bDelayLengths.value)
            {
                if (d > maxDelayLength) maxDelayLength = d;
            }

            delayLineDisplay.setMaxDelayLength(maxDelayLength);
            delayLineDisplay.setPulseOffset(proc->getPulseOffset());
            delayLineDisplay.setMarkers(proc->getBeatPositionsInBuffer());
            delayLineDisplay.setPlayheads(Array<uint64>({proc->getInPoint(), proc->getOutPoint()}));
            if (proc->getResetPhase())
            {
                delayLineDisplay.resetPhase();
                proc->setResetPhase(false);
            }
            
            // dim target comboboxes that aren't activated by a Keymap
            for (int i = TargetTypeBlendronicPatternSync; i <= TargetTypeBlendronicOpenCloseOutput; i++)
            {
                bool makeBright = false;
                for (auto km : proc->getKeymaps())
                    if (km->getTargetStates()[(KeymapTargetType) i]) makeBright = true;
                
                if (makeBright)
                {
                    targetControlCBs[i - TargetTypeBlendronicPatternSync]->setAlpha(1.);
                    targetControlCBLabels[i - TargetTypeBlendronicPatternSync]->setAlpha(1.);
                    
                    targetControlCBs[i - TargetTypeBlendronicPatternSync]->setEnabled(true);
                }
                else
                {
                    targetControlCBs[i - TargetTypeBlendronicPatternSync]->setAlpha(0.25);
                    targetControlCBLabels[i - TargetTypeBlendronicPatternSync]->setAlpha(0.25);
                    
                    targetControlCBs[i - TargetTypeBlendronicPatternSync]->setEnabled(false);
                }
            }
        
            int counter = 0, size = 0;
            
            for (int i = 0; i < paramSliders.size(); i++)
            {
                //if (paramSliders[i]->getName() == "beat lengths")
                if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicBeats]))
                {
                    size = paramSliders[i]->getNumVisible();
                    counter = proc->getBeatIndex();
                    paramSliders[i]->setCurrentSlider((counter >= size || counter < 0) ? 0 : counter);
                }
                else if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicDelayLengths]))
                {
                    size = paramSliders[i]->getNumVisible();
                    counter = proc->getDelayIndex();
                    paramSliders[i]->setCurrentSlider((counter >= size || counter < 0) ? 0 : counter);
                }
                else if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicSmoothLengths]))
                {
                    size = paramSliders[i]->getNumVisible();
                    counter = proc->getSmoothIndex();
                    paramSliders[i]->setCurrentSlider((counter >= size || counter < 0) ? 0 : counter);
                }
                else if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicFeedbackCoeffs]))
                {
                    size = paramSliders[i]->getNumVisible();
                    counter = proc->getFeedbackIndex();
                    paramSliders[i]->setCurrentSlider((counter >= size || counter < 0) ? 0 : counter);
                }
            }
            
            if (prep->outGain.didChange()) gainSlider->setValue(prep->outGain.value, dontSendNotification);
        }
    }
}

void BlendronicPreparationEditor::multiSliderDidChange(String name, int whichSlider, Array<float> values)
{
    BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
    
    DBG("BlendronicPreparationEditor::multiSliderDidChange " + name + " " + String(whichSlider) + " " + String(values[0]));
//
//    if (name == "beat lengths")
//    {
//        prep    ->setBeat(whichSlider, values[0]);
//    }
//    else if (name == "delay lengths")
//    {
//        prep    ->setDelayLength(whichSlider, values[0]);
//    }
//    else if (name == "smoothing (ms)")
//    {
//        //prep    ->setSmoothValue(whichSlider, values[0]);
//        //active  ->setSmoothValue(whichSlider, values[0]);
//        prep    ->bSmoothLengths.set(whichSlider, values[0]);
//    }
//    else if (name == "feedback coefficients")
//    {
//        prep    ->setFeedbackCoefficient(whichSlider, values[0]);
//    }
    
    processor.updateState->editsMade = true;
}

void BlendronicPreparationEditor::multiSlidersDidChange(String name, Array<Array<float>> values, Array<bool> states)
{
    BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
    
    Array<float> newvals = Array<float>();
    for(int i=0; i<values.size(); i++) newvals.add(values[i][0]);
    
    DBG("BlendronicPreparationEditor::multiSlidersDidChange " + name);
    
    if (name == "beat lengths")
    {
        prep    ->bBeats.set(newvals);
        prep    ->bBeatsStates.set(states);
    }
    else if (name == "delay lengths")
    {
        prep    ->bDelayLengths.set(newvals);
        prep    ->bDelayLengthsStates.set(states);
    }
    else if (name == "smoothing (ms)")
    {
        prep    ->bSmoothLengths.set(newvals);
        prep    ->bSmoothLengthsStates.set(states);
    }
    else if (name == "feedback coefficients")
    {
        prep    ->bFeedbackCoefficients.set(newvals);
        prep    ->bFeedbackCoefficientsStates.set(states);
    }
    
    processor.updateState->editsMade = true;
}

/*
void BlendronicPreparationEditor::multiSlidersDidChange(String name, Array<Array<float>> values)
{
    BlendronicPreparation::Ptr prep = processor.gallery->getStaticBlendronicPreparation(processor.updateState->currentBlendronicId);
    BlendronicPreparation::Ptr active = processor.gallery->getActiveBlendronicPreparation(processor.updateState->currentBlendronicId);
    
    Array<float> newvals = Array<float>();
    for(int i=0; i<values.size(); i++) newvals.add(values[i][0]);
    
    DBG("BlendronicPreparationEditor::multiSlidersDidChange " + name);
    
    if (name == "beat lengths")
    {
        prep    ->bBeats.set(newvals);
        active  ->bBeats.set(newvals);
    }
    else if (name == "delay lengths")
    {
        prep    ->bDelayLengths.set(newvals);
        active  ->bDelayLengths.set(newvals);
    }
    else if (name == "smoothing (ms)")
    {
        prep    ->bSmoothLengths.set(newvals);
        active  ->bSmoothLengths.set(newvals);
    }
    else if (name == "feedback coefficients")
    {
        prep    ->bFeedbackCoefficients.set(newvals);
        active  ->bFeedbackCoefficients.set(newvals);
    }
}
 */

void BlendronicPreparationEditor::buttonClicked (Button* b)
{
    BlendronicPreparation::Ptr prep = processor.gallery->getBlendronicPreparation(processor.updateState->currentBlendronicId);
    
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(DisplayNil);
        setSubWindowInFront(false);
    }
    else if (b == &actionButton)
    {
        bool single = processor.gallery->getAllBlendronic().size() == 2;
        getPrepOptionMenu(PreparationTypeBlendronic, single).showMenuAsync (PopupMenu::Options().withTargetComponent (&actionButton), ModalCallbackFunction::forComponent (actionButtonCallback, this) );
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
}

void BlendronicPreparationEditor::BKRangeSliderValueChanged(String name, double minval, double maxval)
{
    
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ BlendronicModificationEditor ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

BlendronicModificationEditor::BlendronicModificationEditor(BKAudioProcessor& p, BKItemGraph* theGraph):
BlendronicViewController(p, theGraph)
{
    numTabs = 1;
    
    greyOutAllComponents();
    
    fillSelectCB(-1,-1);
    
    rightArrow.setVisible(false);
    leftArrow.setVisible(false);
    
    selectCB.addListener(this);
    selectCB.addMyListener(this);
    
    gainSlider->addMyListener(this);
    gainSlider->addModdableComponentListener(this);
    
    alternateMod.addListener(this);
    alternateMod.setVisible(true);
}

void BlendronicModificationEditor::greyOutAllComponents()
{
    gainSlider->setDim(gModAlpha);
    
    for(int i = 0; i < paramSliders.size(); i++)
    {
        paramSliders[i]->setAlpha(gModAlpha);
    }
}

void BlendronicModificationEditor::highlightModedComponents()
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
    
    if(mod->getDirty(BlendronicOutGain)) gainSlider->setBright();
    
    if (mod->getDirty(BlendronicBeats))
    {
        for(int i = 0; i < paramSliders.size(); i++)
        {
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicBeats]))
                paramSliders[i]->setAlpha(1.);
        }
    }
    
    if (mod->getDirty(BlendronicDelayLengths))
    {
        for(int i = 0; i < paramSliders.size(); i++)
        {
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicDelayLengths]))
                paramSliders[i]->setAlpha(1.);
        }
    }
    
    if (mod->getDirty(BlendronicSmoothLengths))
    {
        for(int i = 0; i < paramSliders.size(); i++)
        {
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicSmoothLengths]))
                paramSliders[i]->setAlpha(1.);
        }
    }
    
//    if (mod->getDirty(BlendronicSmoothValues))
//    {
//        for(int i = 0; i < paramSliders.size(); i++)
//        {
//            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicSmoothValues]))
//                paramSliders[i]->setAlpha(1.);
//        }
//    }
    
    if (mod->getDirty(BlendronicFeedbackCoeffs))
    {
        for(int i = 0; i < paramSliders.size(); i++)
        {
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicFeedbackCoeffs]))
                paramSliders[i]->setAlpha(1.);
        }
    }
}

void BlendronicModificationEditor::update(void)
{
    if (processor.updateState->currentModBlendronicId < 0) return;
    
    greyOutAllComponents();
    highlightModedComponents();
    
    selectCB.setSelectedId(processor.updateState->currentModBlendronicId, dontSendNotification);
    
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
    
    if (mod != nullptr)
    {
        gainSlider->setValue(mod->outGain.value, dontSendNotification);
        
        alternateMod.setToggleState(mod->altMod, dontSendNotification);
        
        for (int i = TargetTypeBlendronicPatternSync; i <= TargetTypeBlendronicOpenCloseOutput; i++)
        {
            targetControlCBs[i - TargetTypeBlendronicPatternSync]->setSelectedItemIndex
            (mod->getTargetTypeBlendronic(KeymapTargetType(i)), dontSendNotification);
        }
        
        for(int i = 0; i < paramSliders.size(); i++)
        {
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicBeats]))
            {
                // paramSliders[i]->setTo(mod->bBeats.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(mod->bBeats.value, mod->bBeatsStates.value, dontSendNotification);
            }
            
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicDelayLengths]))
            {
                // paramSliders[i]->setTo(mod->bDelayLengths.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(mod->bDelayLengths.value, mod->bDelayLengthsStates.value, dontSendNotification);
            }
            
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicSmoothLengths]))
            {
                // paramSliders[i]->setTo(mod->bSmoothLengths.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(mod->bSmoothLengths.value, mod->bSmoothLengthsStates.value, dontSendNotification);
            }
            
            if(!paramSliders[i]->getName().compare(cBlendronicParameterTypes[BlendronicFeedbackCoeffs]))
            {
                // paramSliders[i]->setTo(mod->bFeedbackCoefficients.value, dontSendNotification);
                paramSliders[i]->setToOnlyActive(mod->bFeedbackCoefficients.value, mod->bFeedbackCoefficientsStates.value, dontSendNotification);
            }
        }
    }
    
}

void BlendronicModificationEditor::fillSelectCB(int last, int current)
{
    selectCB.clear(dontSendNotification);
    
    for (auto prep : processor.gallery->getBlendronicModifications())
    {
        int Id = prep->getId();;
        String name = prep->getName();
        
        if (name != String())  selectCB.addItem(name, Id);
        else                        selectCB.addItem("BlendronicMod"+String(Id), Id);
        
        selectCB.setItemEnabled(Id, true);
        if (processor.currentPiano->isActive(PreparationTypeBlendronic, Id))
            selectCB.setItemEnabled(Id, false);
    }
    
    if (last != 0)      selectCB.setItemEnabled(last, true);
    if (current != 0)   selectCB.setItemEnabled(current, false);
    
    int selectedId = processor.updateState->currentBlendronicId;
    
    selectCB.setSelectedId(selectedId, NotificationType::dontSendNotification);
    
    selectCB.setItemEnabled(selectedId, false);
    
    lastId = selectedId;
}

void BlendronicModificationEditor::timerCallback()
{

}

void BlendronicModificationEditor::multiSliderDidChange(String name, int whichSlider, Array<float> values)
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
    
    if (!name.compare(cBlendronicParameterTypes[BlendronicBeats]))
    {
        Array<float> beats = mod->bBeats.value;
        beats.set(whichSlider, values[0]);
        
        mod->setDirty(BlendronicBeats);
        mod->bBeats.set(beats);
    }
    else if (!name.compare(cBlendronicParameterTypes[BlendronicDelayLengths]))
    {
        Array<float> delayLengths = mod->bDelayLengths.value;
        delayLengths.set(whichSlider, values[0]);
        
        mod->setDirty(BlendronicDelayLengths);
        mod->bDelayLengths.set(delayLengths);
    }
    else if (!name.compare(cBlendronicParameterTypes[BlendronicSmoothLengths]))
    {
        Array<float> smoothLengths = mod->bSmoothLengths.value;
        smoothLengths.set(whichSlider, values[0]);
        
        mod->setDirty(BlendronicSmoothLengths);
        mod->bSmoothLengths.set(smoothLengths);
    }
    else if (!name.compare(cBlendronicParameterTypes[BlendronicFeedbackCoeffs]))
    {
        Array<float> feedbackCoeffs = mod->bFeedbackCoefficients.value;
        feedbackCoeffs.set(whichSlider, values[0]);
        
        mod->setDirty(BlendronicFeedbackCoeffs);
        mod->bFeedbackCoefficients.set(feedbackCoeffs);
    }
    
    for(int i = 0; i < paramSliders.size(); i++)
    {
        if(paramSliders[i]->getName() == name) paramSliders[i]->setAlpha(1.);
    }
    
    updateModification();
    
}

void BlendronicModificationEditor::multiSlidersDidChange(String name, Array<Array<float>> values, Array<bool> states)
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
    
    //only transposition allows multiple simultaneous vals, so trim down to 1D array
    Array<float> newvals = Array<float>();
    for(int i=0; i<values.size(); i++) newvals.add(values[i][0]);
    
    if (!name.compare(cBlendronicParameterTypes[BlendronicBeats]))
    {
        mod->bBeats.set(newvals);
        mod->bBeatsStates.set(states);
        mod->setDirty(BlendronicBeats);
    }
    else if (!name.compare(cBlendronicParameterTypes[BlendronicDelayLengths]))
    {
        mod->bDelayLengths.set(newvals);
        mod->bDelayLengthsStates.set(states);
        mod->setDirty(BlendronicDelayLengths);
    }
    else if (!name.compare(cBlendronicParameterTypes[BlendronicSmoothLengths]))
    {
        mod->bSmoothLengths.set(newvals);
        mod->bSmoothLengthsStates.set(states);
        mod->setDirty(BlendronicSmoothLengths);
    }
    else if (!name.compare(cBlendronicParameterTypes[BlendronicFeedbackCoeffs]))
    {
        mod->bFeedbackCoefficients.set(newvals);
        mod->bFeedbackCoefficientsStates.set(states);
        mod->setDirty(BlendronicFeedbackCoeffs);
    }
    
    for(int i = 0; i < paramSliders.size(); i++)
    {
        if(paramSliders[i]->getName() == name) paramSliders[i]->setAlpha(1.);
    }
    
    updateModification();
}

void BlendronicModificationEditor::BKEditableComboBoxChanged(String name, BKEditableComboBox* cb)
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
    
    mod->setName(name);
    
    updateModification();
}

void BlendronicModificationEditor::bkMessageReceived (const String& message)
{
    if (message == "blendronic/update")
    {
        update();
    }
}

int BlendronicModificationEditor::addPreparation(void)
{
    processor.gallery->add(PreparationTypeBlendronicMod);
    
    processor.updateState->editsMade = true;
    
    return processor.gallery->getBlendronicModifications().getLast()->getId();
}

int BlendronicModificationEditor::duplicatePreparation(void)
{
    processor.gallery->duplicate(PreparationTypeBlendronicMod, processor.updateState->currentModBlendronicId);
    
    processor.updateState->editsMade = true;
    
    return processor.gallery->getBlendronicModifications().getLast()->getId();
}

void BlendronicModificationEditor::deleteCurrent(void)
{
    int oldId = selectCB.getSelectedId();
    int index = selectCB.getSelectedItemIndex();
    
    if ((index == 0) && (selectCB.getItemId(index+1) == -1)) return;
    
    processor.gallery->remove(PreparationTypeBlendronicMod, oldId);
    
    fillSelectCB(0, 0);
    
    selectCB.setSelectedItemIndex(0, dontSendNotification);
    int newId = selectCB.getSelectedId();

    setCurrentId(newId);
}

void BlendronicModificationEditor::setCurrentId(int Id)
{
    processor.updateState->currentModBlendronicId = Id;
    
    processor.updateState->idDidChange = true;
    
    update();
    
    fillSelectCB(lastId, Id);
    
    lastId = Id;
}

void BlendronicModificationEditor::actionButtonCallback(int action, BlendronicModificationEditor* vc)
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
        processor.saveGalleryToHistory("New Blendronic Modification");
    }
    else if (action == 2)
    {
        int Id = vc->duplicatePreparation();
        vc->setCurrentId(Id);
        processor.saveGalleryToHistory("Duplicate Blendronic Modification");
    }
    else if (action == 3)
    {
        vc->deleteCurrent();
        processor.saveGalleryToHistory("Delete Blendronic Modification");
    }
    else if (action == 5)
    {
        processor.clear(PreparationTypeBlendronicMod, processor.updateState->currentModBlendronicId);
        vc->update();
        vc->updateModification();
        processor.saveGalleryToHistory("Clear Blendronic Modification");
    }
    else if (action == 6)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentModBlendronicId;
        BlendronicModification::Ptr prep = processor.gallery->getBlendronicModification(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            prep->setName(name);
            vc->fillSelectCB(Id, Id);
            processor.saveGalleryToHistory("Rename Blendronic Modification");
        }
        
        vc->update();
    }
    else if (action == 7)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.updateState->currentModBlendronicId;
        BlendronicModification::Ptr prep = processor.gallery->getBlendronicModification(Id);
        
        prompt.addTextEditor("name", prep->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        DBG("name: " + String(name));
        
        if (result == 1)
        {
            processor.exportPreparation(PreparationTypeBlendronicMod, Id, name);
        }
    }
    else if (action >= 100)
    {
        int which = action - 100;
        processor.importPreparation(PreparationTypeBlendronicMod, processor.updateState->currentModBlendronicId, which);
        vc->update();
        processor.saveGalleryToHistory("Import Blendronic Modification");
    }
}

void BlendronicModificationEditor::bkComboBoxDidChange (ComboBox* box)
{
    String name = box->getName();
    int Id = box->getSelectedId();
//    int index = box->getSelectedItemIndex();
    
    if (name == "Blendronic")
    {
        setCurrentId(Id);
        
        processor.updateState->editsMade = true;
    }
}

void BlendronicModificationEditor::BKSingleSliderValueChanged(BKSingleSlider* slider, String name, double val)
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
   
    if (slider->getName() == gainSlider->getName())
    {
        mod->outGain.set(val);
        
        mod->setDirty(BlendronicOutGain);
        gainSlider->setBright();
        
        processor.updateState->editsMade = true;
    }
    
    updateModification();
}

void BlendronicModificationEditor::updateModification(void)
{
    processor.updateState->modificationDidChange = true;
    
    processor.updateState->editsMade = true;
}

void BlendronicModificationEditor::buttonClicked (Button* b)
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
    
    if (b == &hideOrShow)
    {
        processor.updateState->setCurrentDisplay(DisplayNil);
    }
    else if (b == &actionButton)
    {
        bool single = processor.gallery->getBlendronicModifications().size() == 2;
        getModOptionMenu(PreparationTypeBlendronicMod, single).showMenuAsync (PopupMenu::Options().withTargetComponent (&actionButton), ModalCallbackFunction::forComponent (actionButtonCallback, this) );
    }
    else if (b == &alternateMod)
    {
        BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
        mod->altMod = alternateMod.getToggleState();
    }
//    else if (b == &rightArrow)
//    {
//        arrowPressed(RightArrow);
//
//        DBG("currentTab: " + String(currentTab));
//
//        displayTab(currentTab);
//    }
//    else if (b == &leftArrow)
//    {
//        arrowPressed(LeftArrow);
//
//        DBG("currentTab: " + String(currentTab));
//
//        displayTab(currentTab);
//    }
    
    updateModification();
}

void BlendronicModificationEditor::BKRangeSliderValueChanged(String name, double minval, double maxval)
{
    BlendronicModification::Ptr mod = processor.gallery->getBlendronicModification(processor.updateState->currentModBlendronicId);
}
