/*
  ==============================================================================

    Direct.h
    Created: 6 Dec 2016 12:46:37pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef DIRECT_H_INCLUDED
#define DIRECT_H_INCLUDED

#include "BKUtilities.h"
#include "BKSynthesiser.h"
#include "Keymap.h"
#include "Tuning.h"
#include "Blendronic.h"

class DirectModification;

/*
DirectPreparation holds all the state variable values for the
Direct preparation. As with other preparation types, bK will use
two instantiations of DirectPreparation for every active
Direct in the gallery, one to store the static state of the
preparation, and the other to store the active state. These will
be the same, unless a Modification is triggered, in which case the
active state will be changed (and a Reset will revert the active state
to the static state).
*/

class DirectPreparation : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<DirectPreparation>   Ptr;
    typedef Array<DirectPreparation::Ptr>                  PtrArr;
    typedef Array<DirectPreparation::Ptr, CriticalSection> CSPtrArr;
    typedef OwnedArray<DirectPreparation>                  Arr;
    typedef OwnedArray<DirectPreparation, CriticalSection> CSArr;
    
    DirectPreparation(DirectPreparation::Ptr p)
    {
        copy(p);
    }
    
    DirectPreparation(Array<float> transp,
                      float gain,
                      bool resAndHammer,
                      float resGain,
                      float hamGain,
                      float blendGain,
                      int atk,
                      int dca,
                      float sust,
                      int rel):
    modded(false),
    dGain(gain),
    dTransposition(transp),
    dResonanceGain(resGain),
    dHammerGain(hamGain),
    dBlendronicGain(blendGain),
    dAttack(atk),
    dDecay(dca),
    dRelease(rel),
    dSustain(sust),
    dUseGlobalSoundSet(true),
    dSoundSet(-1),
    dSoundSetName(String()),
    velocityMin(0),
    velocityMax(127)
    {
        
    }
    
    DirectPreparation(void):
    modded(false),
    dGain(1.0),
    dTransposition(Array<float>({0.0})),
    dTranspUsesTuning(false),
    dResonanceGain(0.5),
    dHammerGain(0.5),
    dBlendronicGain(1.0f),
    dAttack(3),
    dDecay(3),
    dRelease(30),
    dSustain(1.),
    dUseGlobalSoundSet(true),
    dSoundSet(-1),
    dSoundSetName(String()),
    velocityMin(0),
    velocityMax(127)
    {
        
    }
    
    
    
    
    ~DirectPreparation()
    {
        
    }
    
    inline void copy(DirectPreparation::Ptr d)
    {
        dTransposition = d->getTransposition();
        dGain = d->dGain;
        dResonanceGain = d->getResonanceGain();
        dHammerGain = d->getHammerGain();
        dBlendronicGain = d->getBlendronicGain();
        dAttack = d->getAttack();
        dDecay = d->getDecay();
        dSustain = d->getSustain();
        dRelease = d->getRelease();
        dTranspUsesTuning = d->getTranspUsesTuning();
        dUseGlobalSoundSet = d->getUseGlobalSoundSet();
        dSoundSet = d->getSoundSet();
        dSoundSetName = d->getSoundSetName();
        velocityMin = d->getVelocityMin();
        velocityMax = d->getVelocityMax();
    }
    
    // Using a raw pointer here instead of ReferenceCountedObjectPtr (Ptr)
    // so we can use forwarded declared DirectModification
    void performModification(DirectModification* d, Array<bool> dirty);
    
    inline void stepModdables()
    {
        dGain.step();
    }
    
    inline void resetModdables()
    {
        dGain.reset();
    }
    
    inline bool compare(DirectPreparation::Ptr d)
    {
        return  (dTransposition     ==      d->getTransposition()   )   &&
                (dTranspUsesTuning  ==      d->getTranspUsesTuning())   &&
                (dGain.base         ==      d->dGain.base           )   &&
                (dResonanceGain     ==      d->getResonanceGain()   )   &&
                (dHammerGain        ==      d->getHammerGain()      )   &&
                (dBlendronicGain    ==      d->getBlendronicGain()  )   &&
                (dAttack            ==      d->getAttack()          )   &&
                (dDecay             ==      d->getDecay()           )   &&
                (dSustain           ==      d->getSustain()         )   &&
                (dRelease           ==      d->getRelease()         )   ;
    }
    
    // for unit-testing
    inline void randomize(void)
    {
		Random::getSystemRandom().setSeedRandomly();

        float r[20];
        
        for (int i = 0; i < 20; i++)    r[i] = (Random::getSystemRandom().nextFloat());
        int idx = 0;
        
        dTransposition.clear();
        for (int i = 0; i < Random::getSystemRandom().nextInt(10); ++i)
        {
			dTransposition.add(i, (Random::getSystemRandom().nextFloat() * 48.0f - 24.0f));
        }
        
        dGain = r[idx++];
        dResonanceGain = r[idx++];
        dHammerGain = r[idx++];
        dBlendronicGain = r[idx++];
        dAttack = r[idx++] * 2000.0f + 1.0f;
        dDecay = r[idx++] * 2000.0f + 1.0f;
        dSustain = r[idx++];
        dRelease = r[idx++] * 2000.0f + 1.0f;
    }
    
    inline const String getName() const noexcept {return name;}
    inline void setName(String n){name = n;}
    
    inline Array<float> getTransposition() const noexcept               { return dTransposition; }
    inline const bool getTranspUsesTuning() const noexcept              { return dTranspUsesTuning;}
    inline const float getResonanceGain() const noexcept                { return dResonanceGain; }
    inline const float getHammerGain() const noexcept                   { return dHammerGain;    }
    inline const float getBlendronicGain() const noexcept               { return dBlendronicGain; }
    inline float* getGainPtr() { return &dGain.value;          }
