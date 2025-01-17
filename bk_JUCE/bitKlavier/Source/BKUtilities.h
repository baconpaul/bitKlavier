/*
 ==============================================================================
 
 BKUtilities.h
 Created: 16 Nov 2016 11:12:15am
 Author:  Michael R Mulshine
 
 ==============================================================================
 */

#ifndef BKUTILITIES_H_INCLUDED
#define BKUTILITIES_H_INCLUDED

#include <regex>

#include "../JuceLibraryCode/JuceHeader.h"

#include "BKReferenceCountedBuffer.h"

#include "GraphicsConstants.h"

#include "AudioConstants.h"

#include "BKMenu.h"

#include "Moddable.h"

#define BK_UNIT_TESTS 0

#define NUM_EPOCHS 10

#define SAVE_ID 1
#define SAVEAS_ID 2
#define OPEN_ID 3
#define NEW_ID 4
#define CLEAN_ID 5
#define SETTINGS_ID 6
#define OPENOLD_ID 7
#define DIRECT_ID 8
#define NOSTALGIC_ID 9
#define SYNCHRONIC_ID 10
#define TUNING_ID 11
#define TEMPO_ID 12
#define MODIFICATION_ID 13
#define PIANOMAP_ID 14
#define RESET_ID 15
#define NEWGALLERY_ID 16
#define DELETE_ID 17
#define KEYMAP_ID 18
#define COPY_ID 19
#define PASTE_ID 20
#define ALIGN_VERTICAL 21
#define ALIGN_HORIZONTAL 22
#define UNDO_ID 23
#define REDO_ID 24
#define CUT_ID 25
#define EDIT_ID 26

#define KEYMAP_EDIT_ID 27
#define DIRECT_EDIT_ID 28
#define NOSTALGIC_EDIT_ID 29
#define SYNCHRONIC_EDIT_ID 30
#define TUNING_EDIT_ID 31
#define TEMPO_EDIT_ID 32
#define DIRECTMOD_EDIT_ID 33
#define NOSTALGICMOD_EDIT_ID 34
#define SYNCHRONICMOD_EDIT_ID 35
#define TUNINGMOD_EDIT_ID 36
#define TEMPOMOD_EDIT_ID 37


#define LOAD_LITE 38
#define LOAD_MEDIUM 39
#define LOAD_HEAVY 40
#define LOAD_LITEST 41

#define SHARE_EMAIL_ID 42
#define SHARE_FACEBOOK_ID 43
#define SHARE_MESSAGE_ID 44

#define RENAME_ID 45
#define COMMENT_ID 46
#define OFF_ID 47
#define KEYBOARD_ID 48

#define ABOUT_ID 49
#define EXPORT_ID 50
#define IMPORT_ID 51

#define BLENDRONIC_ID 52 //should these defines be replaced with a typedef enum for menu items?
#define BLENDRONIC_EDIT_ID 53
#define BLENDRONICMOD_EDIT_ID 54

#define CONNECTION_ID 55
#define CONNECT_ALL_ID 56
#define DISCONNECT_FROM_ID 57
#define DISCONNECT_BETWEEN_ID 58

#define RESONANCE_ID 59
#define RESONANCE_EDIT_ID 60
#define RESONANCEMOD_EDIT_ID 61

#define SOUNDFONT_ID 1000

#define MIDIOUT_ID 2000

#define SF_DEFAULT_0 800
#define SF_DEFAULT_1 801
#define SF_DEFAULT_2 802
#define SF_DEFAULT_3 803
#define SF_DEFAULT_4 804
#define SF_DEFAULT_5 805
#define SF_DEFAULT_6 806



inline PopupMenu getNewItemMenu(LookAndFeel* laf)
{
    BKPopupMenu newMenu;
    
    newMenu.addItem(KEYMAP_ID, "Keymap" + gKeymapShortcut);
    newMenu.addItem(DIRECT_ID, "Direct" + gDirectShortcut);
    newMenu.addItem(NOSTALGIC_ID, "Nostalgic" + gNostalgicShortcut);
    newMenu.addItem(SYNCHRONIC_ID, "Synchronic" + gSynchronicShortcut);
    newMenu.addItem(BLENDRONIC_ID, "Blendronic" + gBlendronicShortcut);
    newMenu.addItem(RESONANCE_ID, "Resonance" + gResonanceShortcut);
    newMenu.addItem(TUNING_ID, "Tuning" + gTuningShortcut);
    newMenu.addItem(TEMPO_ID, "Tempo" + gTempoShortcut);
    newMenu.addSeparator();
    newMenu.addItem(MODIFICATION_ID, "Modification" + gModificationShortcut);
    newMenu.addItem(PIANOMAP_ID, "Piano Map" + gPianoMapShortcut);
    newMenu.addItem(RESET_ID, "Reset" + gResetShortcut);
    newMenu.addSeparator();
    newMenu.addItem(COMMENT_ID, "Comment" + gCommentShortcut);
    
    return std::move(newMenu);
}

