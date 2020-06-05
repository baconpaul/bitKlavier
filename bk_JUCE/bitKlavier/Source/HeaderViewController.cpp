/*
  ==============================================================================

    HeaderViewController.cpp
    Created: 27 Mar 2017 12:47:45pm
    Author:  Michael R Mulshine

  ==============================================================================
*/



#include "HeaderViewController.h"


HeaderViewController::HeaderViewController (BKAudioProcessor& p, BKConstructionSite* c):
processor (p),
construction(c)
{
    setLookAndFeel(new BKButtonAndMenuLAF());

    addAndMakeVisible(galleryB);
    galleryB.setButtonText("Gallery");
    galleryB.setTooltip("Create, duplicate, rename, share current Gallery. Also access bitKlavier settings");
    galleryB.addListener(this);
    
    addAndMakeVisible(pianoB);
    pianoB.setButtonText("Piano");
    pianoB.setTooltip("Create, duplicate, rename, or delete current Piano");
    pianoB.addListener(this);
    
    addAndMakeVisible(editB);
    editB.setButtonText("Action");
    editB.setTooltip("Add a preparation. Also, stop all internal bitKlavier signals");
    editB.addListener(this);
    
    // Gallery CB
    addAndMakeVisible(galleryCB);
    galleryCB.setName("galleryCB");
    galleryCB.setTooltip("Select and load saved bitKlavier Galleries. Indicates currently loaded Gallery");
    galleryCB.addListener(this);
    //galleryCB.BKSetJustificationType(juce::Justification::centredRight);
    BKButtonAndMenuLAF* comboBoxLeftJustifyLAF = new BKButtonAndMenuLAF();
    galleryCB.setLookAndFeel(comboBoxLeftJustifyLAF);
    galleryCB.setSelectedId(0, dontSendNotification);
    
    // Piano CB
    addAndMakeVisible(pianoCB);
    pianoCB.setName("pianoCB");
    pianoCB.setTooltip("Select and load saved bitKlavier Pianos. Indicates currently loaded Piano");
    pianoCB.addListener(this);
    
#if JUCE_IOS || JUCE_MAC
    bot.setBounds(0,0,20,20);
    addAndMakeVisible(bot);
#endif
    
    BKButtonAndMenuLAF* comboBoxRightJustifyLAF = new BKButtonAndMenuLAF();
    pianoCB.setLookAndFeel(comboBoxRightJustifyLAF);
    comboBoxRightJustifyLAF->setComboBoxJustificationType(juce::Justification::centredRight);

    pianoCB.setSelectedId(0, dontSendNotification);
    
    galleryModalCallBackIsOpen = false;
    
    //loadDefaultGalleries();
    
    fillGalleryCB();
    fillPianoCB();
    processor.updateState->pianoDidChangeForGraph = true;
    
    startTimerHz (5);
}

HeaderViewController::~HeaderViewController()
{
    PopupMenu::dismissAllActiveMenus();
    galleryCB.setLookAndFeel(nullptr);
    pianoCB.setLookAndFeel(nullptr);
    
    pianoB.setLookAndFeel(nullptr);
    galleryB.setLookAndFeel(nullptr);
    editB.setLookAndFeel(nullptr);
    
    setLookAndFeel(nullptr);
}

void HeaderViewController::paint (Graphics& g)
{
    g.fillAll(Colours::black);
}

void HeaderViewController::resized()
{
    Rectangle<int> area (getLocalBounds());
    area.reduce(0, gYSpacing);
    
    float width = area.getWidth() / 7;
    
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
    editB.setBounds(area);
}

