/*
** RC_prefs.h - Editor Preferences Window
**
** Nur Einstellungen die texteditor.gadget tatsaechlich unterstuetzt.
**
** Benoetigt in RC_setup.h:
**   LabelBase   = OpenLibrary("images/label.image",       0L);
**   IntegerBase  = OpenLibrary("gadgets/integer.gadget",  0L);
**   CheckBoxBase = OpenLibrary("gadgets/checkbox.gadget", 0L);
**
** Mapping auf texteditor.gadget-Attribute:
**   autoIndent      -> GA_TEXTEDITOR_TabKeyPolicy (IndentsAfter/IndentsLine)
**   wordWrap        -> GA_TEXTEDITOR_WrapBorder (0 = aus, >0 = Spalte)
**   fixedFont       -> GA_TEXTEDITOR_FixedFont
**   showLineNumbers -> GA_TEXTEDITOR_ShowLineNumbers
**   readOnly        -> GA_ReadOnly
**   tabSize         -> GA_TEXTEDITOR_SpacesPerTAB
**   indentWidth     -> GA_TEXTEDITOR_IndentWidth
**   wrapMargin      -> GA_TEXTEDITOR_WrapBorder
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
/* Nur Felder die texteditor.gadget direkt unterstuetzt.              */
/* ------------------------------------------------------------------ */

struct EditorPrefs
{
    /* GA_TEXTEDITOR_TabKeyPolicy */
    BOOL autoIndent;

    /* GA_TEXTEDITOR_WrapBorder (0=aus) */
    LONG wrapMargin;

    /* GA_TEXTEDITOR_FixedFont */
    BOOL fixedFont;

    /* GA_TEXTEDITOR_ShowLineNumbers */
    BOOL showLineNumbers;

    /* GA_ReadOnly */
    BOOL readOnly;

    /* GA_TEXTEDITOR_SpacesPerTAB */
    LONG tabSize;

    /* GA_TEXTEDITOR_IndentWidth (0=Tabs, >0=Spaces pro Ebene) */
    LONG indentWidth;
};

/* ------------------------------------------------------------------ */
/* Gadget-Indizes                                                      */
/* ------------------------------------------------------------------ */
enum prefs_idx {
    pr_grp,
    pr_row1,
    pr_autoindent,
    pr_fixedfont,
    pr_row2,
    pr_showlinenumbers,
    pr_row_nums,
    pr_grp_wrap,
    pr_wrapmargin,
    pr_grp_tab,
    pr_tabsize,
    pr_grp_indent,
    pr_indentwidth,
    pr_row_btn,
    pr_ubern,
    pr_save,
    pr_cancel,
    PR_LAST
};

/* ------------------------------------------------------------------ */
/* Gadget-IDs                                                          */
/* ------------------------------------------------------------------ */
enum prefs_id {
    PR_ID_AUTOINDENT      = 101,
    PR_ID_FIXEDFONT       = 102,
    PR_ID_SHOWLINENUMBERS = 103,

    PR_ID_WRAPMARGIN      = 104,
    PR_ID_TABSIZE         = 105,
    PR_ID_INDENTWIDTH     = 106,
    PR_ID_UBERN           = 110,
    PR_ID_SAVE            = 111,
    PR_ID_CANCEL          = 112
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
        WA_Title,         "Editor Preferences",
        WA_ScreenTitle,   "RC - Editor Preferences",
        WA_Left,          50,
        WA_Top,           50,
        WA_Width,         360,
        WA_Height,        320,
        WA_MinWidth,      300,
        WA_MinHeight,     250,
        WA_MaxWidth,      8192,
        WA_MaxHeight,     8192,
        WA_CloseGadget,   TRUE,
        WA_DepthGadget,   TRUE,
        WA_DragBar,       TRUE,
        WA_Activate,      TRUE,
        WA_NoCareRefresh, TRUE,

        WINDOW_ParentGroup, VLayoutObject,
            LAYOUT_SpaceOuter,  TRUE,
            LAYOUT_DeferLayout, TRUE,