// What is this for?
inline PopupMenu getEditItemMenu(LookAndFeel* laf)
{
    BKPopupMenu menu;
    
    menu.addItem(KEYMAP_EDIT_ID, "Keymap");
    menu.addItem(DIRECT_EDIT_ID, "Direct");
    menu.addItem(NOSTALGIC_EDIT_ID, "Nostalgic");
    menu.addItem(SYNCHRONIC_EDIT_ID, "Synchronic");
    menu.addItem(BLENDRONIC_EDIT_ID, "Blendronic");
    menu.addItem(RESONANCE_EDIT_ID, "Resonance");
    menu.addItem(TUNING_EDIT_ID, "Tuning");
    menu.addItem(TEMPO_EDIT_ID, "Tempo");
    menu.addItem(DIRECTMOD_EDIT_ID, "Direct Mod");
    menu.addItem(NOSTALGICMOD_EDIT_ID, "Nostalgic Mod");
    menu.addItem(SYNCHRONICMOD_EDIT_ID, "Synchronic Mod");
    menu.addItem(TUNINGMOD_EDIT_ID, "Tuning Mod");
    menu.addItem(TEMPOMOD_EDIT_ID, "Tempo Mod");
    
    return std::move(menu);
}

inline PopupMenu getAlignMenu(LookAndFeel* laf)
{
    BKPopupMenu menu;
    
    menu.addItem(ALIGN_VERTICAL, "Row");
    menu.addItem(ALIGN_HORIZONTAL, "Column");
    
    return std::move(menu);
}

inline PopupMenu getEditMenuStandalone(LookAndFeel* laf, int numItemsSelected, bool onGraph = false, bool rightClick = false)
{
    BKPopupMenu menu;
    
#if JUCE_IOS
    if (!onGraph) menu.addSubMenu("Add...", getNewItemMenu(laf));
#else
    menu.addSubMenu("Add...", getNewItemMenu(laf));
#endif
    
    if (numItemsSelected)
    {
        menu.addSeparator();
        menu.addItem(COPY_ID, "Copy" + gCopyShortcut);
        menu.addItem(CUT_ID, "Cut" + gCutShortcut);
        menu.addItem(PASTE_ID, "Paste" + gPasteShortcut);
        menu.addItem(DELETE_ID, "Delete");
        
        menu.addSeparator();
        menu.addItem(UNDO_ID, "Undo" + gUndoShortcut);
        menu.addItem(REDO_ID, "Redo" + gRedoShortcut);
        
        if (numItemsSelected == 1)
        {
            menu.addSeparator();
            menu.addItem(EDIT_ID, "Edit" + gEditShortcut);
            menu.addSeparator();
            menu.addItem(CONNECTION_ID, "Make Connection" + gConnectionShortcut);
            menu.addItem(DISCONNECT_FROM_ID, "Remove Connections To Selected" + gDisconnectFromShortcut);
        }
        
        if (numItemsSelected > 1)
        {
            menu.addSeparator();
            menu.addItem(CONNECT_ALL_ID, "Connect Selected" + gConnectAllShortcut);
            menu.addItem(DISCONNECT_FROM_ID, "Remove Connections To Selected" + gDisconnectFromShortcut);
            menu.addItem(DISCONNECT_BETWEEN_ID, "Remove Connections Between Selected" + gDisconnectBetweenShortcut);
            menu.addSeparator();
            menu.addSubMenu("Align" + gAlignShortcut, getAlignMenu(laf));
        }
    }
    else if (numItemsSelected == 0)
    {
        menu.addSeparator();
        menu.addItem(PASTE_ID, "Paste" + gPasteShortcut);
        
        menu.addSeparator();
        menu.addItem(UNDO_ID, "Undo" + gUndoShortcut);
        menu.addItem(REDO_ID, "Redo" + gRedoShortcut);
    }
    
    if (!rightClick)
    {
        menu.addSeparator();
        menu.addItem(OFF_ID, "All Off" + gAllOffShortcut);
    }
    
    return std::move(menu);
}

