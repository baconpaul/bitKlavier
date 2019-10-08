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
#include "Tuning.h"
#include "Tempo.h"
#include "Blendronic.h"

class Modification
{
public:
    Modification(int Id, int numParams):
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
    
protected:
    int             Id;
    
    Array<int>      targets;
    Array<bool>     dirty;
    
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
    
    DirectModification(int Id):
    Modification(Id, DirectParameterTypeNil),
    DirectPreparation()
    {
        DBG("dirtysize: " + String(dirty.size()));
    }

    ~DirectModification(void)
    {
        
    }
    
    inline DirectModification::Ptr duplicate(void)
    {
        DirectModification::Ptr mod = new DirectModification(-1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (DirectModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        
        DirectPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModDirect);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        
        ValueTree dirtyVT( "dirty");
        int count = 0;
        for (auto b : dirty)
        {
            dirtyVT.setProperty( "d" + String(count++), (int)b, 0);
        }
        prep.addChild(dirtyVT, -1, 0);
        
        prep.addChild(DirectPreparation::getState(), -1, 0);
        
        return prep;
    }
    
    inline void setState(XmlElement* e)
    {
        
        Id = e->getStringAttribute("Id").getIntValue();
        
        String n = e->getStringAttribute("name");
        
        if (n != String())     setName(n);
        else                        setName(String(Id));
    
        XmlElement* dirtyXml = e->getChildByName("dirty");
        XmlElement* paramsXml = e->getChildByName("params");
        
        if (dirtyXml != nullptr && paramsXml != nullptr)
        {
            dirty.clear();
            for (int k = 0; k < DirectParameterTypeNil; k++)
            {
                String attr = dirtyXml->getStringAttribute("d" + String(k));
                
                if (attr == String()) dirty.add(false);
                else
                {
                    dirty.add((bool)attr.getIntValue());
                }
            }
            
            DirectPreparation::setState(paramsXml);
        }
        else
        {
            setStateOld(e);
        }
    }
    
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
    
    SynchronicModification(int Id):
    Modification(Id, SynchronicParameterTypeNil),
    SynchronicPreparation()
    {
    }
    
    ~SynchronicModification(void)
    {
        
    }
    
    inline SynchronicModification::Ptr duplicate(void)
    {
        SynchronicModification::Ptr mod = new SynchronicModification(-1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (SynchronicModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        
        SynchronicPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModSynchronic);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        
        ValueTree dirtyVT( "dirty");
        int count = 0;
        for (auto b : dirty)
        {
            dirtyVT.setProperty( "d" + String(count++), (int)b, 0);
        }
        prep.addChild(dirtyVT, -1, 0);
        
        prep.addChild(SynchronicPreparation::getState(), -1, 0);
        
        return prep;
    }
    
    inline void setState(XmlElement* e)
    {
        Id = e->getStringAttribute("Id").getIntValue();
        
        String n = e->getStringAttribute("name");
        
        if (n != String())     setName(n);
        else                        setName(String(Id));
        
        XmlElement* dirtyXml = e->getChildByName("dirty");
        XmlElement* paramsXml = e->getChildByName("params");
        
        if (dirtyXml != nullptr && paramsXml != nullptr)
        {
            dirty.clear();
            for (int k = 0; k < SynchronicParameterTypeNil; k++)
            {
                String attr = dirtyXml->getStringAttribute("d" + String(k));
                
                if (attr == String()) dirty.add(false);
                else
                {
                    dirty.add((bool)attr.getIntValue());
                }
            }
            
            SynchronicPreparation::setState(paramsXml);
        }
        else
        {
            setStateOld(e);
        }
    }
    
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
    
    NostalgicModification(int Id):
    Modification(Id, NostalgicParameterTypeNil),
    NostalgicPreparation()
    {
    }
    
    ~NostalgicModification(void)
    {
        
    }
    
    inline NostalgicModification::Ptr duplicate(void)
    {
        NostalgicModification::Ptr mod = new NostalgicModification(-1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (NostalgicModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        
        NostalgicPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModNostalgic);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        
        ValueTree dirtyVT( "dirty");
        int count = 0;
        for (auto b : dirty)
        {
            dirtyVT.setProperty( "d" + String(count++), (int)b, 0);
        }
        prep.addChild(dirtyVT, -1, 0);
        
        prep.addChild(NostalgicPreparation::getState(), -1, 0);
        
        return prep;
    }
    
    inline void setState(XmlElement* e)
    {
        Id = e->getStringAttribute("Id").getIntValue();
        
        String n = e->getStringAttribute("name");
        
        if (n != String())     setName(n);
        else                        setName(String(Id));
        
        XmlElement* dirtyXml = e->getChildByName("dirty");
        XmlElement* paramsXml = e->getChildByName("params");
        
        if (dirtyXml != nullptr && paramsXml != nullptr)
        {
            dirty.clear();
            for (int k = 0; k < NostalgicParameterTypeNil; k++)
            {
                String attr = dirtyXml->getStringAttribute("d" + String(k));
                
                if (attr == String()) dirty.add(false);
                else
                {
                    dirty.add((bool)attr.getIntValue());
                }
            }
            
            NostalgicPreparation::setState(paramsXml);
        }
        else
        {
            setStateOld(e);
        }
    }
    
    void setStateOld(XmlElement* e);
    
private:
    
    JUCE_LEAK_DETECTOR(NostalgicModification)
};


class TuningModification :
public Modification,
public TuningPreparation
{
public:
    typedef ReferenceCountedObjectPtr<TuningModification>   Ptr;
    typedef Array<TuningModification::Ptr>                  PtrArr;
    
    TuningModification(int Id):
    Modification(Id, TuningParameterTypeNil),
    TuningPreparation()
    {
        //Array<bool> tetherWeightsActive;
        //Array<bool> springWeightsActive;
        
        for(int i=0; i<128; i++) tetherWeightsActive.insert(i, false);
        for(int i=0; i<12; i++) springWeightsActive.insert(i, false);
    }
    
    ~TuningModification(void)
    {
        
    }
    
    inline TuningModification::Ptr duplicate(void)
    {
        TuningModification::Ptr mod = new TuningModification(-1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (TuningModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        
        TuningPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModTuning);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        
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
    
    TempoModification(int Id):
    Modification(Id, TempoParameterTypeNil),
    TempoPreparation()
    {
        
    }
    
    ~TempoModification(void)
    {
        
    }
    
    inline TempoModification::Ptr duplicate(void)
    {
        TempoModification::Ptr mod = new TempoModification(-1);
        
        mod->copy(this);
        
        return mod;
    }
    
    inline void copy (TempoModification::Ptr mod)
    {
        setName(mod->getName() + "copy");
        
        TempoPreparation::copy(mod);
    }
    
    inline ValueTree getState(void)
    {
        ValueTree prep(vtagModTempo);
        
        prep.setProperty( "Id", Id, 0);
        prep.setProperty( "name", getName(), 0);
        
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


#endif  // MODIFICATION_H_INCLUDED

class BlendronicModification :
	public Modification,
	public BlendronicPreparation
{
public:
	typedef ReferenceCountedObjectPtr<BlendronicModification>   Ptr;
	typedef Array<BlendronicModification::Ptr>                  PtrArr;

	BlendronicModification(int Id) :
		Modification(Id, BlendronicParameterTypeNil),
		BlendronicPreparation()
	{
	}

	~BlendronicModification(void)
	{

	}

	inline BlendronicModification::Ptr duplicate(void)
	{
		BlendronicModification::Ptr mod = new BlendronicModification(-1);

		mod->copy(this);

		return mod;
	}

	inline void copy(BlendronicModification::Ptr mod)
	{
		setName(mod->getName() + "copy");

		BlendronicPreparation::copy(mod);
	}

	//need to figure out how to get around ValueTrees
	inline ValueTree getState(void)
	{
		ValueTree prep(vtagModBlendronic);

		prep.setProperty("Id", Id, 0);
		prep.setProperty("name", getName(), 0);

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

		if (n != String())     setName(n);
		else                        setName(String(Id));

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
