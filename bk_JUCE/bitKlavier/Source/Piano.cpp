/*
  ==============================================================================

    Piano.cpp
    Created: 7 Dec 2016 10:25:40am
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "Piano.h"

#include "PluginProcessor.h"

#include "Gallery.h"

Piano::Piano(BKAudioProcessor& p,
             int Id):
prepMap(new PreparationMap()),
processor(p),
Id(Id)
{
    prepMap->prepareToPlay(processor.getCurrentSampleRate());
    
    modificationMap = OwnedArray<Modifications>();
    modificationMap.ensureStorageAllocated(128);
    
    for (int i = 0; i < 128; i++)
    {
        modificationMap.add(new Modifications());
    }
}

Piano::~Piano()
{
    for (int i = 0; i < items.size(); i++)  items[i]->connections.clear();;
    items.clear();
}

Piano::Ptr Piano::duplicate(bool withSameId)
{
    Piano::Ptr copyPiano = new Piano(processor, withSameId ? Id : -1);
    
    BKItem::PtrArr newItems;
    
    for (auto item : items)
    {
        BKItem* newItem = new BKItem(item->getType(), item->getId(), processor);
        
        newItem->setCommentText(item->getCommentText());
        
        newItem->setTopLeftPosition(item->getPosition());
        newItem->setName(item->getName());
        
        copyPiano->add(newItem);
        newItems.add(newItem);
    }
    
    int idx = 0;
    for (auto item : items)
    {
        BKItem::Ptr newItem = newItems.getUnchecked(idx++);
        
        BKItem::PtrArr oldConnections = item->getConnections();
        
        for (auto connection : item->getConnections())
        {
            for (auto newConnection : newItems)
            {
                if ((newConnection->getType() == connection->getType()) &&
                    (newConnection->getId() == connection->getId()))
                {
                    newItem->addConnection(newConnection);
                }
            }
        }
    }
    
    copyPiano->setName(pianoName );
    
    copyPiano->prepareToPlay(processor.getCurrentSampleRate());
    
    copyPiano->configure();
    
    return copyPiano;
}

void Piano::clear(void)
{
    items.clear();
}

void Piano::deconfigure(void)
{
    prepMap = new PreparationMap();
    numPMaps = 0;
    
    /*
    for (auto proc : dprocessor) proc->reset();
    for (auto proc : mprocessor) proc->reset();
    for (auto proc : sprocessor) proc->reset();
    for (auto proc : nprocessor) proc->reset();
    for (auto proc : tprocessor) proc->reset();
    */
    
    dprocessor.clear();
    mprocessor.clear();
    sprocessor.clear();
    nprocessor.clear();
    tprocessor.clear();
    for (auto b : bprocessor)
    {
        b->getSynth()->removeBlendronicProcessor(b->getId());
    }
    bprocessor.clear();
    rprocessor.clear();
    
    for (int key = 0; key < 128; key++)
    {
        modificationMap[key]->clearModifications();
        modificationMap[key]->clearResets();
        modificationMap[key]->pianoMaps.clear();
    }
}