PopupMenu HeaderViewController::getLoadMenu(void)
{
    BKPopupMenu loadMenu;
    
    loadMenu.addItem(LOAD_LITEST,   "Lightest", processor.globalSampleType != BKLoadLitest, processor.globalSampleType == BKLoadLitest);
    loadMenu.addItem(LOAD_LITE,     "Light", processor.globalSampleType != BKLoadLite, processor.globalSampleType == BKLoadLite);
    loadMenu.addItem(LOAD_MEDIUM,   "Medium", processor.globalSampleType != BKLoadMedium, processor.globalSampleType == BKLoadMedium);
    loadMenu.addItem(LOAD_HEAVY,    "Heavy", processor.globalSampleType != BKLoadHeavy, processor.globalSampleType == BKLoadHeavy);
    
    loadMenu.addSeparator();
    
#if JUCE_IOS
    
    loadMenu.addItem(SF_DEFAULT_0, "Rhodes", true, false);
    loadMenu.addItem(SF_DEFAULT_1, "Harpsichord", true, false);
    loadMenu.addItem(SF_DEFAULT_2, "Drums", true, false);
    loadMenu.addItem(SF_DEFAULT_3, "Saw", true, false);
    loadMenu.addItem(SF_DEFAULT_4, "Electric Bass", true, false);
    
    loadMenu.addSeparator();
    
#endif
    
    int i = 0;
    for (auto sf : processor.soundfontNames)
    {
        String sfName = sf.fromLastOccurrenceOf("/", false, true).upToFirstOccurrenceOf(".sf", false, true);
        sfName = sfName.replace("%20", " ");
        loadMenu.addItem(SOUNDFONT_ID + (i++), sfName);
    }
    
    return loadMenu;
}

PopupMenu HeaderViewController::getExportedPianoMenu(void)
{
    BKPopupMenu menu;
    
    StringArray names = processor.exportedPianos;
    for (int i = 0; i < names.size(); i++)
    {
        menu.addItem(i+100, names[i]);
    }
    
    return menu;
}

PopupMenu HeaderViewController::getPianoMenu(void)
{
    BKPopupMenu pianoMenu;
    
    pianoMenu.addItem(1, "New");
    pianoMenu.addItem(2, "Linked Copy"); // add another called Duplicate that is like copy/paste?
    pianoMenu.addItem(4, "Rename");
    pianoMenu.addItem(3, "Remove");
    pianoMenu.addSeparator();
    pianoMenu.addItem(7, "Export");
    
    PopupMenu exported = getExportedPianoMenu();
    pianoMenu.addSubMenu("Import...", exported);
    
    return pianoMenu;
}

PopupMenu HeaderViewController::getGalleryMenu(void)
{
    BKPopupMenu galleryMenu;
    
    galleryMenu.addItem(NEWGALLERY_ID, "New");
    
    if (processor.wrapperType == juce::AudioPluginInstance::wrapperType_Standalone)
    {
        galleryMenu.addItem(SAVE_ID, "Save" + gSaveShortcut);
        galleryMenu.addItem(SAVEAS_ID, "Save as" + gSaveAsShortcut);
    }
    else
    {
        galleryMenu.addItem(SAVE_ID, "Save");
        galleryMenu.addItem(SAVEAS_ID, "Save as");
    }
    
    if (!processor.defaultLoaded)
    {
        galleryMenu.addItem(RENAME_ID, "Rename");
        galleryMenu.addItem(DELETE_ID, "Remove");
    }
    
#if JUCE_IOS
    galleryMenu.addSeparator();
    galleryMenu.addItem(EXPORT_ID, "Export");
    galleryMenu.addItem(IMPORT_ID, "Import");
#else
    galleryMenu.addItem(OPEN_ID, "Open");
    galleryMenu.addItem(OPENOLD_ID, "Open (legacy)");
#endif
    
    galleryMenu.addSeparator();
    galleryMenu.addItem(CLEAN_ID, "Clean");
    
#if JUCE_IOS
    galleryMenu.addSeparator();
    galleryMenu.addSubMenu("Load samples...", getLoadMenu());
#endif
    
    // ~ ~ ~ share menu ~ ~ ~
    if (!processor.defaultLoaded)
    {
#if JUCE_MAC
    BKPopupMenu shareMenu;
    
    shareMenu.addItem(SHARE_EMAIL_ID, "Email");
    galleryMenu.addSeparator();
    shareMenu.addItem(SHARE_MESSAGE_ID, "Messages");
    galleryMenu.addSeparator();
    shareMenu.addItem(SHARE_FACEBOOK_ID, "Facebook");
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    galleryMenu.addSeparator();
    galleryMenu.addSubMenu("Share", shareMenu);
#elif JUCE_IOS
    galleryMenu.addSeparator();
    galleryMenu.addItem(SHARE_MESSAGE_ID, "Share");
#endif
    }
    
    galleryMenu.addSeparator();
    galleryMenu.addItem(SETTINGS_ID, "Settings");

    galleryMenu.addSeparator();
    galleryMenu.addItem(ABOUT_ID, "About bitKlavier...");
    
    return galleryMenu;
    
}