            /* ======================================================= */
            /* Gruppe: Boolean-Einstellungen                            */
            /* ======================================================= */
            LAYOUT_AddChild, pg[pr_grp] = LayoutObject,
                LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                LAYOUT_BevelStyle,  BVS_GROUP,
                LAYOUT_Label,       "Verhalten",
                LAYOUT_SpaceInner,  TRUE,

                /* Row 1: Auto Indent / Fixed Font */
                LAYOUT_AddChild, pg[pr_row1] = LayoutObject,
                    LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                    LAYOUT_LeftSpacing, 5,
                    LAYOUT_TopSpacing,  5,

                    LAYOUT_AddChild, pg[pr_autoindent] = CheckBoxObject,
                        GA_ID,              PR_ID_AUTOINDENT,
                        GA_Text,            "_Auto Indent",
                        GA_Selected,        tmp.autoIndent,
                        GA_RelVerify,       TRUE,
                        CHECKBOX_TextPlace, PLACETEXT_RIGHT,
                    CheckBoxEnd,

                    LAYOUT_AddChild, pg[pr_fixedfont] = CheckBoxObject,
                        GA_ID,              PR_ID_FIXEDFONT,
                        GA_Text,            "_Fixed Font",
                        GA_Selected,        tmp.fixedFont,
                        GA_RelVerify,       TRUE,
                        CHECKBOX_TextPlace, PLACETEXT_RIGHT,
                    CheckBoxEnd,
                LayoutEnd,
                CHILD_WeightedHeight, 0,

                /* Row 2: Zeilennummern / Read Only */
                LAYOUT_AddChild, pg[pr_row2] = LayoutObject,
                    LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                    LAYOUT_LeftSpacing, 5,
                    LAYOUT_TopSpacing,  5,

                    LAYOUT_AddChild, pg[pr_showlinenumbers] = CheckBoxObject,
                        GA_ID,              PR_ID_SHOWLINENUMBERS,
                        GA_Text,            "_Zeilennummern",
                        GA_Selected,        tmp.showLineNumbers,
                        GA_RelVerify,       TRUE,
                        CHECKBOX_TextPlace, PLACETEXT_RIGHT,
                    CheckBoxEnd,

                LayoutEnd,
                CHILD_WeightedHeight, 0,

            LayoutEnd, /* pr_grp */
            CHILD_WeightedHeight, 0,

            /* ======================================================= */
            /* Gruppe: Numerische Einstellungen                         */
            /* ======================================================= */
            LAYOUT_AddChild, pg[pr_row_nums] = LayoutObject,
                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                LAYOUT_BevelStyle,  BVS_GROUP,
                LAYOUT_Label,       "Werte",
                LAYOUT_SpaceInner,  TRUE,
                LAYOUT_LeftSpacing, 5,
                LAYOUT_TopSpacing,  5,

                /* Wrap Margin: 0 = aus, >0 = Spalte */
                LAYOUT_AddChild, pg[pr_grp_wrap] = LayoutObject,
                    LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                    LAYOUT_TopSpacing,  5,

                    LAYOUT_AddChild, pg[pr_wrapmargin] = IntegerObject,
                        GA_ID,           PR_ID_WRAPMARGIN,
                        GA_RelVerify,    TRUE,
                        GA_TabCycle,     TRUE,
                        INTEGER_Number,tmp.wrapMargin,
                        INTEGER_Minimum, 0,
                        INTEGER_Maximum, 999,
                        INTEGER_MaxChars,3,
                    IntegerEnd,
                    CHILD_Label, LabelObject,
                        LABEL_Text, "Wrap Margin",
                    LabelEnd,
                    CHILD_WeightedWidth, 10,
                LayoutEnd,
                CHILD_WeightedWidth, 10,

                /* Tab Size: Spaces pro TAB */
                LAYOUT_AddChild, pg[pr_grp_tab] = LayoutObject,
                    LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                    LAYOUT_LeftSpacing, 5,
                    LAYOUT_TopSpacing,  5,

