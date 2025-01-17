/*
 ==============================================================================
 
 BKSynthesiser.h
 Created: 19 Oct 2016 9:59:49am
 Author:  Michael R Mulshine
 
 ==============================================================================
 */

#ifndef BKSYNTHESISER_H_INCLUDED
#define BKSYNTHESISER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "SFZero/SFZero.h"

#include "AudioConstants.h"

#include "General.h"

#include "Tuning.h"

#include "Blendronic.h"

#include "BKSTK.h"

#if 0
class SFRegion : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<SFRegion>   Ptr;
    typedef Array<SFRegion::Ptr>                  PtrArr;
    SFRegion(sfzero::Region::Ptr r):
    region(r){}
    
    ~SFRegion(){}
    
    sfzero::Region::Ptr region;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SFRegion)
};
#endif

class JUCE_API  BKSynthesiserSound    : public ReferenceCountedObject
{
 
protected:
    //==============================================================================
    BKSynthesiserSound(void);
    
public:
    /** Destructor. */
    virtual ~BKSynthesiserSound();
    
    //==============================================================================
    /** Returns true if this sound should be played when a given midi note is pressed.
     
     The Synthesiser will use this information when deciding which sounds to trigger
     for a given note.
     */
    virtual bool appliesToNote (int midiNoteNumber) = 0;
    
    /** Returns true if the sound should be triggered by midi events on a given channel.
     
     The Synthesiser will use this information when deciding which sounds to trigger
     for a given note.
     */
    virtual bool appliesToChannel (int midiChannel) = 0;
    
    /** Returns true if the sound should be triggered by midi veloctity.
     
     The Synthesiser will use this information when deciding which sounds to trigger
     for a given note.
     */
    virtual bool appliesToVelocity (int midiVelocity) = 0;
    
    /** The class is reference-counted, so this is a handy pointer class for it. */
    typedef ReferenceCountedObjectPtr<BKSynthesiserSound> Ptr;
    
    sfzero::Region::Ptr region_;
    uint64 sampleLength;
    sfzero::Region::Trigger trigger;
    bool pedal;
    String sampleName;
    
private:
    
    //==============================================================================
    JUCE_LEAK_DETECTOR (BKSynthesiserSound)
};


//==============================================================================
/**
 Represents a voice that a BKSynthesiser can use to play a BKSynthesiserSound.
 
 A voice plays a single sound at a time, and a BKSynthesiser holds an array of
 voices so that it can play polyphonically.
 
 @see BKSynthesiser, BKSynthesiserSound
 */
