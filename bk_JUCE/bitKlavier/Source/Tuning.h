/*
 ==============================================================================
 
 Tuning.h
 Created: 4 Dec 2016 3:40:02pm
 Author:  Daniel Trueman
 
 ==============================================================================
 */

#ifndef TUNING_H_INCLUDED
#define TUNING_H_INCLUDED

#include "BKUtilities.h"
#include "AudioConstants.h"

#include "Keymap.h"

#include "SpringTuning.h"

class TuningPreparation : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<TuningPreparation>    Ptr;
    typedef Array<TuningPreparation::Ptr>                   PtrArr;
    typedef Array<TuningPreparation::Ptr, CriticalSection>  CSPtrArr;
    typedef OwnedArray<TuningPreparation>                        Arr;
    typedef OwnedArray<TuningPreparation, CriticalSection>       CSArr;
    
    TuningPreparation(TuningPreparation::Ptr p):
    tScale(p->getScale()),
    tFundamental(p->getFundamental()),
    tFundamentalOffset(p->getFundamentalOffset()),
    tAdaptiveIntervalScale(p->getAdaptiveIntervalScale()),
    tAdaptiveInversional(p->getAdaptiveInversional()),
    tAdaptiveAnchorScale(p->getAdaptiveAnchorScale()),
    tAdaptiveAnchorFundamental(p->getAdaptiveAnchorFundamental()),
    tAdaptiveClusterThresh(p->getAdaptiveClusterThresh()),
    tAdaptiveHistory(p->getAdaptiveHistory()),
    tCustom(p->getCustomScale()),
    tAbsolute(p->getAbsoluteOffsets()),
    nToneSemitoneWidth(p->getNToneSemitoneWidth()),
    nToneRoot(p->getNToneRoot()),
    stuning(new SpringTuning(p->getSpringTuning()))
    {

    }
    
    inline void copy(TuningPreparation::Ptr p)
    {
        tScale = p->getScale();
        tFundamental = p->getFundamental();
        tFundamentalOffset = p->getFundamentalOffset();
        tAdaptiveIntervalScale = p->getAdaptiveIntervalScale();
        tAdaptiveInversional = p->getAdaptiveInversional();
        tAdaptiveAnchorScale = p->getAdaptiveAnchorScale();
        tAdaptiveAnchorFundamental = p->getAdaptiveAnchorFundamental();
        tAdaptiveClusterThresh = p->getAdaptiveClusterThresh();
        tAdaptiveHistory = p->getAdaptiveHistory();
        tCustom = p->getCustomScale();
        tAbsolute = p->getAbsoluteOffsets();
        nToneSemitoneWidth = p->getNToneSemitoneWidth();
        nToneRoot = p->getNToneRoot();
        stuning = new SpringTuning(p->getSpringTuning());
    }
    
    inline bool compare (TuningPreparation::Ptr p)
    {
        bool custom = true;
        bool absolute = true;
        
        for (int i = p->getCustomScale().size(); --i>=0;)
        {
            if (p->getCustomScale()[i] != tCustom[i])
            {
                custom = false;
                break;
 
            }
        }
        
        for (int i = p->getAbsoluteOffsets().size(); --i>=0;)
        {
            if (p->getAbsoluteOffsets()[i] != tAbsolute[i])
            {
                absolute = false;
                break;
                
            }
        }
        
        return (tScale == p->getScale() &&
                tFundamental == p->getFundamental() &&
                tFundamentalOffset == p->getFundamentalOffset() &&
                tAdaptiveIntervalScale == p->getAdaptiveIntervalScale() &&
                tAdaptiveInversional == p->getAdaptiveInversional() &&
                tAdaptiveAnchorScale == p->getAdaptiveAnchorScale() &&
                tAdaptiveAnchorFundamental == p->getAdaptiveAnchorFundamental() &&
                tAdaptiveClusterThresh == p->getAdaptiveClusterThresh() &&
                nToneSemitoneWidth == p->getNToneSemitoneWidth() &&
                nToneRoot == p->getNToneRoot() &&
                (tAdaptiveHistory == p->getAdaptiveHistory()) && custom && absolute);
    }

	inline void randomize()
	{
		Random::getSystemRandom().setSeedRandomly();

		float r[21];

		for (int i = 0; i < 21; i++)  r[i] = (Random::getSystemRandom().nextFloat());
		int idx = 0;


		tScale = (TuningSystem)(int)( r[idx++] * TuningSystemNil);
		tFundamental = (PitchClass)(int)(r[idx++] * PitchClassNil);
		tFundamentalOffset = r[idx++] * 48.0f - 24.0f;
		tAdaptiveIntervalScale = (TuningSystem)(int)(r[idx++] * TuningSystemNil);
		tAdaptiveInversional = (bool)((int) (r[idx++] * 2));
		tAdaptiveAnchorScale = (TuningSystem)(int)(r[idx++] * TuningSystemNil);
		tAdaptiveAnchorFundamental = (PitchClass)(int)(r[idx++] * PitchClassNil);
		tAdaptiveClusterThresh = (uint64) (r[idx++] * 50000);
		tAdaptiveHistory = (int)(r[idx++] * 100) + 1;
		tCustom.clear();
		for (int i = 0; i < 12; ++i)
		{
			tCustom.add(i, (Random::getSystemRandom().nextFloat() * 2.0f - 1.0f));
		}
		tAbsolute.clear();
		for (int i = 0; i < 128; ++i)
		{
			tAbsolute.add(i, (Random::getSystemRandom().nextFloat() * 2.0f - 1.0f));
		}
		nToneSemitoneWidth = r[idx++] * 200.0f;
		nToneRoot = (int)(r[idx++] * 127) + 1;
	}
    
    TuningPreparation(TuningSystem whichTuning,
                      PitchClass fundamental,
                      float fundamentalOffset,
                      TuningSystem adaptiveIntervalScale,
                      bool adaptiveInversional,
                      TuningSystem adaptiveAnchorScale,
                      PitchClass adaptiveAnchorFundamental,
                      uint64 adaptiveClusterThresh,
                      int adaptiveHistory,
                      Array<float> customScale,
                      float semitoneWidth,
                      int semitoneRoot,
                      SpringTuning::Ptr st):
    tScale(whichTuning),
    tFundamental(fundamental),
    tFundamentalOffset(fundamentalOffset),
    tAdaptiveIntervalScale(adaptiveIntervalScale),
    tAdaptiveInversional(adaptiveInversional),
    tAdaptiveAnchorScale(adaptiveAnchorScale),
    tAdaptiveAnchorFundamental(adaptiveAnchorFundamental),
    tAdaptiveClusterThresh(adaptiveClusterThresh),
    tAdaptiveHistory(adaptiveHistory),
    tCustom(customScale),
    nToneSemitoneWidth(semitoneWidth),
    nToneRoot(semitoneRoot),
    stuning(new SpringTuning(st))
    {
        tAbsolute.ensureStorageAllocated(128);
        for(int i=0; i<128; i++) tAbsolute.set(i, 0.);
    }
    
    TuningPreparation(void):
    tScale(EqualTemperament),
    tFundamental(C),
    tFundamentalOffset(0.),
    tAdaptiveIntervalScale(JustTuning),
    tAdaptiveInversional(true),
    tAdaptiveAnchorScale(EqualTemperament),
    tAdaptiveAnchorFundamental(C),
    tAdaptiveClusterThresh(100),
    tAdaptiveHistory(4),
    tCustom({0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}),
    nToneSemitoneWidth(100),
    nToneRoot(60)
    {
        tAbsolute.ensureStorageAllocated(128);
        for(int i=0; i<128; i++) tAbsolute.set(i, 0.);
        
        stuning = new SpringTuning();

    }
    
    ~TuningPreparation()
    {
        
    }
    
    inline const String getName() const noexcept {return name;}
    inline const TuningSystem getScale() const noexcept                    {return tScale;               }
    inline const PitchClass getFundamental() const noexcept                 {return tFundamental;               }
    inline const float getFundamentalOffset() const noexcept                {return tFundamentalOffset;         }
    inline const TuningSystem getAdaptiveIntervalScale() const noexcept     {return tAdaptiveIntervalScale;     }
    inline const bool getAdaptiveInversional() const noexcept               {return tAdaptiveInversional;       }
    inline const TuningSystem getAdaptiveAnchorScale() const noexcept       {return tAdaptiveAnchorScale;       }
    inline const PitchClass getAdaptiveAnchorFundamental() const noexcept   {return tAdaptiveAnchorFundamental; }
    inline const uint64 getAdaptiveClusterThresh() const noexcept           {return tAdaptiveClusterThresh;     }
    inline const int getAdaptiveHistory() const noexcept                    {return tAdaptiveHistory;           }
    inline const Array<float> getCustomScale() const noexcept               {return tCustom;                    }
    inline const Array<float> getAbsoluteOffsets() const noexcept           {return tAbsolute;                  }
    float getAbsoluteOffset(int midiNoteNumber) const noexcept              {return tAbsolute.getUnchecked(midiNoteNumber);}
    inline const float getNToneSemitoneWidth() const noexcept               {return nToneSemitoneWidth;         }
    inline const int getNToneRoot() const noexcept                          {return nToneRoot;                  }
    inline const int getNToneRootPC() const noexcept                        {return nToneRoot % 12;             }
    inline const int getNToneRootOctave() const noexcept                    {return (nToneRoot / 12) - 1;       }
    
    inline const Array<float> getAbsoluteOffsetsCents() const noexcept {
        Array<float> tAbsoluteCents;
        tAbsoluteCents.ensureStorageAllocated(128);
        for(int i=0; i<tAbsolute.size(); i++)
        {
            tAbsoluteCents.set(i, tAbsolute.getUnchecked(i) * 100.);
        }
        return tAbsoluteCents;
    }
    
    inline const Array<float> getCustomScaleCents() const noexcept {
        Array<float> tCustomCents;
        tCustomCents.ensureStorageAllocated(12);
        for(int i=0; i<tCustom.size(); i++)
        {
            tCustomCents.set(i, tCustom.getUnchecked(i) * 100.0);
        }
        return tCustomCents;
    }
    
    inline void setName(String n){name = n; DBG("set tuning name " + name);}
    inline void setScale(TuningSystem tuning)                                      {tScale = tuning;}
    inline void setFundamental(PitchClass fundamental)                              {tFundamental = fundamental; }
    inline void setFundamentalOffset(float offset)                                  {tFundamentalOffset = offset; }
    inline void setAdaptiveIntervalScale(TuningSystem adaptiveIntervalScale)        {tAdaptiveIntervalScale = adaptiveIntervalScale;}
    inline void setAdaptiveInversional(bool adaptiveInversional)                    {tAdaptiveInversional = adaptiveInversional; }
    inline void setAdaptiveAnchorScale(TuningSystem adaptiveAnchorScale)            {tAdaptiveAnchorScale = adaptiveAnchorScale;  }
    inline void setAdaptiveAnchorFundamental(PitchClass adaptiveAnchorFundamental)  {tAdaptiveAnchorFundamental = adaptiveAnchorFundamental;}
    inline void setAdaptiveClusterThresh(uint64 adaptiveClusterThresh)              {tAdaptiveClusterThresh = adaptiveClusterThresh; }
    inline void setAdaptiveHistory(int adaptiveHistory)                             {tAdaptiveHistory = adaptiveHistory; }
    inline void setCustomScale(Array<float> tuning)                                 {tCustom = tuning;      }
    inline void setAbsoluteOffsets(Array<float> abs)                                {tAbsolute = abs;   }
    void setAbsoluteOffset(int which, float val)                                    {tAbsolute.set(which, val);  }
    inline void setNToneSemitoneWidth(float width)                                  {nToneSemitoneWidth = width; }
    inline void setNToneRoot(int root)
    {
        nToneRoot = root;
        nToneRootPC = nToneRoot % 12;
        nToneRootOctave = (nToneRoot / 12) - 1;
    }
    inline void setNToneRootPC(int pc)
    {
        nToneRootPC = pc;
        nToneRoot = (nToneRootOctave + 1) * 12 + nToneRootPC;
    }
    inline void setNToneRootOctave(int octave)
    {
        nToneRootOctave = octave;
        nToneRoot = (nToneRootOctave + 1) * 12 + nToneRootPC;
    }

    inline void setCustomScaleCents(Array<float> tuning) {
        for(int i=0; i<tCustom.size() && i<tuning.size(); i++)
        {
            tCustom.setUnchecked(i, tuning.getUnchecked(i) * 0.01f);
        }
    }
    
    inline void setAbsoluteOffsetCents(Array<float> abs) {
        for(int i=0; i<tAbsolute.size() && i<abs.size(); i++)
            tAbsolute.setUnchecked(i, abs.getUnchecked(i) * 0.01f);
    }
    
    
    void print(void)
    {
        DBG("tScale: " +                  String(tScale));
        DBG("tFundamental: " +                  String(tFundamental));
        DBG("tFundamentalOffset: " +            String(tFundamentalOffset));
        DBG("tAdaptiveIntervalScale: " +        String(tAdaptiveIntervalScale));
        DBG("tAdaptiveInversional: " +          String((int)tAdaptiveInversional));
        DBG("tAdaptiveAnchorScale: " +          String(tAdaptiveAnchorScale));
        DBG("tAdaptiveAnchorFundamental: " +    String(tAdaptiveAnchorFundamental));
        DBG("tAdaptiveClusterThresh: " +        String(tAdaptiveClusterThresh));
        DBG("tAdaptiveHistory: " +              String(tAdaptiveHistory));
        DBG("tCustom: " +                       floatArrayToString(tCustom));
        DBG("tAbsolute: " +                     floatArrayToString(tAbsolute));
        DBG("nToneSemitoneWidth: " +            String(nToneSemitoneWidth));
        DBG("nToneRoot: " +                     String(nToneRoot));
    }
    
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ SPRING TUNING ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    inline SpringTuning::Ptr getSpringTuning(void) { return stuning; }
    inline void setSpringTuning(SpringTuning::Ptr st) { stuning = st;}
    
    inline Particle::PtrArr getParticles(void) { return getSpringTuning()->getParticles(); }
    inline Particle::PtrArr getTetherParticles(void) { return getSpringTuning()->getTetherParticles(); }
    inline Spring::PtrArr getSprings(void) { return getSpringTuning()->getSprings(); }
    inline Spring::PtrArr getTetherSprings(void) { return getSpringTuning()->getTetherSprings(); }
    
    inline void setSpringWeight(int which, double weight)
    {
        getSpringTuning()->setSpringWeight(which, weight);
    }
    
    inline void setTetherWeight(int which, double weight)
    {
        getSpringTuning()->setTetherWeight(which, weight);
    }
    
    inline void setTetherLock(int which, bool lock)
    {
        getSpringTuning()->setTetherLock(which, lock);
    }
    
    inline bool getTetherLock(int which)
    {
        return getSpringTuning()->getTetherLock(which);
    }
    
    inline void setSpringsActive(bool status) { getSpringTuning()->setActive(status); }
    inline bool getSpringsActive(void) { return getSpringTuning()->getActive(); }
    
    inline void setSpringRate(double rate) { getSpringTuning()->setRate(rate);}
    inline double getSpringRate(void) { return getSpringTuning()->getRate();}
    
    inline void setSpringStiffness(double stiff) { getSpringTuning()->setStiffness(stiff);}
    inline double getSpringStiffness(void) {return getSpringTuning()->getStiffness();}
    
    inline TuningSystem getCurrentSpringScaleId(void) { return getSpringTuning()->getScaleId(); }
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    
private:
    String name;
    // basic tuning settings, for static tuning
    TuningSystem    tScale;               //which tuning system to use
    PitchClass      tFundamental;               //fundamental for tuning system
    float           tFundamentalOffset;         //offset
    
    // adaptive tuning params
    TuningSystem    tAdaptiveIntervalScale;     //scale to use to determine successive interval tuning
    bool            tAdaptiveInversional;       //treat the scale inversionally?
    
    TuningSystem    tAdaptiveAnchorScale;       //scale to tune new fundamentals to when in anchored
    PitchClass      tAdaptiveAnchorFundamental; //fundamental for anchor scale
    
    uint64          tAdaptiveClusterThresh;     //ms; max time before fundamental is reset
    int             tAdaptiveHistory;           //cluster max; max number of notes before fundamental is reset
    
    // custom scale and absolute offsets
    Array<float>    tCustom = Array<float>({0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}); //custom scale
    Array<float>    tAbsolute;  //offset (in MIDI fractional offsets, like other tunings) for specific notes; size = 128
    
    float nToneSemitoneWidth;
    int nToneRoot;              //which key matches 12-tone ET; 60 by default
    int nToneRootPC = 0;        //which pitch class; 0 by default
    int nToneRootOctave = 4;    //which octave; 4 by default, so C4
    
    // SPRING TUNING STUFF
    SpringTuning::Ptr stuning;
    
    JUCE_LEAK_DETECTOR(TuningPreparation);
};