void HeaderViewController::pianoMenuCallback(int res, HeaderViewController* hvc)
{
    if (hvc == nullptr)
    {
        PopupMenu::dismissAllActiveMenus();
        return;
    }
    
    BKAudioProcessor& processor = hvc->processor;
    
    if (res == 1) // New piano
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        prompt.addTextEditor("name", "My New Piano");
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            int newId = processor.gallery->getNewId(PreparationTypePiano);
            
            processor.gallery->addTypeWithId(PreparationTypePiano, newId);
            
            processor.gallery->getPianos().getLast()->setName(name);
            
            hvc->fillPianoCB();
            
            processor.setCurrentPiano(newId);
        }
    }
    else if (res == 2) // Duplicate
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        prompt.addTextEditor("name", processor.currentPiano->getName() + " (2)");
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            int newId = processor.gallery->duplicate(PreparationTypePiano, processor.currentPiano->getId());
            
            processor.setCurrentPiano(newId);
            
            processor.currentPiano->setName(name);
            
            hvc->fillPianoCB();
        }
    }
    else if (res == 3) // Remove piano
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
    else if (res == 4) // Rename
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        prompt.addTextEditor("name", processor.currentPiano->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            processor.currentPiano->setName(name);
        }
        
        hvc->fillPianoCB();
    }
    else if (res == 7)
    {
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
        
        int Id = processor.currentPiano->getId();
        Piano::Ptr piano = processor.currentPiano;
        
        prompt.addTextEditor("name", piano->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        DBG("name: " + String(name));
        
        if (result == 1)
        {
            processor.exportPiano(Id, name);
        }
    }
    else if (res >= 100)
    {
        int which = res - 100;
        processor.importPiano(processor.currentPiano->getId(), which);
    }
}

void HeaderViewController::galleryMenuCallback(int result, HeaderViewController* gvc)
{
    if (gvc == nullptr)
    {
        PopupMenu::dismissAllActiveMenus();
        return;
    }
    
	BKAudioProcessor& processor = gvc->processor;

	if (result == RENAME_ID)
	{
		AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);

		prompt.addTextEditor("name", processor.gallery->getName().upToFirstOccurrenceOf(".xml", false, false));

		prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
		prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));

		int result = prompt.runModalLoop();

		String name = prompt.getTextEditorContents("name");

		if (result == 1)
		{
			processor.renameGallery(name);
		}

		gvc->fillGalleryCB();
	}
#if (JUCE_MAC || JUCE_IOS)
	else if (result == SHARE_EMAIL_ID)
	{
		gvc->bot.share(processor.gallery->getURL(), 0);
	}
	else if (result == SHARE_MESSAGE_ID)
	{
		gvc->bot.share(processor.gallery->getURL(), 1);
	}
	else if (result == SHARE_FACEBOOK_ID)
	{
		gvc->bot.share(processor.gallery->getURL(), 2);
	}
#endif
    else if (result == LOAD_LITEST)
    {
        processor.loadSamples(BKLoadLitest);
    }
    else if (result == LOAD_LITE)
    {
        processor.loadSamples(BKLoadLite);
    }
    else if (result == LOAD_MEDIUM)
    {
        processor.loadSamples(BKLoadMedium);
    }
    else if (result == LOAD_HEAVY)
    {
        processor.loadSamples(BKLoadHeavy);
    }
    else if (result >= SF_DEFAULT_0 && result < SOUNDFONT_ID)
    {
        processor.loadSamples(BKLoadSoundfont, "default.sf" + String(result - SF_DEFAULT_0), 0);
    }
    else if (result == SAVE_ID && !processor.defaultLoaded)
    {
        processor.saveCurrentGallery();
        
        //processor.createGalleryWithName(processor.gallery->getName());
    }
    else if (result >= SOUNDFONT_ID && result < MIDIOUT_ID)
    {
        processor.loadSamples(BKLoadSoundfont, processor.soundfontNames[result-SOUNDFONT_ID], 0);
    }
    else if (result == SAVEAS_ID || (result == SAVE_ID && processor.defaultLoaded))
    {
#if JUCE_IOS
        AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon, gvc);
        
        prompt.addTextEditor("name", processor.gallery->getName());
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        
        prompt.setTopLeftPosition(gvc->getWidth() / 2, gvc->getBottom() + gYSpacing);
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            String url = File::getSpecialLocation(File::userDocumentsDirectory).getFullPathName() + "/" + name.upToFirstOccurrenceOf(".xml", false, false) + ".xml";
            processor.writeCurrentGalleryToURL(url);
        }
        
        gvc->fillGalleryCB();