class  BKSynthesiserVoice : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<BKSynthesiserVoice>   Ptr;
    //==============================================================================
    /** Creates a voice. */
    BKSynthesiserVoice();
    
    /** Destructor. */
    virtual ~BKSynthesiserVoice();
    
    //==============================================================================
    /** Returns the midi note that this voice is currently playing.
     Returns a value less than 0 if no note is playing.
     */
    int getCurrentlyPlayingNote() const noexcept                        { return currentlyPlayingNote; }
    int getCurrentlyPlayingKey() const noexcept                         { return currentlyPlayingKey; }
    
    /** Returns the sound that this voice is currently playing.
     Returns nullptr if it's not playing.
     */
    BKSynthesiserSound::Ptr getCurrentlyPlayingSound() const noexcept     { return currentlyPlayingSound; }
    
    /** Must return true if this voice object is capable of playing the given sound.
     
     If there are different classes of sound, and different classes of voice, a voice can
     choose which ones it wants to take on.
     
     A typical implementation of this method may just return true if there's only one type
     of voice and sound, or it might check the type of the sound object passed-in and
     see if it's one that it understands.
     */
    virtual bool canPlaySound (BKSynthesiserSound*) = 0;
    
    /** Called to start a new note.
     This will be called during the rendering callback, so must be fast and thread-safe.
     */
    virtual void startNote (int midiNoteNumber,
                            float offset,
                            float gain,
                            float velocity,
                            PianoSamplerNoteDirection direction,
                            PianoSamplerNoteType type,
                            BKNoteType bktype,
                            uint64 startingPosition,
                            uint64 length,
                            uint64 voiceRampOn,
                            uint64 voiceRampOff,
                            BKSynthesiserSound* sound,
                            float* dynamicGain,
							float* blendronicGain,
							BlendronicProcessor::PtrArr blendronic) = 0;
    
    virtual void startNote (int midiNoteNumber,
                            float offset,
                            int pitchWheelValue,
                            float gain,
                            float velocity,
                            //float rangeExtend,
                            PianoSamplerNoteDirection direction,
                            PianoSamplerNoteType type,
                            BKNoteType bktype,
                            uint64 startingPosition,
                            uint64 length,
                            uint64 adsrAttack,
                            uint64 adsrDecay,
                            float adsrSustain,
                            uint64 adsrRelease,
                            BKSynthesiserSound* sound,
                            float* dynamicGain,
							float* blendronicGain,
							BlendronicProcessor::PtrArr blendronic) = 0;
    
    /** Called to stop a note.
     
     This will be called during the rendering callback, so must be fast and thread-safe.
     
     The velocity indicates how quickly the note was released - 0 is slowly, 1 is quickly.
     
     If allowTailOff is false or the voice doesn't want to tail-off, then it must stop all
     sound immediately, and must call clearCurrentNote() to reset the state of this voice
     and allow the synth to reassign it another sound.
     
     If allowTailOff is true and the voice decides to do a tail-off, then it's allowed to
     begin fading out its sound, and it can stop playing until it's finished. As soon as it
     finishes playing (during the rendering callback), it must make sure that it calls
     clearCurrentNote().
     */
    virtual void stopNote (float velocity, bool allowTailOff) = 0;
    
    /** Returns true if this voice is currently busy playing a sound.
     By default this just checks the getCurrentlyPlayingNote() value, but can
     be overridden for more advanced checking.
     */
    virtual bool isVoiceActive() const;
    
    /** Called to let the voice know that the pitch wheel has been moved.
     This will be called during the rendering callback, so must be fast and thread-safe.
     */
    virtual void pitchWheelMoved (int newPitchWheelValue) = 0;
    
    /** Called to let the voice know that a midi controller has been moved.
     This will be called during the rendering callback, so must be fast and thread-safe.
     */
    virtual void controllerMoved (int controllerNumber, int newControllerValue) = 0;
    
    /** Called to let the voice know that the aftertouch has changed.
     This will be called during the rendering callback, so must be fast and thread-safe.
     */
    virtual void aftertouchChanged (int newAftertouchValue);
    
    /** Called to let the voice know that the channel pressure has changed.
     This will be called during the rendering callback, so must be fast and thread-safe.
     */
    virtual void channelPressureChanged (int newChannelPressureValue);
    
    //==============================================================================
    /** Renders the next block of data for this voice.
     
     The output audio data must be added to the current contents of the buffer provided.
     Only the region of the buffer between startSample and (startSample + numSamples)
     should be altered by this method.
     
     If the voice is currently silent, it should just return without doing anything.
     
     If the sound that the voice is playing finishes during the course of this rendered
     block, it must call clearCurrentNote(), to tell the BKSynthesiser that it has finished.
     
     The size of the blocks that are rendered can change each time it is called, and may
     involve rendering as little as 1 sample at a time. In between rendering callbacks,
     the voice's methods will be called to tell it about note and controller events.
     */
    virtual void renderNextBlock (AudioBuffer<float>& outputBuffer,
                                  int startSample,
                                  int numSamples) = 0;
    virtual void renderNextBlock (AudioBuffer<double>& outputBuffer,
                                  int startSample,
                                  int numSamples);
    
    /** Changes the voice's reference sample rate.
     
     The rate is set so that subclasses know the output rate and can set their pitch
     accordingly.
     
     This method is called by the synth, and subclasses can access the current rate with
     the currentSampleRate member.
     */
    virtual void setCurrentPlaybackSampleRate (double newRate);
    
    /** Returns true if the voice is currently playing a sound which is mapped to the given
     midi channel.
     
     If it's not currently playing, this will return false.
     */
    virtual bool isPlayingChannel (int midiChannel) const;
    
    /** Returns the current target sample rate at which rendering is being done.
     Subclasses may need to know this so that they can pitch things correctly.
     */
    double getSampleRate() const noexcept                       { return currentSampleRate; }
    
    /** Returns true if the key that triggered this voice is still held down.
     Note that the voice may still be playing after the key was released (e.g because the
     sostenuto pedal is down).
     */
    bool isKeyDown() const noexcept                             { return keyIsDown; }
    
    /** Returns true if the sustain pedal is currently active for this voice. */
    bool isSustainPedalDown() const noexcept                    { return sustainPedalDown; }
    
    /** Returns true if the sostenuto pedal is currently active for this voice. */
    bool isSostenutoPedalDown() const noexcept                  { return sostenutoPedalDown; }
    
    /** Returns true if a voice is sounding in its release phase **/
    bool isPlayingButReleased() const noexcept
    {
        return isVoiceActive() && ! (isKeyDown() || isSostenutoPedalDown() || isSustainPedalDown());
    }
    
    /** Returns true if this voice started playing its current note before the other voice did. */
    bool wasStartedBefore (const BKSynthesiserVoice& other) const noexcept;
    
    GeneralSettings::Ptr generalSettings;
    void setGeneralSettings(GeneralSettings::Ptr gen) {generalSettings = gen;}
    
    TuningProcessor::Ptr tuning;
	BlendronicProcessor::PtrArr blendronic;
    
    double currentSampleRate;
    