class Tuning : public ReferenceCountedObject
{
    
public:
    typedef ReferenceCountedObjectPtr<Tuning>   Ptr;
    typedef Array<Tuning::Ptr>                  PtrArr;
    typedef Array<Tuning::Ptr, CriticalSection> CSPtrArr;
    typedef OwnedArray<Tuning>                  Arr;
    typedef OwnedArray<Tuning, CriticalSection> CSArr;
    
    
    Tuning(TuningPreparation::Ptr prep,
           int Id,
           SpringTuning::Ptr st = nullptr):
    sPrep(new TuningPreparation(prep)),
    aPrep(new TuningPreparation(sPrep)),
    Id(Id),
    name(String(Id))
    {
        tuningLibrary.ensureStorageAllocated((int)cTuningSystemNames.size());
        for(int i=0; i<cTuningSystemNames.size(); i++) tuningLibrary.insert(EqualTemperament, tEqualTuning);
        
        tuningLibrary.set(PartialTuning, tPartialTuning);
        tuningLibrary.set(JustTuning, tJustTuning);
        tuningLibrary.set(EqualTemperament, tEqualTuning);
        tuningLibrary.set(DuodeneTuning, tDuodeneTuning);
        tuningLibrary.set(OtonalTuning, tOtonalTuning);
        tuningLibrary.set(UtonalTuning, tUtonalTuning);
        
        sPrep->getSpringTuning()->stop();
    }
    
