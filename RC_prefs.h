/*
** RC_prefs.h  ?  Editor Preferences Window
**
** Nachbau des "Editor Preferences"-Fensters (siehe Screenshot).
**
** Ben?tigte Libraries (zus?tzlich zu den bereits in RC_setup.h
** ge?ffneten):
**   checkbox.gadget   (CheckBoxBase)
**   integer.gadget    (IntegerBase)
**
** Einbinden in RC_setup.h:
**   struct Library *CheckBoxBase = NULL;
**   struct Library *IntegerBase  = NULL;
**
**   In openLibraries():
**     CheckBoxBase = OpenLibrary("gadgets/checkbox.gadget", 0L);
**     IntegerBase  = OpenLibrary("gadgets/integer.gadget",  0L);
**
**   In closeLibraries():
**     if(CheckBoxBase) CloseLibrary(CheckBoxBase);
**     if(IntegerBase)  CloseLibrary(IntegerBase);
**
** Verwendung:
**   struct EditorPrefs prefs = { FALSE,FALSE,FALSE,FALSE,
**                                TRUE, FALSE,FALSE,FALSE,FALSE,
**                                76, 8 };
**   openPrefsWindow(window, &prefs);
**   // Nach R?ckkehr enth?lt prefs die (ggf. ge?nderten) Werte.
*/

#ifndef RC_PREFS_H
#define RC_PREFS_H

#include <proto/checkbox.h>
#include <proto/integer.h>
#include <gadgets/checkbox.h>
#include <gadgets/integer.h>

/* ------------------------------------------------------------------ */
/* Preferences-Datenstruktur                                           */
/* ------------------------------------------------------------------ */

struct EditorPrefs
{
    /* Gruppe 1 */
    BOOL autoIndent;
    BOOL wordWrap;
    BOOL stripEOL;
    BOOL autoPadEOL;

    /* Gruppe 2 */
    BOOL blockCursor;
    BOOL flashCursor;
    BOOL searchZoom;
    BOOL newSlider;
    BOOL showLFs;
    BOOL showTABs;

    /* Gruppe 3 ? numerisch */
    LONG wrapMargin;
    LONG tabSize;
};

/* ------------------------------------------------------------------ */
/* GadgetIDs f?r das Prefs-Fenster                                     */
/* ------------------------------------------------------------------ */

enum
{
    PGID_MAIN = 0,

    /* Gruppe 1 */
    PGID_AUTOINDENT,
    PGID_STRIPEDOL,
    PGID_WORDWRAP,
    PGID_AUTOPADEDOL,

    /* Gruppe 2 */
    PGID_BLOCKCURSOR,
    PGID_FLASHCURSOR,
    PGID_SEARCHZOOM,
    PGID_NEWSLIDER,
    PGID_SHOWLFS,
    PGID_SHOWTABS,

    /* Gruppe 3 */
    PGID_WRAPMARGIN,
    PGID_TABSIZE,

    /* Buttons */

    PGID_ACCEPT,
    PGID_SAVE,
    PGID_CANCEL,


    /*nicht direkt verwendet */
    PGID_HORIZ_33, /* Layout-Objekt f?r die Buttons (nicht direkt verwendet) */

    PGID_LAST
};

/* ------------------------------------------------------------------ */
/* Hilfsmakro: ein Label+Checkbox-Paar nebeneinander                   */
/* LAYOUT_AddChild ? LabelObject ? End,                                */
/*                   CHILD_Label ?                                     */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* openPrefsWindow()                                                   */
/*   ?ffnet das Preferences-Fenster modal (eigene Event-Loop).         */
/*   Gibt TRUE zur?ck wenn "Accept" gedr?ckt wurde,                    */
/*   FALSE bei "Cancel" oder Schliessen.                               */
/* ------------------------------------------------------------------ */

