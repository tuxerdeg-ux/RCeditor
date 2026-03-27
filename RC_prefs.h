/*
** RC_prefs.h - Editor Preferences Window
**
** Benoetigt zusaetzlich zu RC_setup.h:
**   LabelBase  = OpenLibrary("images/label.image",  0L);
**   IntegerBase = OpenLibrary("gadgets/integer.gadget", 0L);
**   CheckBoxBase = OpenLibrary("gadgets/checkbox.gadget", 0L);
*/

#ifndef RC_PREFS_H
#define RC_PREFS_H

#include <proto/checkbox.h>
#include <proto/integer.h>
#include <proto/label.h>
#include <gadgets/checkbox.h>
#include <gadgets/integer.h>
#include <images/label.h>
#include <proto/space.h>
#include <gadgets/space.h>

/* ------------------------------------------------------------------ */
/* Preferences-Datenstruktur                                           */
/* ------------------------------------------------------------------ */

struct EditorPrefs
{
    BOOL autoIndent;
    BOOL wordWrap;
    BOOL blockCursor;
    BOOL flashCursor;
    BOOL showLFs;
    BOOL showTABs;
    BOOL showLineNumbers;
    LONG wrapMargin;
    LONG tabSize;
};

/* ------------------------------------------------------------------ */
/* Gadget-Indizes (Array-Position)                                     */
/* ------------------------------------------------------------------ */
enum prefs_idx {
    pr_grp,
    pr_row1,
    pr_autoindent,
    pr_wordwrap,
    pr_row2,
    pr_blockcursor,
    pr_flashcursor,
    pr_row3,
    pr_showlfs,
    pr_showtabs,
    pr_row4,
    pr_showlinenumbers,
    pr_row5,
    pr_grp_wrap,
    pr_wrapmargin,
    pr_grp_tab,
    pr_tabsize,
    pr_row_btn,
    pr_ubern,
    pr_save,
    pr_cancel,
    PR_LAST
};

/* ------------------------------------------------------------------ */
/* Gadget-IDs (fuer WMHI_GADGETMASK)                                   */
/* ------------------------------------------------------------------ */
enum prefs_id {
    PR_ID_AUTOINDENT     = 101,
    PR_ID_WORDWRAP       = 102,
    PR_ID_BLOCKCURSOR    = 103,
    PR_ID_FLASHCURSOR    = 104,
    PR_ID_SHOWLFS        = 105,
    PR_ID_SHOWTABS       = 106,
    PR_ID_SHOWLINENUMBERS= 107,
    PR_ID_WRAPMARGIN     = 108,
    PR_ID_TABSIZE        = 109,
    PR_ID_UBERN          = 110,
    PR_ID_SAVE           = 111,
    PR_ID_CANCEL         = 112
};

/* ------------------------------------------------------------------ */
/* openPrefsWindow()                                                   */
/* ------------------------------------------------------------------ */