	Tuning(int Id, bool random = false) :
    Id(Id),
    name(String(Id))
    {
		sPrep = new TuningPreparation();
		aPrep = new TuningPreparation(sPrep);
        
		if (random) randomize();
        
        tuningLibrary.ensureStorageAllocated((int)cTuningSystemNames.size());
        for(int i=0; i<cTuningSystemNames.size(); i++) tuningLibrary.insert(EqualTemperament, tEqualTuning);
        
        tuningLibrary.set(PartialTuning, tPartialTuning);
        tuningLibrary.set(JustTuning, tJustTuning);
        tuningLibrary.set(EqualTemperament, tEqualTuning);
        tuningLibrary.set(DuodeneTuning, tDuodeneTuning);
        tuningLibrary.set(OtonalTuning, tOtonalTuning);
        tuningLibrary.set(UtonalTuning, tUtonalTuning);
        
        sPrep->getSpringTuning()->stop();
    }
    
    inline Tuning::Ptr duplicate()
    {
        TuningPreparation::Ptr copyPrep = new TuningPreparation(sPrep);
        
        Tuning::Ptr copy = new Tuning(copyPrep, -1);
        
        copy->sPrep->getSpringTuning()->stop();
        
        copy->setName(name );
        
        return copy;
    }
    
    inline void clear(void)
    {
        sPrep       = new TuningPreparation();
        aPrep       = new TuningPreparation(sPrep);
    }
    
    
    ValueTree getState(void);
    void setState(XmlElement*);
    