BOOL openPrefsWindow(struct Window *parentWin, struct EditorPrefs *prefs)
{
    Object *prefsWin;
    struct Window *pw;
    struct Gadget *pg[PGID_LAST];

    ULONG signal, result;
    BOOL  done   = FALSE;
    BOOL  accept = FALSE;

    /* tempor?re Arbeitskopie ? erst bei Accept ?bernehmen */
    struct EditorPrefs tmp = *prefs;

    prefsWin = WindowObject,
        WA_Title,        "Editor Preferences",
        WA_ScreenTitle,  "RC ? Editor Preferences",
        WA_DragBar,      TRUE,
        WA_DepthGadget,  TRUE,
        WA_CloseGadget,  TRUE,
        WA_Activate,     TRUE,
        WA_Width,        380,
        WA_AutoAdjust,   TRUE,
        WA_NewLookMenus, FALSE,

        WINDOW_ParentGroup, pg[PGID_MAIN] = VGroupObject,
            LAYOUT_SpaceOuter, TRUE,
            LAYOUT_DeferLayout, TRUE,

            /* ======================================================= */
            /* Gruppe 1: Auto Indent / Strip EOL / Word Wrap / AutoPad  */
            /* ======================================================= */
            LAYOUT_AddChild, VGroupObject,
                LAYOUT_BevelStyle, BVS_GROUP,
                LAYOUT_SpaceInner, TRUE,

                LAYOUT_AddChild, HGroupObject,
                    LAYOUT_SpaceOuter, FALSE,

                    /* linke Spalte */
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_AUTOINDENT] = CheckBoxObject,
                            GA_ID,       PGID_AUTOINDENT,
                            GA_Text,     "_Auto Indent",
                            GA_Selected, tmp.autoIndent,
                            GA_RelVerify, TRUE,
                        End,
                    End,

                    /* rechte Spalte */
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_STRIPEDOL] = CheckBoxObject,
                            GA_ID,       PGID_STRIPEDOL,
                            GA_Text,     "Strip _EOL",
                            GA_Selected, tmp.stripEOL,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                End, /* HGroup Zeile 1 */

                LAYOUT_AddChild, HGroupObject,
                    LAYOUT_SpaceOuter, FALSE,

                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_WORDWRAP] = CheckBoxObject,
                            GA_ID,       PGID_WORDWRAP,
                            GA_Text,     "_Word Wrap",
                            GA_Selected, tmp.wordWrap,
                            GA_RelVerify, TRUE,
                        End,
                    End,

                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_AUTOPADEDOL] = CheckBoxObject,
                            GA_ID,       PGID_AUTOPADEDOL,
                            GA_Text,     "Auto _Pad EOL",
                            GA_Selected, tmp.autoPadEOL,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                End, /* HGroup Zeile 2 */

            End, /* Gruppe 1 */
            CHILD_WeightedHeight, 0,

            /* ======================================================= */
            /* Gruppe 2: Block Cursor / Flash Cursor / etc.             */
            /* ======================================================= */
            LAYOUT_AddChild, VGroupObject,
                LAYOUT_BevelStyle, BVS_GROUP,
                LAYOUT_SpaceInner, TRUE,

                LAYOUT_AddChild, HGroupObject,
                    LAYOUT_SpaceOuter, FALSE,
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_BLOCKCURSOR] = CheckBoxObject,
                            GA_ID,       PGID_BLOCKCURSOR,
                            GA_Text,     "_Block Cursor",
                            GA_Selected, tmp.blockCursor,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_FLASHCURSOR] = CheckBoxObject,
                            GA_ID,       PGID_FLASHCURSOR,
                            GA_Text,     "_Flash Cursor",
                            GA_Selected, tmp.flashCursor,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                End,

                LAYOUT_AddChild, HGroupObject,
                    LAYOUT_SpaceOuter, FALSE,
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_SEARCHZOOM] = CheckBoxObject,
                            GA_ID,       PGID_SEARCHZOOM,
                            GA_Text,     "_Search Zoom",
                            GA_Selected, tmp.searchZoom,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_NEWSLIDER] = CheckBoxObject,
                            GA_ID,       PGID_NEWSLIDER,
                            GA_Text,     "_New Slider",
                            GA_Selected, tmp.newSlider,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                End,

                LAYOUT_AddChild, HGroupObject,
                    LAYOUT_SpaceOuter, FALSE,
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_SHOWLFS] = CheckBoxObject,
                            GA_ID,       PGID_SHOWLFS,
                            GA_Text,     "Show _LF's",
                            GA_Selected, tmp.showLFs,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                    LAYOUT_AddChild, HGroupObject,
                        LAYOUT_AddChild, pg[PGID_SHOWTABS] = CheckBoxObject,
                            GA_ID,       PGID_SHOWTABS,
                            GA_Text,     "Show _TAB's",
                            GA_Selected, tmp.showTABs,
                            GA_RelVerify, TRUE,
                        End,
                    End,
                End,

            End, /* Gruppe 2 */
            CHILD_WeightedHeight, 0,

            /* ======================================================= */
            /* Gruppe 3: Wrap Margin / Tab Size                         */
            /* ======================================================= */
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_BevelStyle, BVS_GROUP,
                LAYOUT_SpaceInner, TRUE,

                /* Wrap Margin: LAYOUT_Label als CHILD-Tag nach dem Gadget */
                LAYOUT_AddChild, pg[PGID_WRAPMARGIN] = IntegerObject,
                    GA_ID,           PGID_WRAPMARGIN,
                    GA_RelVerify,    TRUE,
                    GA_TabCycle,     TRUE,
                    INTEGER_Number,  tmp.wrapMargin,
                    INTEGER_Minimum, 0,
                    INTEGER_Maximum, 999,
                End,
                CHILD_Label,         "Wrap _Margin :",
                CHILD_WeightedWidth, 30,

                /* Tab Size */
                LAYOUT_AddChild, pg[PGID_TABSIZE] = IntegerObject,
                    GA_ID,           PGID_TABSIZE,
                    GA_RelVerify,    TRUE,
                    GA_TabCycle,     TRUE,
                    INTEGER_Number,  tmp.tabSize,
                    INTEGER_Minimum, 1,
                    INTEGER_Maximum, 32,
                End,
                CHILD_Label,         "_Tab Size :",
                CHILD_WeightedWidth, 20,

            End, /* Gruppe 3 */
            CHILD_WeightedHeight, 0,

            /* ======================================================= */
            /* Buttons: ACCEPT / CANCEL                                 */
            /* ======================================================= */
        LAYOUT_AddChild, pg[PGID_HORIZ_33] = LayoutObject,
          GA_ID, PGID_HORIZ_33,
          LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
          LAYOUT_AddChild, pg[PGID_ACCEPT] = ButtonObject,
            GA_ID, PGID_ACCEPT,
            GA_Text, "Übernehmen",
            GA_RelVerify, TRUE,
            GA_TabCycle, TRUE,
            BUTTON_TextPen, 1,
            BUTTON_BackgroundPen, 0,
            BUTTON_FillTextPen, 1,
            BUTTON_FillPen, 3,
          ButtonEnd,
          LAYOUT_AddChild, pg[PGID_SAVE] = ButtonObject,
            GA_ID, PGID_SAVE,
            GA_Text, "Save",
            GA_RelVerify, TRUE,
            GA_TabCycle, TRUE,
            BUTTON_TextPen, 1,
            BUTTON_BackgroundPen, 0,
            BUTTON_FillTextPen, 1,
            BUTTON_FillPen, 3,
          ButtonEnd,
          LAYOUT_AddChild, pg[PGID_CANCEL] = ButtonObject,
            GA_ID, PGID_CANCEL,
            GA_Text, "Abbruch",
            GA_RelVerify, TRUE,
            GA_TabCycle, TRUE,
            BUTTON_TextPen, 1,
            BUTTON_BackgroundPen, 0,
            BUTTON_FillTextPen, 1,
            BUTTON_FillPen, 3,
          ButtonEnd,
          
        LayoutEnd,
            CHILD_WeightedHeight, 0,

        End, /* VGroup PGID_MAIN */

    EndWindow;

    if(!prefsWin) return FALSE;

    pw = (struct Window *) RA_OpenWindow(prefsWin);
    if(!pw)
    {
        DisposeObject(prefsWin);
        return FALSE;
    }

    GetAttr(WINDOW_SigMask, prefsWin, &signal);

    /* ---------------------------------------------------------------- */
    /* Eigene Event-Schleife (modal)                                     */
    /* ---------------------------------------------------------------- */
    while(!done)
    {
        Wait(signal);

        while((result = RA_HandleInput(prefsWin, NULL)) != WMHI_LASTMSG)
        {
            switch(result & WMHI_CLASSMASK)
            {
                case WMHI_CLOSEWINDOW:
                    done = TRUE;
                    break;

                case WMHI_GADGETUP:
                {
                    ULONG val;

                    switch(result & WMHI_GADGETMASK)
                    {
                        /* ---- Checkboxen ---- */
                        case PGID_AUTOINDENT:
                            GetAttr(GA_Selected, pg[PGID_AUTOINDENT], &val);
                            tmp.autoIndent = (BOOL)val;
                            break;
                        case PGID_STRIPEDOL:
                            GetAttr(GA_Selected, pg[PGID_STRIPEDOL], &val);
                            tmp.stripEOL = (BOOL)val;
                            break;
                        case PGID_WORDWRAP:
                            GetAttr(GA_Selected, pg[PGID_WORDWRAP], &val);
                            tmp.wordWrap = (BOOL)val;
                            break;
                        case PGID_AUTOPADEDOL:
                            GetAttr(GA_Selected, pg[PGID_AUTOPADEDOL], &val);
                            tmp.autoPadEOL = (BOOL)val;
                            break;
                        case PGID_BLOCKCURSOR:
                            GetAttr(GA_Selected, pg[PGID_BLOCKCURSOR], &val);
                            tmp.blockCursor = (BOOL)val;
                            break;
                        case PGID_FLASHCURSOR:
                            GetAttr(GA_Selected, pg[PGID_FLASHCURSOR], &val);
                            tmp.flashCursor = (BOOL)val;
                            break;
                        case PGID_SEARCHZOOM:
                            GetAttr(GA_Selected, pg[PGID_SEARCHZOOM], &val);
                            tmp.searchZoom = (BOOL)val;
                            break;
                        case PGID_NEWSLIDER:
                            GetAttr(GA_Selected, pg[PGID_NEWSLIDER], &val);
                            tmp.newSlider = (BOOL)val;
                            break;
                        case PGID_SHOWLFS:
                            GetAttr(GA_Selected, pg[PGID_SHOWLFS], &val);
                            tmp.showLFs = (BOOL)val;
                            break;
                        case PGID_SHOWTABS:
                            GetAttr(GA_Selected, pg[PGID_SHOWTABS], &val);
                            tmp.showTABs = (BOOL)val;
                            break;

                        /* ---- Integer-Felder ---- */
                        case PGID_WRAPMARGIN:
                            GetAttr(INTEGER_Number, pg[PGID_WRAPMARGIN], &val);
                            tmp.wrapMargin = (LONG)val;
                            break;
                        case PGID_TABSIZE:
                            GetAttr(INTEGER_Number, pg[PGID_TABSIZE], &val);
                            tmp.tabSize = (LONG)val;
                            break;

                        /* ---- Buttons ---- */
                        case PGID_ACCEPT:
                            *prefs = tmp;   /* Werte ?bernehmen */
                            accept = TRUE;
                            done   = TRUE;
                            break;
                        case PGID_SAVE:
                            *prefs = tmp;   /* Werte ?bernehmen */
                            accept = TRUE;
                            done   = TRUE;
                            break;
                        case PGID_CANCEL:
                            done = TRUE;
                            break;
                    }
                    break;
                }
            }
        }
    }

    DisposeObject(prefsWin);
    return accept;
}

#endif /* RC_PREFS_H */