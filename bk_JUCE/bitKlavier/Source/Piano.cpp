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
prepMap(PreparationMap::Ptr()),
processor(p),
Id(Id)
{
    numPMaps = 0;
    pianoMap.ensureStorageAllocated(128);
    
    modificationMap = OwnedArray<Modifications>();
    modificationMap.ensureStorageAllocated(128);
    
    for (int i = 0; i < 128; i++)
    {
        pianoMap.set(i, -1);
        
        pianoMaps.add(Array<int>());
        pianoMaps.add(Array<int>());
        
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
        BKItem* newItem = newItems.getUnchecked(idx++);
        
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
    prepMap = nullptr;
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
    
    for (int key = 0; key < 128; key++)
    {
        modificationMap[key]->clearModifications();
        modificationMap[key]->clearResets();
        pianoMap.set(key, -1);
    }
    pianoMapInputs.clear();
}

#define DEFAULT_ID -1
void Piano::configure(void)
{
    deconfigure();
    
    defaultT = getTuningProcessor(DEFAULT_ID);
    
    defaultM = getTempoProcessor(DEFAULT_ID);
    
    defaultS = getSynchronicProcessor(DEFAULT_ID);
    
    //defaultB = getBlendronicProcessor(DEFAULT_ID);
    
    for (auto item : items)
    {
        BKPreparationType thisType = item->getType();
        int thisId = item->getId();
        
        if (thisId > processor.gallery->getIdCount(thisType)) processor.gallery->setIdCount(thisType, thisId);
        
        addProcessor(thisType, thisId);
    }
    
    for (auto item : items)
    {
        BKPreparationType type = item->getType();
        int Id = item->getId();
        
        //TODO: figure out connecting blendronic
        // Connect keymaps to everything
        // Connect tunings, tempos, synchronics to preparations
        // Connect mods and resets to all their targets
        // Configure piano maps
        // ... should be all configured if done in that order ...
        if (type == PreparationTypeKeymap)
        {
            Keymap::Ptr keymap = processor.gallery->getKeymap(Id);
            // make a prep map for the first keymap, this will be the only prep map
            if (prepMap == nullptr) addPreparationMap(keymap);
            // add any subsequent keymaps to the same prep map (maybe we can get rid of prep maps?)
            else prepMap->addKeymap(keymap);
            
            BKItem::PtrArr connex = item->getConnections();
            for (auto target : connex)
            {
                BKPreparationType targetType = target->getType();
                int targetId = target->getId();
                
                if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeTempo) || targetType == PreparationTypeBlendronic)
                {
                    DBG(String(targetType) + " linked with keymap");
                    linkPreparationWithKeymap(targetType, targetId, Id);
                }
                else if ((targetType >= PreparationTypeDirectMod && targetType <= PreparationTypeTempoMod) || targetType == PreparationTypeBlendronicMod)
                {
                    configureModification(target);
                }
                else if (targetType == PreparationTypePianoMap)
                {
                    configurePianoMap(target);
                }
                else if (targetType == PreparationTypeReset)
                {
                    configureReset(target);
                }
            }
            
            connex.clear();

        }
        else if (type == PreparationTypeTuning)
        {
            // Look for synchronic, direct, nostalgic, and blendronic targets
            for (auto target : item->getConnections())
            {
                BKPreparationType targetType = target->getType();
                int targetId = target->getId();
                
                if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeNostalgic))
                {
                    linkPreparationWithTuning(targetType, targetId, processor.gallery->getTuning(Id));
                }
            }
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
                    targetType == PreparationTypeBlendronicMod)
				{
					linkPreparationWithBlendronic(targetType, targetId, processor.gallery->getBlendronic(Id));
				}
			}
		}
    }
    
    //processor.updateState->pianoDidChangeForGraph = true;
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
}

bool Piano::contains(BKItem::Ptr thisItem)
{
    for (auto item : items) if (item == (BKItem *)thisItem) return true;
    
    return false;
}

