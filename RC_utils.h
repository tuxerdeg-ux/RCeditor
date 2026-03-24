/* RC_utils.h - Hilfsfunktionen */

#include <proto/checkbox.h>
#include <gadgets/checkbox.h>



char *neuASL(struct Window *win, char *dir, char *text, char *okText, BOOL dirOnly)
{
    struct FileRequester *fr;
    char *ret = NULL;

    fr = AllocFileRequest();
    if(!fr)
        return NULL;

    if(AslRequestTags(fr,
        ASL_Dir,           dir,
        ASL_Window,        win,
        ASL_Hail,          text,
        ASL_OKText,        okText,
        ASLFR_DrawersOnly, dirOnly,
        TAG_END))
    {
        if(dirOnly)
        {
            ret = (char *)AllocVec(strlen(fr->rf_Dir) + 1, MEMF_CLEAR);
            if(ret)
                sprintf(ret, "%s", fr->rf_Dir);
        }
        else
        {
            if(strlen(fr->rf_Dir) > 0)
            {
                ret = (char *)AllocVec(
                    strlen(fr->rf_Dir) + 1 + strlen(fr->rf_File) + 1,
                    MEMF_CLEAR);
                if(ret)
                    sprintf(ret, "%s/%s", fr->rf_Dir, fr->rf_File);
            }
            else
            {
                ret = (char *)AllocVec(strlen(fr->rf_File) + 1, MEMF_CLEAR);
                if(ret)
                    sprintf(ret, "%s", fr->rf_File);
            }
        }
    }

    FreeFileRequest(fr);
    return ret;
}

void updateStatusBar(ULONG row, ULONG col, ULONG lines, ULONG size,
                     struct Gadget *statusbar, struct Window *win)
{
    char statusText[256];
    
    sprintf(statusText, "Zeile: %ld  Spalte: %ld  |  Zeilen: %ld  |  Groesse: %ld Bytes",
            (long)row + 1, (long)col + 1, (long)lines, (long)size);
    
    SetGadgetAttrs(statusbar, win, NULL,
        STRINGA_TextVal, statusText,
        TAG_DONE);
}

ULONG gotoLineDialog(struct Window *parent, ULONG currentLine)
{
    Object        *dlg_obj;
    struct Window *dlg_win;
    struct Gadget *str_gad;
    ULONG          dlg_signal, dlg_result;
    ULONG          retLine = 0;
    BOOL           dlg_done = FALSE;
    char           initBuf[32];

    enum { DLG_MAIN=0, DLG_STRING=1, DLG_OK=2, DLG_CANCEL=3 };

    sprintf(initBuf, "%ld", (long)currentLine);

    dlg_obj = WindowObject,
        WA_Title,       "Gehe zu Zeile",
        WA_Width,       280,
        WA_Height,      80,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        WA_Activate,    TRUE,
        WINDOW_Position, WPOS_CENTERSCREEN,
        WINDOW_ParentGroup, VGroupObject,
            LAYOUT_SpaceOuter, TRUE,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_Label, "Zeilennummer:",
                LAYOUT_AddChild, str_gad = StringObject,
                    GA_ID,            DLG_STRING,
                    GA_TabCycle,      TRUE,
                    STRINGA_TextVal,  initBuf,
                    STRINGA_MaxChars, 10,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, ButtonObject,
                    GA_ID,        DLG_OK,
                    GA_Text,      "_OK",
                    GA_RelVerify, TRUE,
                End,
                LAYOUT_AddChild, ButtonObject,
                    GA_ID,        DLG_CANCEL,
                    GA_Text,      "_Abbruch",
                    GA_RelVerify, TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
        End,
    EndWindow;

    if(!dlg_obj) return 0;
    dlg_win = (struct Window *)RA_OpenWindow(dlg_obj);
    if(!dlg_win) { DisposeObject(dlg_obj); return 0; }
    GetAttr(WINDOW_SigMask, dlg_obj, &dlg_signal);

    while(!dlg_done)
    {
        Wait(dlg_signal);
        while((dlg_result = RA_HandleInput(dlg_obj, NULL)) != WMHI_LASTMSG)
        {
            switch(dlg_result & WMHI_CLASSMASK)
            {
                case WMHI_CLOSEWINDOW:
                    dlg_done = TRUE;
                    break;
                case WMHI_GADGETUP:
                    switch(dlg_result & WMHI_GADGETMASK)
                    {
                        case DLG_OK:
                        {
                            char *p = NULL;
                            GetAttr(STRINGA_TextVal, str_gad, (ULONG *)&p);
                            if(p && p[0] != '\0')
                                retLine = (ULONG)atol(p);
                            dlg_done = TRUE;
                            break;
                        }
                        case DLG_CANCEL:
                            dlg_done = TRUE;
                            break;
                    }
                    break;
            }
        }
    }
    DisposeObject(dlg_obj);
    return retLine;
}