//    inline float* getResonanceGainPtr() { return &(dResonanceGain.value); }
//    inline float* getHammerGainPtr() { return &(dHammerGain.value);    }
//    inline float* getBlendronicGainPtr() { return &(dBlendronicGain.value); }
    inline const int getAttack() const noexcept                         { return dAttack;        }
    inline const int getDecay() const noexcept                          { return dDecay;         }
    inline const float getSustain() const noexcept                      { return dSustain;       }
    inline const int getRelease() const noexcept                        { return dRelease;       }
    inline const Array<float> getADSRvals() const noexcept              { return {(float) dAttack, (float) dDecay,(float) dSustain, (float)dRelease}; }
    
    inline void setTransposition(Array<float> val)                      { dTransposition = val;  }
    inline void setTranspUsesTuning(bool val)                           { dTranspUsesTuning = val;}
//    inline void setGain(float val)                                      { dGain = val;           }
    inline void setResonanceGain(float val)                             { dResonanceGain = val;  }
    inline void setHammerGain(float val)                                { dHammerGain = val;     }
    inline void setBlendronicGain(float val)                            { dBlendronicGain = val;  }
    inline void setAttack(int val)                                      { dAttack = val;         }
    inline void setDecay(int val)                                       { dDecay = val;          }
    inline void setSustain(float val)                                   { dSustain = val;        }
    inline void setRelease(int val)                                     { dRelease = val;      DBG("setting Direct Release time :" + String(val));  }
    inline void setADSRvals(Array<float> vals)
    {
        dAttack = vals[0];
        dDecay = vals[1];
        dSustain = vals[2];
        dRelease = vals[3];
    }
    
    inline const int getVelocityMin() const noexcept { return velocityMin; }
    inline const int getVelocityMax() const noexcept { return velocityMax; }
    
    inline const void setVelocityMin(int min)  { velocityMin = min; }
    inline const void setVelocityMax(int max)  { velocityMax = max; }
    
    
    void print(void)
    {
        DBG("dTransposition: "  + floatArrayToString(dTransposition));
        DBG("dGain: "           + String(dGain.value));
        DBG("dResGain: "        + String(dResonanceGain));
        DBG("dHammerGain: "     + String(dHammerGain));
        DBG("dBlendronicGain: " + String(dBlendronicGain));
        DBG("dAttack: "         + String(dAttack));
        DBG("dDecay: "          + String(dDecay));
        DBG("dSustain: "        + String(dSustain));
        DBG("dRelease: "        + String(dRelease));
    }
    
    ValueTree getState(void)
    {
        ValueTree prep( "params" );
    
        dGain.getState(prep, ptagDirect_gain);
        prep.setProperty( ptagDirect_resGain,           getResonanceGain(), 0);
        prep.setProperty( ptagDirect_hammerGain,        getHammerGain(), 0);
        prep.setProperty( ptagDirect_blendronicGain,    getBlendronicGain(), 0);
        prep.setProperty( ptagDirect_transpUsesTuning,  getTranspUsesTuning() ? 1 : 0, 0);
        
        ValueTree transp( vtagDirect_transposition);
        Array<float> m = getTransposition();
        int count = 0;
        for (auto f : m)    transp.setProperty( ptagFloat + String(count++), f, 0);
        prep.addChild(transp, -1, 0);
        
        ValueTree ADSRvals( vtagDirect_ADSR);
        m = getADSRvals();
        count = 0;
        for (auto f : m) ADSRvals.setProperty( ptagFloat + String(count++), f, 0);
        prep.addChild(ADSRvals, -1, 0);
        
        prep.setProperty( ptagDirect_useGlobalSoundSet, getUseGlobalSoundSet() ? 1 : 0, 0);

        prep.setProperty( ptagDirect_soundSet, getSoundSetName(), 0);
        
        prep.setProperty( ptagDirect_velocityMin, getVelocityMin(), 0);
        prep.setProperty( ptagDirect_velocityMax, getVelocityMax(), 0);
        
        return prep;
    }
    
    void setState(XmlElement* e)
    {
        float f;
        
        dGain.setState(e, ptagDirect_gain, 1.0f);
                     
        f = e->getStringAttribute(ptagDirect_hammerGain).getFloatValue();
        setHammerGain(f);
        
        f = e->getStringAttribute(ptagDirect_resGain).getFloatValue();
        setResonanceGain(f);

        setBlendronicGain(e->getDoubleAttribute(ptagDirect_blendronicGain, 1.0f));
        
        String str = e->getStringAttribute(ptagDirect_transpUsesTuning);
        if (str != "") setTranspUsesTuning((bool) str.getIntValue());
        else setTranspUsesTuning(false);
        
        str = e->getStringAttribute(ptagDirect_useGlobalSoundSet);
        if (str == String()) setUseGlobalSoundSet(true);
        else setUseGlobalSoundSet((bool) str.getIntValue());
        
        str = e->getStringAttribute(ptagDirect_soundSet);
        setSoundSetName(str);
        
        setVelocityMin(e->getIntAttribute(ptagDirect_velocityMin, 0));
        setVelocityMax(e->getIntAttribute(ptagDirect_velocityMax, 127));
        
        forEachXmlChildElement (*e, sub)
        {
            if (sub->hasTagName(vtagDirect_transposition))
            {
                Array<float> transp;
                for (int k = 0; k < sub->getNumAttributes(); k++)
                {
                    String attr = sub->getStringAttribute(ptagFloat + String(k));
                    
                    if (attr == String()) break;
                    else
                    {
                        f = attr.getFloatValue();
                        transp.add(f);
                    }
                }
                
                setTransposition(transp);
                
            }
            else  if (sub->hasTagName(vtagDirect_ADSR))
            {
                Array<float> envVals;
                for (int k = 0; k < sub->getNumAttributes(); k++)
                {
                    String attr = sub->getStringAttribute(ptagFloat + String(k));
                    
                    if (attr == String()) break;
                    else
                    {
                        f = attr.getFloatValue();
                        envVals.add(f);
                    }
                }
                
                setADSRvals(envVals);
                
            }
        }
    }
    
    inline void setUseGlobalSoundSet(bool use) { dUseGlobalSoundSet = use; }
    inline void setSoundSet(int Id) { dSoundSet = Id; }
    inline void setSoundSetName(String name) { dSoundSetName = name; }
    
    inline bool getUseGlobalSoundSet(void) { return dUseGlobalSoundSet; }
    inline int getSoundSet(void) { return dUseGlobalSoundSet ? -1 : dSoundSet; }
    inline String getSoundSetName(void) { return dSoundSetName; }

    bool modded;
    // output gain multiplier
    Moddable<float> dGain;
    