void Piano::add(BKItem::Ptr item)
{
    bool added = items.addIfNotAlreadyThere(item);
    
    if (added)  configure();
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
}

void Piano::linkPreparationWithKeymap(BKPreparationType thisType, int thisId, int keymapId)
{
    // Get the preparation map for this keymap
    PreparationMap::Ptr thisPreparationMap = getPreparationMapWithKeymap(keymapId);
    PreparationMap::Ptr thatPreparationMap = getPreparationMapWithPreparation(thisType, thisId);
    
    Keymap::Ptr keymap = processor.gallery->getKeymap(keymapId);
    
    // If this keymap doesn't have a prep map
    if (thisPreparationMap == nullptr)
    {
        // If what it's connecting to doesn't have a prep map
        if (thatPreparationMap == nullptr)
        {
            // Make a new preparation map
            addPreparationMap(keymap);
        }
        else
        {   // Add this keymap to the prep map
            thatPreparationMap->addKeymap(keymap);
            thisPreparationMap = thatPreparationMap;
        }
    }
    
    if (thisType == PreparationTypeDirect)
    {
        DirectProcessor::Ptr dproc = getDirectProcessor(thisId);
        thisPreparationMap->addDirectProcessor(dproc);
        
        keymap->addTarget(TargetTypeDirect);
    }
    else if (thisType == PreparationTypeSynchronic)
    {
        SynchronicProcessor::Ptr sproc = getSynchronicProcessor(thisId);
        thisPreparationMap->addSynchronicProcessor(sproc);
        
        keymap->addTarget(TargetTypeSynchronic);
        for (int i = TargetTypeSynchronic+1; i <= TargetTypeSynchronicRotate; i++)
        {
            keymap->addTarget((KeymapTargetType) i, TargetStateDisabled);
        }
    }
    else if (thisType == PreparationTypeNostalgic)
    {
        NostalgicProcessor::Ptr nproc = getNostalgicProcessor(thisId);
        thisPreparationMap->addNostalgicProcessor(nproc);
        
        keymap->addTarget(TargetTypeNostalgic);
    }
    else if (thisType == PreparationTypeBlendronic)
    {
        BlendronicProcessor::Ptr bproc = getBlendronicProcessor(thisId);
        thisPreparationMap->addBlendronicProcessor(bproc);
        
        for (int i = TargetTypeBlendronicPatternSync; i <= TargetTypeBlendronicOpenCloseOutput; i++)
        {
            keymap->addTarget((KeymapTargetType) i, TargetStateDisabled);
        }
    }
    else if (thisType == PreparationTypeTempo)
    {
        TempoProcessor::Ptr mproc = getTempoProcessor(thisId);
        thisPreparationMap->addTempoProcessor(mproc);
        
        keymap->addTarget(TargetTypeTempo);
    }
    else if (thisType == PreparationTypeTuning)
    {
        TuningProcessor::Ptr tproc = getTuningProcessor(thisId);
        thisPreparationMap->addTuningProcessor(tproc);
        
        keymap->addTarget(TargetTypeTuning);
    }
    thisPreparationMap->linkKeymapToPreparation(keymapId, thisType, thisId);
}

void Piano::configureDirectModification(DirectModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Array<bool> otherKeys;
    for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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
        
            otherKeys.set(key, false);
        }
    }
    
    deconfigureDirectModificationForKeys(mod, otherKeys);
}

void Piano::deconfigureDirectModificationForKeys(DirectModification::Ptr mod, Array<bool> keys)
{
    int whichMod = mod->getId();

    for (int key = 0; key < 128; key++)
    {
        if (keys[key])
        {
            // Remove Modification from Key
            modificationMap[key]->removeDirectModification(whichMod);
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

    for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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
            
            otherKeys.set(key, false);
        }
    }
    
    
    deconfigureResetForKeys(item, otherKeys);
    
}