protected:
    /** Resets the state of this voice after a sound has finished playing.
     
     The subclass must call this when it finishes playing a note and becomes available
     to play new ones.
     
     It must either call it in the stopNote() method, or if the voice is tailing off,
     then it should call it later during the renderNextBlock method, as soon as it
     finishes its tail-off.
     
     It can also be called at any time during the render callback if the sound happens
     to have finished, e.g. if it's playing a sample and the sample finishes.
     */
    void clearCurrentNote();
    
    
private:
    //==============================================================================
    friend class BKSynthesiser;
    
    int currentlyPlayingNote, currentPlayingMidiChannel, currentlyPlayingKey;
        //sometimes Note and Key might be different, with non-zero transpositions, hence the need for currentlyPlayingKey
    uint32 length;
    PianoSamplerNoteType direction;
    PianoSamplerNoteType type;
    BKNoteType bktype;
    int layerId;
    uint32 noteOnTime;
    BKSynthesiserSound::Ptr currentlyPlayingSound;
    bool keyIsDown, sustainPedalDown, sostenutoPedalDown;
	float* blendronicGain;
    
    AudioBuffer<float> tempBuffer;
    
#if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // Note the new parameters for this method.
    virtual int stopNote (bool) { return 0; }
#endif
    
    JUCE_LEAK_DETECTOR (BKSynthesiserVoice)
};


//==============================================================================
/**
 Base class for a musical device that can play sounds.
 
 To create a BKSynthesiser, you'll need to create a subclass of BKSynthesiserSound
 to describe each sound available to your synth, and a subclass of BKSynthesiserVoice
 which can play back one of these sounds.
 
 Then you can use the addVoice() and addSound() methods to give the BKSynthesiser a
 set of sounds, and a set of voices it can use to play them. If you only give it
 one voice it will be monophonic - the more voices it has, the more polyphony it'll
 have available.
 
 Then repeatedly call the renderNextBlock() method to produce the audio. Any midi
 events that go in will be scanned for note on/off messages, and these are used to
 start and stop the voices playing the appropriate sounds.
 
 While it's playing, you can also cause notes to be triggered by calling the noteOn(),
 noteOff() and other controller methods.
 
 Before rendering, be sure to call the setCurrentPlaybackSampleRate() to tell it
 what the target playback rate is. This value is passed on to the voices so that
 they can pitch their output correctly.
 */
class JUCE_API  BKSynthesiser
{
public:
    //==============================================================================
    /** Creates a new BKSynthesiser.
     You'll need to add some sounds and voices before it'll make any sound.
     */
    BKSynthesiser(BKAudioProcessor& processor, GeneralSettings::Ptr);
    BKSynthesiser(BKAudioProcessor& processor);
    