    ~Tuning() {};
    
    inline int getId() {return Id;}
    inline void setId(int newId) { Id = newId;}
    
    TuningPreparation::Ptr      sPrep;
    TuningPreparation::Ptr      aPrep;
    
    void reset()
    {
        aPrep->copy(sPrep);
        DBG("resetting tuning");
    }
    
    inline void copy(Tuning::Ptr from)
    {
        sPrep->copy(from->sPrep);
        aPrep->copy(sPrep);
    }

	inline void randomize()
	{
		clear();
		sPrep->randomize();
		aPrep->randomize();
		Id = Random::getSystemRandom().nextInt(Range<int>(1, 1000));
		name = "random";
	}
    
    inline TuningSystem getCurrentScaleId(void)
    {
        return aPrep->getScale();
    }
    
    inline SpringTuning::Ptr getCurrentSpringTuning(void)
    {
        return aPrep->getSpringTuning();
    }
    
    inline String getName(void) const noexcept {return name;}
    
    inline void setName(String newName)
    {
        name = newName;
    }
    
    Array<float> getCurrentScale()
    {
        if(aPrep->getScale() == CustomTuning)
        {
            return aPrep->getCustomScale();
        }
        
        return getTuningOffsets(aPrep->getScale());
    }
    
    Array<float> getScale(int which)
    {
        TuningSystem tuning = (TuningSystem) which;
        
        if (tuning == CustomTuning)
        {
            return aPrep->getCustomScale();
        }
        
        return getTuningOffsets(tuning);
    }
    