void Piano::deconfigureResetForKeys(BKItem::Ptr item, Array<bool> otherKeys)
{
    Array<int> direct = item->getConnectionIdsOfType(PreparationTypeDirect);
    Array<int> nostalgic = item->getConnectionIdsOfType(PreparationTypeNostalgic);
    Array<int> synchronic = item->getConnectionIdsOfType(PreparationTypeSynchronic);
    Array<int> tempo = item->getConnectionIdsOfType(PreparationTypeTempo);
    Array<int> tuning = item->getConnectionIdsOfType(PreparationTypeTuning);
	Array<int> blendronic = item->getConnectionIdsOfType(PreparationTypeBlendronic);
    
    for (int key = 0; key < 128; key++)
    {
        if (otherKeys[key])
        {
            for (auto id : direct)
            {
                for (int i = modificationMap[key]->directResets.size(); --i>=0;)
                {
                    if (modificationMap[key]->directResets[i].prepId == id) modificationMap[key]->directResets.remove(i);
                }
            }
            
            for (auto id : synchronic)
            {
                for (int i = modificationMap[key]->synchronicResets.size(); --i>=0;)
                {
                    if (modificationMap[key]->synchronicResets[i].prepId == id) modificationMap[key]->synchronicResets.remove(i);
                }
            }
            
            for (auto id : nostalgic)
            {
                for (int i = modificationMap[key]->nostalgicResets.size(); --i>=0;)
                {
                    if (modificationMap[key]->nostalgicResets[i].prepId == id) modificationMap[key]->nostalgicResets.remove(i);
                }
            }
            
            for (auto id : tuning)
            {
                for (int i = modificationMap[key]->tuningResets.size(); --i>=0;)
                {
                    if (modificationMap[key]->tuningResets[i].prepId == id) modificationMap[key]->tuningResets.remove(i);
                }
            }
            
            for (auto id : tempo)
            {
                for (int i = modificationMap[key]->tempoResets.size(); --i>=0;)
                {
                    if (modificationMap[key]->tempoResets[i].prepId == id) modificationMap[key]->tempoResets.remove(i);
                }
            }

			for (auto id : blendronic)
			{
				for (int i = modificationMap[key]->blendronicResets.size(); --i>0;)
				{
					if (modificationMap[key]->blendronicResets[i].prepId == id) modificationMap[key]->blendronicResets.remove(i);
				}
			}
        }        
    }
}

void Piano::configurePianoMap(BKItem::Ptr map)
{
    int pianoTarget = map->getPianoTarget();
    
    if (pianoTarget == getId()) return;
    
    Array<int> keymaps = map->getConnectionIdsOfType(PreparationTypeKeymap);
    
    for (auto keymap : keymaps)
    {
        Keymap::Ptr thisKeymap = processor.gallery->getKeymap(keymap);
        for (auto source : thisKeymap->getAllMidiInputSources())
        {
            pianoMapInputs.addIfNotAlreadyThere(source);
        }
        for (auto key : thisKeymap->keys())
        {
            pianoMap.set(key, pianoTarget);
        }
    }
}

void Piano::configureModification(BKItem::Ptr map)
{
    map->print();
    
    BKPreparationType modType = map->getType();
    BKPreparationType targetType = modToPrepType(modType);
    int Id = map->getId();
    
    Array<int> whichPreps = map->getConnectionIdsOfType(targetType);
    
    Array<int> whichKeymaps = map->getConnectionIdsOfType(PreparationTypeKeymap);
    
    DBG("keymaps: " + intArrayToString(whichKeymaps) + " preps: " + intArrayToString(whichPreps));
    
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

}

void Piano::configureNostalgicModification(NostalgicModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Array<bool> otherKeys;
    for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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
            
            otherKeys.set(key, false);
        }
    }
    
    deconfigureNostalgicModificationForKeys(mod, otherKeys);
}

void Piano::deconfigureNostalgicModificationForKeys(NostalgicModification::Ptr mod, Array<bool> keys)
{
    int whichMod = mod->getId();
    
    for (int key = 0; key < 128; key++)
    {
        if (keys[key])
        {
            // Remove Modification from Key
            modificationMap[key]->removeNostalgicModification(whichMod);
        }
    }
}