#else
        processor.saveCurrentGalleryAs();
#endif
    }
    else if (result == EXPORT_ID)
    {
#if JUCE_IOS
        processor.exportCurrentGallery();
#endif
    }
    else if (result == IMPORT_ID)
    {
#if JUCE_IOS
        processor.importCurrentGallery();
#endif
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
    else if (result == DELETE_ID) // Delete
    {
        AlertWindow prompt("", "Are you sure you want to delete this gallery?", AlertWindow::AlertIconType::QuestionIcon);
        
        prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
        prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
        
        int result = prompt.runModalLoop();
        
        String name = prompt.getTextEditorContents("name");
        
        if (result == 1)
        {
            processor.deleteGallery();
        }
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
        
        if (result == 1)
        {
            processor.createNewGallery(name);
        }
    }
    else if (result == ABOUT_ID)
    {
        processor.updateState->setCurrentDisplay(DisplayAbout);
    }
//    else if (result >= MIDIOUT_ID)
//    {
//        // Try to open the output selected from the Gallery menu
//        Array<MidiDeviceInfo> availableOutputs = MidiOutput::getAvailableDevices();
//        MidiDeviceInfo outputInfo = availableOutputs[result - MIDIOUT_ID];
//        std::unique_ptr<MidiOutput> output = MidiOutput::openDevice(outputInfo.identifier);
//        // If the output successfully opens
//        if (output) {
//            DBG("MIDI output set to " + outputInfo.name);
//            processor.midiOutput = std::move(output);
//        }
//        else {
//            DBG("Could not open MIDI output device");
//        }
//    }
}

void HeaderViewController::bkButtonClicked (Button* b)
{
    String name = b->getName();
    
    if (b == &editB)
    {
        if (processor.wrapperType == juce::AudioPluginInstance::wrapperType_Standalone)
        {
             getEditMenuStandalone(new BKButtonAndMenuLAF(), construction->getNumSelected()).showMenuAsync(PopupMenu::Options().withTargetComponent (b), ModalCallbackFunction::forComponent(BKConstructionSite::editMenuCallback, construction) );
        }
        else
        {
            getEditMenu(new BKButtonAndMenuLAF(), construction->getNumSelected()).showMenuAsync(PopupMenu::Options().withTargetComponent(b), ModalCallbackFunction::forComponent(BKConstructionSite::editMenuCallback, construction) );
        }
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

void HeaderViewController::addGalleriesFromFolder(File folder)
{
    
}


void HeaderViewController::loadDefaultGalleries(void)
{
    if(!galleryModalCallBackIsOpen)
    {
        galleryCB.clear(dontSendNotification);
        
        PopupMenu* popupRoot = galleryCB.getRootMenu();
        
        String data,name, resource;
        int id = 1;
        
        int size;
        
        PopupMenu mikroetudes_menu, ns_etudes_menu, bk_examples_menu;
        
        //data = BinaryData::Basic_Piano_xml;
        for (int i = 0; i < BinaryData::namedResourceListSize; i++)
        {
            resource = BinaryData::namedResourceList[i];
            
            if (resource.contains("_xml"))
            {
                data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], size);
                
                name = data.fromFirstOccurrenceOf("<gallery name=\"", false, true).upToFirstOccurrenceOf("\"", false, true);

                if (processor.mikroetudes.contains(name))       mikroetudes_menu.addItem(id++, name);
                else if (processor.ns_etudes.contains(name))    ns_etudes_menu.addItem(id++, name);
                else if (processor.bk_examples.contains(name))  bk_examples_menu.addItem(id++, name);
                else                                            galleryCB.addItem(name, id++);
                
            }
        }
        
        popupRoot->addSubMenu("Examples", bk_examples_menu);
        popupRoot->addSubMenu("Nostalgic Synchronic", ns_etudes_menu);
        popupRoot->addSubMenu("Mikroetudes", mikroetudes_menu);
        
        galleryCB.addSeparator();
        
        numberOfDefaultGalleryItems = galleryCB.getNumItems();
    }
}