    Array<float> getScaleCents(int which)
    {
        Array<float> mScale = getScale(which);
        Array<float> cScale;
        
        for(int i=0; i<12; i++)
        {
            cScale.insert(i, mScale.getUnchecked(i) * 100.0);
        }
        
        return cScale;
    }
    
    Array<float> getCurrentScaleCents()
    {
        Array<float> mScale = getCurrentScale();
        Array<float> cScale;
        
        for(int i=0; i<12; i++)
        {
            cScale.insert(i, mScale.getUnchecked(i) * 100.0);
        }
        
        return cScale;
    }
    
    
    Array<Array<float>> tuningLibrary;
    
private:
    int Id;
    String name;

    
    Array<float> getTuningOffsets(TuningSystem which) {return tuningLibrary.getUnchecked(which); }
    
    
    const Array<float> tEqualTuning = Array<float>( {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.} );
	const Array<float> tJustTuning = Array<float>({ 0., .117313, .039101, .156414, -.13686, -.019547, -.174873, .019547, .136864, -.15641, -.311745, -.11731 });
	const Array<float> tPartialTuning = Array<float>({ 0., .117313, .039101, -.331291, -.13686, -.019547, -.486824, .019547, .405273, -.15641, -.311745, -.506371 });
	const Array<float> tDuodeneTuning = Array<float>({ 0., .117313, .039101, .156414, -.13686, -.019547, -.097763, .019547, .136864, -.15641, -.039101, -.11731 });
	const Array<float> tOtonalTuning = Array<float>({ 0., .049553, .039101, -.02872, -.13686, -.292191, -.486824, .019547, .405273, .058647, -.311745, -.11731 } );
	const Array<float> tUtonalTuning = Array<float>({ 0., .117313, .311745, .156414, -.405273, -.019547, .486824, .292191, .136864, .024847, -.039101, -.049553 } );
    
