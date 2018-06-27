/*
  ==============================================================================

    Nostalgic.cpp
    Created: 22 Nov 2016 3:46:45pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "Nostalgic.h"

NostalgicProcessor::NostalgicProcessor(Nostalgic::Ptr nostalgic,
                                       TuningProcessor::Ptr tuning,
                                       SynchronicProcessor::Ptr synchronic,
                                       BKSynthesiser *s):
synth(s),
nostalgic(nostalgic),
tuner(tuning),
synchronic(synchronic)
{
    noteLengthTimers.ensureStorageAllocated(128);
    velocities.ensureStorageAllocated(128);
    noteOn.ensureStorageAllocated(128);
    
    for (int i = 0; i < 128; i++)
    {
        noteLengthTimers.insert(i, 0); //initialize timers for all notes
        velocities.insert(i, 0); //store noteOn velocities to set Nostalgic velocities
        noteOn.set(i, false);
    }

}

NostalgicProcessor::~NostalgicProcessor()
{
    
}

//begin reverse note; called when key is released
void NostalgicProcessor::postRelease(int midiNoteNumber, int midiChannel)
{
    // turn note length timers off
    activeNotes.removeFirstMatchingValue(midiNoteNumber);
    noteOn.set(midiNoteNumber, false);
    noteLengthTimers.set(midiNoteNumber, 0);
    

}

//begin reverse note; called when key is released
void NostalgicProcessor::keyReleased(int midiNoteNumber, float midiVelocity, int midiChannel, bool post)
{
    float duration = 0.0;
    
    if (post || noteOn[midiNoteNumber])
    {
        
        int offRamp;
        if (nostalgic->aPrep->getUndertow() > 0) offRamp = aRampUndertowCrossMS;
        else offRamp = aRampNostalgicOffMS;

        SynchronicSyncMode syncTargetMode = synchronic->getSynchronic()->aPrep->getMode();
        
        if (nostalgic->aPrep->getMode() == SynchronicSync2)
        {
            float duration = 0.0;
            
            int offRamp;
            if (nostalgic->aPrep->getUndertow() > 0) offRamp = aRampUndertowCrossMS;
            else offRamp = aRampNostalgicOffMS;
            
            //get time in ms to target beat, summing over skipped beat lengths
            SynchronicSyncMode syncTargetMode = synchronic->getMode();
            
            //if(syncTargetMode == FirstNoteOnSync || syncTargetMode == AnyNoteOnSync)
            {
                duration = synchronic->getTimeToBeatMS(nostalgic->aPrep->getBeatsToSkip()) + offRamp + 30; // sum
                
                for (auto t : nostalgic->aPrep->getTransposition())
                {
                    float offset = t + tuner->getOffset(midiNoteNumber), synthOffset = offset;
                    int synthNoteNumber = midiNoteNumber;
                    
                    //if (sampleType < BKLoadSoundfont)
                    {
                        synthNoteNumber += (int)offset;
                        synthOffset     -= (int)offset;
                    }
                    
                    //play nostalgic note

                    DBG("reverse note on noteNum/offset " +
                        String(synthNoteNumber) + " " +
                        String(synthOffset));
                    
                    synth->keyOn(
                                 midiChannel,
                                 midiNoteNumber,
                                 synthNoteNumber,
                                 synthOffset,
                                 velocities.getUnchecked(midiNoteNumber),
                                 nostalgic->aPrep->getGain() * aGlobalGain,
                                 Reverse,
                                 FixedLengthFixedStart,
                                 NostalgicNote,
                                 nostalgic->getId(),
                                 duration + nostalgic->aPrep->getWavedistance(),
                                 duration,  // length
                                 nostalgic->aPrep->getReverseAttack(),
                                 nostalgic->aPrep->getReverseDecay(),
                                 nostalgic->aPrep->getReverseSustain(),
                                 nostalgic->aPrep->getReverseRelease() );
                }
                
                activeNotes.removeFirstMatchingValue(midiNoteNumber);
                noteOn.set(midiNoteNumber, false);
                noteLengthTimers.set(midiNoteNumber, 0);
                
                reverseNotes.insert(0, new NostalgicNoteStuff(midiNoteNumber));
                NostalgicNoteStuff* currentNote = reverseNotes.getUnchecked(0);
                currentNote->setPrepAtKeyOn(nostalgic->aPrep);
                currentNote->setTuningAtKeyOn(tuner->getOffset(midiNoteNumber));
                currentNote->setVelocityAtKeyOn(velocities.getUnchecked(midiNoteNumber));
                currentNote->setReverseStartPosition((duration + nostalgic->aPrep->getWavedistance()) * sampleRate/1000.);
                //currentNote->setReverseTargetLength((duration - aRampUndertowCrossMS) * sampleRate/1000.);
                currentNote->setReverseTargetLength((duration - nostalgic->aPrep->getReverseRelease()) * sampleRate/1000.);
                currentNote->setUndertowTargetLength(nostalgic->aPrep->getUndertow() * sampleRate/1000.);
            }
        }
        else if (nostalgic->aPrep->getMode() == NoteLengthSync)
        {
            //get length of played notes, subtract wave distance to set nostalgic reverse note length
            duration =  (noteLengthTimers.getUnchecked(midiNoteNumber) *
                        nostalgic->aPrep->getLengthMultiplier() +
                        (offRamp + 30)) *          //offRamp + onRamp
                        (1000.0 / sampleRate);
            
            for (auto t : nostalgic->aPrep->getTransposition())
            {
                float offset = t + tuner->getOffset(midiNoteNumber), synthOffset = offset;
                int synthNoteNumber = midiNoteNumber;
                
                //if (sampleType < BKLoadSoundfont)
                {
                    synthNoteNumber += (int)offset;
                    synthOffset     -= (int)offset;
                }
                
                //play nostalgic note
                DBG("reverse note on noteNum/offset " +
                    String(synthNoteNumber) + " " +
                    String(synthOffset));
                
                synth->keyOn(
                             midiChannel,
                             midiNoteNumber,
                             synthNoteNumber,
                             synthOffset,
                             velocities.getUnchecked(midiNoteNumber),
                             nostalgic->aPrep->getGain() * aGlobalGain,
                             Reverse,
                             FixedLengthFixedStart,
                             NostalgicNote,
                             nostalgic->getId(),
                             duration + nostalgic->aPrep->getWavedistance(),
                             duration,  // length
                             nostalgic->aPrep->getReverseAttack(),
                             nostalgic->aPrep->getReverseDecay(),
                             nostalgic->aPrep->getReverseSustain(),
                             nostalgic->aPrep->getReverseRelease() );
            }
            
            // turn note length timers off
            activeNotes.removeFirstMatchingValue(midiNoteNumber);
            noteOn.set(midiNoteNumber, false);
            noteLengthTimers.set(midiNoteNumber, 0);
            //DBG("nostalgic removed active note " + String(midiNoteNumber));
            
            reverseNotes.insert(0, new NostalgicNoteStuff(midiNoteNumber));
            NostalgicNoteStuff* currentNote = reverseNotes.getUnchecked(0);
            currentNote->setPrepAtKeyOn(nostalgic->aPrep);
            currentNote->setTuningAtKeyOn(tuner->getOffset(midiNoteNumber));
            currentNote->setVelocityAtKeyOn(velocities.getUnchecked(midiNoteNumber));
            currentNote->setReverseStartPosition((duration + nostalgic->aPrep->getWavedistance()) * sampleRate/1000.);
            //currentNote->setReverseTargetLength((duration - (aRampUndertowCrossMS + 30)) * sampleRate/1000.);
            //currentNote->setReverseTargetLength((duration - (aRampUndertowCrossMS)) * sampleRate/1000.);
            currentNote->setReverseTargetLength((duration - nostalgic->aPrep->getReverseRelease()) * sampleRate/1000.);
            currentNote->setUndertowTargetLength(nostalgic->aPrep->getUndertow() * sampleRate/1000.);
        }
        else if(syncTargetMode == LastNoteOffSync || syncTargetMode == AnyNoteOffSync)
        {
            duration = synchronic->getTimeToBeatMS(nostalgic->aPrep->getBeatsToSkip()) + offRamp + 30; // sum
            
            for (auto t : nostalgic->aPrep->getTransposition())
            {
                float offset = t + tuner->getOffset(midiNoteNumber), synthOffset = offset;
                int synthNoteNumber = midiNoteNumber;
                
                //if (sampleType < BKLoadSoundfont)
                {
                    synthNoteNumber += (int)offset;
                    synthOffset     -= (int)offset;
                }
                
                //play nostalgic note
                DBG("reverse note on noteNum/offset " +
                    String(synthNoteNumber) + " " +
                    String(synthOffset));
                
                synth->keyOn(midiChannel,
                             midiNoteNumber,
                             synthNoteNumber,
                             synthOffset,
                             velocities.getUnchecked(midiNoteNumber),
                             nostalgic->aPrep->getGain() * aGlobalGain,
                             Reverse,
                             FixedLengthFixedStart,
                             NostalgicNote,
                             nostalgic->getId(),
                             duration + nostalgic->aPrep->getWavedistance(),
                             duration,  // length
                             nostalgic->aPrep->getReverseAttack(),
                             nostalgic->aPrep->getReverseDecay(),
                             nostalgic->aPrep->getReverseSustain(),
                             nostalgic->aPrep->getReverseRelease() );
            }
            
            reverseNotes.insert(0, new NostalgicNoteStuff(midiNoteNumber));
            NostalgicNoteStuff* currentNote = reverseNotes.getUnchecked(0);
            currentNote->setPrepAtKeyOn(nostalgic->aPrep);
            currentNote->setTuningAtKeyOn(tuner->getOffset(midiNoteNumber));
            currentNote->setVelocityAtKeyOn(velocities.getUnchecked(midiNoteNumber) * nostalgic->aPrep->getGain());
            currentNote->setReverseStartPosition((duration + nostalgic->aPrep->getWavedistance()) * sampleRate/1000.);
            //currentNote->setReverseTargetLength((duration - aRampUndertowCrossMS) * sampleRate/1000.);
            currentNote->setReverseTargetLength((duration - nostalgic->aPrep->getReverseRelease()) * sampleRate/1000.);
            currentNote->setUndertowTargetLength(nostalgic->aPrep->getUndertow() * sampleRate/1000.);
        }
    }

}

//start timer for length of a particular note; called when key is pressed
void NostalgicProcessor::keyPressed(int midiNoteNumber, float midiNoteVelocity, int midiChannel)
{
    
    if (nostalgic->aPrep->getMode() == SynchronicSync)
    {
        float duration = 0.0;
        
        int offRamp;
        if (nostalgic->aPrep->getUndertow() > 0) offRamp = aRampUndertowCrossMS;
        else offRamp = aRampNostalgicOffMS;
        
        //get time in ms to target beat, summing over skipped beat lengths
        SynchronicSyncMode syncTargetMode = synchronic->getMode();
        
        if(syncTargetMode == FirstNoteOnSync || syncTargetMode == AnyNoteOnSync)
        {
            duration = synchronic->getTimeToBeatMS(nostalgic->aPrep->getBeatsToSkip()) + offRamp + 30; // sum
            
            for (auto t : nostalgic->aPrep->getTransposition())
            {
                float offset = t + tuner->getOffset(midiNoteNumber), synthOffset = offset;
                int synthNoteNumber = midiNoteNumber;
                
                //if (sampleType < BKLoadSoundfont)
                {
                    synthNoteNumber += (int)offset;
                    synthOffset     -= (int)offset;
                }

                //play nostalgic note
                DBG("reverse note on noteNum/offset " +
                    String(synthNoteNumber) + " " +
                    String(synthOffset));
                
                synth->keyOn(
                             midiChannel,
                             midiNoteNumber,
                             synthNoteNumber,
                             synthOffset,
                             midiNoteVelocity,
                             nostalgic->aPrep->getGain() * aGlobalGain,
                             Reverse,
                             FixedLengthFixedStart,
                             NostalgicNote,
                             nostalgic->getId(),
                             duration + nostalgic->aPrep->getWavedistance(),
                             duration,  // length
                             nostalgic->aPrep->getReverseAttack(),
                             nostalgic->aPrep->getReverseDecay(),
                             nostalgic->aPrep->getReverseSustain(),
                             nostalgic->aPrep->getReverseRelease() );
            }
            
            reverseNotes.insert(0, new NostalgicNoteStuff(midiNoteNumber));
            NostalgicNoteStuff* currentNote = reverseNotes.getUnchecked(0);
            currentNote->setPrepAtKeyOn(nostalgic->aPrep);
            currentNote->setTuningAtKeyOn(tuner->getOffset(midiNoteNumber));
            currentNote->setVelocityAtKeyOn(midiNoteVelocity);
            currentNote->setReverseStartPosition((duration + nostalgic->aPrep->getWavedistance()) * sampleRate/1000.);
            //currentNote->setReverseTargetLength((duration - aRampUndertowCrossMS) * sampleRate/1000.);
            currentNote->setReverseTargetLength((duration - nostalgic->aPrep->getReverseRelease()) * sampleRate/1000.);
            currentNote->setUndertowTargetLength(nostalgic->aPrep->getUndertow() * sampleRate/1000.);
        }
    }
    
    //activeNotes is for measuring lengths of held notes, so only relevant in NoteLengthSync mode
    if(nostalgic->aPrep->getMode() == NoteLengthSync)
    {
        activeNotes.addIfNotAlreadyThere(midiNoteNumber);
        noteLengthTimers.set(midiNoteNumber, 0);
    }
    
    noteOn.set(midiNoteNumber, true);
    velocities.set(midiNoteNumber, midiNoteVelocity);
    
}

//main scheduling function
void NostalgicProcessor::processBlock(int numSamples, int midiChannel, BKSampleLoadType type)
{
    sampleType = type;
    incrementTimers(numSamples);

    for(int i = undertowNotes.size() - 1; i >= 0; --i)
    //for(int i=0; i<undertowNotes.size(); i++)
    {
        if(undertowNotes.getUnchecked(i)->undertowTimerExceedsTarget())
            undertowNotes.remove(i);
    }
    
    for(int i = reverseNotes.size() - 1; i >= 0; --i)
    //for(int i=0; i<reverseNotes.size(); i++)
    {
        NostalgicNoteStuff* thisNote = reverseNotes.getUnchecked(i);
        
        if(thisNote->reverseTimerExceedsTarget())
        {
            NostalgicPreparation::Ptr noteOnPrep = thisNote->getPrepAtKeyOn();
            
            if(noteOnPrep->getUndertow() > 0)
            {
                for (auto t : noteOnPrep->getTransposition())
                {
                    float offset = t + thisNote->getTuningAtKeyOn(), synthOffset = offset;
                    int synthNoteNumber = thisNote->getNoteNumber();
                    
                    //if (sampleType < BKLoadSoundfont)
                    {
                        synthNoteNumber += (int)offset;
                        synthOffset     -= (int)offset;
                    }
                    
                    DBG("undertow note on noteNum/offset/duration " +
                        String(synthNoteNumber) + " " +
                        String(synthOffset) + " " +
                        String(noteOnPrep->getUndertow()));
                    
                    synth->keyOn(midiChannel,
                                 thisNote->getNoteNumber(),
                                 synthNoteNumber,
                                 synthOffset,
                                 thisNote->getVelocityAtKeyOn(),
                                 noteOnPrep->getGain() * aGlobalGain,
                                 Forward,
                                 FixedLengthFixedStart,
                                 NostalgicNote,
                                 nostalgic->getId(),
                                 noteOnPrep->getWavedistance(),                        //start position
                                 noteOnPrep->getUndertow(),                            //play length
                                 nostalgic->aPrep->getUndertowAttack(),
                                 nostalgic->aPrep->getUndertowDecay(),
                                 nostalgic->aPrep->getUndertowSustain(),
                                 nostalgic->aPrep->getUndertowRelease() );
                }

                undertowNotes.insert(0, new NostalgicNoteStuff(thisNote->getNoteNumber()));
                NostalgicNoteStuff* newNote = undertowNotes.getUnchecked(0);
                newNote->setUndertowTargetLength(thisNote->getUndertowTargetLength());
                newNote->setUndertowStartPosition(noteOnPrep->getWavedistance() * sampleRate/1000.);
            }
            
            //remove from active notes list
            reverseNotes.remove(i);
        }
        
        if(!thisNote->isActive()) reverseNotes.remove(i);
    }
}

//increment timers for all active notes, and all currently reversing notes
void NostalgicProcessor::incrementTimers(int numSamples)
{
    
    for(int i=(activeNotes.size()-1); i >= 0; --i)
    {
        noteLengthTimers.set(activeNotes.getUnchecked(i),
                             noteLengthTimers.getUnchecked(activeNotes.getUnchecked(i)) + numSamples);
    }
    
    for(int i = 0; i<reverseNotes.size(); i++)
    {
        reverseNotes.getUnchecked(i)->incrementReverseTimer(numSamples);
    }
    for(int i = 0; i<undertowNotes.size(); i++)
    {
        undertowNotes.getUnchecked(i)->incrementUndertowTimer(numSamples);
    }
}

Array<int> NostalgicProcessor::getPlayPositions() //return playback positions in ms, not samples
{
    Array<int> newpositions;
    
    for(int i = 0; i<reverseNotes.size(); i++)
    {
        newpositions.set(i, reverseNotes.getUnchecked(i)->getReversePlayPosition() * 1000./sampleRate);
    }
    
    return newpositions;
}

Array<int> NostalgicProcessor::getUndertowPositions() //return playback positions in ms, not samples
{
    Array<int> newpositions;

    for(int i = 0; i<undertowNotes.size(); i++)
    {
        newpositions.set(i, undertowNotes.getUnchecked(i)->getUndertowPlayPosition() * 1000./sampleRate);
    }
    
    return newpositions;
}