void HeaderViewController::fillGalleryCB(void)
{
    if (processor.gallery == nullptr) return;
    
    if(!galleryModalCallBackIsOpen)
    {
        loadDefaultGalleries();
        
        File bkGalleries;
        
        File moreGalleries = File::getSpecialLocation(File::userDocumentsDirectory);
        
#if (JUCE_MAC)
        bkGalleries = bkGalleries.getSpecialLocation(File::globalApplicationsDirectory).getChildFile("bitKlavier").getChildFile("galleries");
#endif
#if (JUCE_WINDOWS || JUCE_LINUX)
        bkGalleries = bkGalleries.getSpecialLocation(File::userDocumentsDirectory).getChildFile("bitKlavier").getChildFile("galleries");
#endif
        
        PopupMenu* galleryCBPopUp = galleryCB.getRootMenu();
        
        int id = numberOfDefaultGalleryItems, index = 0;
        bool creatingSubmenu = false;
        String submenuName;
        
        StringArray submenuNames;
        OwnedArray<PopupMenu> submenus;

		processor.galleryNames.sortNatural();
        /*
		DBG("Gallery Names:");
		for (auto name : processor.galleryNames)
		{
			DBG(name + "\n");
		}
         */
		/*StringArray orderedNames = StringArray(processor.galleryNames);
		orderedNames.sortNatural();
		DBG("Ordered Names:");
		for (auto name : orderedNames)
		{
			DBG(name + "\n");
		}*/
        
        for (int i = 0; i < processor.galleryNames.size(); i++)
        {
            File thisFile(processor.galleryNames[i]);
            
            String galleryName = thisFile.getFileName().upToFirstOccurrenceOf(".xml", false, false);
            
            //moving on to new submenu, if there is one, add add last submenu to popup now that it's done
            if(creatingSubmenu && thisFile.getParentDirectory().getFileName() != submenuName)
            {
                galleryCBPopUp->addSubMenu(submenuName, *submenus.getLast());
                creatingSubmenu = false;
            }
            
            //add toplevel item, if there is one
            if(thisFile.getParentDirectory().getFileName() == bkGalleries.getFileName() ||
               thisFile.getParentDirectory().getFileName() == moreGalleries.getFileName() ||
               thisFile.getParentDirectory().getFileName() == moreGalleries.getChildFile("Inbox").getFileName()) //if the file is in the main galleries directory....
            {
                galleryCB.addItem(galleryName, ++id); //add to toplevel popup
            }
            
            //otherwise add to or create submenu with name of subfolder
            else
            {
                creatingSubmenu = true;
                
                submenuName = thisFile.getParentDirectory().getFileName(); //name of submenu
                
                if(submenuNames.contains(submenuName)) //add to existing submenu
                {
                    PopupMenu* existingMenu = submenus.getUnchecked(submenuNames.indexOf(submenuName));
                    existingMenu->addItem(++id, galleryName);
                }
                else
                {
                    submenus.add(new PopupMenu());
                    submenuNames.add(submenuName);
                    
                    submenus.getLast()->addItem(++id, galleryName);;
                }
            }
            
            if (thisFile.getFileName() == processor.currentGallery)
            {
                index = i;
                lastGalleryCBId = id;
            }
        }
        
        //add last submenu to popup, if there is one
        if(creatingSubmenu)
        {
            galleryCBPopUp->addSubMenu(submenuName, *submenus.getLast());
            creatingSubmenu = false;
        }
        
        // THIS IS WHERE NAME OF GALLERY DISPLAYED IS SET
        galleryCB.setSelectedId(lastGalleryCBId, NotificationType::dontSendNotification);
        galleryCB.setText(processor.gallery->getName().upToFirstOccurrenceOf(".xml", false, true), NotificationType::dontSendNotification);
    }
}

void HeaderViewController::update(void)
{
    if (processor.updateState->currentDisplay == DisplayNil)
    {
        editB.setEnabled(true);
        pianoB.setEnabled(true);
        galleryB.setEnabled(true);
        pianoCB.setEnabled(true);
        galleryCB.setEnabled(true);
    }
    else
    {
        editB.setEnabled(false);
        pianoB.setEnabled(false);
        galleryB.setEnabled(false);
        pianoCB.setEnabled(false);
        galleryCB.setEnabled(false);
    }
}

void HeaderViewController::switchGallery()
{
    fillGalleryCB();
    fillPianoCB();
}