    void setGeneralSettings(GeneralSettings::Ptr gen);
    void updateGeneralSettings(GeneralSettings::Ptr gen);
    
    /** Destructor. */
    virtual ~BKSynthesiser();
    
    //==============================================================================
    /** Deletes all voices. */
    void clearVoices();
    
    /** Returns the number of voices that have been added. */
    int getNumVoices() const noexcept                               { return voices.size(); }

    ReferenceCountedArray<BKSynthesiserVoice>& getVoices() { return voices; }

    /** Returns one of the voices that have been added. */
    BKSynthesiserVoice* getVoice (int index) const;
    
    /** Adds a new voice to the synth.
     
     All the voices should be the same class of object and are treated equally.
     
     The object passed in will be managed by the BKSynthesiser, which will delete
     it later on when no longer needed. The caller should not retain a pointer to the
     voice.
     */
    BKSynthesiserVoice* addVoice (const BKSynthesiserVoice::Ptr& newVoice);
    
    /** Deletes one of the voices. */
    void removeVoice (int index);
    
    //==============================================================================
    /** Deletes all sounds. */
    void clearSounds(int set);
    
    /** Returns the number of sounds that have been added to the synth. */
    int getNumSounds(int set) const noexcept                               { return soundSets.getUnchecked(set)->size(); }

    OwnedArray < ReferenceCountedArray<BKSynthesiserSound>>& getSounds() { return soundSets; }
    
    /** Returns one of the sounds. */
    BKSynthesiserSound* getSound (int set, int index) const noexcept           { return soundSets.getUnchecked(set)->getUnchecked(index); }
    
    /** Adds a new sound to the BKSynthesiser.
     
     The object passed in is reference counted, so will be deleted when the
     BKSynthesiser and all voices are no longer using it.
     */
    BKSynthesiserSound* addSound (int set, const BKSynthesiserSound::Ptr& newSound);
    
    /** Removes and deletes one of the sounds. */
    void removeSound (int set, int index);
    
    //==============================================================================
    /** If set to true, then the synth will try to take over an existing voice if
     it runs out and needs to play another note.
     
     The value of this boolean is passed into findFreeVoice(), so the result will
     depend on the implementation of this method.
     */
    void setNoteStealingEnabled (bool shouldStealNotes);
    
    /** Returns true if note-stealing is enabled.
     @see setNoteStealingEnabled
     */
    bool isNoteStealingEnabled() const noexcept                     { return shouldStealNotes; }
    
    //==============================================================================
    /** Triggers a note-on event.
     
     The default method here will find all the sounds that want to be triggered by
     this note/channel. For each sound, it'll try to find a free voice, and use the
     voice to start playing the sound.
     
     Subclasses might want to override this if they need a more complex algorithm.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     
     The midiChannel parameter is the channel, between 1 and 16 inclusive.
     */
    virtual BKSynthesiserVoice* keyOn ( int midiChannel,
                                        int keyNoteNumber,
                                        int midiNoteNumber,
                                        float transp,
                                        float velocity,
                                        float gain,
                                        PianoSamplerNoteDirection direction,
                                        PianoSamplerNoteType type,
                                        BKNoteType bktype,
                                        int set,
                                        int layer,
                                        float startingPositionMS,
                                        float lengthMS,
                                        float rampOnMS,
                                        float rampOffMS,
                                        TuningProcessor::Ptr tuner = nullptr,
                                        float* dynamicGain = nullptr,
                                        float* blendronicGain = nullptr,
                                        BlendronicProcessor::PtrArr blendronic = BlendronicProcessor::PtrArr());
    