                    LAYOUT_AddChild, pg[pr_tabsize] = IntegerObject,
                        GA_ID,           PR_ID_TABSIZE,
                        GA_RelVerify,    TRUE,
                        GA_TabCycle,     TRUE,
                        INTEGER_Minimum, 1,
                        INTEGER_Maximum, 32,
                        INTEGER_MaxChars,2,
                        INTEGER_Number,tmp.tabSize,
                    IntegerEnd,
                    CHILD_Label, LabelObject,
                        LABEL_Text, "Tab Size",
                    LabelEnd,
                    CHILD_WeightedWidth, 10,
                LayoutEnd,
                CHILD_WeightedWidth, 10,

                /* Indent Width: 0=Tabs, >0=Spaces pro Ebene */
                LAYOUT_AddChild, pg[pr_grp_indent] = LayoutObject,
                    LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                    LAYOUT_LeftSpacing, 5,
                    LAYOUT_TopSpacing,  5,

                    LAYOUT_AddChild, pg[pr_indentwidth] = IntegerObject,
                        GA_ID,           PR_ID_INDENTWIDTH,
                        GA_RelVerify,    TRUE,
                        GA_TabCycle,     TRUE,
                        INTEGER_Number,  tmp.indentWidth,
                        INTEGER_Minimum, 0,
                        INTEGER_Maximum, 16,
                        INTEGER_MaxChars,2,
                    IntegerEnd,
                    CHILD_Label, LabelObject,
                        LABEL_Text, "Indent Width",
                    LabelEnd,
                    CHILD_WeightedWidth, 10,
                LayoutEnd,
                CHILD_WeightedWidth, 10,

            LayoutEnd, /* pr_row_nums */
            CHILD_WeightedHeight, 0,

            /* Spacer */
            LAYOUT_AddChild, SpaceObject,
            SpaceEnd,
            CHILD_WeightedHeight, 100,

            /* ======================================================= */
            /* Buttons                                                  */
            /* ======================================================= */
            LAYOUT_AddChild, pg[pr_row_btn] = LayoutObject,
                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,

                LAYOUT_AddChild, pg[pr_ubern] = ButtonObject,
                    GA_ID,        PR_ID_UBERN,
                    GA_Text,      "_Uebernehmen",
                    GA_RelVerify, TRUE,
                    GA_TabCycle,  TRUE,
                ButtonEnd,

                LAYOUT_AddChild, pg[pr_save] = ButtonObject,
                    GA_ID,        PR_ID_SAVE,
                    GA_Text,      "_Save",
                    GA_RelVerify, TRUE,
                    GA_TabCycle,  TRUE,
                ButtonEnd,

                LAYOUT_AddChild, pg[pr_cancel] = ButtonObject,
                    GA_ID,        PR_ID_CANCEL,
                    GA_Text,      "_Abbruch",
                    GA_RelVerify, TRUE,
                    GA_TabCycle,  TRUE,
                ButtonEnd,

            LayoutEnd,
            CHILD_WeightedHeight, 0,

        LayoutEnd, /* VLayoutObject */

    EndWindow;

    if(!prefsWin) return FALSE;

    pw = (struct Window *) RA_OpenWindow(prefsWin);
    if(!pw)
    {
        DisposeObject(prefsWin);
        return FALSE;
    }

    GetAttr(WINDOW_SigMask, prefsWin, &signal);

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
                        case PR_ID_FIXEDFONT:
                            GetAttr(GA_Selected, pg[pr_fixedfont], &val);
                            tmp.fixedFont = (BOOL)val;
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
                        case PR_ID_INDENTWIDTH:
                            GetAttr(INTEGER_Number, pg[pr_indentwidth], &val);
                            tmp.indentWidth = (LONG)val;
                            break;

                        /* Uebernehmen: sofort anwenden, Fenster bleibt offen */
                        case PR_ID_UBERN:
                            *prefs = tmp;
                            accept = 1;
                            done   = TRUE;
                            break;
                        case PR_ID_SAVE:
                            *prefs = tmp;
                            accept = 2;
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