#define DEFAULT_ID -1
void Piano::configure(void)
{
    deconfigure();
    
    defaultT = getTuningProcessor(DEFAULT_ID);
    
    defaultM = getTempoProcessor(DEFAULT_ID);
    
    defaultS = getSynchronicProcessor(DEFAULT_ID);
    
    //defaultB = getBlendronicProcessor(DEFAULT_ID);

    defaultR = getResonanceProcessor(DEFAULT_ID);
    
    //test of resonance preparation - make a default resonance and link it to a piano and tuning
    //ResonancePreparation::Ptr testResPrep = new ResonancePreparation();
    //Resonance::Ptr testRes = new Resonance(testResPrep, 11111);
    //ResonanceProcessor::Ptr testResProc = new ResonanceProcessor(testRes, defaultT, processor.gallery->getGeneralSettings(), &processor.mainPianoSynth);
    //DBG("ID = " + String(testResProc->getId()));
    
    //processor.gallery->addResonanceWithId(11111);
    //ResonanceProcessor::Ptr testResProc = addResonanceProcessor(11111);
    
    //linkPreparationWithTuning(PreparationTypeResonance, 11111, defaultT->getTuning());


    for (auto item : items)
    {
        BKPreparationType thisType = item->getType();
        int thisId = item->getId();
        
        if (thisId > processor.gallery->getIdCount(thisType)) processor.gallery->setIdCount(thisType, thisId);
        
        addProcessor(thisType, thisId);
    }

    //bool testLinkedWithTuning = false;
    
    for (auto item : items)
    {
        BKPreparationType type = item->getType();
        int Id = item->getId();
        
        // Connect keymaps to everything
        // Connect tunings, tempos, synchronics to preparations
        // Connect mods and resets to all their targets
        // Configure piano maps
        // ... should be all configured if done in that order ...
        if (type == PreparationTypeKeymap)
        {
            Keymap::Ptr keymap = processor.gallery->getKeymap(Id);
            prepMap->addKeymap(keymap);
            
            BKItem::PtrArr connex = item->getConnections();
            for (auto target : connex)
            {
                BKPreparationType targetType = target->getType();
                int targetId = target->getId();
                
                if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeTempo))
                {
                    // DBG(String(targetType) + " linked with keymap");
                    linkPreparationWithKeymap(targetType, targetId, Id);
                }
            }
            //testing by linking all the keymaps with the test resonance
            //linkPreparationWithKeymap(PreparationTypeResonance, 11111, Id);
            connex.clear();
        }
        else if (type == PreparationTypeTuning)
        {
            // Look for synchronic, direct, nostalgic, blendronic, and resonance targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                int targetId = target->getId();
                DBG("linking preparation type to tuning: " + String(targetType));
                if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeNostalgic) || targetType == PreparationTypeResonance)
                {
                    linkPreparationWithTuning(targetType, targetId, processor.gallery->getTuning(Id));
                }
            }
            /*if (!testLinkedWithTuning)
            {
                linkPreparationWithTuning(PreparationTypeResonance, 11111, processor.gallery->getTuning(Id));
                testLinkedWithTuning = true;
            }*/
        }
        else if (type == PreparationTypeTempo)
        {
            // Look for synchronic targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                int targetId = target->getId();
                
                if (targetType == PreparationTypeSynchronic || targetType == PreparationTypeBlendronic)
                {
                    linkPreparationWithTempo(targetType, targetId, processor.gallery->getTempo(Id));
                }
            }
        }
        else if (type == PreparationTypeSynchronic)
        {
            // Look for nostalgic targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                int targetId = target->getId();
                
                if (targetType == PreparationTypeNostalgic)
                {
                    linkNostalgicWithSynchronic(processor.gallery->getNostalgic(targetId), processor.gallery->getSynchronic(Id));
                }
            }
        }
		else if (type == PreparationTypeBlendronic)
		{
			//look for direct, nostalgic, and synchronic targets
			for (auto target : item->getConnections())
			{
				BKPreparationType targetType = target->getType();
				int targetId = target->getId();

                if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeNostalgic) ||
                    targetType == PreparationTypeResonance)
				{
					linkPreparationWithBlendronic(targetType, targetId, processor.gallery->getBlendronic(Id));
				}
			}
		}
        
        // These three cases used to be handling in the keymap case as keymap connections, but
        // they handle all connected keymaps internally so that was redundant. Should be fine here.
        else if (type >= PreparationTypeDirectMod && type <= PreparationTypeTempoMod)
        {
            configureModification(item);
        }
        else if (type == PreparationTypePianoMap)
        {
            configurePianoMap(item);
        }
        else if (type == PreparationTypeReset)
        {
            configureReset(item);
        }
    }
    
    //processor.updateState->pianoDidChangeForGraph = true;
    DBG(String(getResonanceProcessors().size()));
}

SynchronicProcessor::Ptr Piano::addSynchronicProcessor(int thisId)
{
    SynchronicProcessor::Ptr sproc = new SynchronicProcessor(processor.gallery->getSynchronic(thisId),
                                        defaultT,
                                        defaultM,
										defaultBA,
                                        &processor.mainPianoSynth,
                                        processor.gallery->getGeneralSettings());
    sproc->prepareToPlay(processor.getCurrentSampleRate(), &processor.mainPianoSynth);
    sprocessor.add(sproc);
    
    return sproc;
}

DirectProcessor::Ptr Piano::getDirectProcessor(int Id, bool add)
{
    for (auto proc : dprocessor)
    {
        if (proc->getId() == Id) return proc;
    }
    
    return add ? addDirectProcessor(Id) : nullptr;
}

NostalgicProcessor::Ptr Piano::getNostalgicProcessor(int Id, bool add)
{
    for (auto proc : nprocessor)
    {
        if (proc->getId() == Id) return proc;
    }
    
    return add ? addNostalgicProcessor(Id) : nullptr;
}

SynchronicProcessor::Ptr Piano::getSynchronicProcessor(int Id, bool add)
{
    for (auto proc : sprocessor)
    {
        if (proc->getId() == Id) return proc;
    }
    
    return add ? addSynchronicProcessor(Id) : nullptr;
}

TuningProcessor::Ptr Piano::getTuningProcessor(int Id, bool add)
{
    for (auto proc : tprocessor)
    {
        if (proc->getId() == Id) return proc;
    }
    
    return add ? addTuningProcessor(Id) : nullptr;
}