private:
    
    //* Instance Variables *//
    
    // name of this one
    String  name;
    
    // transposition, in half steps
    Array<float> dTransposition;
    
    // are Transposition values filtered by the attached Tuning?
    // original/default = false
    bool dTranspUsesTuning;
    
//    // output gain multiplier
//    Moddable<float> dGain;
    
    // gain multipliers for release resonance and hammer
    float dResonanceGain, dHammerGain;
    
    float dBlendronicGain;
    
    // ADSR, in ms, or gain multiplier for sustain
    int dAttack, dDecay, dRelease;
    float dSustain;
    
    bool dUseGlobalSoundSet;
    int dSoundSet;
    String dSoundSetName;
    
    int velocityMin, velocityMax;
    
    JUCE_LEAK_DETECTOR(DirectPreparation);
};


/*
This class owns two DirectPreparations: sPrep and aPrep
As with other preparation, sPrep is the static preparation, while
aPrep is the active preparation currently in use. sPrep and aPrep
remain the same unless a Modification is triggered, which will change
aPrep but not sPrep. aPrep will be restored to sPrep when a Reset
is triggered.
*/

class Direct : public ReferenceCountedObject
{
    
public:
    typedef ReferenceCountedObjectPtr<Direct>   Ptr;
    typedef Array<Direct::Ptr>                  PtrArr;
    typedef Array<Direct::Ptr, CriticalSection> CSPtrArr;
    typedef OwnedArray<Direct>                  Arr;
    typedef OwnedArray<Direct, CriticalSection> CSArr;
    
    
    Direct(DirectPreparation::Ptr d,
           int Id):
    prep(new DirectPreparation(d)),
    Id(Id),
    name("Direct "+String(Id))
    {
        
    }
    