inline PopupMenu getEditMenu(LookAndFeel* laf, int numItemsSelected, bool onGraph = false, bool rightClick = false)
{
    BKPopupMenu menu;
    
#if JUCE_IOS
    if (!onGraph) menu.addSubMenu("Add...", getNewItemMenu(laf));
#else
    menu.addSubMenu("Add...", getNewItemMenu(laf));
#endif
    
    if (numItemsSelected)
    {
        menu.addSeparator();
        menu.addItem(COPY_ID, "Copy");
        menu.addItem(CUT_ID, "Cut");
        menu.addItem(PASTE_ID, "Paste");
        menu.addItem(DELETE_ID, "Delete");
        
        menu.addSeparator();
        menu.addItem(UNDO_ID, "Undo");
        menu.addItem(REDO_ID, "Redo");
        
        if (numItemsSelected == 1)
        {
            menu.addSeparator();
            menu.addItem(EDIT_ID, "Edit" + gEditShortcut);
            menu.addSeparator();
            menu.addItem(CONNECTION_ID, "Make Connection" + gConnectionShortcut);
            menu.addItem(DISCONNECT_FROM_ID, "Remove Connections To Selected" + gDisconnectFromShortcut);
        }
        
        if (numItemsSelected > 1)
        {
            menu.addSeparator();
            menu.addItem(CONNECT_ALL_ID, "Connect Selected" + gConnectAllShortcut);
            menu.addItem(DISCONNECT_FROM_ID, "Remove Connections To Selected" + gDisconnectFromShortcut);
            menu.addItem(DISCONNECT_BETWEEN_ID, "Remove Connections Between Selected" + gDisconnectBetweenShortcut);
            menu.addSeparator();
            menu.addSubMenu("Align" + gAlignShortcut, getAlignMenu(laf));
        }
    }
    else if (numItemsSelected == 0)
    {
        menu.addSeparator();
        menu.addItem(PASTE_ID, "Paste");
        
        menu.addSeparator();
        menu.addItem(UNDO_ID, "Undo");
        menu.addItem(REDO_ID, "Redo");
    }
    if (!rightClick)
    {
        menu.addSeparator();
        menu.addItem(OFF_ID, "All Off" + gAllOffShortcut);
    }
    
    return std::move(menu);
}


typedef enum BKPlatform
{
    BKOSX,
    BKIOS,
    BKWindows,
    BKAndroid,
    BKLinux,
    BKPlatformNil
} BKPlatform;


typedef enum DisplayType
{
    DisplayDefault = 0,
    DisplayConstruction,
    DisplayKeyboard,
    DisplayTypeNil
}DisplayType;

String midiToPitchClass(int midi);

PitchClass      letterNoteToPitchClass(String note);
TuningSystem    tuningStringToTuningSystem(String tuning);

String          intArrayToString(Array<int> arr);

String          floatArrayToString(Array<float> arr);
String          arrayFloatArrayToString(Array<Array<float>> arr);
String          arrayActiveFloatArrayToString(Array<Array<float>> afarr, Array<bool> act);

String          arrayIntArrayToString(Array<Array<int>> arr);
Array<float>    stringToFloatArray(String s);
Array<Array<float>> stringToArrayFloatArray(String s);
Array<bool>     slashToFalse(String s);

String          offsetArrayToString(Array<float> arr);
String          offsetArrayToString2(Array<float> arr);
String          offsetArrayToString3(Array<float> arr, float mid);
String          boolArrayToString(Array<bool> arr);
Array<int>      stringToIntArray(String s);
Array<int>      keymapStringToIntArray(String s);

Array<float>    stringOrderedPairsToFloatArray(String s, int size, float init);

double          mtof(double f);
double          ftom(double f);

double          mtof(double f, double sr);
double          ftom(double f, double sr);


//these require inval to be between 0 and 1, and k != 1
double          dt_asymwarp(double inval, double k);
double          dt_asymwarp_inverse(double inval, double k);
double          dt_symwarp(double inval, double k);
double          dt_warpscale(double inval, double asym_k, double sym_k, double scale, double offset);

int mod(int a, int b);

String rectangleToString(Rectangle<int> rect);
String rectangleToString(Rectangle<float> rect);



BKParameterDataType getBKDataType ( SynchronicParameterType param);
BKParameterDataType getBKDataType ( TuningParameterType param);
BKParameterDataType getBKDataType ( DirectParameterType param);
BKParameterDataType getBKDataType ( NostalgicParameterType param);

inline int layerToLayerId(BKNoteType type, int layer) { return (50*type)+layer;}

inline int gainToMidiVelocity(float gain) { return sqrt(127*127*gain); }

typedef enum BKTextFieldType
{
    BKParameter = 0,
    BKModification,
    BKTFNil
    
} BKTextFieldType;

typedef enum BKEditorType
{
    BKPreparationEditor,
    BKModificationEditor,
    BKEditorTypeNil
} BKEditorType;

#endif  // BKUTILITIES_H_INCLUDED