    virtual BKSynthesiserVoice* keyOn ( int midiChannel,
                                        int keyNoteNumber,
                                        int midiNoteNumber,
                                        float transp,
                                        float velocity,
                                        float gain,
                                        PianoSamplerNoteDirection direction,
                                        PianoSamplerNoteType type,
                                        BKNoteType bktype,
                                        int set,
                                        int layer,
                                        float startingPositionMS,
                                        float lengthMS,
                                        float adsrAttackMS,
                                        float adsrDecayMS,
                                        float adsrSustain,
                                        float adsrReleaseMS,
                                        TuningProcessor::Ptr tuner = nullptr,
                                        float* dynamicGain = nullptr,
                                        float* blendronicGain = nullptr,
                                        BlendronicProcessor::PtrArr blendronic = BlendronicProcessor::PtrArr()
                                        );
    
    /** Triggers a note-off event.
     
     This will turn off any voices that are playing a sound for the given note/channel.
     
     If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
     (if they can do). If this is false, the notes will all be cut off immediately.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     
     The midiChannel parameter is the channel, between 1 and 16 inclusive.
     */
    virtual void keyOff (int midiChannel,
                         BKNoteType type,
                         int set, 
                         int layerId,
                         int keyNoteNumber,
                         int midiNoteNumber,
                         float velocity,
                         float gain,
                         float* dynamicGain,
                         bool allowTailOff,
                         bool nostalgicOff = false);
    
    
    /** Turns off all notes.
     
     This will turn off any voices that are playing a sound on the given midi channel.
     
     If midiChannel is 0 or less, then all voices will be turned off, regardless of
     which channel they're playing. Otherwise it represents a valid midi channel, from
     1 to 16 inclusive.
     
     If allowTailOff is true, the voices will be allowed to fade out the notes gracefully
     (if they can do). If this is false, the notes will all be cut off immediately.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     */
    virtual void allNotesOff (int midiChannel,
                              bool allowTailOff);
    
    
    /** Sends a pitch-wheel message to any active voices.
     
     This will send a pitch-wheel message to any voices that are playing sounds on
     the given midi channel.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     
     @param midiChannel          the midi channel, from 1 to 16 inclusive
     @param wheelValue           the wheel position, from 0 to 0x3fff, as returned by MidiMessage::getPitchWheelValue()
     */
    virtual void handlePitchWheel (int midiChannel,
                                   int wheelValue);
    
    /** Sends a midi controller message to any active voices.
     
     This will send a midi controller message to any voices that are playing sounds on
     the given midi channel.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     
     @param midiChannel          the midi channel, from 1 to 16 inclusive
     @param controllerNumber     the midi controller type, as returned by MidiMessage::getControllerNumber()
     @param controllerValue      the midi controller value, between 0 and 127, as returned by MidiMessage::getControllerValue()
     */
    virtual void handleController (int midiChannel,
                                   int controllerNumber,
                                   int controllerValue);
    
    /** Sends an aftertouch message.
     
     This will send an aftertouch message to any voices that are playing sounds on
     the given midi channel and note number.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     
     @param midiChannel          the midi channel, from 1 to 16 inclusive
     @param midiNoteNumber       the midi note number, 0 to 127
     @param aftertouchValue      the aftertouch value, between 0 and 127,
     as returned by MidiMessage::getAftertouchValue()
     */
    virtual void handleAftertouch (int midiChannel, int midiNoteNumber, int aftertouchValue);
    
    /** Sends a channel pressure message.
     
     This will send a channel pressure message to any voices that are playing sounds on
     the given midi channel.
     
     This method will be called automatically according to the midi data passed into
     renderNextBlock(), but may be called explicitly too.
     
     @param midiChannel              the midi channel, from 1 to 16 inclusive
     @param channelPressureValue     the pressure value, between 0 and 127, as returned
     by MidiMessage::getChannelPressureValue()
     */
    virtual void handleChannelPressure (int midiChannel, int channelPressureValue);
    
    /** Handles a sustain pedal event. */
    virtual void handleSustainPedal (int midiChannel, bool isDown);
    
    /** Handles a sostenuto pedal event. */
    virtual void handleSostenutoPedal (int midiChannel, bool isDown);
    
    /** Can be overridden to handle soft pedal events. */
    virtual void handleSoftPedal (int midiChannel, bool isDown);
    
    /** Can be overridden to handle an incoming program change message.
     The base class implementation of this has no effect, but you may want to make your
     own synth react to program changes.
     */
    virtual void handleProgramChange (int midiChannel,
                                      int programNumber);
    