TempoProcessor::Ptr Piano::getTempoProcessor(int Id, bool add)
{
    for (auto proc : mprocessor)
    {
        if (proc->getId() == Id) return proc;
    }
    
    return add ? addTempoProcessor(Id) : nullptr;
}

BlendronicProcessor::Ptr Piano::getBlendronicProcessor(int Id, bool add)
{
	for (auto proc : bprocessor)
	{
		if (proc->getId() == Id) return proc;
	}

	return add ? addBlendronicProcessor(Id) : nullptr;
}

ResonanceProcessor::Ptr Piano::getResonanceProcessor(int Id, bool add)
{
    for (auto proc : rprocessor)
    {
        if (proc->getId() == Id) return proc;
    }
    
    return add ? addResonanceProcessor(Id) : nullptr;
}


NostalgicProcessor::Ptr Piano::addNostalgicProcessor(int thisId)
{
    NostalgicProcessor::Ptr nproc = new NostalgicProcessor(processor.gallery->getNostalgic(thisId),
                                       defaultT,
                                       defaultS,
										defaultBA,
                                       &processor.mainPianoSynth);
    nproc->prepareToPlay(processor.getCurrentSampleRate(), &processor.mainPianoSynth);
    nprocessor.add(nproc);
    
    return nproc;
}

DirectProcessor::Ptr Piano::addDirectProcessor(int thisId)
{
    DirectProcessor::Ptr dproc = new DirectProcessor(processor.gallery->getDirect(thisId),
                                                     defaultT,
                                                     defaultBA,
                                                     &processor.mainPianoSynth,
                                                     &processor.resonanceReleaseSynth,
                                                     &processor.hammerReleaseSynth);
    
    dproc->prepareToPlay(processor.getCurrentSampleRate(),
                         &processor.mainPianoSynth,
                         &processor.resonanceReleaseSynth,
                         &processor.hammerReleaseSynth);
    
    dprocessor.add(dproc);
    
    return dproc;
}

TuningProcessor::Ptr Piano::addTuningProcessor(int thisId)
{
    TuningProcessor::Ptr tproc = new TuningProcessor(processor, processor.gallery->getTuning(thisId));
    tproc->prepareToPlay(processor.getCurrentSampleRate());
    tprocessor.add(tproc);
    
    return tproc;
}

TempoProcessor::Ptr Piano::addTempoProcessor(int thisId)
{
    TempoProcessor::Ptr mproc = new TempoProcessor(processor, processor.gallery->getTempo(thisId));
    mproc->prepareToPlay(processor.getCurrentSampleRate());
    mprocessor.add(mproc);

    return mproc;
}

BlendronicProcessor::Ptr Piano::addBlendronicProcessor(int thisId)
{
	BlendronicProcessor::Ptr bproc = new BlendronicProcessor(processor.gallery->getBlendronic(thisId),
                                                             defaultM,
                                                             processor.gallery->getGeneralSettings(),
                                                             &processor.mainPianoSynth);
	bproc->prepareToPlay(processor.getCurrentSampleRate());
	bprocessor.add(bproc);
    processor.mainPianoSynth.addBlendronicProcessor(bproc);

	return bproc;
}

ResonanceProcessor::Ptr Piano::addResonanceProcessor(int thisId)
{
    ResonanceProcessor::Ptr rproc = new ResonanceProcessor(processor.gallery->getResonance(thisId), defaultT, processor.gallery->getGeneralSettings(), &processor.mainPianoSynth);
    rproc->prepareToPlay(processor.getCurrentSampleRate());
    rprocessor.add(rproc);
    
    return rproc;
}

void Piano::reset(void)
{
    configure();
}

bool Piano::containsProcessor(BKPreparationType thisType, int thisId)
{
    if (thisType == PreparationTypeDirect)
    {
        return (getDirectProcessor(thisId) == nullptr) ? false : true;
    }
    else if (thisType == PreparationTypeSynchronic)
    {
        return (getSynchronicProcessor(thisId) == nullptr) ? false : true;
    }
    else if (thisType == PreparationTypeNostalgic)
    {
        return (getNostalgicProcessor(thisId) == nullptr) ? false : true;
    }
    else if (thisType == PreparationTypeResonance)
    {
        return (getResonanceProcessor(thisId) == nullptr) ? false : true;
    }
    else if (thisType == PreparationTypeTuning)
    {
        return (getTuningProcessor(thisId) == nullptr) ? false : true;
    }
    else if (thisType == PreparationTypeTempo)
    {
        return (getTempoProcessor(thisId) == nullptr) ? false : true;
    }
    
    return false;
}