void HeaderViewController::fillPianoCB(void)
{
    if (processor.gallery == nullptr) return;
    
    pianoCB.clear(dontSendNotification);

	//now alphabetizes the list (turning into string) before putting it into the combo box
	StringArray nameIDStrings = StringArray();

	//concatenate name and ID into a single string using the newline character (something a user couldn't type) as a separator, then sorts the list of strings
	for (auto piano : processor.gallery->getPianos())
	{
		String name = piano->getName();
		String idString = String(piano->getId());
		if (name != String()) nameIDStrings.add(name + char(10) + idString);
		else nameIDStrings.add("Piano" + String(idString) + char(10) + idString);
	}

	nameIDStrings.sort(false);

	//separates the combined string into name and ID parts, then adds to the combo box
	for (auto combo : nameIDStrings)
	{
		int separatorIndex = combo.indexOfChar(char(10));
		String name = combo.substring(0, separatorIndex);
		int id = combo.substring(separatorIndex + 1).getIntValue();
		pianoCB.addItem(name, id);
	}

	//old code
    /*for (auto piano : processor.gallery->getPianos())
    {
        String name = piano->getName();
        
        if (name != String())  pianoCB.addItem(name,  piano->getId());
        else                        pianoCB.addItem("Piano" + String(piano->getId()), piano->getId());
    }*/

    pianoCB.setSelectedId(processor.currentPiano->getId(), dontSendNotification);
}

void HeaderViewController::bkTextFieldDidChange(TextEditor& tf)
{
    String text = tf.getText();
    String name = tf.getName();
    
    DBG(text);
    
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
                                                                     "Do you want to save first?",
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
        
        if (processor.defaultLoaded)
        {
            AlertWindow prompt("", "", AlertWindow::AlertIconType::QuestionIcon);
            
            prompt.addTextEditor("name", processor.gallery->getName());
            
            prompt.addButton("Ok", 1, KeyPress(KeyPress::returnKey));
            prompt.addButton("Cancel", 2, KeyPress(KeyPress::escapeKey));
            
            int result = prompt.runModalLoop();
            
            String name = prompt.getTextEditorContents("name").upToFirstOccurrenceOf(".xml", false, false);
            
            if (result == 1)
            {
#if JUCE_IOS
                File newFile = File::getSpecialLocation(File::userDocumentsDirectory);
#endif
#if JUCE_MAC
                File newFile = File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("bitKlavier").getChildFile("galleries");
#endif
#if JUCE_WINDOWS || JUCE_LINUX
                File newFile = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("bitKlavier").getChildFile("galleries");
#endif
                
                String newURL = newFile.getFullPathName() + name + ".xml";
                processor.writeCurrentGalleryToURL(newURL);
            }
        }
        else
        {
            processor.saveCurrentGallery();
        }
        
        fillGalleryCB();
        
        shouldSwitch = true;
    }
    else if(galleryIsDirtyAlertResult == 2)
    {
        // DBG("not saving");
        shouldSwitch = true;
    }
    
    galleryModalCallBackIsOpen = false;
    
    return shouldSwitch;
}


void HeaderViewController::bkComboBoxDidChange (ComboBox* cb)
{
    String name = cb->getName();
    int Id = cb->getSelectedId();

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
            lastGalleryCBId = Id;
            int index = Id - 1;

            //if (index < numberOfDefaultGalleryItems)
            if(cb->getSelectedItemIndex() < numberOfDefaultGalleryItems)
            {
                int size;
                String xmlData = CharPointer_UTF8 (BinaryData::getNamedResource(BinaryData::namedResourceList[index], size));
                
                processor.defaultLoaded = true;
                processor.defaultName = BinaryData::namedResourceList[index];
                
                processor.loadGalleryFromXml(XmlDocument::parse(xmlData).get());
            }
            else
            {
                index = index - numberOfDefaultGalleryItems;
                String path = processor.galleryNames[index];
                
                processor.defaultLoaded = false;
                processor.defaultName = "";
         
                if (path.endsWith(".xml"))          processor.loadGalleryFromPath(path);
                else  if (path.endsWith(".json"))   processor.loadJsonGalleryFromPath(path);
                
                DBG("HeaderViewController::bkComboBoxDidChange combobox text = " + galleryCB.getText());
            }
            
        }
        else
        {
            cb->setSelectedId(lastGalleryCBId, dontSendNotification);
        }
    }
}

void HeaderViewController::timerCallback()
{
    if (!galleryCB.isPopupActive())
    {
        fillGalleryCB();
    }
}