    JUCE_LEAK_DETECTOR(Tuning)
};

class TuningModPreparation : public ReferenceCountedObject
{
public:
    
    typedef ReferenceCountedObjectPtr<TuningModPreparation>   Ptr;
    typedef Array<TuningModPreparation::Ptr>                  PtrArr;
    typedef Array<TuningModPreparation::Ptr, CriticalSection> CSPtrArr;
    typedef OwnedArray<TuningModPreparation>                  Arr;
    typedef OwnedArray<TuningModPreparation, CriticalSection> CSArr;
    
    TuningModPreparation(TuningPreparation::Ptr p, int Id):
    Id(Id)
    {
        param.ensureStorageAllocated((int)cTuningParameterTypes.size());
        
        param.set(TuningScale, String(p->getScale()));
        param.set(TuningFundamental, String(p->getFundamental()));
        param.set(TuningOffset, String(p->getFundamentalOffset()));
        param.set(TuningA1IntervalScale, String(p->getAdaptiveIntervalScale()));
        param.set(TuningA1Inversional, String((int)p->getAdaptiveInversional()));
        param.set(TuningA1AnchorScale, String(p->getAdaptiveAnchorScale()));
        param.set(TuningA1AnchorFundamental, String(p->getAdaptiveAnchorFundamental()));
        param.set(TuningA1ClusterThresh, String(p->getAdaptiveClusterThresh()));
        param.set(TuningA1History, String(p->getAdaptiveHistory()));
        param.set(TuningCustomScale, floatArrayToString(p->getCustomScale()));
        param.set(TuningAbsoluteOffsets, floatArrayToString(p->getAbsoluteOffsets()));
        
    }
    
    
    TuningModPreparation(int Id):
    Id(Id)
    {
        param.set(TuningScale, "");
        param.set(TuningFundamental, "");
        param.set(TuningOffset, "");
        param.set(TuningA1IntervalScale, "");
        param.set(TuningA1Inversional, "");
        param.set(TuningA1AnchorScale, "");
        param.set(TuningA1AnchorFundamental, "");
        param.set(TuningA1ClusterThresh, "");
        param.set(TuningA1History, "");
        param.set(TuningCustomScale, "");
        param.set(TuningAbsoluteOffsets, "");
    }
    