void Piano::addProcessor(BKPreparationType thisType, int thisId)
{
    if (thisType == PreparationTypeDirect)
    {
        addDirectProcessor(thisId);
    }
    else if (thisType == PreparationTypeSynchronic)
    {
        addSynchronicProcessor(thisId);
    }
    else if (thisType == PreparationTypeNostalgic)
    {
        addNostalgicProcessor(thisId);
    }
    else if (thisType == PreparationTypeTuning)
    {
        addTuningProcessor(thisId);
    }
    else if (thisType == PreparationTypeTempo)
    {
        addTempoProcessor(thisId);
    }
	else if (thisType == PreparationTypeBlendronic)
	{
		addBlendronicProcessor(thisId);
	}
    else if (thisType == PreparationTypeResonance)
    {
        addResonanceProcessor(thisId);
    }
}

bool Piano::contains(BKItem::Ptr thisItem)
{
    for (auto item : items) if (item == (BKItem *)thisItem) return true;
    
    return false;
}

void Piano::add(BKItem::Ptr item, bool configureIfAdded)
{
    bool added = items.addIfNotAlreadyThere(item);
    
    if (added && configureIfAdded)  configure();
}

void Piano::remove(BKItem::Ptr item)
{
    bool removed = false;
    for (int i = items.size(); --i >= 0; )
    {
        items[i]->print();

        if (items[i] == item)
        {
            items.remove(i);
            removed = true;
        }
    }
    
    if (removed) configure();
}

void Piano::linkPreparationWithTempo(BKPreparationType thisType, int thisId, Tempo::Ptr thisTempo)
{
    TempoProcessor::Ptr mproc = getTempoProcessor(thisTempo->getId());
    
    if (thisType == PreparationTypeSynchronic)
    {
        SynchronicProcessor::Ptr sproc = getSynchronicProcessor(thisId);
        
        sproc->setTempo(mproc);
    }
    else if (thisType == PreparationTypeBlendronic)
    {
        BlendronicProcessor::Ptr bproc = getBlendronicProcessor(thisId);
        
        bproc->setTempo(mproc);
    }
}

void Piano::linkNostalgicWithSynchronic(Nostalgic::Ptr nostalgic, Synchronic::Ptr synchronic)
{
    NostalgicProcessor::Ptr proc = getNostalgicProcessor(nostalgic->getId());
    
    proc->setSynchronic(getSynchronicProcessor(synchronic->getId()));
}

void Piano::linkPreparationWithTuning(BKPreparationType thisType, int thisId, Tuning::Ptr thisTuning)
{
    TuningProcessor::Ptr tproc = getTuningProcessor(thisTuning->getId());
    
    //DBG("linking Tuning and SOMETHING " + String(thisType));
    
    if (thisType == PreparationTypeDirect)
    {
        DirectProcessor::Ptr dproc = getDirectProcessor(thisId);
        
        dproc->setTuning(tproc);
    }
    else if (thisType == PreparationTypeSynchronic)
    {
        SynchronicProcessor::Ptr sproc = getSynchronicProcessor(thisId);
        
        sproc->setTuning(tproc);
    }
    else if (thisType == PreparationTypeNostalgic)
    {
        NostalgicProcessor::Ptr nproc = getNostalgicProcessor(thisId);
        
        nproc->setTuning(tproc);
    }
    else if (thisType == PreparationTypeResonance)
    {
        //DBG("linking Tuning and Resonance");
        ResonanceProcessor::Ptr rproc = getResonanceProcessor(thisId);
        rproc->setTuning(tproc);
    }
}

void Piano::linkPreparationWithBlendronic(BKPreparationType thisType, int thisId, Blendronic::Ptr thisBlend)
{
	BlendronicProcessor::Ptr bproc = getBlendronicProcessor(thisBlend->getId());

	if (thisType == PreparationTypeDirect)
	{
		DirectProcessor::Ptr dproc = getDirectProcessor(thisId);
		
		dproc->addBlendronic(bproc);
	}
	else if (thisType == PreparationTypeSynchronic)
	{
		SynchronicProcessor::Ptr sproc = getSynchronicProcessor(thisId);

		sproc->addBlendronic(bproc);
	}
	else if (thisType == PreparationTypeNostalgic)
	{
		NostalgicProcessor::Ptr nproc = getNostalgicProcessor(thisId);

		nproc->addBlendronic(bproc);
	}
    else if (thisType == PreparationTypeResonance)
    {
        ResonanceProcessor::Ptr rproc = getResonanceProcessor(thisId);
        rproc->addBlendronic(bproc);
    }
}