/* Suchen-Dialog mit ECHTEN CHECKBOXEN!
 * Gibt den Suchtext zurück oder NULL bei Abbruch.
 */
char *searchDialog(struct Window *parent, BOOL *caseSensitive, BOOL *fromTop)
{
    Object        *dlg_obj;
    struct Window *dlg_win;
    struct Gadget *str_gad, *case_gad, *top_gad;
    ULONG          dlg_signal, dlg_result;
    char          *retText = NULL;
    BOOL           dlg_done = FALSE;

    enum { DLG_MAIN=0, DLG_STRING=1, DLG_CASE=2, DLG_TOP=3, DLG_OK=4, DLG_CANCEL=5 };

    dlg_obj = WindowObject,
        WA_Title,       "Suchen",
        WA_Width,       380,
        WA_Height,      110,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        WA_Activate,    TRUE,
        WINDOW_Position, WPOS_CENTERSCREEN,
        WINDOW_ParentGroup, VGroupObject,
            LAYOUT_SpaceOuter, TRUE,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_Label, "Suchen nach:",
                LAYOUT_AddChild, str_gad = StringObject,
                    GA_ID,            DLG_STRING,
                    GA_TabCycle,      TRUE,
                    STRINGA_MaxChars, 128,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, case_gad = CheckBoxObject,
                    GA_ID,        DLG_CASE,
                    GA_Text,      "_Gross-/Kleinschreibung",
                    GA_Selected,  FALSE,
                    GA_TabCycle,  TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, top_gad = CheckBoxObject,
                    GA_ID,        DLG_TOP,
                    GA_Text,      "Von _vorne suchen",
                    GA_Selected,  FALSE,
                    GA_TabCycle,  TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, ButtonObject,
                    GA_ID,        DLG_OK,
                    GA_Text,      "_Suchen",
                    GA_RelVerify, TRUE,
                End,
                LAYOUT_AddChild, ButtonObject,
                    GA_ID,        DLG_CANCEL,
                    GA_Text,      "_Abbruch",
                    GA_RelVerify, TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
        End,
    EndWindow;

    if(!dlg_obj) return NULL;
    dlg_win = (struct Window *)RA_OpenWindow(dlg_obj);
    if(!dlg_win) { DisposeObject(dlg_obj); return NULL; }
    GetAttr(WINDOW_SigMask, dlg_obj, &dlg_signal);

    while(!dlg_done)
    {
        Wait(dlg_signal);
        while((dlg_result = RA_HandleInput(dlg_obj, NULL)) != WMHI_LASTMSG)
        {
            switch(dlg_result & WMHI_CLASSMASK)
            {
                case WMHI_CLOSEWINDOW:
                    dlg_done = TRUE;
                    break;
                case WMHI_GADGETUP:
                    switch(dlg_result & WMHI_GADGETMASK)
                    {
                        case DLG_OK:
                        {
                            char *p = NULL;
                            ULONG sel = 0;
                            
                            GetAttr(STRINGA_TextVal, str_gad, (ULONG *)&p);
                            if(p && p[0] != '\0')
                            {
                                retText = (char *)AllocVec(strlen(p) + 1, MEMF_CLEAR);
                                if(retText)
                                    strcpy(retText, p);
                            }
                            
                            GetAttr(GA_Selected, case_gad, &sel);
                            *caseSensitive = (sel != 0);
                            
                            GetAttr(GA_Selected, top_gad, &sel);
                            *fromTop = (sel != 0);
                            
                            dlg_done = TRUE;
                            break;
                        }
                        case DLG_CANCEL:
                            dlg_done = TRUE;
                            break;
                    }
                    break;
            }
        }
    }
    DisposeObject(dlg_obj);
    return retText;
}