void Piano::configureSynchronicModification(SynchronicModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Array<bool> otherKeys;
    for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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
            
            otherKeys.set(key,false);
        }
    }
    
    deconfigureSynchronicModificationForKeys(mod, otherKeys);
}

void Piano::deconfigureSynchronicModificationForKeys(SynchronicModification::Ptr mod, Array<bool> keys)
{
    int whichMod = mod->getId();

    for (int key = 0; key < 128; key++)
    {
        if (keys[key])
        {
            // Remove Modification from Key
            modificationMap[key]->removeSynchronicModification(whichMod);
        }
    }
}

void Piano::configureTempoModification(TempoModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Array<bool> otherKeys;
    for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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
            
            otherKeys.set(key, false);
        }
    }
    
    deconfigureTempoModificationForKeys(mod, otherKeys);
}

void Piano::deconfigureTempoModificationForKeys(TempoModification::Ptr mod, Array<bool> keys)
{
    int whichMod = mod->getId();
    
    for (int key = 0; key < 128; key++)
    {
        if (keys[key])
        {
            // Remove Modification from Key
            modificationMap[key]->removeTempoModification(whichMod);
        
            //DBG("REMOVE whichmod: " + String(whichMod) + " FROM key: " +String(key));
        }
    }
}

void Piano::configureBlendronicModification(BlendronicModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
	mod->setTargets(whichPreps);

	Array<bool> otherKeys;
	for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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

			otherKeys.set(key, false);
		}
	}

	deconfigureBlendronicModificationForKeys(mod, otherKeys);
}

void Piano::deconfigureBlendronicModificationForKeys(BlendronicModification::Ptr mod, Array<bool> keys)
{
	int whichMod = mod->getId();

	for (int key = 0; key < 128; key++)
	{
		if (keys[key])
		{
			// Remove Modification from Key
			modificationMap[key]->removeBlendronicModification(whichMod);

			//DBG("REMOVE whichmod: " + String(whichMod) + " FROM key: " +String(key));
		}
	}
}

void Piano::configureTuningModification(TuningModification::Ptr mod, Array<int> whichKeymaps, Array<int> whichPreps)
{
    mod->setTargets(whichPreps);
    
    Array<bool> otherKeys;
    for (int i = 0; i < 128; i++) otherKeys.add(true);
    
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
            
            otherKeys.set(key, false);
        }
    }

    deconfigureTuningModificationForKeys(mod, otherKeys);
}

void Piano::deconfigureTuningModificationForKeys(TuningModification::Ptr mod, Array<bool> keys)
{
    int whichMod = mod->getId();
    
    for (int key = 0; key < 128; key++)
    {
        if (keys[key])
        {
            // Remove Modification from Key
            modificationMap[key]->removeTuningModification(whichMod);
            
            DBG("REMOVE whichmod: " + String(whichMod) + " FROM key: " + String(key));
        }
        
    }
}

// Add preparation map, return its Id.
int Piano::addPreparationMap(void)
{
    prepMap = new PreparationMap(processor.gallery->getKeymap(0), numPMaps++);
    prepMap->prepareToPlay(processor.getCurrentSampleRate());
    return 0;
}

// Add preparation map, return its Id.
int Piano::addPreparationMap(Keymap::Ptr keymap)
{
    prepMap = new PreparationMap(keymap, numPMaps++);
    prepMap->prepareToPlay(processor.getCurrentSampleRate());
    return 0;
}

