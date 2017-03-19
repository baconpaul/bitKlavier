
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BKPianoSampler.h"

#define NOST_KEY_OFF 1

//==============================================================================
BKAudioProcessor::BKAudioProcessor():
general                 (new GeneralSettings()),
updateState             (new BKUpdateState()),
mainPianoSynth          (general),
hammerReleaseSynth      (general),
resonanceReleaseSynth   (general)
{
    didLoadHammersAndRes            = false;
    didLoadMainPianoSamples         = false;
    
    
    bkKeymaps       .ensureStorageAllocated(aMaxNumPreparationKeymaps);
    bkPianos        .ensureStorageAllocated(aMaxNumPianos);
    prevPianos      .ensureStorageAllocated(aMaxNumPianos);
    
    tempo          .ensureStorageAllocated(aMaxTempoPreparations);
    modTempo       .ensureStorageAllocated(aMaxTempoPreparations);
    
    tuning          .ensureStorageAllocated(aMaxTuningPreparations);
    modTuning       .ensureStorageAllocated(aMaxTuningPreparations);
    
    synchronic      .ensureStorageAllocated(aMaxTotalPreparations);
    modSynchronic   .ensureStorageAllocated(aMaxTotalPreparations);
    
    nostalgic       .ensureStorageAllocated(aMaxTotalPreparations);
    modNostalgic    .ensureStorageAllocated(aMaxTotalPreparations);
    
    direct          .ensureStorageAllocated(aMaxTotalPreparations);
    modDirect       .ensureStorageAllocated(aMaxTotalPreparations);
    
    
    bkKeymaps.add(new Keymap(0));
    bkKeymaps.add(new Keymap(1));
    
    // Start with one of each kind of preparation.
    for (int i = 0; i < 2; i++)
    {
        addTempo();
        addTuning(); // always create first tuning first
        addSynchronic();
        addNostalgic();
        addDirect();
        
        addTempoMod();
        addTuningMod();
        addDirectMod();
        addSynchronicMod();
        addNostalgicMod();
        
        nostalgic[i]->sPrep->setSyncTargetProcessor(synchronic[0]->processor);
        nostalgic[i]->aPrep->setSyncTargetProcessor(synchronic[0]->processor);
    }
    
    // Default all on for
    for (int i = 0; i < 128; i++) bkKeymaps[1]->addNote(i);
    
    // Make a piano.
    bkPianos.add(new Piano(synchronic, nostalgic, direct, tuning, tempo,
                           modSynchronic, modNostalgic, modDirect, modTuning, modTempo,
                           bkKeymaps, 0)); // initializing piano 0
    
    // Initialize first piano.
    prevPiano = bkPianos[0];
    currentPiano = bkPianos[0];
    
    collectGalleries();
    
    loadGalleryFromPath(galleryNames[0]);
}

void BKAudioProcessor::collectGalleries(void)
{
    galleryNames.clear();
    
    DirectoryIterator xmlIter (File ("~/bk"), true, "*.xml");
    while (xmlIter.next())
    {
        File galleryFile (xmlIter.getFile());
        
        galleryNames.add(galleryFile.getFullPathName());
    }
    
    
    DirectoryIterator jsonIter (File ("~/bk"), true, "*.json");
    while (jsonIter.next())
    {
        File galleryFile (jsonIter.getFile());
        
        galleryNames.add(galleryFile.getFullPathName());
    }
}

void BKAudioProcessor::updateGalleries()
{
    collectGalleries();
    
    updateState->galleryDidChange = true;
}

void BKAudioProcessor::addPiano()
{
    int numPianos = bkPianos.size();
    bkPianos.add(new Piano(synchronic, nostalgic, direct, tuning, tempo,
                           modSynchronic, modNostalgic, modDirect, modTuning, modTempo,
                           bkKeymaps, numPianos));
}

void BKAudioProcessor::addDirectMod()
{
    modDirect.add           (new DirectModPreparation());
}

void BKAudioProcessor::addSynchronicMod()
{
    modSynchronic.add       (new SynchronicModPreparation());
}

void BKAudioProcessor::addNostalgicMod()
{
    modNostalgic.add        (new NostalgicModPreparation());
}

void BKAudioProcessor::addTuningMod()
{
    modTuning.add           (new TuningModPreparation());
}

void BKAudioProcessor::addTuningMod(TuningModPreparation::Ptr tmod)
{
    modTuning.add           (tmod);
}

void BKAudioProcessor::addTempoMod()
{
    modTempo.add           (new TempoModPreparation());
}

void BKAudioProcessor::addTempoMod(TempoModPreparation::Ptr tmod)
{
    modTempo.add           (tmod);
}

void BKAudioProcessor::addKeymap(void)
{
    int numKeymaps = bkKeymaps.size();
    bkKeymaps.add(new Keymap(numKeymaps));
}

void BKAudioProcessor::addKeymap(Keymap::Ptr k)
{
    k->setId(bkKeymaps.size());
    bkKeymaps.add(k);
}