/* Ersetzen-Dialog mit ECHTEN CHECKBOXEN! */
BOOL replaceDialog(struct Window *parent, 
                   char **searchText, char **replaceText,
                   BOOL *caseSensitive, BOOL *replaceAll)
{
    Object        *dlg_obj;
    struct Window *dlg_win;
    struct Gadget *search_gad, *replace_gad, *case_gad, *all_gad;
    ULONG          dlg_signal, dlg_result;
    BOOL           retOK = FALSE;
    BOOL           dlg_done = FALSE;

    enum { DLG_MAIN=0, DLG_SEARCH=1, DLG_REPLACE=2, DLG_CASE=3, 
           DLG_ALL=4, DLG_OK=5, DLG_CANCEL=6 };

    *searchText = NULL;
    *replaceText = NULL;

    dlg_obj = WindowObject,
        WA_Title,       "Ersetzen",
        WA_Width,       400,
        WA_Height,      150,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        WA_Activate,    TRUE,
        WINDOW_Position, WPOS_CENTERSCREEN,
        WINDOW_ParentGroup, VGroupObject,
            LAYOUT_SpaceOuter, TRUE,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_Label, "Suchen nach:",
                LAYOUT_AddChild, search_gad = StringObject,
                    GA_ID,            DLG_SEARCH,
                    GA_TabCycle,      TRUE,
                    STRINGA_MaxChars, 128,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_Label, "Ersetzen mit:",
                LAYOUT_AddChild, replace_gad = StringObject,
                    GA_ID,            DLG_REPLACE,
                    GA_TabCycle,      TRUE,
                    STRINGA_MaxChars, 128,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, case_gad = CheckBoxObject,
                    GA_ID,        DLG_CASE,
                    GA_Text,      "_Gross-/Kleinschreibung",
                    GA_Selected,  FALSE,
                    GA_TabCycle,  TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, all_gad = CheckBoxObject,
                    GA_ID,        DLG_ALL,
                    GA_Text,      "_Alle ersetzen",
                    GA_Selected,  FALSE,
                    GA_TabCycle,  TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_AddChild, ButtonObject,
                    GA_ID,        DLG_OK,
                    GA_Text,      "_Ersetzen",
                    GA_RelVerify, TRUE,
                End,
                LAYOUT_AddChild, ButtonObject,
                    GA_ID,        DLG_CANCEL,
                    GA_Text,      "_Abbruch",
                    GA_RelVerify, TRUE,
                End,
            End,
            CHILD_WeightedHeight, 0,
        End,
    EndWindow;

    if(!dlg_obj) return FALSE;
    dlg_win = (struct Window *)RA_OpenWindow(dlg_obj);
    if(!dlg_win) { DisposeObject(dlg_obj); return FALSE; }
    GetAttr(WINDOW_SigMask, dlg_obj, &dlg_signal);

    while(!dlg_done)
    {
        Wait(dlg_signal);
        while((dlg_result = RA_HandleInput(dlg_obj, NULL)) != WMHI_LASTMSG)
        {
            switch(dlg_result & WMHI_CLASSMASK)
            {
                case WMHI_CLOSEWINDOW:
                    dlg_done = TRUE;
                    break;
                case WMHI_GADGETUP:
                    switch(dlg_result & WMHI_GADGETMASK)
                    {
                        case DLG_OK:
                        {
                            char *p1 = NULL, *p2 = NULL;
                            ULONG sel = 0;
                            
                            GetAttr(STRINGA_TextVal, search_gad, (ULONG *)&p1);
                            GetAttr(STRINGA_TextVal, replace_gad, (ULONG *)&p2);
                            
                            if(p1 && p1[0] != '\0')
                            {
                                *searchText = (char *)AllocVec(strlen(p1) + 1, MEMF_CLEAR);
                                if(*searchText)
                                    strcpy(*searchText, p1);
                                
                                if(p2)
                                {
                                    *replaceText = (char *)AllocVec(strlen(p2) + 1, MEMF_CLEAR);
                                    if(*replaceText)
                                        strcpy(*replaceText, p2);
                                }
                                else
                                {
                                    *replaceText = (char *)AllocVec(1, MEMF_CLEAR);
                                    if(*replaceText)
                                        (*replaceText)[0] = '\0';
                                }
                                
                                GetAttr(GA_Selected, case_gad, &sel);
                                *caseSensitive = (sel != 0);
                                
                                GetAttr(GA_Selected, all_gad, &sel);
                                *replaceAll = (sel != 0);
                                
                                retOK = TRUE;
                            }
                            
                            dlg_done = TRUE;
                            break;
                        }
                        case DLG_CANCEL:
                            dlg_done = TRUE;
                            break;
                    }
                    break;
            }
        }
    }
    DisposeObject(dlg_obj);
    return retOK;
}