    inline TuningModPreparation::Ptr duplicate(void)
    {
        TuningModPreparation::Ptr copyPrep = new TuningModPreparation(-1);
       
        copyPrep->copy(this);
        
        copyPrep->setName(this->getName() );
        
        return copyPrep;
    }
    
    inline void setId(int newId) { Id = newId; }
    inline int getId(void) const noexcept { return Id; }
    
    ~TuningModPreparation(void)
    {
        
    }
    
    inline void copy(TuningPreparation::Ptr p)
    {
        param.set(TuningScale, String(p->getScale()));
        param.set(TuningFundamental, String(p->getFundamental()));
        param.set(TuningOffset, String(p->getFundamentalOffset()));
        param.set(TuningA1IntervalScale, String(p->getAdaptiveIntervalScale()));
        param.set(TuningA1Inversional, String((int)p->getAdaptiveInversional()));
        param.set(TuningA1AnchorScale, String(p->getAdaptiveAnchorScale()));
        param.set(TuningA1AnchorFundamental, String(p->getAdaptiveAnchorFundamental()));
        param.set(TuningA1ClusterThresh, String(p->getAdaptiveClusterThresh()));
        param.set(TuningA1History, String(p->getAdaptiveHistory()));
        param.set(TuningCustomScale, floatArrayToString(p->getCustomScale()));
        param.set(TuningAbsoluteOffsets, offsetArrayToString(p->getAbsoluteOffsets()));
    }
    
    inline void copy(TuningModPreparation::Ptr p)
    {
        for (int i = TuningId+1; i < TuningParameterTypeNil; i++)
        {
            param.set(i, p->getParam((TuningParameterType)i));
        }
    }
    
    inline bool compare(TuningModPreparation::Ptr t)
    {
        return (getParam(TuningScale) == t->getParam(TuningScale) &&
                getParam(TuningFundamental) == t->getParam(TuningFundamental) &&
                getParam(TuningOffset) == t->getParam(TuningOffset) &&
                getParam(TuningA1IntervalScale) == t->getParam(TuningA1IntervalScale) &&
                getParam(TuningA1Inversional) == t->getParam(TuningA1Inversional) &&
                getParam(TuningA1AnchorScale) == t->getParam(TuningA1AnchorScale) &&
                getParam(TuningA1AnchorFundamental) == t->getParam(TuningA1AnchorFundamental) &&
                getParam(TuningA1ClusterThresh) == t->getParam(TuningA1ClusterThresh) &&
                getParam(TuningA1History) == t->getParam(TuningA1History) &&
                getParam(TuningCustomScale) == t->getParam(TuningCustomScale) &&
                getParam(TuningAbsoluteOffsets) == t->getParam(TuningAbsoluteOffsets));
            
    }

	inline void randomize()
	{
		TuningPreparation p;
		p.randomize();

		param.set(TuningScale, String(p.getScale()));
		param.set(TuningFundamental, String(p.getFundamental()));
		param.set(TuningOffset, String(p.getFundamentalOffset()));
		param.set(TuningA1IntervalScale, String(p.getAdaptiveIntervalScale()));
		param.set(TuningA1Inversional, String((int)p.getAdaptiveInversional()));
		param.set(TuningA1AnchorScale, String(p.getAdaptiveAnchorScale()));
		param.set(TuningA1AnchorFundamental, String(p.getAdaptiveAnchorFundamental()));
		param.set(TuningA1ClusterThresh, String(p.getAdaptiveClusterThresh()));
		param.set(TuningA1History, String(p.getAdaptiveHistory()));
		param.set(TuningCustomScale, floatArrayToString(p.getCustomScale()));
		param.set(TuningAbsoluteOffsets, offsetArrayToString(p.getAbsoluteOffsets()));
	}
    