void Piano::linkPreparationWithKeymap(BKPreparationType thisType, int thisId, int keymapId)
{
    Keymap::Ptr keymap = processor.gallery->getKeymap(keymapId);
    
    prepMap->addKeymap(keymap);
    
    if (thisType == PreparationTypeDirect)
    {
        DirectProcessor::Ptr dproc = getDirectProcessor(thisId);
        prepMap->addDirectProcessor(dproc);
    }
    else if (thisType == PreparationTypeSynchronic)
    {
        SynchronicProcessor::Ptr sproc = getSynchronicProcessor(thisId);
        prepMap->addSynchronicProcessor(sproc);
    }
    else if (thisType == PreparationTypeNostalgic)
    {
        NostalgicProcessor::Ptr nproc = getNostalgicProcessor(thisId);
        prepMap->addNostalgicProcessor(nproc);
    }
    else if (thisType == PreparationTypeBlendronic)
    {
        BlendronicProcessor::Ptr bproc = getBlendronicProcessor(thisId);
        prepMap->addBlendronicProcessor(bproc);
    }
    else if (thisType == PreparationTypeTempo)
    {
        TempoProcessor::Ptr mproc = getTempoProcessor(thisId);
        prepMap->addTempoProcessor(mproc);
    }
    else if (thisType == PreparationTypeTuning)
    {
        TuningProcessor::Ptr tproc = getTuningProcessor(thisId);
        prepMap->addTuningProcessor(tproc);
    }
    else if (thisType == PreparationTypeResonance)
    {
        ResonanceProcessor::Ptr rproc = getResonanceProcessor(thisId);
        prepMap->addResonanceProcessor(rproc);
    }
    prepMap->linkKeymapToPreparation(keymapId, thisType, thisId);
}

void Piano::configureDirectModification(DirectModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);
    
    for (auto keymap : whichKeymaps)
    {
        for (auto key : processor.gallery->getKeymap(keymap)->keys())
        {
            modificationMap[key]->addDirectModification(mod);
        }
    }
}

void Piano::configureReset(BKItem::Ptr item)
{
    Array<bool> otherKeys;
    
    Array<int> whichKeymaps = item->getConnectionIdsOfType(PreparationTypeKeymap);
    
    Array<int> direct = item->getConnectionIdsOfType(PreparationTypeDirect);
    Array<int> nostalgic = item->getConnectionIdsOfType(PreparationTypeNostalgic);
    Array<int> synchronic = item->getConnectionIdsOfType(PreparationTypeSynchronic);
    Array<int> tempo = item->getConnectionIdsOfType(PreparationTypeTempo);
    Array<int> tuning = item->getConnectionIdsOfType(PreparationTypeTuning);
	Array<int> blendronic = item->getConnectionIdsOfType(PreparationTypeBlendronic);
    Array<int> resonance = item->getConnectionIdsOfType(PreparationTypeResonance);
    
    Array<int> directMod = item->getConnectionIdsOfType(PreparationTypeDirectMod);
    Array<int> nostalgicMod = item->getConnectionIdsOfType(PreparationTypeNostalgicMod);
    Array<int> synchronicMod = item->getConnectionIdsOfType(PreparationTypeSynchronicMod);
    Array<int> tempoMod = item->getConnectionIdsOfType(PreparationTypeTempoMod);
    Array<int> tuningMod = item->getConnectionIdsOfType(PreparationTypeTuningMod);
    Array<int> blendronicMod = item->getConnectionIdsOfType(PreparationTypeBlendronicMod);
    Array<int> resonanceMod = item->getConnectionIdsOfType(PreparationTypeResonanceMod);

    Modifications::Reset resetWithKeymaps;
    
    for (auto keymap : whichKeymaps)
    {
        resetWithKeymaps.keymapIds.addIfNotAlreadyThere(keymap);
    }
    
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        
        for (auto key : thisKeymap->keys())
        {
            for (auto id : direct)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->directResets.add(resetToAdd);
            }
            for (auto id : synchronic)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->synchronicResets.add(resetToAdd);
            }
            for (auto id : nostalgic)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->nostalgicResets.add(resetToAdd);
            }
            for (auto id : tuning)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->tuningResets.add(resetToAdd);
            }
            for (auto id : tempo)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->tempoResets.add(resetToAdd);
            }
			for (auto id : blendronic)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->blendronicResets.add(resetToAdd);
            }
            for (auto id : resonance)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->resonanceResets.add(resetToAdd);
            }
            for (auto id : directMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->directModResets.add(resetToAdd);
            }
            for (auto id : synchronicMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->synchronicModResets.add(resetToAdd);
            }
            for (auto id : nostalgicMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->nostalgicModResets.add(resetToAdd);
            }
            for (auto id : resonanceMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->resonanceModResets.add(resetToAdd);
            }
            for (auto id : tuningMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->tuningModResets.add(resetToAdd);
            }
            for (auto id : tempoMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->tempoModResets.add(resetToAdd);
            }
            for (auto id : blendronicMod)
            {
                Modifications::Reset resetToAdd = resetWithKeymaps;
                resetToAdd.prepId = id;
                modificationMap[key]->blendronicModResets.add(resetToAdd);
            }
        }
    }
}

