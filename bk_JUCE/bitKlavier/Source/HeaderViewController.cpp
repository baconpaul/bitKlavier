/*
  ==============================================================================

    HeaderViewController.cpp
    Created: 27 Mar 2017 12:47:45pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "HeaderViewController.h"

#define SAVE_ID 1
#define SAVEAS_ID 2
#define OPEN_ID 3
#define NEW_ID 4
#define CLEAN_ID 5
#define SETTINGS_ID 6
#define OPENOLD_ID 7
#define DIRECT_ID 8
#define NOSTALGIC_ID 9
#define SYNCHRONIC_ID 10
#define TUNING_ID 11
#define TEMPO_ID 12
#define MODIFICATION_ID 13
#define PIANOMAP_ID 14
#define RESET_ID 15
#define NEWGALLERY_ID 16
#define DELETE_ID 17
#define KEYMAP_ID 18

HeaderViewController::HeaderViewController (BKAudioProcessor& p, BKConstructionSite* c):
processor (p),
construction(c)
{
    
    setLookAndFeel(&buttonsAndMenusLAF);
    
    addAndMakeVisible(loadB);
    loadB.setButtonText("Load Samples");
    loadB.addListener(this);
    
    addAndMakeVisible(galleryB);
    galleryB.setButtonText("Gallery Action");
    galleryB.addListener(this);
    
    addAndMakeVisible(pianoB);
    pianoB.setButtonText("Piano Action");
    pianoB.addListener(this);
    
    // Gallery CB
    addAndMakeVisible(galleryCB);
    galleryCB.setName("galleryCB");
    galleryCB.addListener(this);
    galleryCB.addMyListener(this);
    //galleryCB.BKSetJustificationType(juce::Justification::centredRight);
    
    galleryCB.setSelectedId(0, dontSendNotification);
    lastGalleryCBId = galleryCB.getSelectedId();
    
    // Piano CB
    addAndMakeVisible(pianoCB);
    pianoCB.setName("pianoCB");
    pianoCB.addListener(this);
    pianoCB.addMyListener(this);
    pianoCB.BKSetJustificationType(juce::Justification::centredRight);
    
    pianoCB.setSelectedId(0, dontSendNotification);
    
    galleryModalCallBackIsOpen = false;
    fillGalleryCB();
    
}

HeaderViewController::~HeaderViewController()
{
    
}

void HeaderViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void HeaderViewController::resized()
{
    float width = getWidth() / 7 - gXSpacing;
    
    Rectangle<int> area (getLocalBounds());
    area.reduce(0, gYSpacing);
    
    area.removeFromLeft(gXSpacing);
    galleryB.setBounds(area.removeFromLeft(width));
    
    area.removeFromLeft(gXSpacing);
    galleryCB.setBounds(area.removeFromLeft(2*width));
    
    area.removeFromRight(gXSpacing);
    pianoB.setBounds(area.removeFromRight(width));
    
    area.removeFromRight(gXSpacing);
    pianoCB.setBounds(area.removeFromRight(2*width));
    area.removeFromRight(gXSpacing);

    area.removeFromLeft(gXSpacing);
    loadB.setBounds(area);
}

PopupMenu HeaderViewController::getLoadMenu(void)
{
    PopupMenu loadMenu;
    loadMenu.setLookAndFeel(&buttonsAndMenusLAF);
    
    int count = 0;
    for (auto loadType : cBKSampleLoadTypes)
        loadMenu.addItem(++count, loadType);
    
    return loadMenu;
}

PopupMenu HeaderViewController::getPianoMenu(void)
{
    PopupMenu pianoMenu;
    pianoMenu.setLookAndFeel(&buttonsAndMenusLAF);
    
    pianoMenu.addItem(1, "New");
    pianoMenu.addItem(2, "Duplicate");
    pianoMenu.addItem(3, "Delete");
    
    pianoMenu.addSeparator();
    pianoMenu.addSubMenu("Add...", getNewMenu());
    pianoMenu.addSeparator();
   
    //pianoMenu.addSeparator();
    //pianoMenu.addItem(3, "Settings");
    
    return pianoMenu;
}

PopupMenu HeaderViewController::getNewMenu(void)
{
    PopupMenu newMenu;
    newMenu.setLookAndFeel(&buttonsAndMenusLAF);
    
    newMenu.addItem(KEYMAP_ID, "Keymap (k)");
    newMenu.addItem(DIRECT_ID, "Direct (d)");
    newMenu.addItem(NOSTALGIC_ID, "Nostalgic (n)");
    newMenu.addItem(SYNCHRONIC_ID, "Synchronic (s)");
    newMenu.addItem(TUNING_ID, "Tuning (t)");
    newMenu.addItem(TEMPO_ID, "Tempo (m)");
    newMenu.addItem(MODIFICATION_ID, "Modification (c)");
    newMenu.addItem(PIANOMAP_ID, "Piano Map (p)");
    newMenu.addItem(RESET_ID, "Reset (r)");
    
    return newMenu;
}

PopupMenu HeaderViewController::getGalleryMenu(void)
{
    PopupMenu galleryMenu;
    galleryMenu.setLookAndFeel(&buttonsAndMenusLAF);
    
    galleryMenu.addItem(SETTINGS_ID, "Settings...");
    galleryMenu.addSeparator();
    galleryMenu.addItem(NEWGALLERY_ID, "New...");
    galleryMenu.addSeparator();
    galleryMenu.addItem(OPEN_ID, "Open...");
    galleryMenu.addItem(OPENOLD_ID, "Open (legacy)...");
    galleryMenu.addSeparator();
    
    String saveKeystroke = "(Cmd-S)";
    String saveAsKeystroke = "(Shift-Cmd-S)";
    
#if JUCE_WINDOWS
    saveKeystroke = "(Ctrl-S)";
	saveAsKeystroke = "(Shift-Ctrl-S)";
#endif
    
    galleryMenu.addItem(SAVE_ID, "Save " );
    galleryMenu.addItem(SAVEAS_ID, "Save as... ");
    galleryMenu.addSeparator();
    galleryMenu.addItem(CLEAN_ID, "Clean");
    galleryMenu.addSeparator();
    galleryMenu.addItem(DELETE_ID, "Delete");
    
    
    return galleryMenu;
    
}

void HeaderViewController::loadMenuCallback(int result, HeaderViewController* gvc)
{
    gvc->processor.loadPianoSamples((BKSampleLoadType)(result-1));
}

void HeaderViewController::pianoMenuCallback(int result, HeaderViewController* hvc)
{
    BKAudioProcessor& processor = hvc->processor;
    BKConstructionSite* construction = hvc->construction;
    BKEditableComboBox* pianoCB = &hvc->pianoCB;
    
    if (result == 1) // New piano
    {
        int newId = processor.gallery->getNewId(PreparationTypePiano);
        
        processor.gallery->addTypeWithId(PreparationTypePiano, newId);
        
        String newName = "Piano"+String(newId);
        
        processor.gallery->getPianos().getLast()->setName(newName);
        
        hvc->fillPianoCB();
        
        processor.setCurrentPiano(newId);
    }
    else if (result == 2) // Duplicate
    {
        int newId = processor.gallery->duplicate(PreparationTypePiano, processor.currentPiano->getId());
        
        String newName = "Piano"+String(newId);
        
        hvc->fillPianoCB();
        
        processor.setCurrentPiano(newId);
    }
    else if (result == 3) // Remove piano
    {
        
        int pianoId = hvc->pianoCB.getSelectedId();
        int index = hvc->pianoCB.getSelectedItemIndex();
        
        if ((index == 0) && (hvc->pianoCB.getNumItems() == 1)) return;
        
        processor.gallery->remove(PreparationTypePiano, pianoId);
        
        hvc->fillPianoCB();
        
        int newPianoId = hvc->pianoCB.getItemId(index);
        
        if (newPianoId == 0) newPianoId = hvc->pianoCB.getItemId(index-1);
        
        hvc->pianoCB.setSelectedId(newPianoId, dontSendNotification);
        
        processor.setCurrentPiano(newPianoId);
    }
    else if (result == KEYMAP_ID)
    {
        construction->addItem(PreparationTypeKeymap, true);
    }
    else if (result == DIRECT_ID)
    {
        construction->addItem(PreparationTypeDirect, true);
    }
    else if (result == NOSTALGIC_ID)
    {
        construction->addItem(PreparationTypeNostalgic, true);
    }
    else if (result == SYNCHRONIC_ID)
    {
        construction->addItem(PreparationTypeSynchronic, true);
    }
    else if (result == TUNING_ID)
    {
        construction->addItem(PreparationTypeTuning, true);
    }
    else if (result == TEMPO_ID)
    {
        construction->addItem(PreparationTypeTempo, true);
    }
    else if (result == MODIFICATION_ID)
    {
        construction->addItem(PreparationTypeGenericMod, true);
    }
    else if (result == PIANOMAP_ID)
    {
        construction->addItem(PreparationTypePianoMap, true);
    }
    else if (result == RESET_ID)
    {
        construction->addItem(PreparationTypeReset, true);
    }
    
}

void HeaderViewController::galleryMenuCallback(int result, HeaderViewController* gvc)
{
    BKAudioProcessor& processor = gvc->processor;
    if (result == SAVE_ID)
    {
        processor.saveGallery();
    }
    if (result == SAVEAS_ID)
    {
        processor.saveGalleryAs();
    }
    else if (result == OPEN_ID) // Load
    {
        processor.loadGalleryDialog();
    }
    else if (result == SETTINGS_ID) // open General settings
    {
        processor.updateState->setCurrentDisplay(DisplayGeneral);
    }
    else if (result == CLEAN_ID) // Clean
    {
        processor.gallery->clean();
    }
    else if (result == DELETE_ID) // Clean
    {
        processor.deleteGallery();
    }
    else if (result == OPENOLD_ID) // Load (old)
    {
        processor.loadJsonGalleryDialog();
    }
    else if (result == NEWGALLERY_ID)
    {
        bool shouldContinue = gvc->handleGalleryChange();
        
        if (!shouldContinue) return;
        
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        prompt.addTextEditor("name", "My New Gallery");
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        DBG(name);
        
        if (result == 1)
        {
            processor.createNewGallery(name);
        }
    }
}

void HeaderViewController::bkButtonClicked (Button* b)
{
    String name = b->getName();
    
    if (b == &loadB)
    {
        PopupMenu loadMenu = getLoadMenu();
        loadMenu.setLookAndFeel(&buttonsAndMenusLAF);
        
        loadMenu.showMenuAsync (PopupMenu::Options().withTargetComponent (&loadB), ModalCallbackFunction::forComponent (loadMenuCallback, this) );
    }
    else if (b == &pianoB)
    {
        getPianoMenu().showMenuAsync (PopupMenu::Options().withTargetComponent (&pianoB), ModalCallbackFunction::forComponent (pianoMenuCallback, this) );
    }
    else if (b == &galleryB)
    {
        getGalleryMenu().showMenuAsync (PopupMenu::Options().withTargetComponent (&galleryB), ModalCallbackFunction::forComponent (galleryMenuCallback, this) );
    }
    
    
}

void HeaderViewController::fillGalleryCB(void)
{
    
    if(!galleryModalCallBackIsOpen)
    {
        galleryCB.clear(dontSendNotification);
        
        PopupMenu* galleryCBPopUp = galleryCB.getRootMenu();
        
        int index = 0;
        bool creatingSubmenu = false;
        String submenuName;
        
        StringArray submenuNames;
        OwnedArray<PopupMenu> submenus;
        
        File bkGalleries;
        bkGalleries = bkGalleries.getSpecialLocation(File::userDocumentsDirectory).getChildFile("bitKlavier resources").getChildFile("galleries");
        
        for (int i = 0; i < processor.galleryNames.size(); i++)
        {
            File thisFile(processor.galleryNames[i]);
            
            String galleryName = thisFile.getFileName().upToFirstOccurrenceOf(".xml", false, false);
            
            //moving on to new submenu, if there is one, add add last submenu to popup now that it's done
            //if(creatingSubmenu && thisFile.getRelativePathFrom(File ("~/bkGalleries")).initialSectionNotContaining("/") != submenuName)
            if(creatingSubmenu && thisFile.getParentDirectory().getFileName() != submenuName)
            {
                galleryCBPopUp->addSubMenu(submenuName, *submenus.getLast());
                creatingSubmenu = false;
            }
            
            //add toplevel item, if there is one
            //if(thisFile.getFileName() == thisFile.getRelativePathFrom(File ("~/bkGalleries"))) //if the file is in the main galleries
            if(thisFile.getParentDirectory().getFileName() == bkGalleries.getFileName()) //if the file is in the main galleries directory....
            {
                galleryCB.addItem(galleryName, i+1); //add to toplevel popup
            }
            
            //otherwise add to or create submenu with name of subfolder
            else
            {
                creatingSubmenu = true;
                
                //submenuName = thisFile.getRelativePathFrom(File ("~/bkGalleries")).initialSectionNotContaining("/"); //name of submenu
                submenuName = thisFile.getParentDirectory().getFileName(); //name of submenu
                
                if(submenuNames.contains(submenuName)) //add to existing submenu
                {
                    PopupMenu* existingMenu = submenus.getUnchecked(submenuNames.indexOf(submenuName));
                    existingMenu->addItem(i + 1, galleryName);
                }
                else
                {
                    submenus.add(new PopupMenu());
                    submenuNames.add(submenuName);

                    submenus.getLast()->addItem(i + 1, galleryName);;
                }
            }
     
            if (thisFile.getFileName() == processor.currentGallery) index = i;
        }
        
        //add last submenu to popup, if there is one
        if(creatingSubmenu)
        {
            galleryCBPopUp->addSubMenu(submenuName, *submenus.getLast());
            creatingSubmenu = false;
        }

        galleryCB.setSelectedId(index+1, NotificationType::dontSendNotification);
        
        File selectedFile(processor.galleryNames[index]);
        processor.gallery->setURL(selectedFile.getFullPathName());
        
    }
}

void HeaderViewController::update(void)
{
    
}

void HeaderViewController::switchGallery()
{
    fillPianoCB();
    fillGalleryCB();
}

void HeaderViewController::fillPianoCB(void)
{
    pianoCB.clear(dontSendNotification);

    for (auto piano : processor.gallery->getPianos())
    {
        String name = piano->getName();
        
        DBG("pianoName: " + String(piano->getName()));
        
        if (name != String::empty)  pianoCB.addItem(name,  piano->getId());
        else                        pianoCB.addItem("Piano" + String(piano->getId()), piano->getId());
    }

    pianoCB.setSelectedId(processor.currentPiano->getId(), dontSendNotification);
}

void HeaderViewController::bkTextFieldDidChange(TextEditor& tf)
{
    String text = tf.getText();
    String name = tf.getName();
    
    DBG(text);
    
}

void HeaderViewController::BKEditableComboBoxChanged(String text, BKEditableComboBox* cb)
{
    if (cb == &pianoCB)
    {
        processor.currentPiano->setName(text);
    }
    else if (cb == &galleryCB)
    {
        processor.renameGallery(text);
        fillGalleryCB();
    }
}

bool HeaderViewController::handleGalleryChange(void)
{
    String name = galleryCB.getName();
    
    bool shouldSwitch = false;
    
    galleryIsDirtyAlertResult = 2;
    
    if(processor.gallery->isGalleryDirty())
    {
        DBG("GALLERY IS DIRTY, CHECK FOR SAVE HERE");
        galleryModalCallBackIsOpen = true; //not sure, maybe should be doing some kind of Lock
        
        galleryIsDirtyAlertResult = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                                                     "The current gallery has changed.",
                                                                     "Do you want to save before switching galleries?",
                                                                     String(),
                                                                     String(),
                                                                     String(),
                                                                     0,
                                                                     //ModalCallbackFunction::forComponent (alertBoxResultChosen, this)
                                                                     nullptr);
    }
    
    if(galleryIsDirtyAlertResult == 0)
    {
        DBG("cancelled");
    }
    else if(galleryIsDirtyAlertResult == 1)
    {
        DBG("saving gallery");
        processor.saveGallery();
        
        shouldSwitch = true;
    }
    else if(galleryIsDirtyAlertResult == 2)
    {
        DBG("not saving");
        shouldSwitch = true;
    }
    
    galleryModalCallBackIsOpen = false;
    
    return shouldSwitch;
}


void HeaderViewController::bkComboBoxDidChange (ComboBox* cb)
{
    String name = cb->getName();
    int Id = cb->getSelectedId();
    int index = cb->getSelectedItemIndex();

    
    if (name == "pianoCB")
    {
        processor.setCurrentPiano(Id);
        
        fillPianoCB();
        
        update();
        
    }
    else if (name == "galleryCB")
    {
        bool shouldSwitch = handleGalleryChange();
        
        if (shouldSwitch)
        {
            String path = processor.galleryNames[index];
            lastGalleryCBId = Id;
            
            if (path.endsWith(".xml"))          processor.loadGalleryFromPath(path);
            else  if (path.endsWith(".json"))   processor.loadJsonGalleryFromPath(path);
        }
        else
        {
            cb->setSelectedId(lastGalleryCBId, dontSendNotification);
        }
    }
}