    void clearAll()
    {
        for (int i = TuningId+1; i < TuningParameterTypeNil; i++)
        {
            param.set(i, "");
        }
    }
    
    ValueTree getState(void);
    
    void setState(XmlElement*);
    
    inline const String getParam(TuningParameterType type)
    {
        if (type != TuningId)   return param[type];
        else                    return "";
    }
    
    inline const StringArray getStringArray(void) { return param; }
    
    inline void setParam(TuningParameterType type, String val)
    {
        param.set(type, val);
    }
    
    void print(void)
    {
        
    }
    
    inline String getName(void) const noexcept {return name;}
    inline void setName(String newName) {name = newName;}

private:
    int Id; 
    String name;
    StringArray          param;
    
    JUCE_LEAK_DETECTOR(TuningModPreparation);
};

class TuningProcessor : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<TuningProcessor>      Ptr;
    typedef Array<TuningProcessor::Ptr>                     PtrArr;
    typedef Array<TuningProcessor::Ptr, CriticalSection>    CSArr;
    typedef OwnedArray<TuningProcessor>                          Arr;
    typedef OwnedArray<TuningProcessor, CriticalSection>         CSPtrArr;
    
    TuningProcessor(Tuning::Ptr tuning);
    ~TuningProcessor();
    
    inline void prepareToPlay(double sr) { sampleRate = sr; }
    
    //returns tuning offsets; add to integer PitchClass
    float getOffset(int midiNoteNumber, bool updateLastInterval);
    
    //for calculating adaptive tuning
    void keyPressed(int midiNoteNumber);
    void keyReleased(int midiNoteNumber);
    
    inline int getId(void) const noexcept { return tuning->getId(); }
    
    inline void setTuning(Tuning::Ptr newTuning) { tuning = newTuning;}
    inline Tuning::Ptr getTuning(void) const noexcept { return tuning; }
    
    //for cluster timing
    void processBlock(int numSamples);
    
    //for global tuning adjustment, A442, etc...
    void setGlobalTuningReference(float tuningRef) { globalTuningReference = tuningRef;}
    const float getGlobalTuningReference(void) const noexcept {return globalTuningReference;}
    
    float getLastNoteTuning() { return lastNoteTuning; }
    float getLastIntervalTuning() { return lastIntervalTuning; }
    
    int getAdaptiveFundamentalNote() { return adaptiveFundamentalNote;}
    float getAdaptiveFundamentalFreq() { return adaptiveFundamentalFreq;}
    int getAdaptiveHistoryCounter() { return adaptiveHistoryCounter;}
    int getAdaptiveClusterTimer() { return clusterTime * (1000.0 / sampleRate); }
    
    void setAdaptiveFundamentalNote(int newNote) { adaptiveFundamentalNote = newNote;}
    void setAdaptiveFundamentalFreq(float newFreq) { adaptiveFundamentalFreq = newFreq;}
    void setAdaptiveHistoryCounter(int newCounter) { adaptiveHistoryCounter = newCounter;}
    
    //reset adaptive tuning
    void adaptiveReset();
    
    inline void reset(void)
    {
        adaptiveReset();
        tuning->aPrep->copy(tuning->sPrep);
    }
    
private:
    Tuning::Ptr tuning;

    float   intervalToRatio(float interval) const noexcept { return mtof(interval + 60.) / mtof(60.); }
    float   lastNote[128];
    float   globalTuningReference = 440.; //A440
    
    float lastNoteTuning;
    float lastIntervalTuning;
    
    //adaptive tuning functions
    float   adaptiveCalculate(int midiNoteNumber) const;
    void    newNote(int midiNoteNumber, TuningSystem tuningType);
    float   adaptiveCalculateRatio(int midiNoteNumber) const;
    uint64  clusterTime;
    
    int     adaptiveFundamentalNote = 60; //moves with adaptive tuning
    float   adaptiveFundamentalFreq = mtof(adaptiveFundamentalNote);
    int     adaptiveHistoryCounter = 0;
    
    double sampleRate;
    
    JUCE_LEAK_DETECTOR(TuningProcessor);
};

#endif  // TUNING_H_INCLUDED