void Piano::configurePianoMap(BKItem::Ptr map)
{
    Array<int> whichKeymaps = map->getConnectionIdsOfType(PreparationTypeKeymap);
    
    Modifications::PianoMap pianoMap;
    
    pianoMap.pianoTarget = map->getPianoTarget();
    
    if (pianoMap.pianoTarget == getId()) return;
    
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        pianoMap.keymaps.addIfNotAlreadyThere(thisKeymap);
    }
    
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        
        for (auto key : thisKeymap->keys())
        {
            modificationMap[key]->pianoMaps.add(pianoMap);
        }
    }
}

void Piano::configureModification(BKItem::Ptr map)
{
    DBG("Piano::configureModification");
    map->print();
    
    BKPreparationType modType = map->getType();
    BKPreparationType targetType = modToPrepType(modType);
    int Id = map->getId();
    
    Array<int> whichPreps = map->getConnectionIdsOfType(targetType);
    Array<int> whichKeymaps = map->getConnectionIdsOfType(PreparationTypeKeymap);
    
    // DBG("keymaps: " + intArrayToString(whichKeymaps) + " preps: " + intArrayToString(whichPreps));
    
    if (modType == BKPreparationTypeNil) return;
    else if (modType == PreparationTypeDirectMod)
    {
        configureDirectModification(processor.gallery->getDirectModification(Id), whichKeymaps, whichPreps);
    }
    else if (modType == PreparationTypeSynchronicMod)
    {
        configureSynchronicModification(processor.gallery->getSynchronicModification(Id), whichKeymaps, whichPreps);
    }
    else if (modType == PreparationTypeNostalgicMod)
    {
        configureNostalgicModification(processor.gallery->getNostalgicModification(Id), whichKeymaps, whichPreps);
    }
    else if (modType == PreparationTypeTuningMod)
    {
        configureTuningModification(processor.gallery->getTuningModification(Id), whichKeymaps, whichPreps);
    }
    else if (modType == PreparationTypeTempoMod)
    {
        configureTempoModification(processor.gallery->getTempoModification(Id), whichKeymaps, whichPreps);
    }
	else if (modType == PreparationTypeBlendronicMod)
	{
		configureBlendronicModification(processor.gallery->getBlendronicModification(Id), whichKeymaps, whichPreps);
	}
    else if (modType == PreparationTypeResonanceMod)
    {
        configureResonanceModification(processor.gallery->getResonanceModification(Id), whichKeymaps, whichPreps);
    }

}

void Piano::configureNostalgicModification(NostalgicModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);

    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);
    
    for (auto keymap : whichKeymaps)
    {
        for (auto key : processor.gallery->getKeymap(keymap)->keys())
        {
            modificationMap[key]->addNostalgicModification(mod);
        }
    }
}

void Piano::configureSynchronicModification(SynchronicModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);
    
    for (auto keymap : whichKeymaps)
    {
        for (auto key : processor.gallery->getKeymap(keymap)->keys())
        {
            modificationMap[key]->addSynchronicModification(mod);
        }
    }
}

void Piano::configureTempoModification(TempoModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);
    
    for (auto keymap : whichKeymaps)
    {
        for (auto key : processor.gallery->getKeymap(keymap)->keys())
        {
            modificationMap[key]->addTempoModification(mod);
        }
    }
}

void Piano::configureBlendronicModification(BlendronicModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
	mod->setTargets(whichPreps);
    
    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);

	for (auto keymap : whichKeymaps)
	{
		for (auto key : processor.gallery->getKeymap(keymap)->keys())
		{
			modificationMap[key]->addBlendronicModification(mod);
		}
	}
}

void Piano::configureResonanceModification(ResonanceModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    DBG("Piano::configureResonanceModification");
    mod->setTargets(whichPreps);
    
    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);

    for (auto keymap : whichKeymaps)
    {
        for (auto key : processor.gallery->getKeymap(keymap)->keys())
        {
            modificationMap[key]->addResonanceModification(mod);
        }
    }
}

void Piano::configureTuningModification(TuningModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);

    Keymap::PtrArr keymaps;
    for (auto keymap : whichKeymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        keymaps.add(thisKeymap);
    }
    
    mod->setKeymaps(keymaps);
    
    for (auto keymap : whichKeymaps)
    {
        for (auto key : processor.gallery->getKeymap(keymap)->keys())
        {
            modificationMap[key]->addTuningModification(mod);
        }
    }
}