void BKAudioProcessor::addSynchronic(void)
{
    int numSynchronic = synchronic.size();
    synchronic.add(new Synchronic(&mainPianoSynth, tuning[0], tempo[0], general, updateState, numSynchronic));
    synchronic.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

void BKAudioProcessor::addSynchronic(SynchronicPreparation::Ptr sync)
{
    int numSynchronic = synchronic.size();
    synchronic.add(new Synchronic(&mainPianoSynth, sync, general, numSynchronic));
    synchronic.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

// Returns index of Synchronic to be added/configured.
int  BKAudioProcessor::addSynchronicIfNotAlreadyThere(SynchronicPreparation::Ptr sync)
{
    bool alreadyThere = false; int which = 0;
    for (int i = 0; i < synchronic.size() - 1; i++)
    {
        if (sync->compare(synchronic[i]->sPrep))
        {
            alreadyThere = true;
            which = i;
            break;
        }
    }
    
    if (alreadyThere)   return which;
    else
    {
        addSynchronic(sync);
        return synchronic.size()-1;
    }
    
}

void BKAudioProcessor::addNostalgic(void)
{
    int numNostalgic = nostalgic.size();
    nostalgic.add(new Nostalgic(&mainPianoSynth, tuning[0], updateState, numNostalgic));
    nostalgic.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

void BKAudioProcessor::addNostalgic(NostalgicPreparation::Ptr nost)
{
    int numNostalgic = nostalgic.size();
    nostalgic.add(new Nostalgic(&mainPianoSynth, nost, numNostalgic));
    nostalgic.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

// Returns index of Nostalgic to be added/configured.
int  BKAudioProcessor::addNostalgicIfNotAlreadyThere(NostalgicPreparation::Ptr nost)
{
    bool alreadyThere = false; int which = 0;
    for (int i = 0; i < nostalgic.size() - 1; i++)
    {
        if (nost->compare(nostalgic[i]->sPrep))
        {
            alreadyThere = true;
            which = i;
            break;
        }
    }
    
    if (alreadyThere)   return which;
    else
    {
        addNostalgic(nost);
        return nostalgic.size()-1;
    }
    
}

void BKAudioProcessor::addTuning(void)
{
    int numTuning = tuning.size();
    tuning.add(new Tuning(numTuning));
    tuning.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

void BKAudioProcessor::addTuning(TuningPreparation::Ptr tune)
{
    int numTuning = tuning.size();
    tuning.add(new Tuning(tune, numTuning));
    tuning.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

// Returns index of Tuning to be added/configured.
int  BKAudioProcessor::addTuningIfNotAlreadyThere(TuningPreparation::Ptr tune)
{
    bool alreadyThere = false; int which = 0;
    for (int i = 0; i < tuning.size() - 1; i++)
    {
        if (tune->compare(tuning[i]->sPrep))
        {
            alreadyThere = true;
            which = i;
            break;
        }
    }
    
    if (alreadyThere)
    {
        return which;
    }
    else
    {
        addTuning(tune);
        return tuning.size()-1;
    }
    
}

void BKAudioProcessor::addTempo(void)
{   
    int numTempo = tempo.size();
    tempo.add(new Tempo(numTempo));
    tempo.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

void BKAudioProcessor::addTempo(TempoPreparation::Ptr tmp)
{
    int numTempo = tempo.size();
    tempo.add(new Tempo(tmp, numTempo));
    tempo.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

// Returns index of Tempo to be added/configured.
int  BKAudioProcessor::addTempoIfNotAlreadyThere(TempoPreparation::Ptr tmp)
{
    bool alreadyThere = false; int which = 0;
    for (int i = 0; i < tempo.size() - 1; i++)
    {
        if (tmp->compare(tempo[i]->sPrep))
        {
            alreadyThere = true;
            which = i;
            break;
        }
    }
    
    if (alreadyThere)
    {
        return which;
    }
    else
    {
        addTempo(tmp);
        return tempo.size()-1;
    }
    
}


void BKAudioProcessor::addDirect(void)
{
    int numDirect = direct.size();
    direct.add(new Direct(&mainPianoSynth, &resonanceReleaseSynth, &hammerReleaseSynth, tuning[0], updateState, numDirect));
    direct.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}

void BKAudioProcessor::addDirect(DirectPreparation::Ptr drct)
{
    int numDirect = direct.size();
    direct.add(new Direct(&mainPianoSynth, &resonanceReleaseSynth, &hammerReleaseSynth, drct, numDirect));
    direct.getLast()->processor->setCurrentPlaybackSampleRate(bkSampleRate);
}
// Returns index of tuning to be added/configured.
int  BKAudioProcessor::addDirectIfNotAlreadyThere(DirectPreparation::Ptr drct)
{
    bool alreadyThere = false; int which = 0;
    for (int i = 0; i < direct.size() - 1; i++)
    {
        if (drct->compare(direct[i]->sPrep))
        {
            alreadyThere = true;
            which = i;
            break;
        }
    }
    
    if (alreadyThere)   return which;
    else
    {
        addDirect(drct);
        return direct.size()-1;
    }
    
}

#define jsonGetValue(ID) data.getProperty(ID, "").getArray()->getFirst()
#define jsonGetProperty(ID) data.getProperty(ID, "")
#define jsonPropertyDoesExist(ID) (data.getProperty(ID, "") != String::empty)

#define jsonGetKeys(ID) \
isLayer = true; \
kvar = data.getProperty(ID, ""); \
keys.clearQuick(); \
if (kvar == String::empty) isLayer = false; \
if (isLayer) \
{\
    for (int k = 0; k < kvar.size(); k++) keys.add(kvar[k]);\
}; \
if (!keys.size() || keys[0] == -1) isLayer = false;

void BKAudioProcessor::loadJsonGalleryDialog(void)
{
    FileChooser myChooser ("Load gallery from json file...",
                           File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    
    
    if (myChooser.browseForFileToOpen())
    {
        File myFile (myChooser.getResult());
        
        currentGallery = myFile.getFileName();
        
        var myJson = JSON::parse(myFile);
        
        loadJsonGalleryFromVar(myJson);
    }

}

void BKAudioProcessor::loadJsonGalleryFromPath(String path)
{
    File myFile (path);
    
    currentGallery = myFile.getFileName();
    
    var myJson = JSON::parse(myFile);
    
    loadJsonGalleryFromVar(myJson);
}

void BKAudioProcessor::loadJsonGalleryFromVar(var myJson)
{
    tempoAlreadyLoaded.clear();
    /* * * * Clear gallery * * * */
    // Should thread this or prevent audio from happening
    
    tempo.clearQuick();
    synchronic.clearQuick();
    nostalgic.clearQuick();
    direct.clearQuick();
    tuning.clearQuick();
    
    addTempo();
    addTuning();
    addSynchronic();
    addNostalgic();
    addDirect();
    
    modSynchronic.clearQuick();
    modNostalgic.clearQuick();
    modDirect.clearQuick();
    modTuning.clearQuick();
    modTempo.clearQuick();
    
    
    addTempoMod();
    addTuningMod();
    addSynchronicMod();
    addNostalgicMod();
    addDirectMod();
    
    bkKeymaps.clearQuick();
    bkPianos.clearQuick();
    /* * * * * * * * * * * * * * */
    
    var pattr = myJson.getProperty("pattrstorage", "");
    
    String name = pattr.getProperty("name", "").toString();
    
    var pianos = pattr.getProperty("slots", "");

    int sId,nId,dId,tId,oId;
    Array<int> keys; var kvar;  bool isLayer; bool isOld = true; int modTuningCount = 1;
    
    
    bkKeymaps.add(new Keymap(0));
    bkKeymaps.add(new Keymap(1));
    // Default all on for
    for (int i = 0; i < 128; i++) bkKeymaps[1]->addNote(i);
    
    for (int i = 1; i <= 500; i++) //arbs
    {
        var piano = pianos.getProperty(String(i), 0);
        
        
        if (piano.equals(0)) break;
        else
        {
            addPiano();
            Piano::Ptr thisPiano = bkPianos.getLast();
            thisPiano->setId(i);
            
            String name = piano.getProperty("name", "").toString();
            thisPiano->setName(name);
            int pianoId = piano.getProperty("id", "");
            var data = piano.getProperty("data", "");
        
            // V2 test
            if(jsonPropertyDoesExist(jsonSynchronicLayerX+"1::"+"tempo"))   isOld = false;
            
            bool alreadyExists; int which;
            
            // TUNING
            String tx = "tuning::";
            int scale = jsonGetValue("tuningMenu");
            int fund = jsonGetValue("fundamentalMenu");
            int ais = jsonGetValue(tx+"adaptiveIntervalScale");
            int aas = jsonGetValue(tx+"adaptiveAnchorScale");
            int inv = jsonGetValue(tx+"adaptiveIntervalScale_verticalInversion");
            int clustThresh = jsonGetValue(tx+"clusterThresh");
            float offset = jsonGetValue(tx+"globalFundamentalOffset");
            int howMany = jsonGetValue(tx+"howMany");
            
            Array<float> custom;
            var cvar =  jsonGetProperty(tx+"customScale");
            for (int c = 0; c < cvar.size(); c++) custom.add(cvar[c]);

            
            // tuningMap, setTuning::fundOffset, etc. should be modifications
            
            TuningPreparation::Ptr defaultTuning = new TuningPreparation();
            
            defaultTuning->setTuning((TuningSystem)scale);
            
            defaultTuning->setFundamental((PitchClass)((12-fund)%12));
            
            defaultTuning->setCustomScale(custom);
            
            defaultTuning->setAdaptiveHistory(howMany);
            
            defaultTuning->setAdaptiveAnchorScale((TuningSystem)aas);
            
            defaultTuning->setAdaptiveIntervalScale((TuningSystem)ais);
            
            defaultTuning->setAdaptiveInversional((bool)inv);
            
            defaultTuning->setAdaptiveClusterThresh(clustThresh);
            
            defaultTuning->setFundamentalOffset(offset);
            
            tId = addTuningIfNotAlreadyThere(defaultTuning);
            
            DirectPreparation::Ptr mainPianoPrep = new DirectPreparation(tuning[tId]);
            dId = addDirectIfNotAlreadyThere(mainPianoPrep);
            
            thisPiano->addPreparationMap();
            
            thisPiano->prepMaps[0]->setKeymap(bkKeymaps[1]);
            thisPiano->prepMaps[0]->addDirect(direct[dId]);

            if (scale == AdaptiveTuning || scale == AdaptiveAnchoredTuning) thisPiano->prepMaps[0]->addTuning(tuning[tId]);
            
            // Direct, Synchronic, Nostalgic preparations
            String dx = jsonDirectX;
            String nx = jsonNostalgicX;
            String sx = jsonSynchronicX;
            String ax = "adaptiveTempo::";
    
            for (int L = 1; L <= (isOld ? 1 : 12); L++)
            {
                if (!isOld)
                {
                    dx = jsonDirectLayerX + String(L) + "::";
                    nx = jsonNostalgicLayerX + String(L) + "::";
                    sx = jsonSynchronicLayerX + String(L) + "::";
                }
                
                // FIGURE OUT ADAPTIVE STUFF
                /*
                int atHistory = jsonGetValue(ax+"AT_history");
                float atMin = jsonGetValue(ax+"AT_mintime");
                float atMax = jsonGetValue(ax+"AT_maxtime");
                int atMode = jsonGetValue(ax+"AT_mode");
                float atSub = jsonGetValue(ax+"AT_subdivisions");
                 */
                
                // AT
                jsonGetKeys(sx+"ATkeys");
                
                // Link adaptive
                if (isLayer)
                {
                    
                }
               
                // SYNCHRONIC
                jsonGetKeys(sx+"SynchronicKeys");
            
                if (isLayer)
                {
                    // Make tuning for this Synchronic layer.
                    PitchClass metroFundamental = (PitchClass)int(jsonGetValue("synchronic::metroFundamentalMenu"));
                    TuningSystem metroTuning = (TuningSystem)int(jsonGetValue("synchronic::metroTuningMenu"));
                    
                    TuningPreparation::Ptr tunePrep = new TuningPreparation(defaultTuning);
                    tunePrep->setTuning(metroTuning);
                    tunePrep->setFundamental((PitchClass)((12-metroFundamental)%12));
                    tId = addTuningIfNotAlreadyThere(tunePrep);
                    
                    TempoPreparation::Ptr tempoPrep = new TempoPreparation();
                    tempoPrep->setTempo(120);
                    oId = addTempoIfNotAlreadyThere(tempoPrep);
                    
                    SynchronicPreparation::Ptr syncPrep = new SynchronicPreparation(tuning[tId], tempo[oId]);
                    
                    /*
                    Array<float> lens;
                    var lm = jsonGetProperty(sx+"NoteLengthMultList");
                    //for (int c = 0; c < lm.size(); c++) lens.add(lm[c]);
                    for (int c = 0; c < lm.size(); c++) lens.add((float)lm[c] * 50. * 120./60000.);
                    
                    syncPrep->setLengthMultipliers(lens);
                     */
                    
                    Array<float> accents;
                    var am =  jsonGetProperty(sx+"accentsList");
                    for (int c = 0; c < am.size(); c++) accents.add(am[c]);
                    
                    syncPrep->setAccentMultipliers(accents);
                    
                    Array<float> transp;
                    var tm =  jsonGetProperty(sx+"NoteTranspList");
                    for (int c = 0; c < tm.size(); c++) transp.add(tm[c]);
                    
                    Array<Array<float>> atransp;
                    atransp.add(transp);
                    
                    syncPrep->setTransposition(atransp);
                    
                    Array<float> beats;
                    var bm =  jsonGetProperty(sx+"lengthMultList");
                    for (int c = 0; c < bm.size(); c++) beats.add(bm[c]);
                    
                    syncPrep->setBeatMultipliers(beats);
                    
                    int clusterMin = jsonGetValue(sx+"clusterMin");
                    
                    syncPrep->setClusterMin(clusterMin);
                    
                    int clusterMax = jsonGetValue(sx+"clusterMax");
                    
                    syncPrep->setClusterMax(clusterMax);
                    
                    int clusterThresh = jsonGetValue(sx+"clusterThresh");
                    
                    syncPrep->setClusterThresh(clusterThresh);
                    
                    int howMany = jsonGetValue(sx+"howMany");
                    
                    syncPrep->setNumBeats(howMany);
                    
                    float tmp = jsonGetValue(sx+"tempo");
                    
                    int tmpId = 0;
                    if (tempoAlreadyLoaded.contains(tmp))
                    {
                        
                        for (int i = 1; i < tempo.size(); i++)
                        {
                            if (tempo[i]->sPrep->getTempo() == tmp)
                            {
                                
                                tmpId = i;
                                break;
                            }
                        }
                    }
                    else
                    {
                        addTempo();
                        tempoAlreadyLoaded.add(tmp);
                        tmpId = tempo.size()-1;
                        
                        tempo[tmpId]->sPrep->setTempo(tmp);
                        tempo[tmpId]->aPrep->setTempo(tmp);
                    }
                    
                    DBG("tmpId: "+String(tmpId));
                    syncPrep->setTempoControl(tempo[tmpId]);
                    
                    int syncMode = jsonGetValue(sx+"syncMode");
                    
                    if (syncMode < 0 || syncMode > 4) syncMode = 0;
                    
                    if (syncMode == 0) //last note sync / start
                    {
                        syncPrep->setMode(AnyNoteOnSync);
                        syncPrep->setBeatsToSkip(0);
                    }
                    else if (syncMode == 1) // first-note-sync
                    {
                        syncPrep->setMode(FirstNoteOnSync);
                        syncPrep->setBeatsToSkip(-1);
                    }
                    else if (syncMode == 2) // note-off-sync
                    {
                        syncPrep->setMode(AnyNoteOffSync);
                        syncPrep->setBeatsToSkip(1);
                    }
                    else if (syncMode == 3) // note-off-start
                    {
                        syncPrep->setMode(AnyNoteOffSync);
                        syncPrep->setBeatsToSkip(0);
                    }
                    else if (syncMode == 4) // first-note-start
                    {
                        syncPrep->setMode(FirstNoteOnSync);
                        syncPrep->setBeatsToSkip(0);
                    }
                    
                    Array<float> lens;
                    var lm = jsonGetProperty(sx+"NoteLengthMultList");
                    //for (int c = 0; c < lm.size(); c++) lens.add(lm[c]);
                    for (int c = 0; c < lm.size(); c++) lens.add((float)lm[c] * 50. * tmp/60000.);
                    
                    syncPrep->setLengthMultipliers(lens);
                    
                    sId = addSynchronicIfNotAlreadyThere(syncPrep);
                    
                    
                    // Make new keymap
                    Keymap::Ptr keymap = new Keymap(bkKeymaps.size());
                    for (int k = 0; k < keys.size(); k += 2 )   keymap->addNote(keys[k]);
                    
                    // Compare against current keymaps.
                    alreadyExists = false; which = 0;
                    for (int t = 0; t < bkKeymaps.size()-1; t++)
                    {
                        if (keymap->compare(bkKeymaps[t]))
                        {
                            alreadyExists = true;
                            which = t;
                        }
                    }
                    
                    Keymap::Ptr pmapkeymap = bkKeymaps[which];
                    if (!alreadyExists)
                    {
                        addKeymap(keymap);
                        pmapkeymap = bkKeymaps.getLast();
                        
                        DBG("adding keymap and prepmap");
                    }
                    
                    DBG("adding prep map");
                
                    thisPiano->addPreparationMap();
                    thisPiano->prepMaps.getLast()->setKeymap(pmapkeymap);
                    thisPiano->prepMaps.getLast()->addSynchronic(synchronic[sId]);

                    DBG("\nsynchronicPrep: " + String(sId) +
                        "\nlengths: " + floatArrayToString(lens) +
                        "\naccents: " + floatArrayToString(accents) +
                        "\ntransp: " + floatArrayToString(transp) +
                        "\nbeats: " + floatArrayToString(beats) +
                        "\nclusterMin: " + String(clusterMin) +
                        "\nclusterMax: " + String(clusterMax) +
                        "\nclusterThresh: " + String(clusterThresh) +
                        "\nhowMany: " + String(howMany) +
                        "\nfund: " + String(metroFundamental) +
                        "\ntuning: " + String(metroTuning) +
                        "\nsyncMode: " + String(syncMode) +
                        "\ntempo: " + String(tmp) + "\n");
                }
                
                // NOSTALGIC
                jsonGetKeys(nx+"NostalgicKeys");
                
                if (isLayer)
                {
                    
                    // NOSTALGIC TUNING
                    PitchClass revFundamental = (PitchClass)int(jsonGetValue("nostalgic::reverseFundamentalMenu"));
                    TuningSystem revTuning = (TuningSystem)int(jsonGetValue("nostalgic::reverseTuningMenu"));
                    
                    TuningPreparation::Ptr tunePrep = new TuningPreparation(defaultTuning);
                    
                    tunePrep->setTuning(revTuning);
                    
                    tunePrep->setFundamental((PitchClass)((12-revFundamental)%12));
                    
                    tId = addTuningIfNotAlreadyThere(tunePrep);
                    
                    NostalgicPreparation::Ptr nostPrep = new NostalgicPreparation(tuning[tId]);
                    
                    float gain = jsonGetValue(nx+"gain");
                    
                    nostPrep->setGain(gain);
                    
                    float memoryMult = jsonGetValue(nx+"memoryMult");
                    
                    nostPrep->setLengthMultiplier(memoryMult);
                    
                    float beatsToSkip = jsonGetValue(nx+"revBeatsToSkip");
                    
                    nostPrep->setBeatsToSkip(beatsToSkip);
                    
                    int reverseSyncMode = jsonGetValue(nx+"reverseSyncMode");
                    
                    int reverseSyncTarget = sId;
                    
                    if (!isOld) reverseSyncTarget = jsonGetValue(nx+"reverseSyncTarget");
                    
                    if (reverseSyncMode)
                    {
                        nostPrep->setSyncTarget(reverseSyncTarget);
                        nostPrep->setSyncTargetProcessor(synchronic[reverseSyncTarget]->processor);
                    }
                    
                    float transposition = jsonGetValue(nx+"transposition");
                    
                    Array<float> ntransp;
                    ntransp.add(transposition);
                    
                    nostPrep->setTransposition(ntransp);
                    
                    float undertow = jsonGetValue(nx+"undertow");
                    
                    nostPrep->setUndertow(undertow);
                    
                    float wavedistance = jsonGetValue(nx+"wavedistance");
                    
                    nostPrep->setWaveDistance(wavedistance);
                    
                    // Check if nostalgic[Id] already exists as another nostalgic.
                    nId = addNostalgicIfNotAlreadyThere(nostPrep);
                    
                    // Make new keymap
                    Keymap::Ptr keymap = new Keymap(bkKeymaps.size());
                    for (int k = 0; k < keys.size(); k += 2 )   keymap->addNote(keys[k]);
                    
                    // Compare against current keymaps.
                    alreadyExists = false; which = 0;
                    for (int t = 0; t < bkKeymaps.size()-1; t++)
                    {
                        if (keymap->compare(bkKeymaps[t]))
                        {
                            alreadyExists = true;
                            which = t;
                        }
                    }
                    
                    Keymap::Ptr pmapkeymap = bkKeymaps[which];
                    if (!alreadyExists)
                    {
                        addKeymap(keymap);
                        pmapkeymap = bkKeymaps.getLast();
                        
                        DBG("adding keymap and prepmap");
                    }
                    
                    DBG("adding prep map");
                    
                    thisPiano->addPreparationMap();
                    thisPiano->prepMaps.getLast()->setKeymap(pmapkeymap);
                    thisPiano->prepMaps.getLast()->addNostalgic(nostalgic[nId]);
                    
 
                    
                    DBG("nostalgicPrep: " + String(nId));
                }

                
                // DIRECT
                jsonGetKeys(dx+"DirectKeys");

                
                if (isLayer)
                {
                    
                    DirectPreparation::Ptr dPrep = new DirectPreparation(tuning[0]);
                    
                    
                    if (!isOld)
                    {
                        float dGain     = jsonGetValue(dx+"directGain");
                        float dOverlay  = jsonGetValue(dx+"directOverlay");
                        float dTransp   = jsonGetValue(dx+"directTransp");
                        
                        dPrep->setGain(dGain);
                        
                        Array<float> transp;
                        transp.add(dTransp);
                        
                        dPrep->setTransposition(transp);
                        
                        DBG("keys: " + intArrayToString(keys) +
                            " dgain: " + String(dGain) +
                            " dOverlay: " + String(dOverlay) +
                            " dTransp: " + String(dTransp));
                    }
                    else
                    {
                        dPrep->setGain(0.0f);
                        
                        DBG("keys: " + intArrayToString(keys) +
                            " OFF");
                    }
                    
                    int dId = addDirectIfNotAlreadyThere(dPrep);
                    
                    // Make new keymap
                    Keymap::Ptr keymap = new Keymap(bkKeymaps.size());
                    for (int k = 0; k < keys.size(); k += 2 )   keymap->addNote(keys[k]);
                    
                    // Compare against current keymaps.
                    alreadyExists = false; which = 0;
                    for (int t = 0; t < bkKeymaps.size()-1; t++)
                    {
                        if (keymap->compare(bkKeymaps[t]))
                        {
                            alreadyExists = true;
                            which = t;
                        }
                    }

                    Keymap::Ptr pmapkeymap = bkKeymaps[which];
                    if (!alreadyExists)
                    {
                        addKeymap(keymap);
                        pmapkeymap = bkKeymaps.getLast();
                        
                        DBG("adding keymap and prepmap");
                    }
                    
                    DBG("adding prep map");
                    
                    thisPiano->addPreparationMap();
                    thisPiano->prepMaps.getLast()->setKeymap(pmapkeymap);
                    thisPiano->prepMaps.getLast()->addDirect(direct[dId]);
                    
                    DBG("directPrep: " + String(dId));
                }
                
                
            }
            
            int key, pId;
            var presetmap = jsonGetProperty("presetmaps::presetMap");
            
            if (!(presetmap.size() % 3))
            {
                for (int j = 0; j < presetmap.size(); j += 3)
                {
                    pId = presetmap[j];
                    key = presetmap[j+2];
                    
                    if (key >= 0 && key < 128 && pId >= 1)
                    {
                        thisPiano->pianoMap.set(key, pId);
                    }
                }
            }
            

            String tun,note,notenum;
            
            var tuningmap = jsonGetProperty("tuning::tuningMap");
            if (!(tuningmap.size() % 3))
            {
                for (int j = 0; j < tuningmap.size(); j += 3)
                {
                    tun = tuningmap[j]; note = tuningmap[j+1]; notenum = tuningmap[j+2];
                    
                    PitchClass fund = letterNoteToPitchClass(note);
                    TuningSystem tscale = tuningStringToTuningSystem(tun);
                    int noteNumber = notenum.getIntValue();
                    
                    TuningModPreparation::Ptr myMod = new TuningModPreparation();
                    
                    myMod->setParam(TuningFundamental, String(fund));
                    myMod->setParam(TuningScale, String(tscale));
                    
                    bool dontAdd = false; int whichTMod = 0;
                    for (int c = modTuning.size(); --c>=0;)
                    {
                        if (myMod->compare(modTuning[c]))
                        {
                            whichTMod = c;
                            dontAdd = true;
                            break;
                        }
                    }
                    
                    if (!dontAdd)
                    {
                        addTuningMod(myMod);
                        whichTMod = modTuning.size()-1;
                    }
                    
                    int whichPrep = direct[dId]->aPrep->getTuning()->getId();
                    thisPiano->modMap[noteNumber]->addTuningModification(new TuningModification(noteNumber, whichPrep, TuningFundamental, String(fund), whichTMod));
                    
                    ++modTuningCount;
                    
                    
                    
                }
            }
        }
    }
    
    for (int k = tempo.size(); --k >= 0;)       tempo[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
    for (int k = tuning.size(); --k >= 0;)      tuning[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
    for (int k = synchronic.size(); --k >= 0;)  synchronic[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
    for (int k = nostalgic.size(); --k >= 0;)   nostalgic[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
    for (int k = direct.size(); --k >= 0;)      direct[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
    
    currentPiano = bkPianos[0];
    
    updateUI();
    
    updateGalleries();

}
    
#if 0
    tm = jsonGetProperty("tuning::nostalgicTuningSub::nostalgicTuningMap");
    if (!(tm.size() % 3))
    {
        for (int j = 0; j < tm.size(); j += 3)
        {
            tun = tm[j]; note = tm[j+1]; notenum = tm[j+2];
            
            PitchClass fund = letterNoteToPitchClass(note);
            TuningSystem tscale = tuningStringToTuningSystem(tun);
            int noteNumber = notenum.getIntValue();
            
            TuningModPreparation::Ptr myMod = new TuningModPreparation();
            
            myMod->setParam(TuningFundamental, String(fund));
            myMod->setParam(TuningScale, String(tscale));
            
            bool dontAdd = false; int whichTMod = 0;
            for (int c = modTuning.size(); --c>=0;)
            {
                if (myMod->compare(modTuning[c]))
                {
                    whichTMod = c;
                    dontAdd = true;
                    break;
                }
            }
            
            if (!dontAdd)
            {
                addTuningMod(myMod);
                whichTMod = modTuning.size()-1;
            }
            
            int whichPrep = nostalgic[nId]->aPrep->getTuning()->getId();
            thisPiano->modMap[noteNumber]->addTuningModification(new TuningModification(noteNumber, whichPrep, TuningFundamental, String(fund), whichTMod));
            
            ++modTuningCount;
            
            
            
            
        }
    }
    
    tm = jsonGetProperty("tuning::synchronicTuningSub::synchronicTuningMap");
    if (!(tm.size() % 3))
    {
        for (int j = 0; j < tm.size(); j += 3)
        {
            tun = tm[j]; note = tm[j+1]; notenum = tm[j+2];
            
            PitchClass fund = letterNoteToPitchClass(note);
            TuningSystem tscale = tuningStringToTuningSystem(tun);
            int noteNumber = notenum.getIntValue();
            
            // create TuningModPreparation and TuningModification attached to key
            
            TuningModPreparation::Ptr myMod = new TuningModPreparation();
            
            myMod->setParam(TuningFundamental, String(fund));
            myMod->setParam(TuningScale, String(tscale));
            
            bool dontAdd = false; int whichTMod = 0;
            for (int c = modTuning.size(); --c>=0;)
            {
                if (myMod->compare(modTuning[c]))
                {
                    whichTMod = c;
                    dontAdd = true;
                    break;
                }
            }
            
            if (!dontAdd)
            {
                addTuningMod(myMod);
                whichTMod = modTuning.size()-1;
            }
            
            int whichPrep = synchronic[sId]->aPrep->getTuning()->getId();
            
            thisPiano->modMap[noteNumber]->addTuningModification(new TuningModification(noteNumber, whichPrep, TuningFundamental, String(fund), whichTMod));
            
            ++modTuningCount;
            
            
            
            
        }
    }
#endif
    
void BKAudioProcessor::loadGalleryDialog(void)
{
    FileChooser myChooser ("Load gallery from xml file...",
                           File::getSpecialLocation (File::userHomeDirectory),
                           "*.xml");
    
    if (myChooser.browseForFileToOpen())
    {
        File myFile (myChooser.getResult());
        
        ScopedPointer<XmlElement> xml (XmlDocument::parse (myFile));
        
        if (xml != nullptr /*&& xml->hasTagName ("foobar")*/)
        {
            currentGallery = myFile.getFileName();
            loadGalleryFromXml(xml);
        }
    }
    
}

void BKAudioProcessor::loadGalleryFromPath(String path)
{
    File myFile (path);
    
    ScopedPointer<XmlElement> xml (XmlDocument::parse (myFile));
    
    if (xml != nullptr /*&& xml->hasTagName ("foobar")*/)
    {
        currentGallery = myFile.getFileName();
        loadGalleryFromXml(xml);
    }
}

void BKAudioProcessor::loadGalleryFromXml(ScopedPointer<XmlElement> xml)
{
    
    float f;
    int i;
    bool b;
    Array<float> fa;
    Array<int> fi;

    int pianoCount = 0, sPrepCount = 1, sModPrepCount = 1, nPrepCount = 1, nModPrepCount = 1, dPrepCount = 1, dModPrepCount = 1, tPrepCount = 1, tModPrepCount = 1, keymapCount = 1, tempoPrepCount = 1, tempoModPrepCount = 1;
    
    {
        
        if (xml != nullptr /*&& xml->hasTagName ("foobar")*/)
        {
            
            /* * * * Clear gallery * * * */
            // Should thread this or prevent audio from happening
            synchronic.clearQuick();
            nostalgic.clearQuick();
            direct.clearQuick();
            tuning.clearQuick();
            tempo.clearQuick();

            modSynchronic.clearQuick();
            modNostalgic.clearQuick();
            modDirect.clearQuick();
            modTuning.clearQuick();
            modTempo.clearQuick();
            
            addTempo(); // should happen first
            addTuning();// should happen first
            addSynchronic();
            addNostalgic();
            addDirect();
            
            
            addTuningMod();
            addSynchronicMod();
            addNostalgicMod();
            addDirectMod();
            addTempoMod();
            
            bkKeymaps.clearQuick();
            bkPianos.clearQuick();
            
           
            /* * * * * * * * * * * * * * */
            
            bkKeymaps.set(0, new Keymap(0));
            
            // iterate through its sub-elements
            forEachXmlChildElement (*xml, e)
            {
                if (e->hasTagName( vtagKeymap + String(keymapCount)))
                {
                    int id = keymapCount++;
                    
                    bkKeymaps.set(id, new Keymap(id));
                    
                    Array<int> keys;
                    for (int k = 0; k < 128; k++)
                    {
                        String attr = e->getStringAttribute(ptagKeymap_key + String(k));
                        
                        if (attr == String::empty) break;
                        else
                        {
                            i = attr.getIntValue();
                            keys.add(i);
                        }
                    }
                    
                    bkKeymaps[id]->setKeymap(keys);
                    
                }
                else if (e->hasTagName ( vtagGeneral))
                {
                    general->setState(e);
                }
                else if (e->hasTagName( vtagTuning + String(tPrepCount)))
                {
                    addTuning();
                    
                    int id = tuning.size() - 1;
                    
                    Tuning::Ptr thisTuning = tuning[id];
                    
                    thisTuning->setState(e);
                    
                    ++tPrepCount;
                }
                else if (e->hasTagName( vtagModTuning + String(tModPrepCount)))
                {
                    addTuningMod();
                    
                    int id = modTuning.size() - 1;
                    
                    TuningModPreparation::Ptr thisModTuning = modTuning[id];
                    
                    thisModTuning->setState(e);
                    
                    ++tModPrepCount;
                }
                else if (e->hasTagName( vtagDirect + String(dPrepCount)))
                {
                    addDirect();
                    
                    int id = direct.size()-1;
                    
                    Direct::Ptr thisDirect = direct[id];
                    
                    thisDirect->setState(e, tuning);
                    
                    ++dPrepCount;
                }
                else if (e->hasTagName( vtagModDirect + String(dModPrepCount)))
                {
                    addDirectMod();
                    
                    int id = modDirect.size()-1;
                    
                    DirectModPreparation::Ptr thisModDirect = modDirect[id];
                    
                    thisModDirect->setState(e);
                    
                    ++dModPrepCount;
                }
                else if (e->hasTagName( vtagSynchronic + String(sPrepCount)))
                {
                    addSynchronic();
                    
                    int id = synchronic.size() - 1;
                    
                    Synchronic::Ptr thisSynchronic = synchronic[id];
                    
                    thisSynchronic->setState(e, tuning, tempo);
                    
                    ++sPrepCount;
                    
                }
                else if (e->hasTagName( vtagModSynchronic + String(sModPrepCount)))
                {
                    addSynchronicMod();
                    
                    int id = modSynchronic.size() - 1;
                    
                    SynchronicModPreparation::Ptr thisModSynchronic = modSynchronic[id];
                    
                    thisModSynchronic->setState(e);
                    
                    ++sModPrepCount;
                    
                }
                else if (e->hasTagName( vtagTempo + String(tempoPrepCount)))
                {
                    addTempo();
                    
                    int id = tempo.size() - 1;
                    
                    Tempo::Ptr thisTempo = tempo[id];
                    
                    thisTempo->setState(e);
                    
                    ++tempoPrepCount;
                    
                }
                else if (e->hasTagName( vtagModTempo + String(tempoModPrepCount)))
                {
                    addTempoMod();
                    
                    int id = modTempo.size() - 1;
                    
                    TempoModPreparation::Ptr thisModTempo = modTempo[id];
                    
                    thisModTempo->setState(e);
                    
                    ++tempoModPrepCount;
                    
                }
                else if (e->hasTagName( vtagNostalgic + String(nPrepCount)))
                {
                    addNostalgic();
                    
                    int id = nostalgic.size() - 1;
                    
                    Nostalgic::Ptr thisNostalgic = nostalgic[id];
                    
                    thisNostalgic->setState(e, tuning, synchronic);
                    
                    ++nPrepCount;
                }
                else if (e->hasTagName( vtagModNostalgic + String(nModPrepCount)))
                {
                    addNostalgicMod();
                    
                    int id = modNostalgic.size() - 1;
                    
                    NostalgicModPreparation::Ptr thisModNostalgic = modNostalgic[id];
                    
                    thisModNostalgic->setState(e);
                    
                    ++nModPrepCount;
                }
                else if (e->hasTagName( vtagPiano + String(pianoCount)))
                {
                    int whichPiano = pianoCount++;
                    
                    bkPianos.set(whichPiano, new Piano(synchronic, nostalgic, direct, tuning, tempo,
                                                       modSynchronic, modNostalgic, modDirect, modTuning, modTempo,
                                                       bkKeymaps, whichPiano)); // initializing piano 0
                    
                    Piano::Ptr thisPiano = bkPianos[whichPiano];
                    
                    thisPiano->setState(e);

                }
            }
        }
        
        DBG("SETTING");
        for (int k = tuning.size(); --k >= 0;)      tuning[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
        for (int k = tempo.size(); --k >= 0;)       tempo[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
        for (int k = synchronic.size(); --k >= 0;)  synchronic[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
        for (int k = nostalgic.size(); --k >= 0;)   nostalgic[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
        for (int k = direct.size(); --k >= 0;)      direct[k]->processor->setCurrentPlaybackSampleRate(bkSampleRate);
        
        prevPiano = bkPianos[0];
        currentPiano = bkPianos[0];
        
        updateUI();
        
        updateGalleries();
        
    }
    
    
}

void BKAudioProcessor::updateUI(void)
{
    updateState->pianoDidChange = true;
    updateState->directPreparationDidChange = true;
    updateState->nostalgicPreparationDidChange = true;
    updateState->synchronicPreparationDidChange = true;
    updateState->tuningPreparationDidChange = true;
    updateState->generalSettingsDidChange = true;
}

ScopedPointer<XmlElement>  BKAudioProcessor::saveGallery(void)
{
    ValueTree galleryVT( vtagGallery);
    
    galleryVT.addChild(general->getState(), -1, 0);
    
    // Preparations and keymaps must be first.
    // Tempo and Tuning must be first of the preparations.
    for (int i = 0; i < tempo.size(); i++)                  galleryVT.addChild( tempo[i]->getState(), -1, 0);
    
    for (int i = 0; i < tuning.size(); i++)                 galleryVT.addChild( tuning[i]->getState(), -1, 0);
    
    for (int i = 0; i < direct.size(); i++)                 galleryVT.addChild( direct[i]->getState(), -1, 0);
    
    for (int i = 0; i < synchronic.size(); i++)             galleryVT.addChild( synchronic[i]->getState(), -1, 0);
    
    for (int i = 0; i < nostalgic.size(); i++)              galleryVT.addChild( nostalgic[i]->getState(), -1, 0);

    for (int i = 0; i < modTempo.size(); i++)               galleryVT.addChild( modTempo[i]->getState(i), -1, 0);
    
    for (int i = 0; i < modTuning.size(); i++)              galleryVT.addChild( modTuning[i]->getState(i), -1, 0);
    
    for (int i = 0; i < modDirect.size(); i++)              galleryVT.addChild( modDirect[i]->getState(i), -1, 0);
    
    for (int i = 0; i < modSynchronic.size(); i++)          galleryVT.addChild( modSynchronic[i]->getState(i), -1, 0);
    
    for (int i = 0; i < modNostalgic.size(); i++)           galleryVT.addChild( modNostalgic[i]->getState(i), -1, 0);
    
    for (int i = 0; i < bkKeymaps.size(); i++)              galleryVT.addChild( bkKeymaps[i]->getState(i), -1, 0);
    
    // Pianos
    for (int piano = 0; piano < bkPianos.size(); piano++)   galleryVT.addChild( bkPianos[piano]->getState(), -1, 0);
    
    String xml = galleryVT.toXmlString();
    
    FileChooser myChooser ("Save gallery to file...",
                           File::getSpecialLocation (File::userHomeDirectory),
                           "*.xml");
    
    ScopedPointer<XmlElement> myXML = galleryVT.createXml();
    
    if (myChooser.browseForFileToSave(true))
    {
        File myFile (myChooser.getResult());
        currentGallery = myFile.getFileName();
        myXML->writeToFile(myFile, String::empty);
    }
    
    
    updateGalleries();
    
    galleryDidLoad = true;
    
    return myXML;
    
    
}


void BKAudioProcessor::loadPianoSamples(BKSampleLoadType type)
{
    // TO IMPLEMENT: Should turn off all notes in the processors/synths before loading new samples.
    
    didLoadMainPianoSamples = false;
    
    BKSampleLoader::loadMainPianoSamples(&mainPianoSynth, type);
    
    didLoadMainPianoSamples = true;
    
    if (!didLoadHammersAndRes)
    {
        didLoadHammersAndRes = true;
        BKSampleLoader::loadHammerReleaseSamples(&hammerReleaseSynth);
        BKSampleLoader::loadResonanceReleaseSamples(&resonanceReleaseSynth);
    }
    
}

BKAudioProcessor::~BKAudioProcessor()
{
    
}

//==============================================================================
void BKAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    bkSampleRate = sampleRate;
    
    mainPianoSynth.setCurrentPlaybackSampleRate(sampleRate);
    hammerReleaseSynth.setCurrentPlaybackSampleRate(sampleRate);
    resonanceReleaseSynth.setCurrentPlaybackSampleRate(sampleRate);
    
    for (int i = bkPianos.size(); --i >= 0;) bkPianos[i]->prepareToPlay(sampleRate);
    
    for (int i = tuning.size(); --i >= 0;)      tuning[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
    for (int i = tempo.size(); --i >= 0;)       tempo[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
    for (int i = synchronic.size(); --i >= 0;)  synchronic[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
    for (int i = nostalgic.size(); --i >= 0;)   nostalgic[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
    for (int i = direct.size(); --i >= 0;)      direct[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
    
}

void BKAudioProcessor::performResets(int noteNumber)
{
    // NEEDS OPTIMIZATION
    for (auto prep : currentPiano->modMap.getUnchecked(noteNumber)->directReset)      direct[prep]->reset();
    for (auto prep : currentPiano->modMap.getUnchecked(noteNumber)->synchronicReset)  synchronic[prep]->reset();
    for (auto prep : currentPiano->modMap.getUnchecked(noteNumber)->nostalgicReset)   nostalgic[prep]->reset();
    for (auto prep : currentPiano->modMap.getUnchecked(noteNumber)->tuningReset)      tuning[prep]->reset();
    for (auto prep : currentPiano->modMap.getUnchecked(noteNumber)->tempoReset)      tempo[prep]->reset();
}

void BKAudioProcessor::performModifications(int noteNumber)
{
    // NEEDS OPTIMIZATION 
    Array<float> modfa;
    Array<Array<float>> modafa;
    Array<int> modia;
    float modf;
    int   modi;
    bool  modb;
    
    /*
     TuningId = 0,
     TuningScale,
     TuningFundamental,
     TuningOffset,
     TuningA1IntervalScale,
     TuningA1Inversional,
     TuningA1AnchorScale,
     TuningA1AnchorFundamental,
     TuningA1ClusterThresh,
     TuningA1History,
     TuningCustomScale,
     TuningParameterTypeNil
     */
    
    TuningModification::PtrArr tMod = currentPiano->modMap[noteNumber]->getTuningModifications();
    for (int i = tMod.size(); --i >= 0;)
    {
        TuningPreparation::Ptr active = tuning[tMod[i]->getPrepId()]->aPrep;
        TuningParameterType type = tMod[i]->getParameterType();
        modf = tMod[i]->getModFloat();
        modi = tMod[i]->getModInt();
        modb = tMod[i]->getModBool();
        modfa = tMod[i]->getModFloatArr();
        modia = tMod[i]->getModIntArr();
        
        if (type == TuningScale)               active->setTuning((TuningSystem)modi);
        else if (type == TuningFundamental)         active->setFundamental((PitchClass)modi);
        else if (type == TuningOffset)              active->setFundamentalOffset(modf);
        else if (type == TuningA1IntervalScale)     active->setAdaptiveIntervalScale((TuningSystem)modi);
        else if (type == TuningA1Inversional)       active->setAdaptiveInversional(modb);
        else if (type == TuningA1AnchorScale)       active->setAdaptiveAnchorScale((TuningSystem)modi);
        else if (type == TuningA1ClusterThresh)     active->setAdaptiveClusterThresh(modi);
        else if (type == TuningA1AnchorFundamental) active->setAdaptiveAnchorFundamental((PitchClass) modi);
        else if (type == TuningA1History)           active->setAdaptiveHistory(modi);
        else if (type == TuningCustomScale)         active->setCustomScale(modfa);
        else if (type == TuningAbsoluteOffsets)
        {
            for(int i = 0; i< modfa.size(); i+=2) {
                //DBG("modfa AbsoluteOffsets val = " + String(modfa[i]) + " " + String(modfa[i+1]));
                active->setAbsoluteOffset(modfa[i], modfa[i+1] * .01);
            }
        }
        
        
        updateState->tuningPreparationDidChange = true;
    }
    
    DirectModification::PtrArr dMod = currentPiano->modMap[noteNumber]->getDirectModifications();
    for (int i = dMod.size(); --i >= 0;)
    {
        DirectPreparation::Ptr active = direct[dMod[i]->getPrepId()]->aPrep;
        DirectParameterType type = dMod[i]->getParameterType();
        modfa = dMod[i]->getModFloatArr();
        modf = dMod[i]->getModFloat();
        modi = dMod[i]->getModInt();
        modia = dMod[i]->getModIntArr();

        if (type == DirectTransposition)    active->setTransposition(modfa);
        else if (type == DirectGain)        active->setGain(modf);
        else if (type == DirectHammerGain)  active->setHammerGain(modf);
        else if (type == DirectResGain)     active->setResonanceGain(modf);
        else if (type == DirectTuning)      active->setTuning(tuning[modi]);
        
        
        updateState->directPreparationDidChange = true;
    }
    
    NostalgicModification::PtrArr nMod = currentPiano->modMap[noteNumber]->getNostalgicModifications();
    for (int i = nMod.size(); --i >= 0;)
    {
        NostalgicPreparation::Ptr active = nostalgic[nMod[i]->getPrepId()]->aPrep;
        NostalgicParameterType type = nMod[i]->getParameterType();
        modfa = nMod[i]->getModFloatArr();
        modf = nMod[i]->getModFloat();
        modi = nMod[i]->getModInt();
        modia = nMod[i]->getModIntArr();

        if (type == NostalgicTransposition)         active->setTransposition(modfa);
        else if (type == NostalgicGain)             active->setGain(modf);
        else if (type == NostalgicMode)             active->setMode((NostalgicSyncMode)modi);
        else if (type == NostalgicUndertow)         active->setUndertow(modi);
        else if (type == NostalgicSyncTarget)       active->setSyncTarget(modi);
        else if (type == NostalgicBeatsToSkip)      active->setBeatsToSkip(modf);
        else if (type == NostalgicWaveDistance)     active->setWaveDistance(modi);
        else if (type == NostalgicLengthMultiplier) active->setLengthMultiplier(modf);
        else if (type == NostalgicTuning)           active->setTuning(tuning[modi]);
        
        updateState->nostalgicPreparationDidChange = true;
    }
    
    SynchronicModification::PtrArr sMod = currentPiano->modMap[noteNumber]->getSynchronicModifications();
    for (int i = sMod.size(); --i >= 0;)
    {
        SynchronicPreparation::Ptr active = synchronic[sMod[i]->getPrepId()]->aPrep;
        SynchronicParameterType type = sMod[i]->getParameterType();
        modf = sMod[i]->getModFloat();
        modi = sMod[i]->getModInt();
        modfa = sMod[i]->getModFloatArr();
        modafa = sMod[i]->getModArrFloatArr();
        modia = sMod[i]->getModIntArr();

        if (type == SynchronicTranspOffsets)            active->setTransposition(modafa);
        else if (type == SynchronicTempo)               active->setTempoControl(tempo[modi]);
        else if (type == SynchronicMode)                active->setMode((SynchronicSyncMode)modi);
        else if (type == SynchronicClusterMin)          active->setClusterMin(modi);
        else if (type == SynchronicClusterMax)          active->setClusterMax(modi);
        else if (type == SynchronicClusterThresh)       active->setClusterThresh(modi);
        else if (type == SynchronicNumPulses )          active->setNumBeats(modi);
        else if (type == SynchronicBeatsToSkip)         active->setBeatsToSkip(modi);
        else if (type == SynchronicBeatMultipliers)     active->setBeatMultipliers(modfa);
        else if (type == SynchronicLengthMultipliers)   active->setLengthMultipliers(modfa);
        else if (type == SynchronicAccentMultipliers)   active->setAccentMultipliers(modfa);
        /*
        else if (type == AT1Mode)                       active->setAdaptiveTempo1Mode((AdaptiveTempo1Mode) modi);
        else if (type == AT1Min)                        active->setAdaptiveTempo1Min(modf);
        else if (type == AT1Max)                        active->setAdaptiveTempo1Max(modf);
        else if (type == AT1History)                    active->setAdaptiveTempo1History(modi);
        else if (type == AT1Subdivisions)               active->setAdaptiveTempo1Subdivisions(modf);
         */
        
        updateState->synchronicPreparationDidChange = true;
    }
}

void BKAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    buffer.clear();
    
    if (!didLoadMainPianoSamples) return;
    
    int time;
    MidiMessage m;
    
    int numSamples = buffer.getNumSamples();
    
    // Process all active prep maps in current piano
    for (int p = currentPiano->activePMaps.size(); --p >= 0;)
        currentPiano->activePMaps[p]->processBlock(numSamples, m.getChannel());
    
    for (MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
    {
        int noteNumber = m.getNoteNumber();
        float velocity = m.getFloatVelocity();
        int p, pm; // piano, prepmap
        
        channel = m.getChannel();
        
        if (m.isNoteOn())
        {
            ++noteOnCount;
            
            if (allNotesOff)   allNotesOff = false;
            
            // Check PianoMap for whether piano should change due to key strike.
            int whichPiano = currentPiano->pianoMap[noteNumber] - 1;
            if (whichPiano >= 0 && whichPiano != currentPiano->getId()) setCurrentPiano(whichPiano);
            
            // modifications
            performResets(noteNumber);
            performModifications(noteNumber);
            
            //tempo
            
            // Send key on to each pmap in current piano
            for (p = currentPiano->activePMaps.size(); --p >= 0;) {
                //DBG("noteon: " +String(noteNumber) + " pmap: " + String(p));
                currentPiano->activePMaps[p]->keyPressed(noteNumber, velocity, channel);
            }
        }
        else if (m.isNoteOff())
        {
            
            // Send key off to each pmap in current piano
            for (p = currentPiano->activePMaps.size(); --p >= 0;)
                currentPiano->activePMaps[p]->keyReleased(noteNumber, velocity, channel);
            
            // This is to make sure note offs are sent to Direct and Nostalgic processors from previous pianos with holdover notes.
            for (p = prevPianos.size(); --p >= 0;) {
                for (pm = prevPianos[p]->activePMaps.size(); --pm >= 0;) {
                    prevPianos[p]->activePMaps[pm]->postRelease(noteNumber, velocity, channel);
                }
            }
            
            --noteOnCount;
            
        }
        else if (m.isController())
        {
            int controller = m.getControllerNumber();
            float value = m.getControllerValue() / 128.0f;
            
            int piano = controller-51;
            
            if ((m.getControllerValue() != 0) && piano >= 0 && piano < 5)   setCurrentPiano(piano);

            
        }
    }
    
    // Sets some flags to determine whether to send noteoffs to previous pianos.
    if (!allNotesOff && !noteOnCount) {
        prevPianos.clearQuick();
        allNotesOff = true;
    }

    mainPianoSynth.renderNextBlock(buffer,midiMessages,0, numSamples);
    hammerReleaseSynth.renderNextBlock(buffer,midiMessages,0, numSamples);
    resonanceReleaseSynth.renderNextBlock(buffer,midiMessages,0, numSamples);
    
    
    
}

void  BKAudioProcessor::setCurrentPiano(int which)
{
    
    // Optimizations can be made here. Don't need to iterate through EVERY preparation. 
    for (int i = direct.size(); --i >= 0; ) {
        
        // Need to deal with previous transposition value to make sure notes turn off.
        direct[i]->aPrep->copy(direct[i]->sPrep);
    }
    
    for (int i = nostalgic.size(); --i >= 0; )
        nostalgic[i]->aPrep->copy(nostalgic[i]->sPrep);

    
    for (int i = synchronic.size(); --i >= 0; )
        synchronic[i]->aPrep->copy(synchronic[i]->sPrep);
    
    for (int i = tuning.size(); --i >= 0; )
        tuning[i]->aPrep->copy(tuning[i]->sPrep);
    
    
    if (noteOnCount)  prevPianos.addIfNotAlreadyThere(currentPiano);
    
    prevPiano = currentPiano;
    
    currentPiano = bkPianos[which];

    updateState->pianoDidChange = true;
    updateState->synchronicPreparationDidChange = true;
    updateState->nostalgicPreparationDidChange = true;
    updateState->directPreparationDidChange = true;
}

void BKAudioProcessor::releaseResources() {
    
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    //fileBuffer.setSize (0, 0);
    
}


#ifndef JucePlugin_PreferredChannelConfigurations
bool BKAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) {
    
    // Reject any bus arrangements that are not compatible with your plugin
    
    const int numChannels = preferredSet.size();
    
#if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
#elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
#else
    if (numChannels != 1 && numChannels != 2)
        return false;
    
    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
#endif
    
    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

SynchronicProcessor::Ptr BKAudioProcessor::getSynchronicProcessor(int id)
{
    for (int i = synchronic.size(); --i >= 0;)
    {
        if(synchronic[i]->getId() == id) {
            DBG("got synchronic processor id " + String(id));
            return synchronic[i]->processor;
        }
    }
    
    //else
    DBG("synchronic processor not found, returning first processor");
    return synchronic[0]->processor;
}


//==============================================================================
void BKAudioProcessor::changeListenerCallback(ChangeBroadcaster *source)
{
    
}

//==============================================================================
bool BKAudioProcessor::hasEditor() const
{
    
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BKAudioProcessor::createEditor()
{
    return new BKAudioProcessorEditor (*this);
}

//==============================================================================
void BKAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    if (galleryDidLoad)
    {
        ScopedPointer<XmlElement> xml = saveGallery();
        copyXmlToBinary (*xml, destData);
    }
}

void BKAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    //loadGallery
    if (galleryDidLoad)
        {
        ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
        if (xmlState != nullptr) loadGalleryFromXml(xmlState);
    }
}

//==============================================================================
const String BKAudioProcessor::getName() const {
    
    return JucePlugin_Name;
}

bool BKAudioProcessor::acceptsMidi() const {
    
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BKAudioProcessor::producesMidi() const {
    
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double BKAudioProcessor::getTailLengthSeconds() const {
    
    return 0.0;
}

int BKAudioProcessor::getNumPrograms() {
    
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int BKAudioProcessor::getCurrentProgram() {
    
    return 0;
}

void BKAudioProcessor::setCurrentProgram (int index) {
    
}

const String BKAudioProcessor::getProgramName (int index) {
    
    return String("bitKlavier");
}

void BKAudioProcessor::changeProgramName (int index, const String& newName) {
    
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    
    return new BKAudioProcessor();
}

