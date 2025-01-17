/*
  ==============================================================================

    BKKeymapKeyboardState.h
    Created: 27 Mar 2017 12:06:24pm
    Author:  Michael R Mulshine (modified from JUCE source)

  ==============================================================================
*/

#ifndef JUCE_BKKeymapKeyboardState_H_INCLUDED
#define JUCE_BKKeymapKeyboardState_H_INCLUDED


#include "BKUtilities.h"

#include "Keymap.h"

class BKKeymapKeyboardState;

//==============================================================================
/**
 Receives events from a BKKeymapKeyboardState object.
 
 @see BKKeymapKeyboardState
 */
class JUCE_API  BKKeymapKeyboardStateListener
{
public:
    //==============================================================================
    BKKeymapKeyboardStateListener() noexcept        {}
    virtual ~BKKeymapKeyboardStateListener()        {}
    
    
    virtual void handleNoteOn(BKKeymapKeyboardState* source, int midiNoteNumber, float velocity) {};
    
    virtual void handleNoteOff(BKKeymapKeyboardState* source, int midiNoteNumber, float velocity) {};
    
    virtual void handleKeymapNoteOn(BKKeymapKeyboardState* source, int midiNoteNumber) {};
    
    virtual void handleKeymapNoteOff(BKKeymapKeyboardState* source, int midiNoteNumber) {};
    
    virtual void handleKeymapNoteToggled(BKKeymapKeyboardState* source, int midiNoteNumber) {};
    
};


//==============================================================================
/**
 Represents a piano keyboard, keeping track of which keys are currently pressed.
 
 This object can parse a stream of midi events, using them to update its idea
 of which keys are pressed for each individiual midi channel.
 
 When keys go up or down, it can broadcast these events to listener objects.
 
 It also allows key up/down events to be triggered with its noteOn() and noteOff()
 methods, and midi messages for these events will be merged into the
 midi stream that gets processed by processNextMidiBuffer().
 */
class JUCE_API  BKKeymapKeyboardState
{
public:
    //==============================================================================
    BKKeymapKeyboardState();
    BKKeymapKeyboardState(Keymap::Ptr);
    ~BKKeymapKeyboardState();
    
    //==============================================================================
    /** Resets the state of the object.
     
     All internal data for all the channels is reset, but no events are sent as a
     result.
     
     If you want to release any keys that are currently down, and to send out note-up
     midi messages for this, use the allNotesOff() method instead.
     */
    void reset();
    
    /** Returns true if the given midi key is currently held down for the given midi channel.
     
     The channel number must be between 1 and 16. If you want to see if any notes are
     on for a range of channels, use the isNoteOnForChannels() method.
     */
    bool isNoteOn (int midiChannel, int midiNoteNumber) const noexcept;
    
    /** Returns true if the given midi key is currently held down on any of a set of midi channels.
     
     The channel mask has a bit set for each midi channel you want to test for - bit
     0 = midi channel 1, bit 1 = midi channel 2, etc.
     
     If a note is on for at least one of the specified channels, this returns true.
     */
    bool isNoteOnForChannels (int midiChannelMask, int midiNoteNumber) const noexcept;
    
    /** Turns a specified note on.
     
     This will cause a suitable midi note-on event to be injected into the midi buffer during the
     next call to processNextMidiBuffer().
     
     It will also trigger a synchronous callback to the listeners to tell them that the key has
     gone down.
     */
    void noteOn (int midiChannel, int midiNoteNumber, float velocity);
    
    /** Turns a specified note off.
     
     This will cause a suitable midi note-off event to be injected into the midi buffer during the
     next call to processNextMidiBuffer().
     
     It will also trigger a synchronous callback to the listeners to tell them that the key has
     gone up.
     
     But if the note isn't acutally down for the given channel, this method will in fact do nothing.
     */
    void noteOff (int midiChannel, int midiNoteNumber, float velocity);
    
    void addToKeymap(int midiNoteNumber);
    
    void removeFromKeymap(int midiNoteNumber);
    
    void toggle(int midiNoteNumber);
    
    bool isInKeymap(int midiNoteNumber);
    
    void setKeymap(Array<bool> midiNoteNumber);
    void setKeymap(Array<int> midiNoteNumber);
    void setKeymap(OwnedArray<HashMap<String, int>>& keymap);
    
    /** This will turn off any currently-down notes for the given midi channel.
     
     If you pass 0 for the midi channel, it will in fact turn off all notes on all channels.
     
     Calling this method will make calls to noteOff(), so can trigger synchronous callbacks
     and events being added to the midi stream.
     */
    void allNotesOff (int midiChannel);
    
    //==============================================================================
    /** Looks at a key-up/down event and uses it to update the state of this object.
     
     To process a buffer full of midi messages, use the processNextMidiBuffer() method
     instead.
     */
    void processNextMidiEvent (const MidiMessage& message);
    
    /** Scans a midi stream for up/down events and adds its own events to it.
     
     This will look for any up/down events and use them to update the internal state,
     synchronously making suitable callbacks to the listeners.
     
     If injectIndirectEvents is true, then midi events to produce the recent noteOn()
     and noteOff() calls will be added into the buffer.
     
     Only the section of the buffer whose timestamps are between startSample and
     (startSample + numSamples) will be affected, and any events added will be placed
     between these times.
     
     If you're going to use this method, you'll need to keep calling it regularly for
     it to work satisfactorily.
     
     To process a single midi event at a time, use the processNextMidiEvent() method
     instead.
     */
    void processNextMidiBuffer (MidiBuffer& buffer,
                                int startSample,
                                int numSamples,
                                bool injectIndirectEvents);
    
    //==============================================================================
    /** Registers a listener for callbacks when keys go up or down.
     @see removeListener
     */
    void addListener (BKKeymapKeyboardStateListener* listener);
    
    /** Deregisters a listener.
     @see addListener
     */
    void removeListener (BKKeymapKeyboardStateListener* listener);
    
    
private:
    //==============================================================================
    CriticalSection lock;
    uint16 noteStates [128];
    bool inKeymap[128];
    MidiBuffer eventsToAdd;
    Array <BKKeymapKeyboardStateListener*> listeners;
    
    void noteOnInternal (int midiChannel, int midiNoteNumber, float velocity);
    void noteOffInternal (int midiChannel, int midiNoteNumber, float velocity);

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKKeymapKeyboardState)
};


#endif   // JUCE_BKKeymapKeyboardState_H_INCLUDED
