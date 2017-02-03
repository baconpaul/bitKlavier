
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BKPianoSampler.h"

#define NOST_KEY_OFF 1

//==============================================================================
BKAudioProcessor::BKAudioProcessor():
general                 (new GeneralSettings()),
mainPianoSynth          (general),
hammerReleaseSynth      (general),
resonanceReleaseSynth   (general)
{
    didLoadHammersAndRes    = false;
    didLoadMainPianoSamples = false;
    pianoDidChange          = false;
    
    
    bkKeymaps.ensureStorageAllocated(aMaxNumPreparationKeymaps);
    bkPianos.ensureStorageAllocated(aMaxNumPianos);
    prevPianos.ensureStorageAllocated(aMaxNumPianos);
    
    tPreparation.ensureStorageAllocated(aMaxTuningPreparations);
    modTuning.ensureStorageAllocated(aMaxTuningPreparations);
    
    synchronic.ensureStorageAllocated(aMaxTotalPreparations);
    modSynchronic.ensureStorageAllocated(aMaxTotalPreparations);
    
    nostalgic.ensureStorageAllocated(aMaxTotalPreparations);
    modNostalgic.ensureStorageAllocated(aMaxTotalPreparations);
    
    direct.ensureStorageAllocated(aMaxTotalPreparations);
    modDirect.ensureStorageAllocated(aMaxTotalPreparations);

    // Make a bunch of keymaps.
    for (int i = 0; i < aMaxNumKeymaps; i++)
        bkKeymaps.add(new Keymap(i));
    
    // Make a bunch of tunings.
    for (int i = 0; i < (aMaxTuningPreparations); i++) {
        tPreparation.add(new TuningPreparation(i));
        modTuning.add(new TuningPreparation(i));
    }
    
    
    for (int i = 0; i < aMaxTotalPreparations; i++)
    {
        synchronic.add  (new Synchronic(&mainPianoSynth, tPreparation[0], i));
        nostalgic.add   (new Nostalgic(&mainPianoSynth, tPreparation[0], i));
        direct.add      (new Direct(&mainPianoSynth, &resonanceReleaseSynth, &hammerReleaseSynth, tPreparation[0], i));
        
        modSynchronic.add   (new SynchronicPreparation(i, tPreparation[0]));
        modNostalgic.add    (new NostalgicPreparation(i, tPreparation[0]));
        modDirect.add       (new DirectModPreparation(i));
    }

    // Make a bunch of pianos. Default to zeroth keymap.
    for (int i = 0 ; i < aMaxNumPianos; i++)
    {
        bkPianos.set(i, new Piano(synchronic, nostalgic, direct,
                                  bkKeymaps[0], i)); // initializing piano 0

    }
  
    // Initialize first piano.
    prevPiano = bkPianos[0];
    currentPiano = bkPianos[0];
    
    // Default all on for 
    for (int i = 0; i < 128; i++) bkKeymaps[1]->addNote(i);
    
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
    
    mainPianoSynth.setCurrentPlaybackSampleRate(sampleRate);
    hammerReleaseSynth.setCurrentPlaybackSampleRate(sampleRate);
    resonanceReleaseSynth.setCurrentPlaybackSampleRate(sampleRate);
       
    for (int i = aMaxNumPianos; --i >= 0;)
        bkPianos[i]->prepareToPlay(sampleRate);
    
    for (int i = aMaxTotalPreparations; --i >= 0;)
    {
        synchronic[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
        nostalgic[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
        direct[i]->processor->setCurrentPlaybackSampleRate(sampleRate);
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

            DirectModification::PtrArr dMod = currentPiano->modMap[noteNumber]->getDirectModifications();
            for (int i = dMod.size(); --i >= 0;)
            {
                if (dMod[i]->getParameterType() == DirectTransposition)
                    direct[dMod[i]->getPrepId()]->aPrep->setTransposition(dMod[i]->getModFloat());
                
                preparationDidChange = true;
            }

            
            // Send key on to each pmap in current piano
            for (p = currentPiano->activePMaps.size(); --p >= 0;)
                currentPiano->activePMaps[p]->keyPressed(noteNumber, velocity, channel);
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
        else if (m.isAftertouch())
        {
        }
        else if (m.isPitchWheel())
        {
            
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
    
    for (int i = nostalgic.size(); --i >= 0; ) {
        nostalgic[i]->aPrep->copy(nostalgic[i]->sPrep);
    }
    
    for (int i = synchronic.size(); --i >= 0; ) {
        synchronic[i]->aPrep->copy(synchronic[i]->sPrep);
    }
    
    if (noteOnCount)  prevPianos.addIfNotAlreadyThere(currentPiano);
    
    prevPiano = currentPiano;
    
    currentPiano = bkPianos[which];

    pianoDidChange = true;
    preparationDidChange = true;
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
}

void BKAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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