    Direct(int Id, bool random = false):
    Id(Id),
    name("Direct "+String(Id))
    {
		prep = new DirectPreparation();
		if (random) randomize();
    };
    
    inline void clear(void)
    {
        prep       = new DirectPreparation();
    }
    
    inline Direct::Ptr duplicate()
    {
        DirectPreparation::Ptr copyPrep = new DirectPreparation(prep);
        
        Direct::Ptr copy = new Direct(copyPrep, -1);
        
        copy->setName(name);
        
        return copy;
    }
    
    inline ValueTree getState()//bool active = false)
    {
        ValueTree vt(vtagDirect);
        
        vt.setProperty( "Id",Id, 0);
        vt.setProperty( "name",                          name, 0);
        
        vt.addChild(prep->getState(), -1, 0);
        
        return vt;
    }
    
    inline void setState(XmlElement* e)
    {
        Id = e->getStringAttribute("Id").getIntValue();
        
        String n = e->getStringAttribute("name");
        
        if (n != String())     name = n;
        else                   name = String(Id);
        
        
        XmlElement* params = e->getChildByName("params");
        
        if (params != nullptr)
        {
            prep->setState(params);
        }
        else
        {
            prep->setState(e);
        }
    }
    
    ~Direct() {};
    
    inline int getId() {return Id;}
    inline void setId(int newId) { Id = newId;}
    
    DirectPreparation::Ptr      prep;
    