BOOL openPrefsWindow(struct Window *parentWin, struct EditorPrefs *prefs)
{
    Object         *prefsWin;
    struct Window  *pw;
    struct Gadget  *pg[PR_LAST];

    ULONG  signal, result;
    BOOL   done   = FALSE;
    BOOL   accept = FALSE;

    struct EditorPrefs tmp = *prefs;

    prefsWin = WindowObject,
        WA_Title,        "Editor Preferences",
        WA_ScreenTitle,  "RC - Editor Preferences",
        WA_Left,         50,
        WA_Top,          50,
        WA_Width,        360,
        WA_Height,       280,
        WA_MinWidth,     300,
        WA_MinHeight,    200,
        WA_MaxWidth,     8192,
        WA_MaxHeight,    8192,
        WA_CloseGadget,  TRUE,
        WA_DepthGadget,  TRUE,
        WA_DragBar,      TRUE,
        WA_Activate,     TRUE,
        WA_NoCareRefresh,TRUE,

        WINDOW_ParentGroup, VLayoutObject,
    LAYOUT_SpaceOuter, FALSE,
    LAYOUT_DeferLayout, TRUE,

    /* ---- Checkbox-Gruppe ---- */
    LAYOUT_AddChild, LayoutObject,
        LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
        LAYOUT_BevelStyle, BVS_GROUP,
        LAYOUT_SpaceInner, TRUE,

        /* Row 1 */
        LAYOUT_AddChild, LayoutObject,
            LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
            LAYOUT_LeftSpacing, 5,
            LAYOUT_TopSpacing, 5,

            LAYOUT_AddChild, pg[pr_autoindent] = CheckBoxObject,
                GA_ID, PR_ID_AUTOINDENT,
                GA_Text, "_Auto Indent",
                GA_Selected, tmp.autoIndent,
                GA_RelVerify, TRUE,
            CheckBoxEnd,

            LAYOUT_AddChild, pg[pr_wordwrap] = CheckBoxObject,
                GA_ID, PR_ID_WORDWRAP,
                GA_Text, "_Word Wrap",
                GA_Selected, tmp.wordWrap,
                GA_RelVerify, TRUE,
            CheckBoxEnd,
        LayoutEnd,

        /* Row 2 */
        LAYOUT_AddChild, LayoutObject,
            LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
            LAYOUT_LeftSpacing, 5,
            LAYOUT_TopSpacing, 5,

            LAYOUT_AddChild, pg[pr_blockcursor] = CheckBoxObject,
                GA_ID, PR_ID_BLOCKCURSOR,
                GA_Text, "_Block Cursor",
                GA_Selected, tmp.blockCursor,
                GA_RelVerify, TRUE,
            CheckBoxEnd,

            LAYOUT_AddChild, pg[pr_flashcursor] = CheckBoxObject,
                GA_ID, PR_ID_FLASHCURSOR,
                GA_Text, "_Flash Cursor",
                GA_Selected, tmp.flashCursor,
                GA_RelVerify, TRUE,
            CheckBoxEnd,
        LayoutEnd,

        /* Row 3 */
        LAYOUT_AddChild, LayoutObject,
            LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
            LAYOUT_LeftSpacing, 5,
            LAYOUT_TopSpacing, 5,

            LAYOUT_AddChild, pg[pr_showlfs] = CheckBoxObject,
                GA_ID, PR_ID_SHOWLFS,
                GA_Text, "Show _LF's",
                GA_Selected, tmp.showLFs,
                GA_RelVerify, TRUE,
            CheckBoxEnd,

            LAYOUT_AddChild, pg[pr_showtabs] = CheckBoxObject,
                GA_ID, PR_ID_SHOWTABS,
                GA_Text, "Show _TAB's",
                GA_Selected, tmp.showTABs,
                GA_RelVerify, TRUE,
            CheckBoxEnd,
        LayoutEnd,

        /* Row 4 */
        LAYOUT_AddChild, LayoutObject,
            LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
            LAYOUT_LeftSpacing, 5,
            LAYOUT_TopSpacing, 5,

            LAYOUT_AddChild, pg[pr_showlinenumbers] = CheckBoxObject,
                GA_ID, PR_ID_SHOWLINENUMBERS,
                GA_Text, "_Zeilennummern",
                GA_Selected, tmp.showLineNumbers,
                GA_RelVerify, TRUE,
            CheckBoxEnd,
        LayoutEnd,

        /* Row 5: Zahlen */
        LAYOUT_AddChild, LayoutObject,
            LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
            LAYOUT_LeftSpacing, 5,
            LAYOUT_TopSpacing, 5,
            /* Wrap */
            LAYOUT_AddChild, pg[pr_wrapmargin] = IntegerObject,
                GA_ID, PR_ID_WRAPMARGIN,
                INTEGER_Number, tmp.wrapMargin,
                INTEGER_Minimum, 0,
                INTEGER_Maximum, 999,
                INTEGER_MaxChars, 3,
                INTEGER_Number, 0,
                GA_RelVerify, TRUE,
            IntegerEnd,
            CHILD_Label, LabelObject,
                LABEL_Text, "Wrap Margin",
            LabelEnd,

            /* Tab */
            LAYOUT_AddChild, pg[pr_tabsize] = IntegerObject,
                GA_ID, PR_ID_TABSIZE,
                INTEGER_Number, tmp.tabSize,
                INTEGER_Minimum, 0,
                INTEGER_Maximum, 8,
                INTEGER_MaxChars, 1,
                INTEGER_Number, 1,
                GA_RelVerify, TRUE,
            IntegerEnd,
            CHILD_Label, LabelObject,
                LABEL_Text, "Tab Size",
            LabelEnd,

        LayoutEnd,

    LayoutEnd,
    CHILD_WeightedHeight, 0,

    /* ---- Spacer (wichtig!) ---- */
    LAYOUT_AddChild, SpaceObject,
    SpaceEnd,
    CHILD_WeightedHeight, 100,

    /* ---- Buttons ---- */
    LAYOUT_AddChild, LayoutObject,
        LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,

        LAYOUT_AddChild, pg[pr_ubern] = ButtonObject,
            GA_ID, PR_ID_UBERN,
            GA_Text, "_Uebernehmen",
            GA_RelVerify, TRUE,
        ButtonEnd,

        LAYOUT_AddChild, pg[pr_save] = ButtonObject,
            GA_ID, PR_ID_SAVE,
            GA_Text, "_Save",
            GA_RelVerify, TRUE,
        ButtonEnd,

        LAYOUT_AddChild, pg[pr_cancel] = ButtonObject,
            GA_ID, PR_ID_CANCEL,
            GA_Text, "_Abbruch",
            GA_RelVerify, TRUE,
        ButtonEnd,

    LayoutEnd,
    CHILD_WeightedHeight, 0,

LayoutEnd,
   
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
    /* Event-Loop                                                        */
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
                        case PR_ID_AUTOINDENT:
                            GetAttr(GA_Selected, pg[pr_autoindent], &val);
                            tmp.autoIndent = (BOOL)val;
                            break;
                        case PR_ID_WORDWRAP:
                            GetAttr(GA_Selected, pg[pr_wordwrap], &val);
                            tmp.wordWrap = (BOOL)val;
                            break;
                        case PR_ID_BLOCKCURSOR:
                            GetAttr(GA_Selected, pg[pr_blockcursor], &val);
                            tmp.blockCursor = (BOOL)val;
                            break;
                        case PR_ID_FLASHCURSOR:
                            GetAttr(GA_Selected, pg[pr_flashcursor], &val);
                            tmp.flashCursor = (BOOL)val;
                            break;
                        case PR_ID_SHOWLFS:
                            GetAttr(GA_Selected, pg[pr_showlfs], &val);
                            tmp.showLFs = (BOOL)val;
                            break;
                        case PR_ID_SHOWTABS:
                            GetAttr(GA_Selected, pg[pr_showtabs], &val);
                            tmp.showTABs = (BOOL)val;
                            break;
                        case PR_ID_SHOWLINENUMBERS:
                            GetAttr(GA_Selected, pg[pr_showlinenumbers], &val);
                            tmp.showLineNumbers = (BOOL)val;
                            break;
                        case PR_ID_WRAPMARGIN:
                            GetAttr(INTEGER_Number, pg[pr_wrapmargin], &val);
                            tmp.wrapMargin = (LONG)val;
                            break;
                        case PR_ID_TABSIZE:
                            GetAttr(INTEGER_Number, pg[pr_tabsize], &val);
                            tmp.tabSize = (LONG)val;
                            break;
                        case PR_ID_UBERN:
                            *prefs = tmp;  /* sofort uebernehmen */
                            accept = TRUE;
                            done   = TRUE;
                            break;
                        case PR_ID_SAVE:
                            done   = TRUE;
                            break;
                        case PR_ID_CANCEL:
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