void Piano::prepareToPlay(double sr)
{
    double sampleRate = processor.getCurrentSampleRate();
    
    for (auto dproc : dprocessor)
        dproc->prepareToPlay(sampleRate, &processor.mainPianoSynth, &processor.resonanceReleaseSynth, &processor.hammerReleaseSynth);
    
    for (auto mproc : mprocessor)
        mproc->prepareToPlay(sampleRate);
    
    for (auto nproc : nprocessor)
        nproc->prepareToPlay(sampleRate, &processor.mainPianoSynth);
    
    for (auto sproc : sprocessor)
        sproc->prepareToPlay(sampleRate, &processor.mainPianoSynth);
    
    for (auto tproc : tprocessor)
        tproc->prepareToPlay(sampleRate);

	for (auto bproc : bprocessor)
		bproc->prepareToPlay(sampleRate);
    
    for (auto rproc : rprocessor)
        rproc->prepareToPlay(sampleRate);
}

ValueTree Piano::getState(void)
{
    ValueTree pianoVT( vtagPiano);
    
    String name = getName();
    pianoVT.setProperty("name", ((name == String()) ? "Piano"+String(Id) : name) , 0);
    
    pianoVT.setProperty("Id", Id, 0);
    
    for (auto item : items)
    {
        BKPreparationType type = item->getType();
        
        ValueTree itemVT("item");
        ValueTree connectionsVT("connections");
        
        if (type == PreparationTypeKeymap)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            for (auto target : item->getConnections())
            {
                connectionsVT.addChild(target->getState(), -1, 0);
            }
            
            itemVT.addChild(connectionsVT, -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        else if (type == PreparationTypeTuning)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            // Look for synchronic, direct, blendronic, and nostalgic targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                
                if (((targetType >= PreparationTypeDirect && targetType <= PreparationTypeNostalgic) ||
                     targetType == PreparationTypeResonance))
                {
                    connectionsVT.addChild(target->getState(), -1, 0);
                }
            }
            
            itemVT.addChild(connectionsVT, -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        else if (type == PreparationTypeTempo)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            // Look for synchronic targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                
                if (targetType == PreparationTypeSynchronic || targetType == PreparationTypeBlendronic  )
                {
                    connectionsVT.addChild(target->getState(), -1, 0);
                }
            }
            
            itemVT.addChild(connectionsVT, -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        // Adding these in so they don't disappear on save/load, but we don't need to get their connections
        else if (type == PreparationTypeDirect)
        {
            itemVT.addChild(item->getState(), -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        else if (type == PreparationTypeNostalgic)
        {
            itemVT.addChild(item->getState(), -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        else if (type == PreparationTypeResonance)
        {
            itemVT.addChild(item->getState(), -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        else if (type == PreparationTypeSynchronic)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            // Look for nostalgic targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                
                if (targetType == PreparationTypeNostalgic)
                {
                    connectionsVT.addChild(target->getState(), -1, 0);
                }
            }
            
            itemVT.addChild(connectionsVT, -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
		else if (type == PreparationTypeBlendronic)
		{
			itemVT.addChild(item->getState(), -1, 0);
			//look for direct, nostalgic, and synchronic targets
			for (auto target : item->getConnections())
			{
				BKPreparationType targetType = target->getType();

				if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeNostalgic) ||
                    targetType == PreparationTypeBlendronicMod)
				{
					connectionsVT.addChild(target->getState(), -1, 0);
				}
			}
			itemVT.addChild(connectionsVT, -1, 0);
			pianoVT.addChild(itemVT, -1, 0);
		}
        else if ((type >= PreparationTypeDirectMod && type <= PreparationTypeTempoMod) ||
                 type == PreparationTypeReset ||
                 type == PreparationTypeBlendronicMod ||
                 type == PreparationTypeResonanceMod)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            // Look for non-Keymap connections
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                
                if (targetType != PreparationTypeKeymap)
                {
                    connectionsVT.addChild(target->getState(), -1, 0);
                }
            }
            
            itemVT.addChild(connectionsVT, -1, 0);
            pianoVT.addChild(itemVT, -1, 0);
        }
        else if (type == PreparationTypeComment ||
                 type == PreparationTypeGenericMod ||
                 type == PreparationTypePianoMap)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            pianoVT.addChild(itemVT, -1, 0);
        }
    }
    
    return pianoVT;
}
#define LOAD_VERSION 0

void Piano::setState(XmlElement* e, OwnedArray<HashMap<int,int>>* idmap, int* idcounts)
{
    int i = 0;
    
    //Id = e->getStringAttribute("Id").getIntValue();
    //setId(e->getStringAttribute("Id").getIntValue());
    
    String pianoName = e->getStringAttribute("name");
    
    if (pianoName != String()) setName(pianoName);
    
    BKItem::Ptr thisItem;
    BKItem::Ptr thisConnection;

    for (auto group : e->getChildIterator())
    {
        for (auto item : group->getChildIterator())
        {
            if (item->getTagName() == "item")
            {
                i = item->getStringAttribute("type").getIntValue();
                BKPreparationType type = cPreparationIdToType[i];
                
                if (type == PreparationTypeComment)
                {
                    thisItem = new BKItem(PreparationTypeComment, -1, processor);
                    
                    thisItem->setItemName("Comment");
                    
                    i =  (int) atof(item->getStringAttribute("X").getCharPointer());
                    int x = i;
                    
                    i =  (int) atof(item->getStringAttribute("Y").getCharPointer());
                    int y = i;
                    
                    i =  (int) atof(item->getStringAttribute("W").getCharPointer());
                    int w = i;
                    
                    i =  (int) atof(item->getStringAttribute("H").getCharPointer());
                    int h = i;
                    
                    String s = item->getStringAttribute("text");
                    thisItem->setCommentText(s);
                    
                    thisItem->setSize(w, h);
                    thisItem->setCentrePosition(x, y);
                    
                    items.add(thisItem);
                }
                else
                {
                    int oldId = item->getStringAttribute("Id").getIntValue();
                    
                    int thisId;
                    if (idmap->getUnchecked(type)->contains(oldId))
                    {
                        thisId = idmap->getUnchecked(type)->getReference(oldId);
                    }
                    else
                    {
                        thisId = idcounts[type]++;
                        idmap->getUnchecked(type)->set(oldId, thisId);
                    }
                
                    i = item->getStringAttribute("piano").getIntValue();
                    int piano = i;
                    
                    if (idmap->getUnchecked(PreparationTypePiano)->contains(piano))
                    {
                        piano = idmap->getUnchecked(PreparationTypePiano)->getReference(piano);
                    }
                    
                    thisItem = itemWithTypeAndId(type, thisId);
                    
                    if (thisItem == nullptr)
                    {
                        thisItem = new BKItem(type, thisId, processor);
                        
                        thisItem->setPianoTarget(piano);
                        thisItem->setItemName(item->getStringAttribute("name"));
                        
                        i =  (int) atof(item->getStringAttribute("X").getCharPointer());
                        int x = i;
                        
                        i =  (int) atof(item->getStringAttribute("Y").getCharPointer());
                        int y = i;
                        
                        thisItem->setCentrePosition(x, y);
                        
                        i = item->getStringAttribute("active").getIntValue();
                        bool active = (bool)i;
                        
                        thisItem->setActive(active);
                        
                        items.add(thisItem);
                    }
                }
                
                
            }
            
            XmlElement* connections = group->getChildByName("connections");
            
            if (connections != nullptr)
            {
                for (auto connection : connections->getChildIterator())
                {
                    i = connection->getStringAttribute("type").getIntValue();
                    BKPreparationType cType = cPreparationIdToType[i];
                    
                    i = connection->getStringAttribute("Id").getIntValue();
                    int cId = i;
                    int oldId = cId;
                    
                    if (idmap->getUnchecked(cType)->contains(oldId))
                    {
                        cId = idmap->getUnchecked(cType)->getReference(oldId);
                    }
                    else
                    {
                        cId = idcounts[cType]++;
                        idmap->getUnchecked(cType)->set(oldId, cId);
                    }
                    
                    i = connection->getStringAttribute("piano").getIntValue();
                    int cPiano = i;
                    // DBG("conn piano target old: " + String(cPiano));
                    
                    if (idmap->getUnchecked(PreparationTypePiano)->contains(cPiano))
                    {
                        cPiano = idmap->getUnchecked(PreparationTypePiano)->getReference(cPiano);
                    }
                    
                    // DBG("conn piano target new: " + String(cPiano));
                    
                    thisConnection = itemWithTypeAndId(cType, cId);
                    
                    if (thisConnection == nullptr)
                    {
                        
                        thisConnection = new BKItem(cType, cId, processor);
                        
                        thisConnection->setItemName(connection->getStringAttribute("name"));
                        
                        thisConnection->setPianoTarget(cPiano);
                        
                        i =  (int) atof(connection->getStringAttribute("X").getCharPointer());
                        int x = i;
                        
                        i =  (int) atof(connection->getStringAttribute("Y").getCharPointer());
                        int y = i;
                        
                        thisConnection->setCentrePosition(x, y);
                        
                        i = connection->getStringAttribute("active").getIntValue();
                        bool active = (bool)i;
                        
                        thisConnection->setActive(active);
                        
                        items.add(thisConnection);
                    }
                    
                    thisItem->addConnection(thisConnection);
                    thisConnection->addConnection(thisItem);
                }
            }
        }
    }
    
#if JUCE_IOS
    for (auto item : items)
    {
        DBG("centre x: " + String(item->getX() + item->getWidth() / 2));
        item->setCentrePosition((item->getX() + item->getWidth() / 2) * processor.uiScaleFactor, (item->getY() + item->getHeight() / 2) * processor.uiScaleFactor);
    }
#endif

}