    //==============================================================================
    /** Tells the BKSynthesiser that the sample rate has changed.
     
     The new sample rate is propagated to the voices so that they can use it to render the correct
     pitches.
     */
    virtual void playbackSampleRateChanged ();
    
    /** Creates the next block of audio output.
     
     This will process the next numSamples of data from all the voices, and add that output
     to the audio block supplied, starting from the offset specified. Note that the
     data will be added to the current contents of the buffer, so you should clear it
     before calling this method if necessary.
     
     The midi events in the inputMidi buffer are parsed for note and controller events,
     and these are used to trigger the voices. Note that the startSample offset applies
     both to the audio output buffer and the midi input buffer, so any midi events
     with timestamps outside the specified region will be ignored.
     */
    inline void renderNextBlock (AudioBuffer<float>& outputAudio,
                                 const MidiBuffer& inputMidi,
                                 int startSample,
                                 int numSamples)
    { processNextBlock (outputAudio, inputMidi, startSample, numSamples); }
    
    inline void renderNextBlock (AudioBuffer<double>& outputAudio,
                                 const MidiBuffer& inputMidi,
                                 int startSample,
                                 int numSamples)
    { processNextBlock (outputAudio, inputMidi, startSample, numSamples); }
    
    /** Returns the current target sample rate at which rendering is being done.
     Subclasses may need to know this so that they can pitch things correctly.
     */
    double getSampleRate() const noexcept;
    
    /** Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering.
     
     When rendering, the audio blocks that are passed into renderNextBlock() will be split up
     into smaller blocks that lie between all the incoming midi messages, and it is these smaller
     sub-blocks that are rendered with multiple calls to renderVoices().
     
     Obviously in a pathological case where there are midi messages on every sample, then
     renderVoices() could be called once per sample and lead to poor performance, so this
     setting allows you to set a lower limit on the block size.
     
     The default setting is 32, which means that midi messages are accurate to about < 1ms
     accuracy, which is probably fine for most purposes, but you may want to increase or
     decrease this value for your synth.
     
     If shouldBeStrict is true, the audio sub-blocks will strictly never be smaller than numSamples.
     
     If shouldBeStrict is false (default), the first audio sub-block in the buffer is allowed
     to be smaller, to make sure that the first MIDI event in a buffer will always be sample-accurate
     (this can sometimes help to avoid quantisation or phasing issues).
     */
    void setMinimumRenderingSubdivisionSize (int numSamples, bool shouldBeStrict = false) noexcept;

	BlendronicDelay::Ptr createBlendronicDelay(float delayLength, int delayBufferSize, double sr, bool active = false);
    void addBlendronicProcessor(BlendronicProcessor::Ptr bproc);
    void removeBlendronicProcessor(int Id);
    
	void renderDelays(AudioBuffer<double>& outputAudio, int startSample, int numSamples);
	void renderDelays(AudioBuffer<float>& outputAudio, int startSample, int numSamples);
    void clearNextDelayBlock(int numSamples);
    
    /** Can be overridden to do custom handling of incoming midi events. */
    virtual void handleMidiEvent (const MidiMessage&);
    
    int loadSamples(BKSampleLoadType type, String path ="", int subsound=0, bool updateGlobalSet=true);
	
	GeneralSettings::Ptr generalSettings;
    
    BKAudioProcessor& processor;
    
protected:
    //==============================================================================
    /** This is used to control access to the rendering callback and the note trigger methods. */
    CriticalSection lock;
    
    ReferenceCountedArray<BKSynthesiserVoice> voices;
    OwnedArray<ReferenceCountedArray<BKSynthesiserSound>> soundSets;
    
    /** The last pitch-wheel values for each midi channel. */
    int lastPitchWheelValues [16];
    
    /** Renders the voices for the given range.
     By default this just calls renderNextBlock() on each voice, but you may need
     to override it to handle custom cases.
     */
    virtual void renderVoices (AudioBuffer<float>& outputAudio,
                               int startSample, int numSamples);
    virtual void renderVoices (AudioBuffer<double>& outputAudio,
                               int startSample, int numSamples);
    