PreparationMap::Ptr        Piano::getPreparationMapWithPreparation(BKPreparationType type, int Id)
{
    PreparationMap::Ptr thisPMap = nullptr;

    if (type == PreparationTypeDirect)
    {
        if (prepMap->getDirectProcessor(Id) != nullptr)
        {
            thisPMap = prepMap;
        }
    }
    else if (type == PreparationTypeSynchronic)
    {
        if (prepMap->getSynchronicProcessor(Id) != nullptr)
        {
            thisPMap = prepMap;
        }
    }
    else if (type == PreparationTypeNostalgic)
    {
        if (prepMap->getNostalgicProcessor(Id) != nullptr)
        {
            thisPMap = prepMap;
        }
    }
    else if (type == PreparationTypeBlendronic)
    {
        if (prepMap->getBlendronicProcessor(Id) != nullptr)
        {
            thisPMap = prepMap;
        }
    }
    else if (type == PreparationTypeTempo)
    {
        if (prepMap->getTempoProcessor(Id) != nullptr)
        {
            thisPMap = prepMap;
        }
    }
    else if (type == PreparationTypeTuning)
    {
        if (prepMap->getTuningProcessor(Id) != nullptr)
        {
            thisPMap = prepMap;
        }
    }
    
    return thisPMap;
}

PreparationMap::Ptr        Piano::getPreparationMapWithKeymap(int keymapId)
{
    PreparationMap::Ptr thisPMap = nullptr;

    if (prepMap->getKeymap(keymapId) != nullptr)
    {
        thisPMap = prepMap;
    }
    
    return thisPMap;
}

int Piano::removePreparationMap(int Id)
{
    if (prepMap == nullptr) return 0;
    
    if (prepMap->getId() == Id)
        prepMap = nullptr;
    
    --numPMaps;
    
    return numPMaps;
}

// Add preparation map, return its Id.
int Piano::removePreparationMapWithKeymap(int Id)
{
    if (prepMap == nullptr) return 0;
    
    if (prepMap->getKeymap(Id) != nullptr)
        prepMap = nullptr;
    
    --numPMaps;
    
    return numPMaps;
}

// Add preparation map, return its Id.
int Piano::removeLastPreparationMap(void)
{
    if (prepMap == nullptr) return 0;
    
    prepMap = nullptr;
    
    --numPMaps;

    return numPMaps;
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
                
                if ((targetType >= PreparationTypeDirect && targetType <= PreparationTypeNostalgic))
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
                 type == PreparationTypeBlendronicMod)
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
        else if (type == PreparationTypeComment)
        {
            itemVT.addChild(item->getState(), -1, 0);
            
            pianoVT.addChild(itemVT, -1, 0);
        }
    }
    
    return pianoVT;
}
#define LOAD_VERSION 0

void Piano::setState(XmlElement* e, OwnedArray<HashMap<int,int>>* idmap)
{
    int i = 0;
    
    //Id = e->getStringAttribute("Id").getIntValue();
    //setId(e->getStringAttribute("Id").getIntValue());
    
    String pianoName = e->getStringAttribute("name");
    
    if (pianoName != String()) setName(pianoName);
    
    BKItem::Ptr thisItem;
    BKItem::Ptr thisConnection;

    forEachXmlChildElement (*e, group)
    {
        forEachXmlChildElement(*group, item)
        {
            if (item->getTagName() == "item")
            {
                i = item->getStringAttribute("type").getIntValue();
                BKPreparationType type = (BKPreparationType) i;
                
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
                    
                    int thisId = oldId;
                    
                    if (idmap->getUnchecked(type)->contains(oldId))
                    {
                        thisId = idmap->getUnchecked(type)->getReference(oldId);
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
                forEachXmlChildElement (*connections, connection)
                {
                    i = connection->getStringAttribute("type").getIntValue();
                    BKPreparationType cType = (BKPreparationType) i;
                    
                    i = connection->getStringAttribute("Id").getIntValue();
                    int cId = i;
                    
                    if (idmap->getUnchecked(cType)->contains(cId))
                    {
                        cId = idmap->getUnchecked(cType)->getReference(cId);
                    }
                    
                    i = connection->getStringAttribute("piano").getIntValue();
                    int cPiano = i;
                    DBG("conn piano target old: " + String(cPiano));
                    
                    if (idmap->getUnchecked(PreparationTypePiano)->contains(cPiano))
                    {
                        cPiano = idmap->getUnchecked(PreparationTypePiano)->getReference(cPiano);
                    }
                    
                    DBG("conn piano target new: " + String(cPiano));
                    
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
