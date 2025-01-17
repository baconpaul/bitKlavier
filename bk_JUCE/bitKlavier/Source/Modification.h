/*
  ==============================================================================

    Modification.h
    Created: 1 Feb 2017 5:32:38pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef MODIFICATION_H_INCLUDED
#define MODIFICATION_H_INCLUDED


#include "BKUtilities.h"
#include "Direct.h"
#include "Nostalgic.h"
#include "Synchronic.h"
#include "Resonance.h"
#include "Tuning.h"
#include "Tempo.h"
#include "Blendronic.h"

class Modification
{
public:
    Modification(BKAudioProcessor& processor, int Id, int numParams):
    altMod(false),
    processor(processor),
    Id(Id)
    {
        dirty.ensureStorageAllocated(numParams);
        for (int i = 0; i < numParams; i++) dirty.add(false);
    }
    
    ~Modification()
    {
        
    }
    
    inline void setId(int I){ Id = I; }
    inline void setTargets(Array<int> targ) { targets = targ; }

    inline const int getId(void){return Id;}
    inline const Array<int> getTargets(void) { return targets; }
    
    inline void addTarget(int targ) { targets.addIfNotAlreadyThere(targ); }
    
    inline void removeTarget(int targ)
    {
        for (int i = targets.size(); --i >= 0;)
        {
            if (targets[i] == targ) targets.remove(i);
        }
    }
    
    inline void reset(void)
    {
        for (int i = 0; i < dirty.size(); i++) dirty.setUnchecked(i, false);
    }
    
    inline Array<bool> getDirty(void)
    {
        return dirty;
    }
    
    inline bool getDirty(int param)
    {
        return dirty[param];
    }
    
    inline void setDirty(int param)
    {
        dirty.setUnchecked(param, true);
        
    }
    
    inline void setClean(int param)
    {
        dirty.setUnchecked(param, false);
    }
    
    inline Keymap::PtrArr getKeymaps()
    {
        return keymaps;
    }
    
    inline void setKeymaps(Keymap::PtrArr km)
    {
        keymaps = km;
    }
    
    bool altMod;
    
protected:
    BKAudioProcessor& processor;
    
    int             Id;
    
    Array<int>      targets;
    Array<bool>     dirty;
    Keymap::PtrArr keymaps;
    
private:
    
    JUCE_LEAK_DETECTOR(Modification)
};

class DirectModification :
public Modification,
public DirectPreparation
{
public:
    typedef ReferenceCountedObjectPtr<DirectModification>   Ptr;
    typedef Array<DirectModification::Ptr>                  PtrArr;
    
    DirectModification(BKAudioProcessor& processor, int Id);
    ~DirectModification(void)
    {
        
    }
    
    inline DirectModification::Ptr duplicate(void)
    {
        DirectModification::Ptr mod = new DirectModification(processor, -1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (DirectModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        altMod = mod->altMod;
        DirectPreparation::copy(mod);
    }
    
    ValueTree getState(void);
    void setState(XmlElement* e);
    void setStateOld(XmlElement* e);
    
private:

    JUCE_LEAK_DETECTOR(DirectModification)
};

class SynchronicModification :
public Modification,
public SynchronicPreparation
{
public:
    typedef ReferenceCountedObjectPtr<SynchronicModification>   Ptr;
    typedef Array<SynchronicModification::Ptr>                  PtrArr;
    
    SynchronicModification(BKAudioProcessor& processor, int Id);
    ~SynchronicModification(void)
    {
        
    }
    
    inline SynchronicModification::Ptr duplicate(void)
    {
        SynchronicModification::Ptr mod = new SynchronicModification(processor, -1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (SynchronicModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        altMod = mod->altMod;
        SynchronicPreparation::copy(mod);
    }
    
    ValueTree getState(void);
    void setState(XmlElement* e);
    void setStateOld(XmlElement* e);
    
private:
    
    JUCE_LEAK_DETECTOR(SynchronicModification)
};

class NostalgicModification :
public Modification,
public NostalgicPreparation
{
public:
    typedef ReferenceCountedObjectPtr<NostalgicModification>   Ptr;
    typedef Array<NostalgicModification::Ptr>                  PtrArr;
    
    NostalgicModification(BKAudioProcessor& processor, int Id);
    ~NostalgicModification(void)
    {
        
    }
    
    inline NostalgicModification::Ptr duplicate(void)
    {
        NostalgicModification::Ptr mod = new NostalgicModification(processor, -1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (NostalgicModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        altMod = mod->altMod;
        NostalgicPreparation::copy(mod);
    }
    
    ValueTree getState(void);
    void setState(XmlElement* e);
    void setStateOld(XmlElement* e);
    
private:
    
    JUCE_LEAK_DETECTOR(NostalgicModification)
};

class ResonanceModification :
public Modification,
public ResonancePreparation
{
public:
    typedef ReferenceCountedObjectPtr<ResonanceModification>   Ptr;
    typedef Array<ResonanceModification::Ptr>                  PtrArr;
    
    ResonanceModification(BKAudioProcessor& processor, int Id);
    ~ResonanceModification(void)
    {
        
    }
    
    inline ResonanceModification::Ptr duplicate(void)
    {
        ResonanceModification::Ptr mod = new ResonanceModification(processor, -1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (ResonanceModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        altMod = mod->altMod;
        ResonancePreparation::copy(mod);
    }
    
    ValueTree getState(void);
    void setState(XmlElement* e);
    // void setStateOld(XmlElement* e);
    
private:
    
    JUCE_LEAK_DETECTOR(ResonanceModification)
};



class TuningModification :
public Modification,
public TuningPreparation
{
public:
    typedef ReferenceCountedObjectPtr<TuningModification>   Ptr;
    typedef Array<TuningModification::Ptr>                  PtrArr;
    
    TuningModification(BKAudioProcessor& processor, int Id);
    ~TuningModification(void)
    {
        
    }
    
    inline TuningModification::Ptr duplicate(void)
    {
        TuningModification::Ptr mod = new TuningModification(processor, -1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (TuningModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        altMod = mod->altMod;
        TuningPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModTuning);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        prep.setProperty("alt", altMod, 0);
        ValueTree dirtyVT( "dirty");
        int count = 0;
        for (auto b : dirty)
        {
            dirtyVT.setProperty( "d" + String(count++), (int)b, 0);
        }
        prep.addChild(dirtyVT, -1, 0);
        
        prep.addChild(TuningPreparation::getState(), -1, 0);
        
        return prep;
    }
    
    inline void setState(XmlElement* e)
    {
        Id = e->getStringAttribute("Id").getIntValue();
        
        String n = e->getStringAttribute("name");
        
        altMod = e->getBoolAttribute("alt", false);
        
        if (n != String())     setName(n);
        else                        setName(String(Id));
        
        XmlElement* dirtyXml = e->getChildByName("dirty");
        XmlElement* paramsXml = e->getChildByName("params");
        
        if (dirtyXml != nullptr && paramsXml != nullptr)
        {
            dirty.clear();
            for (int k = 0; k < TuningParameterTypeNil; k++)
            {
                String attr = dirtyXml->getStringAttribute("d" + String(k));
                
                if (attr == String()) dirty.add(false);
                else
                {
                    dirty.add((bool)attr.getIntValue());
                }
            }
            
            TuningPreparation::setState(paramsXml);
        }
        else
        {
            setStateOld(e);
        }
    }
    
    void setStateOld(XmlElement* e);
    
    inline void setTetherWeightsActive(Array<bool> a) { tetherWeightsActive = a; }
    inline Array<bool> getTetherWeightsActive(void) { return tetherWeightsActive; }
    
    inline void setSpringWeightsActive(Array<bool> a) { springWeightsActive = a; }
    inline Array<bool> getSpringWeightsActive(void) { return springWeightsActive; }
    
    inline bool getTetherWeightActive(int i) { return tetherWeightsActive[i]; }
    inline void setTetherWeightActive(int i, bool a) { tetherWeightsActive.setUnchecked(i, a); }
    
    inline bool getSpringWeightActive(int i) { return springWeightsActive[i]; }
    inline void setSpringWeightActive(int i, bool a) { springWeightsActive.setUnchecked(i, a); }
    
private:
    
    Array<bool> tetherWeightsActive;
    Array<bool> springWeightsActive;
    
    JUCE_LEAK_DETECTOR(TuningModification)
};



class TempoModification :
public Modification,
public TempoPreparation
{
public:
    typedef ReferenceCountedObjectPtr<TempoModification>   Ptr;
    typedef Array<TempoModification::Ptr>                  PtrArr;
    
    TempoModification(BKAudioProcessor& processor, int Id);
    ~TempoModification(void)
    {
        
    }
    
    inline TempoModification::Ptr duplicate(void)
    {
        TempoModification::Ptr mod = new TempoModification(processor, -1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (TempoModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        altMod = mod->altMod;
        TempoPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModTempo);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        prep.setProperty("alt", altMod, 0);
        ValueTree dirtyVT( "dirty");
        int count = 0;
        for (auto b : dirty)
        {
            dirtyVT.setProperty( "d" + String(count++), (int)b, 0);
        }
        prep.addChild(dirtyVT, -1, 0);
        
        prep.addChild(TempoPreparation::getState(), -1, 0);
        
        return prep;
    }
    
    inline void setState(XmlElement* e)
    {
        Id = e->getStringAttribute("Id").getIntValue();
        
        String n = e->getStringAttribute("name");
        
        altMod = e->getBoolAttribute("alt", false);
        
        if (n != String())     setName(n);
        else                        setName(String(Id));
        
        XmlElement* dirtyXml = e->getChildByName("dirty");
        XmlElement* paramsXml = e->getChildByName("params");
        
        if (dirtyXml != nullptr && paramsXml != nullptr)
        {
            dirty.clear();
            for (int k = 0; k < TempoParameterTypeNil; k++)
            {
                String attr = dirtyXml->getStringAttribute("d" + String(k));
                
                if (attr == String()) dirty.add(false);
                else
                {
                    dirty.add((bool)attr.getIntValue());
                }
            }
            
            TempoPreparation::setState(paramsXml);
        }
        else
        {
            setStateOld(e);
        }
    }
    
    void setStateOld(XmlElement* e);
    
private:
    
    JUCE_LEAK_DETECTOR(TempoModification)
};

class BlendronicModification :
	public Modification,
	public BlendronicPreparation
{
public:
	typedef ReferenceCountedObjectPtr<BlendronicModification>   Ptr;
	typedef Array<BlendronicModification::Ptr>                  PtrArr;

    BlendronicModification(BKAudioProcessor& processor, int Id);
	~BlendronicModification(void)
	{

	}

	inline BlendronicModification::Ptr duplicate(void)
	{
		BlendronicModification::Ptr mod = new BlendronicModification(processor, -1);

		mod->copy(this);

		return mod;
	}

	inline void copy(BlendronicModification::Ptr mod)
	{
		setName(mod->getName() + "copy");
        altMod = mod->altMod;
		BlendronicPreparation::copy(mod);
	}

	//need to figure out how to get around ValueTrees
	inline ValueTree getState(void)
	{
		ValueTree prep(vtagModBlendronic);

		prep.setProperty("Id", Id, 0);
		prep.setProperty("name", getName(), 0);
        prep.setProperty("alt", altMod, 0);
		ValueTree dirtyVT("dirty");
		int count = 0;
		for (auto b : dirty)
		{
			dirtyVT.setProperty("d" + String(count++), (int)b, 0);
		}
		prep.addChild(dirtyVT, -1, 0);

		prep.addChild(BlendronicPreparation::getState(), -1, 0);

		return prep;
	}

	inline void setState(XmlElement* e)
	{
		Id = e->getStringAttribute("Id").getIntValue();

		String n = e->getStringAttribute("name");

        altMod = e->getBoolAttribute("alt", false);
        
        if (n != String())     setName(n);
		else                   setName(String(Id));

		XmlElement* dirtyXml = e->getChildByName("dirty");
		XmlElement* paramsXml = e->getChildByName("params");

		if (dirtyXml != nullptr && paramsXml != nullptr)
		{
			dirty.clear();
			for (int k = 0; k < BlendronicParameterTypeNil; k++)
			{
				String attr = dirtyXml->getStringAttribute("d" + String(k));

				if (attr == String()) dirty.add(false);
				else
				{
					dirty.add((bool)attr.getIntValue());
				}
			}

			BlendronicPreparation::setState(paramsXml);
		}
		else
		{
			setStateOld(e);
		}
	}

	void setStateOld(XmlElement* e);

private:

	JUCE_LEAK_DETECTOR(BlendronicModification)
};

#endif  // MODIFICATION_H_INCLUDED
