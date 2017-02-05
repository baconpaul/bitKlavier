/*
  ==============================================================================

    AudioConstants.h
    Created: 13 Oct 2016 12:16:10am
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef AUDIOCONSTANTS_H_INCLUDED
#define AUDIOCONSTANTS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#define TEXT_CHANGE_INTERNAL 0

#define USE_SYNTH_INTERNAL 0
#define CRAY_COOL_MUSIC_MAKER 0
#define CRAY_COOL_MUSIC_MAKER_2 0

typedef enum PitchClass {
    C = 0,
    CSharp,
    D,
    DSharp,
    E,
    F,
    FSharp,
    G,
    GSharp,
    A,
    ASharp,
    B,
    DFlat = CSharp,
    EFlat = DSharp,
    GFlat = FSharp,
    AFlat = GSharp,
    BFlat = ASharp,
} PitchClass;

#pragma mark - Synthesiser/Sampler
typedef enum PianoSamplerNoteType {
    Normal = 0,
    NormalFixedStart,
    FixedLength,
    FixedLengthFixedStart,
    PianoSamplerNoteTypeNil
} PianoSamplerNoteType;

typedef enum PianoSamplerNoteDirection {
    Forward = 0,
    Reverse,
    PianoSamplerPlaybackDirectionNil
} PianoSamplerNoteDirection;

typedef enum BKNoteType {
    SynchronicNote = 0,
    NostalgicNote,
    DirectNote,
    MainNote,
    HammerNote,
    ResonanceNote,
    BKNoteTypeNil,
} BKNoteType;

typedef enum BKPreparationType {
    PreparationTypeDirect = 0,
    PreparationTypeSynchronic,
    PreparationTypeNostalgic,
    PreparationTypeTuning,
    BKPreparationTypeNil,
} BKPreparationType;

static const std::vector<std::string> cPreparationTypes = {
    "Main/Direct",
    "Synchronic",
    "Nostalgic",
    "Tuning"
};






static const std::vector<std::string> cPianoName = {
    "Piano1",
    "Piano2",
    "Piano3",
    "Piano4",
    "Piano5",
    "Piano6",
    "Piano7",
    "Piano8",
    "Piano9",
    "Piano10",
    "Piano11",
    "Piano12"
};

static const int aMaxNumPianos = cPianoName.size();


static const std::vector<std::string> cPrepMapName= {
    "PrepMap1",
    "PrepMap2",
    "PrepMap3",
    "PrepMap4",
    "PrepMap5",
    "PrepMap6",
    "PrepMap7",
    "PrepMap8",
    "PrepMap9",
    "PrepMap10",
    "PrepMap11",
    "PrepMap12"
};

static const int aMaxNumPreparationKeymaps = cPrepMapName.size();

typedef enum BKParameterDataType
{
    BKVoid = 0,
    BKBool,
    BKInt,
    BKFloat,
    BKIntArr,
    BKFloatArr,
    BKParameterTypeNil
} BKParameterDataType;

#pragma mark - General
typedef enum GeneralParameterType
{
    GeneralTuningFundamental,
    GeneralGlobalGain,
    GeneralSynchronicGain,
    GeneralNostalgicGain,
    GeneralDirectGain,
    GeneralResonanceGain,
    GeneralHammerGain,
    GeneralTempoMultiplier,
    GeneralResAndHammer,
    GeneralInvertSustain,
    GeneralParameterTypeNil
    
} GeneralParameterType;

static const std::vector<std::string> cGeneralParameterTypes = {
    "GeneralTuningFund",
    "GeneralGlobalGain",
    "GeneralSyncGain",
    "GeneralNostGain",
    "GeneralDrctGain",
    "GeneralResGain",
    "GeneralHamGain",
    "GeneralTempoMult",
    "GeneralResAndHam",
    "GeneralInvertSus"
    
};

#pragma mark - Synchronic
typedef enum AdaptiveTempo1Mode {
    TimeBetweenNotes,
    NoteLength,
    AdaptiveTempo1ModeNil
}AdaptiveTempo1Mode;

typedef enum SynchronicSyncMode {
    FirstNoteOnSync = 0,
    AnyNoteOnSync,
    LastNoteOffSync,
    AnyNoteOffSync,
    SynchronicSyncModeNil
} SynchronicSyncMode;

typedef enum SynchronicParameterType {
    SynchronicId = 0,
    SynchronicTuning,
    SynchronicTempo,
    SynchronicNumPulses,
    SynchronicClusterMin,
    SynchronicClusterMax,
    SynchronicClusterThresh,
    SynchronicMode,
    SynchronicBeatsToSkip,
    SynchronicBeatMultipliers,
    SynchronicLengthMultipliers,
    SynchronicAccentMultipliers,
    SynchronicTranspOffsets,
    AT1Mode,
    AT1History,
    AT1Subdivisions,
    AT1Min,
    AT1Max,
    SynchronicParameterTypeNil
} SynchronicParameterType;

static const std::vector<BKParameterDataType> cSynchronicDataTypes = {
    BKInt,
    BKInt,
    BKFloat,
    BKInt,
    BKInt,
    BKInt,
    BKFloat,
    BKInt,
    BKInt,
    BKFloatArr,
    BKFloatArr,
    BKFloatArr,
    BKFloatArr,
    BKFloat,
    BKInt,
    BKFloat,
    BKFloat,
    BKFloat
};

static const std::vector<std::string> cSynchronicParameterTypes = {
    "Synchronic Id",
    "Tuning Id",
    "Tempo",
    "NumPulses",
    "ClusterMin",
    "ClusterMax",
    "ClusterThresh",
    "Mode",
    "BeatsToSkip",
    "BeatMults",
    "LengthMults",
    "AccentMults",
    "TranspOffsets",
    "AT1Mode",
    "AT1History",
    "AT1Subdivs",
    "AT1Min",
    "AT1Max"
};

#pragma mark - Nostalgic
typedef enum NostalgicSyncMode {
    NoteLengthSync = 0, //reverse note length set by played note length
    SynchronicSync,     //reverse note length set by next synchronic pulse
    NostalgicSyncModeNil
} NostalgicSyncMode;

typedef enum NostalgicParameterType {
    NostalgicId = 0,
    NostalgicTuning,
    NostalgicWaveDistance,
    NostalgicUndertow,
    NostalgicTransposition,
    NostalgicGain,
    NostalgicLengthMultiplier,
    NostalgicBeatsToSkip,
    NostalgicMode,
    NostalgicSyncTarget,
    NostalgicParameterTypeNil
    
} NostalgicParameterType;

static const std::vector<BKParameterDataType> cNostalgicDataTypes =
{
    BKInt,
    BKInt,
    BKInt,
    BKInt,
    BKFloat,
    BKFloat,
    BKFloat,
    BKInt,
    BKInt
};

static const std::vector<std::string> cNostalgicParameterTypes = {
    "Nostalgic Id",
    "Tuning Id",
    "WaveDistance",
    "Undertow",
    "Transposition",
    "Gain",
    "LengthMult",
    "BeatsToSkip",
    "Length Mode",
    "SyncTarget"
};

#pragma mark - Direct
typedef enum DirectParameterType
{
    DirectId = 0,
    DirectTuning,
    DirectTransposition,
    DirectGain,
    DirectResGain,
    DirectHammerGain,
    DirectParameterTypeNil,
    
} DirectParameterType;


static const std::vector<BKParameterDataType> cDirectDataTypes = {
    BKInt,
    BKInt,
    BKFloat,
    BKFloat,
    BKFloat,
    BKFloat
};

static const std::vector<std::string> cDirectParameterTypes = {
    "Direct Id",
    "Tuning Id",
    "Transposition",
    "Gain",
    "ResGain",
    "HammerGain"
};

#pragma mark - Tuning
typedef enum TuningParameterType
{
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
    
} TuningParameterType;

static const std::vector<BKParameterDataType> cTuningDataTypes = {
    BKInt,
    BKInt,
    BKInt,
    BKFloat,
    BKInt,
    BKBool,
    BKInt,
    BKInt,
    BKInt,
    BKInt,
    BKFloatArr
};

static const std::vector<std::string> cTuningParameterTypes = {
    "Tuning Id",
    "Scale",
    "Fundamental",
    "Offset",
    "A1IntervalScale",
    "A1Inversional",
    "A1AnchorScale",
    "A1AnchorFund",
    "A1ClusterThresh",
    "A1History",
    "CustomScale"
};


#pragma mark - PrepMap
typedef enum PrepMapTFIndex {
    PrepMapKeymapId = 0,
    PrepMapPreparationId,
    PrepMapParameterTypeNil
    
} PrepMapTFIndex;

static const std::vector<std::string> cPrepMapParameterTypes = {
    "PrepMapKeymap Id",
    "PrepMapPreparation Id"
};

#pragma mark - Piano
static const std::vector<std::string> cPianoParameterTypes = {
    "Piano"
};

typedef enum PianoCBIndex {
    PianoCBPiano = 0,
    PianoCBIndexNil
} PianoCBIndex;

#pragma mark - Keymap
typedef enum KeymapParameterType
{
    KeymapId = 0,
    KeymapField,
    KeymapParameterTypeNil
    
} KeymapParameterType;

static const std::vector<std::string> cKeymapParameterTypes = {
    "Keymap Id",
    "Keymap"
};

typedef enum BKSampleLoadType
{
    BKLoadLite,
    BKLoadMedium,
    BKLoadHeavy,
    BKLoadNil
    
}BKSampleLoadType;

static const std::vector<std::string> cBKSampleLoadTypes = {
    "Load Light",
    "Load Medium",
    "Load Heavy"
};


// Globals
static const int aMaxNumKeymaps = 30;
static const int aMaxNumPreparationsPerPrepMap = 12;
static const int aMaxTotalPreparations = aMaxNumPreparationKeymaps * aMaxNumPreparationsPerPrepMap;
static const int aMaxTuningPreparations = aMaxTotalPreparations * 3;

static const float aGlobalGain = 0.5; //make this user settable

static const float aMaxSampleLengthSec = 30.0f;
static const float aRampOnTimeSec = 0.004f;
static const float aRampOffTimeSec = 0.03f; //was .004. don't actually use these anymore...
static const int aNumScaleDegrees = 12;
static const int aRampUndertowCrossMS = 50;

// Sample layers
static const int aNumSampleLayers = 2;

static const int aVelocityThresh_Eight[9] = {
    0,
    30,
    50,
    68,
    84,
    98,
    110,
    120,
    128
};

static const int aVelocityThresh_Four[5] = {
    0,
    42,
    76,
    104,
    128
};

static const int aVelocityThresh_Two[3] = {
    0,
    76,
    128
};

static const int aResonanceVelocityThresh[4] = {
    0,
    40,
    85,
    128
};


typedef enum TuningSystem {
    PartialTuning = 0,
    JustTuning,
    EqualTemperament,
    DuodeneTuning,
    OtonalTuning,
    UtonalTuning,
    AdaptiveTuning,
    AdaptiveAnchoredTuning,
    CustomTuning
} Tuning;

#endif  // AUDIOCONSTANTS_H_INCLUDED