    /** Searches through the voices to find one that's not currently playing, and
     which can play the given sound.
     
     Returns nullptr if all voices are busy and stealing isn't enabled.
     
     To implement a custom note-stealing algorithm, you can either override this
     method, or (preferably) override findVoiceToSteal().
     */
    virtual BKSynthesiserVoice* findFreeVoice (BKSynthesiserSound* soundToPlay,
                                               int midiChannel,
                                               int midiNoteNumber,
                                               bool stealIfNoneAvailable) const;
    
    /** Chooses a voice that is most suitable for being re-used.
     The default method will attempt to find the oldest voice that isn't the
     bottom or top note being played. If that's not suitable for your synth,
     you can override this method and do something more cunning instead.
     */
    virtual BKSynthesiserVoice* findVoiceToSteal (BKSynthesiserSound* soundToPlay,
                                                  int midiChannel,
                                                  int midiNoteNumber) const;
    
    /** Starts a specified voice playing a particular sound.
     You'll probably never need to call this, it's used internally by noteOn(), but
     may be needed by subclasses for custom behaviours.
     */
    void startVoice (BKSynthesiserVoice* voice,
                     BKSynthesiserSound* sound,
                     int midiChannel,
                     int keyNoteNumber,
                     int midiNoteNumber,
                     float midiNoteNumberOffset,
                     float gain,
                     float velocity,
                     PianoSamplerNoteDirection direction,
                     PianoSamplerNoteType type,
                     BKNoteType bktype,
                     int layer,
                     uint64 startingPosition,
                     uint64 length,
                     uint64 voiceRampOn,
                     uint64 voiceRampOff,
                     TuningProcessor::Ptr tuner,
                     float* dynamicGain,
					 float* blendronicGain,
					 BlendronicProcessor::PtrArr blendronic);
    
    void startVoice (BKSynthesiserVoice* voice,
                     BKSynthesiserSound* sound,
                     int midiChannel,
                     int keyNoteNumber,
                     int midiNoteNumber,
                     float midiNoteNumberOffset,
                     float gain,
                     float velocity,
                     //float rangeExtend,
                     PianoSamplerNoteDirection direction,
                     PianoSamplerNoteType type,
                     BKNoteType bktype,
                     int layer,
                     uint64 startingPosition,
                     uint64 length,
                     uint64 adsrAttack,
                     uint64 adsrDecay,
                     float adsrSustain,
                     uint64 adsrRelease,
                     TuningProcessor::Ptr tuner,
                     float* dynamicGain,
					 float* blendronicGain,
					 BlendronicProcessor::PtrArr blendronic);
    
    /** Stops a given voice.
     You should never need to call this, it's used internally by noteOff, but is protected
     in case it's useful for some custom subclasses. It basically just calls through to
     BKSynthesiserVoice::stopNote(), and has some assertions to sanity-check a few things.
     */
    void stopVoice (BKSynthesiserVoice*, float velocity, bool allowTailOff);
    
    juce::Array<sfzero::Region::Ptr > regions_;
    
private:
    
    
    //==============================================================================
    template <typename floatType>
    void processNextBlock (AudioBuffer<floatType>& outputAudio,
                           const MidiBuffer& inputMidi,
                           int startSample,
                           int numSamples);
    //==============================================================================
    
    int pitchWheelValue;
    uint32 lastNoteOnCounter;
    int minimumSubBlockSize;
    bool subBlockSubdivisionIsStrict;
    bool shouldStealNotes;
    BigInteger sustainPedalsDown;

    Array<BlendronicProcessor::Ptr> bprocessors;
    
#if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // Note the new parameters for these methods.
    virtual int findFreeVoice (const bool) const { return 0; }
    virtual int noteOff (int, int, int) { return 0; }
    virtual int findFreeVoice (BKSynthesiserSound*, const bool) { return 0; }
    virtual int findVoiceToSteal (BKSynthesiserSound*) const { return 0; }
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BKSynthesiser)
};


#endif  // BKSYNTHESISER_H_INCLUDED