    inline void copy(Direct::Ptr from)
    {
        prep->copy(from->prep);
    }

	inline void randomize()
	{
		clear();
		Random::getSystemRandom().setSeedRandomly();
		prep->randomize();
		Id = Random::getSystemRandom().nextInt(Range<int>(1, 1000));
		name = "random";
	}
    
    inline String getName(void) const noexcept {return name;}
    inline void setName(String newName)
    {
        name = newName;
    }
    
private:
    int Id;
    String name;;
    
    JUCE_LEAK_DETECTOR(Direct)
};


/*
DirectProcessor does the main work, including processing a block
of samples and sending it out. It connects Keymap, Tuning, and
Blendronic preparations together as needed, and gets the Direct
values it needs to behave as expected.
*/

class DirectProcessor : public ReferenceCountedObject
{
    
public:
    typedef ReferenceCountedObjectPtr<DirectProcessor>   Ptr;
    typedef Array<DirectProcessor::Ptr>                  PtrArr;
    typedef Array<DirectProcessor::Ptr, CriticalSection> CSPtrArr;
    typedef OwnedArray<DirectProcessor>                  Arr;
    typedef OwnedArray<DirectProcessor, CriticalSection> CSArr;
    
    
    DirectProcessor(Direct::Ptr direct,
                    TuningProcessor::Ptr tuning, BlendronicProcessor::PtrArr blend,
                    BKSynthesiser *s, BKSynthesiser *res, BKSynthesiser *ham);
    
    ~DirectProcessor();
    
    BKSampleLoadType sampleType;
    void processBlock(int numSamples, int midiChannel, BKSampleLoadType type);

    void    keyPressed(int noteNumber, float velocity, int channel);
    void    keyReleased(int noteNumber, float velocity, int channel, bool soundfont = false);
    void    playReleaseSample(int noteNumber, float velocity, int channel, bool soundfont = false);
    
    inline void prepareToPlay(double sr, BKSynthesiser* main, BKSynthesiser* res, BKSynthesiser* hammer)
    {
        synth = main;
        resonanceSynth = res;
        hammerSynth = hammer;
    }
    
    inline void reset(void)
    {
        direct->prep->resetModdables();
        DBG("direct reset called"); 
    }
    
    inline int getId(void) const noexcept { return direct->getId(); }
    
    inline void setTuning(TuningProcessor::Ptr tuning)
    {
        tuner = tuning;
    }
    
    inline TuningProcessor::Ptr getTuning(void)
    {
        return tuner;
    }

	inline void addBlendronic(BlendronicProcessor::Ptr blend)
	{
		blendronic.add(blend);
	}

	inline BlendronicProcessor::PtrArr getBlendronic(void)
	{
		return blendronic;
	}
    
    inline void addKeymap(Keymap::Ptr keymap)
    {
        keymaps.add(keymap);
    }
    
    inline Keymap::PtrArr getKeymaps(void)
    {
        return keymaps;
    }
    
    inline float getLastVelocity() const noexcept { return lastVelocity; }
    
    bool velocityCheck(int noteNumber);

private:
    BKSynthesiser*      synth;
    BKSynthesiser*      resonanceSynth;
    BKSynthesiser*      hammerSynth;
    
    Direct::Ptr             direct;
    TuningProcessor::Ptr    tuner;
	BlendronicProcessor::PtrArr blendronic;
    
    Keymap::PtrArr      keymaps;
    
    //need to keep track of the actual notes played and their offsets when a particular key is pressed
    //so that they can all be turned off properly, even in the event of a preparation change
    //while the key is held
    Array<int>      keyPlayed[128];         //keep track of pitches played associated with particular key on keyboard
    Array<float>    keyPlayedOffset[128];   //and also the offsets
    
    Array<float> velocities;
    Array<float> velocitiesActive;
    
    float lastVelocity = 0.0f;
    
    JUCE_LEAK_DETECTOR(DirectProcessor);
};

#endif  // DIRECT_H_INCLUDED
