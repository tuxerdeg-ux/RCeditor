/* RC.c - RCed Hauptprogramm

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/scroller.h>
#include <proto/utility.h>
#include <gadgets/scroller.h>
#include <devices/timer.h>
#include <proto/intuition.h>
#include <proto/window.h>
#include <proto/layout.h>
#include <proto/texteditor.h>
#include <proto/requester.h>
#include <proto/asl.h>
#include <proto/button.h>
#include <proto/string.h>
#include <proto/clicktab.h>

#include <intuition/intuition.h>

#include <reaction/reaction_macros.h>

#include <classes/window.h>
#include <classes/requester.h>

#include <gadgets/texteditor.h>
#include <gadgets/clicktab.h>

#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <clib/alib_protos.h>   /* DoMethod(), DoMethodA() aus amiga.lib */

#include "RC_menu.h"

#include "RC_setup.h"

#include "RC_file.h"

#include "RC_req.h"

#include "RC_utils.h"

const char verstag[] = "\0$VER: RC 0.1 (22.03.2026)";
const char *version  = "RC 0.1 (22.03.2026)";

/* VBCC Stack */
LONG __stack = 100000;

/* ====================================================================
 * TextEditor.gadget Definitionen
 * Aus den offiziellen AutoDocs:
 * https://developer.amigaos3.net/autodocs/texteditor.gadget/
 * 
 * WICHTIG: Search-Type Flags sind EXKLUSIV (nur eines)!
 * ==================================================================== */

/* Suchen-Type (nur EINES davon!) */
#ifndef GF_TEXTEDITOR_Search_FromTop
#define GF_TEXTEDITOR_Search_FromTop       0  /* Von vorne suchen */
#define GF_TEXTEDITOR_Search_Backwards     1  /* Rückwärts suchen */
#define GF_TEXTEDITOR_Search_Next          2  /* Vorwärts suchen (default) */
#endif

/* Suchen-Modifier (kombinierbar mit | ) */
#ifndef GF_TEXTEDITOR_Search_CaseSensitive
#define GF_TEXTEDITOR_Search_CaseSensitive (1<<8)  /* Groß-/Kleinschreibung */
#endif

/* ==================================================================== */

/* Globale Library-Basiszeiger */
struct Library *WindowBase;
struct Library *ButtonBase;
struct Library *StringBase;
struct Library *LabelBase;
struct Library *GadToolsBase;
struct Library *LayoutBase;
struct Library *RequesterBase;
struct Library *AslBase;
struct Library *TextFieldBase;
struct Library *ScrollerBase;
struct Library *CheckBoxBase;
struct Library *IconBase;

struct RCed RCed;

/* Hilfsmakro für Zugriff auf das aktive Dokument */
#define CURRENT_DOC (&RCed.documents[RCed.activeDocIndex])


/* ==================================================================
 * Dokument-Verwaltung für Multi-Document-Support
 * ================================================================== */

/* Initialisiert alle 10 Dokumente */
void initializeDocuments(void)
{
    int i;
    for(i = 0; i < 10; i++)
    {
        strcpy(RCed.documents[i].dateiname, "");
        RCed.documents[i].buffer = NULL;
        RCed.documents[i].hasChanged = FALSE;
        RCed.documents[i].fileLines = 0;
        RCed.documents[i].fileSize = 0;
        RCed.documents[i].isOpen = FALSE;
    }
    RCed.activeDocIndex = 0;
    RCed.tabObject = NULL;
    RCed.tabCallback = NULL;
    
    /* Erstes Dokument initialisieren */
    strcpy(RCed.documents[0].dateiname, "Unbenannt");
    RCed.documents[0].buffer = (char *)AllocVec(1, MEMF_CLEAR);
    if(RCed.documents[0].buffer)
        RCed.documents[0].buffer[0] = '\0';
    RCed.documents[0].isOpen = TRUE;
}

/* Gibt aktuelles Dokument zurück */
struct RCedDocument *getCurrentDocument(void)
{
    if(RCed.activeDocIndex < 10)
        return &RCed.documents[RCed.activeDocIndex];
    return NULL;
}

/* Wechselt zum angegebenen Dokument und zeigt es an */
void refreshWindowTitle(struct Window *win)
{
    struct RCedDocument *doc = getCurrentDocument();
    char title[280];

    if(!doc)
        return;

    if(doc->hasChanged)
        SNPrintf(title, sizeof(title), "%s *", doc->dateiname);
    else
        SNPrintf(title, sizeof(title), "%s", doc->dateiname);

    SetWindowTitles(win, title, (UBYTE *)~0);
}


void switchDocument(ULONG index, struct Gadget *editor, struct Window *win)
{
    struct RCedDocument *doc;
    
    if(index >= MAX_DOCUMENTS)
        return;
    
    doc = &RCed.documents[index];
    RCed.activeDocIndex = index;
    
    /* Buffer in Editor laden */
    if(doc->buffer)
    {
        DoGadgetMethod(editor, win, NULL, GM_TEXTEDITOR_ClearText, NULL);
        SetGadgetAttrs(editor, win, NULL,
            GA_TEXTEDITOR_Contents, (APTR)doc->buffer, TAG_DONE);
        SetGadgetAttrs(editor, win, NULL,
            GA_TEXTEDITOR_HasChanged, doc->hasChanged, TAG_DONE);
    }
    else
    {
        DoGadgetMethod(editor, win, NULL, GM_TEXTEDITOR_ClearText, NULL);
        doc->buffer = (char *)AllocVec(1, MEMF_CLEAR);
        if(doc->buffer)
            doc->buffer[0] = '\0';
    }
    RefreshGList(editor, win, NULL, 1);

    /* Title jetzt immer synchron halten */
    refreshWindowTitle(win);
}

/* Speichert den aktuellen Buffer zurück ins aktive Dokument */
void saveCurrentDocumentBuffer(struct Gadget *editor, struct Window *win)
{
    struct RCedDocument *doc = getCurrentDocument();
    UBYTE *exported;
    
    if(!doc)
        return;
    
    /* Buffer aus Editor exportieren */
    exported = (UBYTE *)DoGadgetMethod(editor, win, NULL,
        GM_TEXTEDITOR_ExportText, NULL);
    
    if(exported)
    {
        /* Alten Buffer freigeben */
        if(doc->buffer)
            FreeVec(doc->buffer);
        
        /* Neuen Buffer speichern */
        doc->buffer = (char *)AllocVec(strlen((char *)exported) + 1, MEMF_CLEAR);
        if(doc->buffer)
            strcpy(doc->buffer, (char *)exported);
        
        /* Datei-Statistiken berechnen */
        doc->fileSize = strlen((char *)exported);
        doc->fileLines = 0;
        int i;
        for(i = 0; i < (int)doc->fileSize; i++)
            if(exported[i] == '\n')
                doc->fileLines++;
        if(doc->fileSize > 0)
            doc->fileLines++;
    }
}

/* Öffnet ein neues leeres Dokument */
ULONG openNewDocument(void)
{
    int i;
    
    for(i = 0; i < 10; i++)
    {
        if(!RCed.documents[i].isOpen)
        {
            strcpy(RCed.documents[i].dateiname, "Unbenannt");
            if(RCed.documents[i].buffer)
                FreeVec(RCed.documents[i].buffer);
            RCed.documents[i].buffer = (char *)AllocVec(1, MEMF_CLEAR);
            if(RCed.documents[i].buffer)
                RCed.documents[i].buffer[0] = '\0';
            RCed.documents[i].hasChanged = FALSE;
            RCed.documents[i].fileLines = 0;
            RCed.documents[i].fileSize = 0;
            RCed.documents[i].isOpen = TRUE;
            return (ULONG)i;
        }
    }
    return (ULONG)-1;  /* Kein Platz mehr */
}

/* Schließt ein Dokument */
void closeDocument(ULONG index)
{
    if(index >= 10)
        return;
    
    if(RCed.documents[index].buffer)
    {
        FreeVec(RCed.documents[index].buffer);
        RCed.documents[index].buffer = NULL;
    }
    RCed.documents[index].isOpen = FALSE;
    strcpy(RCed.documents[index].dateiname, "");
    
    /* Wenn aktuelles Dokument geschlossen wird, zu anderem wechseln */
    if(RCed.activeDocIndex == index)
    {
        int i;
        for(i = 0; i < 10; i++)
        {
            if(RCed.documents[i].isOpen)
            {
                RCed.activeDocIndex = i;
                return;
            }
        }
        RCed.activeDocIndex = 0;
    }
}

/* ClickTab Callback - wird bei Tab-Ereignissen aufgerufen */
ULONG tabsCallback(struct TagItem *taglist)
{
    struct TagItem *tstate, *ti;
    ti = taglist;
    tstate = ti;
    
    while (ti = NextTagItem(&tstate))
    {
        switch (ti->ti_Tag)
        {
            case CLICKTAB_NodeClosed:
                /* Tab wurde geschlossen - Dokument schließen */
                {
                    struct Node *node = (struct Node *)ti->ti_Data;
                    /* Finde entsprechenden Index */
                    ULONG i = (ULONG)(node->ln_Name[0] - '0');
                    if(i < 10)
                        closeDocument(i);
                }
                break;
        }
    }
    return TRUE;
}

/* Erstellt die ClickTab-Gadget mit nur 1 Initial-Tab */
void createClickTabs(void)
{
    struct Node *node;
    
    NewList((struct List *)&RCed.tabList);
    
    /* Erstelle einen initial Tab "Unbenannt" */
    node = AllocClickTabNode(
        TNA_Text, "Unbenannt",
        TNA_UserData, (APTR)(ULONG)0,
        TAG_END);
    
    if(node)
        AddTail((struct List *)&RCed.tabList, node);
    
    RCed.tabCallback = NULL;
}

/* Fügt einen neuen Tab für das Dokument hinzu */
void addTabForDocument(ULONG docIndex, struct Window *win)
{
    struct Node *node;
    struct RCedDocument *doc;
    
    if(docIndex >= 10)
        return;
    
    doc = &RCed.documents[docIndex];
    
    /* Neuen Tab-Node erstellen mit Dokumentnamen */
    node = AllocClickTabNode(
        TNA_Text, doc->dateiname,
        TNA_UserData, (APTR)(ULONG)docIndex,
        TAG_END);
    
    if(node && RCed.tabObject)
    {
        AddTail((struct List *)&RCed.tabList, node);
        
        /* ClickTab aktualisieren */
        SetGadgetAttrs((struct Gadget *)RCed.tabObject, win, NULL,
            CLICKTAB_Labels, &RCed.tabList,
            TAG_END);
        
        RefreshGList((struct Gadget *)RCed.tabObject, win, NULL, 1);
    }
}

/* Aktualisiert alle Tab-Labels basierend auf offenen Dokumenten */
void updateTabLabels(struct Window *win)
{
    struct Node *node;
    
    if(!RCed.tabObject)
        return;
    
    /* Gehe durch alle Tabs und aktualisiere über UserData -> Dokumentindex */
    for(node = (struct Node *)RCed.tabList.mlh_Head; node->ln_Succ; node = node->ln_Succ)
    {
        APTR userData;
        ULONG docIndex;

        GetClickTabNodeAttrs(node,
            TNA_UserData, &userData,
            TAG_END);

        docIndex = (ULONG)userData;

        if(docIndex < 10 && RCed.documents[docIndex].isOpen)
        {
            SetClickTabNodeAttrs(node,
                TNA_Text, RCed.documents[docIndex].dateiname,
                TAG_END);
        }
    }
    
    /* Aktualisiere ClickTab */
    RefreshGList((struct Gadget *)RCed.tabObject, win, NULL, 1);
}


/* ------------------------------------------------------------------
 * loadAndDisplay()
 *
 * L?dt eine Datei und zeigt den Inhalt im TextEditor-Gadget an.
 * Gibt den neuen Buffer-Zeiger zur?ck (oder NULL bei Fehler).
 * Den alten Buffer ?bergibt man als *oldBuf -- er wird freigegeben.
 *
 * HINWEIS: GA_TEXTEDITOR_Contents funktioniert beim Reaction
 * TextEditor auch via SetGadgetAttrs() zur Laufzeit, weil die
 * Klasse das Attribut in OM_SET auswertet.
 * Der fr?here Crash kam ausschlie?lich von SetWindowAttrs()
 * (fehlender Linker-Stub), nicht von Contents.
 *
 * GV_TEXTEDITOR_Replace_All wird hier NICHT verwendet, da die
 * Konstante in manchen NDK-Versionen fehlt.
 * ------------------------------------------------------------------ */
static char *loadAndDisplay(const char    *name,
                             char         *oldBuf,
                             struct Gadget *editor,
                             struct Window *win)
{
    char *newBuf = (char *)loadText((char *)name);

    if(!newBuf)
        return oldBuf;   /* Fehler -- alten Buffer behalten */

    /* Alten Buffer freigeben */
    if(oldBuf)
        FreeVec(oldBuf);

    DoGadgetMethod(editor, win, NULL, GM_TEXTEDITOR_ClearText, NULL);
    SetGadgetAttrs(editor, win, NULL,
        GA_TEXTEDITOR_Contents, (APTR)newBuf, TAG_DONE);
    SetGadgetAttrs(editor, win, NULL,
        GA_TEXTEDITOR_HasChanged, FALSE, TAG_DONE);
    RefreshGList(editor, win, NULL, 1);

    return newBuf;
}


#define MY_WIN_WIDTH  (640)
#define MY_WIN_HEIGHT (480)

int main(int argc, char *argv[])
{
    Object        *Win_Object;
    Object        *beenden_object, *neu_object, *uber_object;
    struct Window *window;
    struct Gadget *main_gadgets[GID_LAST];
    struct EasyStruct es = {
        sizeof(struct EasyStruct), 0,
        "Programm-Info",
        "Version: %s\nDanke",
        "OK"
    };
    ULONG  signal, result;
    ULONG  done   = FALSE;
    ULONG  status = 0;
    char  *retASL   = NULL;
    ULONG  timerSig  = 0;
    BOOL   timerOpen = FALSE;
    struct MsgPort     *timerPort = NULL;
    struct timerequest *timerReq  = NULL;
    char   gotoLineBuf[32];
    ULONG  lastChanged = 0;  /* Verfolgung des letzten HasChanged-Status */
    char   titleBuffer[256];  /* Puffer für Titel mit/ohne Asterisk */

    /* Library-Zeiger initialisieren */
    WindowBase = ButtonBase = StringBase = LabelBase  =
    GadToolsBase = LayoutBase = RequesterBase = AslBase =
    TextFieldBase = IconBase = NULL;

    /* Alle Dokumente initialisieren */
    initializeDocuments();

    /* Dateiname als Argument übergeben (ins erste Dokument) */
    if(argc > 1)
    {
        strcpy(RCed.documents[0].dateiname, argv[1]);
        RCed.documents[0].buffer = (char *)loadText(argv[1]);
        if(!RCed.documents[0].buffer)
        {
            RCed.documents[0].buffer = (char *)AllocVec(1, MEMF_CLEAR);
            if(RCed.documents[0].buffer)
                RCed.documents[0].buffer[0] = '\0';
        }
    }

    if(!openLibraries())
        goto cleanup;

    /* ClickTab-Struktur initialisieren */
    createClickTabs();

    /* Requester-Objekte */
    beenden_object = requesterNew("Beenden");
    neu_object     = requesterNew("Neu");
    uber_object    = requesterNew("Über");

    /* Hauptfenster aufbauen */
    Win_Object = WindowObject,
        WA_ScreenTitle,   "RC",
        WA_Title,         "TEST",
        WA_SizeGadget,    TRUE,
        WA_DepthGadget,   TRUE,
        WA_DragBar,       TRUE,
        WA_CloseGadget,   TRUE,
        WA_Activate,      TRUE,
        WA_Width,         MY_WIN_WIDTH,
        WA_Height,        MY_WIN_HEIGHT,
        WA_NewLookMenus,  TRUE,
        WINDOW_Position, WPOS_CENTERSCREEN,
        WINDOW_NewMenu,   mainMenu,

        WINDOW_ParentGroup, main_gadgets[GID_MAIN] = VGroupObject,

            LAYOUT_SpaceOuter,  TRUE,
            LAYOUT_BevelStyle,  BVS_GROUP,
            LAYOUT_DeferLayout, TRUE,

            /* ---- Zeile 1: Dokument-Tabs (ClickTab-Gadget) ---- */
            LAYOUT_AddChild, main_gadgets[GID_TAB_0] = NewObject(CLICKTAB_GetClass(), NULL,
                GA_ID,                          GID_TAB_0,
                GA_RelVerify,                   TRUE,
                CLICKTAB_PageGroupBorder,       FALSE,
                CLICKTAB_AutoTabNumbering,      TRUE,
                CLICKTAB_Labels,                &RCed.tabList,
                TAG_DONE),
            CHILD_WeightedHeight, 0,


            /* ---- Zeile 2: TextEditor + Scrollbalken ---- */
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_BevelStyle, BVS_SBAR_VERT,
                LAYOUT_Label,      "My Editor",

                LAYOUT_AddChild, main_gadgets[GID_TEXTEDITOR] =
                    NewObject(TEXTEDITOR_GetClass(), NULL,
                        GA_ID,                         GID_TEXTEDITOR,
                        GA_TEXTEDITOR_ExportWrap,       0,
                        GA_TEXTEDITOR_ImportWrap,       0,
                        GA_TEXTEDITOR_FixedFont,        FALSE,
                        GA_TEXTEDITOR_Flow,             GV_TEXTEDITOR_Flow_Left,
                        GA_TEXTEDITOR_IndentWidth,      0,
                        GA_TEXTEDITOR_LineEndingExport, LINEENDING_LF,
                        GA_TEXTEDITOR_ShowLineNumbers,  FALSE,
                        GA_TEXTEDITOR_SpacesPerTAB,     4,
                        GA_TEXTEDITOR_TabKeyPolicy,
                        GV_TEXTEDITOR_TabKey_IndentsAfter,
                    End,
                CHILD_WeightedWidth, 100,

                LAYOUT_AddChild, main_gadgets[GID_VSCROLL] = ScrollerObject,
                    GA_ID,                GID_VSCROLL,
                    GA_RelVerify,         TRUE,
                    SCROLLER_Orientation, SORIENT_VERT,
                    SCROLLER_Total,       1,
                    SCROLLER_Visible,     1,
                    SCROLLER_Top,         0,
                End,
                CHILD_WeightedWidth, 0,
                CHILD_MinWidth,      16,

            End, /* HGroupObject */
            CHILD_WeightedHeight, 100,
            CHILD_MinWidth,       300,

            /* ---- Informationsbalken ---- */
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_BevelStyle, BVS_SBAR_VERT,
                LAYOUT_Label,      "Info",
                LAYOUT_AddChild, main_gadgets[GID_STATUSBAR] = StringObject,
                    GA_ID,           GID_STATUSBAR,
                    GA_ReadOnly,     TRUE,
                    STRINGA_TextVal, "Zeile: 1  Spalte: 1  |  Zeilen: 0  |  Groesse: 0 Bytes",
                End,
            End,
            CHILD_WeightedHeight, 0,

            /* ---- Zeile 3: Tools ---- */
            LAYOUT_AddChild, HGroupObject,
                LAYOUT_BevelStyle, BVS_SBAR_VERT,
                LAYOUT_Label,      "Tools and More",
                    LAYOUT_AddChild, main_gadgets[GID_EmptyB] = ButtonObject,
                            GA_ID,              GID_EmptyB,
                            GA_Text,            "test",
                            GA_RelVerify,       TRUE,
                            GA_TabCycle,        TRUE,
                            BUTTON_TextPen,       1,
                            BUTTON_BackgroundPen, 0,
                            BUTTON_FillTextPen,   1,
                            BUTTON_FillPen,       3,
                        End,
                    CHILD_WeightedWidth, 25,
                LAYOUT_AddChild, ButtonObject, GA_Text, "Tool 2", End,
                    CHILD_WeightedWidth, 50,
                LAYOUT_AddChild, ButtonObject, GA_Text, "Tool 3", End,
                    CHILD_WeightedWidth, 75,
                LAYOUT_AddChild, ButtonObject, GA_Text, "Tool 4", End,
                    CHILD_WeightedWidth, 100,
            End,
            CHILD_WeightedHeight, 0,


        End, /* VGroupObject */

    EndWindow;

    if(!Win_Object)
        goto cleanup_libs;

    window = (struct Window *)RA_OpenWindow(Win_Object);
    if(!window)
        goto cleanup_obj;

    /* Speichere den ClickTab-Zeiger für später */
    RCed.tabObject = (APTR)main_gadgets[GID_TAB_0];

    /* Falls Datei per Argument geladen: sofort anzeigen */
    if(argc > 1 && RCed.documents[0].buffer && RCed.documents[0].buffer[0] != '\0')
    {
        DoGadgetMethod(main_gadgets[GID_TEXTEDITOR], window, NULL,
            GM_TEXTEDITOR_ClearText, NULL);
        SetGadgetAttrs(main_gadgets[GID_TEXTEDITOR], window, NULL,
            GA_TEXTEDITOR_Contents, (APTR)RCed.documents[0].buffer, TAG_DONE);
        SetGadgetAttrs(main_gadgets[GID_TEXTEDITOR], window, NULL,
            GA_TEXTEDITOR_HasChanged, FALSE, TAG_DONE);
        RefreshGList(main_gadgets[GID_TEXTEDITOR], window, NULL, 1);
    }

    GetAttr(WINDOW_SigMask, Win_Object, &signal);

    timerPort = CreateMsgPort();
    if(timerPort)
    {
        timerReq = (struct timerequest *)
            CreateIORequest(timerPort, sizeof(struct timerequest));
        if(timerReq)
        {
            if(OpenDevice("timer.device", UNIT_VBLANK,
                          (struct IORequest *)timerReq, 0) == 0)
            {
                timerOpen = TRUE;
                timerSig  = 1L << timerPort->mp_SigBit;
                timerReq->tr_node.io_Command = TR_ADDREQUEST;
                timerReq->tr_time.tv_secs    = 0;
                timerReq->tr_time.tv_micro   = 100000; /* 100ms */
                SendIO((struct IORequest *)timerReq);
            }
        }
    }

    /* ================================================================
     * Event-Loop
     * ================================================================ */
    while(!done)
    {
        ULONG wait = Wait(signal | timerSig | SIGBREAKF_CTRL_C);

        if(wait & SIGBREAKF_CTRL_C)
        {
            done = TRUE;
        }
        else
        {
            while((result = RA_HandleInput(Win_Object, NULL)) != WMHI_LASTMSG)
            {
                switch(result & WMHI_CLASSMASK)
                {
                    /* ---- Fenster schließen ---- */
                    case WMHI_CLOSEWINDOW:
                        done = TRUE;
                        break;

                    /* ---- Menü ---- */
                    case WMHI_MENUPICK:
                    {
                        switch(result & WMHI_MENUMASK)
                        {
                            case MENU_PROJEKT_BEENDEN:
                                uberReq("\033cBeenden?", uber_object, window);
                                done = TRUE;
                                break;

                            case MENU_PROJEKT_NEU:
                            {
                                ULONG newDoc = openNewDocument();
                                if(newDoc != (ULONG)-1)
                                {
                                    struct RCedDocument *doc = &RCed.documents[newDoc];
                                    
                                    /* Neuen Tab hinzufügen */
                                    addTabForDocument(newDoc, window);
                                    
                                    /* Zum neuen Dokument wechseln */
                                    switchDocument(newDoc, main_gadgets[GID_TEXTEDITOR], window);
                                    SetWindowTitles(window, doc->dateiname, (UBYTE *)~0);
                                }
                                else
                                {
                                    uberReq("Maximal 10 Dokumente offen!", uber_object, window);
                                }
                                break;
                            }

                            case MENU_PROJEKT_LADEN:
                            {
                                retASL = neuASL(window, "", "Datei laden",
                                                "Laden", FALSE);
                                if(retASL)
                                {
                                    char *newBuffer = (char *)loadText(retASL);
                                    if(newBuffer)
                                    {
                                        struct RCedDocument *doc = getCurrentDocument();
                                        if(doc)
                                        {
                                            if(doc->buffer)
                                                FreeVec(doc->buffer);

                                            doc->buffer = newBuffer;
                                            strcpy(doc->dateiname, retASL);
                                            doc->hasChanged = FALSE;
                                            doc->fileSize = (ULONG)strlen(newBuffer);
                                            doc->fileLines = 0;
                                            int i;
                                            for(i = 0; i < (int)doc->fileSize; i++)
                                                if(newBuffer[i] == '\n')
                                                    doc->fileLines++;
                                            if(doc->fileSize > 0)
                                                doc->fileLines++;

                                            /* Inhalt in TextEditor laden */
                                            SetGadgetAttrs(main_gadgets[GID_TEXTEDITOR], window, NULL,
                                                GA_TEXTEDITOR_Contents, (APTR)doc->buffer,
                                                GA_TEXTEDITOR_HasChanged, FALSE,
                                                TAG_DONE);
                                            SetWindowTitles(window, doc->dateiname, (UBYTE *)~0);

                                            /* Tab-Name gleich anpassen */
                                            updateTabLabels(window);
                                        }
                                        else
                                        {
                                            FreeVec(newBuffer);
                                        }
                                    }
                                    FreeVec(retASL);
                                    retASL = NULL;
                                }
                                break;
                            }

                            case MENU_PROJEKT_LADEN_NEU_TAB:
                            {
                                retASL = neuASL(window, "", "Datei laden",
                                                "Laden", FALSE);
                                if(retASL)
                                {
                                    char *newBuffer = (char *)loadText(retASL);
                                    if(newBuffer)
                                    {
                                        ULONG newDoc = openNewDocument();
                                        if(newDoc != (ULONG)-1)
                                        {
                                            struct RCedDocument *doc = &RCed.documents[newDoc];
                                            strcpy(doc->dateiname, retASL);
                                            doc->buffer = newBuffer;
                                            doc->hasChanged = FALSE;
                                            doc->fileSize = (ULONG)strlen(newBuffer);
                                            doc->fileLines = 0;
                                            int i;
                                            for(i = 0; i < (int)doc->fileSize; i++)
                                                if(newBuffer[i] == '\n')
                                                    doc->fileLines++;
                                            if(doc->fileSize > 0)
                                                doc->fileLines++;

                                            /* Neuen Tab hinzufügen */
                                            addTabForDocument(newDoc, window);

                                            /* Zum neuen Dokument wechseln */
                                            switchDocument(newDoc, main_gadgets[GID_TEXTEDITOR], window);
                                            refreshWindowTitle(window);
                                        }
                                        else
                                        {
                                            FreeVec(newBuffer);
                                            uberReq("Maximal 10 Dokumente offen!", uber_object, window);
                                        }
                                    }
                                    FreeVec(retASL);
                                    retASL = NULL;
                                }
                                break;
                            }


                            case MENU_PROJEKT_SPEICHERN:
                            {
                                struct RCedDocument *doc = getCurrentDocument();
                                UBYTE *exported;
                                
                                if(!doc)
                                    break;
                                
                                /* Wenn Dateiname "Unbenannt" ist, zu "Speichern als..." gehen */
                                if(strcmp(doc->dateiname, "Unbenannt") == 0)
                                {
                                    retASL = neuASL(window, "",
                                                    "Datei speichern",
                                                    "Speichern", FALSE);
                                    if(retASL)
                                    {
                                        strcpy(doc->dateiname, retASL);
                                        FreeVec(retASL);
                                        retASL = NULL;
                                        
                                        exported = (UBYTE *)DoGadgetMethod(
                                            main_gadgets[GID_TEXTEDITOR],
                                            window, NULL,
                                            GM_TEXTEDITOR_ExportText, NULL);
                                        saveText(doc->dateiname, exported);
                                        SetGadgetAttrs(main_gadgets[GID_TEXTEDITOR],
                                            window, NULL,
                                            GA_TEXTEDITOR_HasChanged, FALSE,
                                            TAG_DONE);
                                        SetWindowTitles(window, doc->dateiname, (UBYTE *)~0);
                                        
                                        /* Tab-Label aktualisieren */
                                        updateTabLabels(window);
                                        
                                        /* Statistiken updaten */
                                        doc->fileSize = strlen((char *)exported);
                                        doc->fileLines = 0;
                                        int i;
                                        for(i = 0; i < (int)doc->fileSize; i++)
                                            if(exported[i] == '\n')
                                                doc->fileLines++;
                                        if(doc->fileSize > 0)
                                            doc->fileLines++;
                                    }
                                }
                                else
                                {
                                    /* Normal speichern unter bestehendem Namen */
                                    exported = (UBYTE *)DoGadgetMethod(
                                        main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GM_TEXTEDITOR_ExportText, NULL);
                                    saveText(doc->dateiname, exported);
                                    SetGadgetAttrs(main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GA_TEXTEDITOR_HasChanged, FALSE,
                                        TAG_DONE);
                                    
                                    /* Statistiken updaten */
                                    doc->fileSize = strlen((char *)exported);
                                    doc->fileLines = 0;
                                    int i;
                                    for(i = 0; i < (int)doc->fileSize; i++)
                                        if(exported[i] == '\n')
                                            doc->fileLines++;
                                    if(doc->fileSize > 0)
                                        doc->fileLines++;
                                }
                                break;
                            }

                            case MENU_PROJEKT_SPEICHERNALS:
                            {
                                struct RCedDocument *doc = getCurrentDocument();
                                retASL = neuASL(window, "",
                                                "Datei speichern",
                                                "Speichern", FALSE);
                                if(retASL && doc)
                                {
                                    UBYTE *exp;
                                    strcpy(doc->dateiname, retASL);
                                    FreeVec(retASL);
                                    retASL = NULL;

                                    exp = (UBYTE *)DoGadgetMethod(
                                        main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GM_TEXTEDITOR_ExportText, NULL);
                                    saveText(doc->dateiname, exp);
                                    SetGadgetAttrs(
                                        main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GA_TEXTEDITOR_HasChanged, FALSE,
                                        TAG_DONE);
                                    SetWindowTitles(window,
                                                    doc->dateiname,
                                                    (UBYTE *)~0);
                                    
                                    /* Tab-Label aktualisieren */
                                    updateTabLabels(window);
                                    
                                    /* Statistiken updaten */
                                    doc->fileSize = strlen((char *)exp);
                                    doc->fileLines = 0;
                                    int i;
                                    for(i = 0; i < (int)doc->fileSize; i++)
                                        if(exp[i] == '\n')
                                            doc->fileLines++;
                                    if(doc->fileSize > 0)
                                        doc->fileLines++;
                                }
                                break;
                            }

                            case MENU_PROJEKT_UBER:
                                EasyRequestArgs(window, &es, NULL,
                                                (APTR)&version);
                                break;

                            case MENU_EINSTELLUNG_ZN:
                            {
                                GetAttr(GA_TEXTEDITOR_ShowLineNumbers,
                                        main_gadgets[GID_TEXTEDITOR], &status);
                                SetGadgetAttrs(
                                    main_gadgets[GID_TEXTEDITOR],
                                    window, NULL,
                                    GA_TEXTEDITOR_ShowLineNumbers,
                                    status ? FALSE : TRUE,
                                    TAG_DONE);
                                break;
                            }


                            case MENU_BEARBEITEN_SUCHEN:
                            {
                                char *searchText;
                                BOOL caseSensitive = FALSE;
                                BOOL fromTop = FALSE;
                                ULONG flags = GF_TEXTEDITOR_Search_Next;  /* Default: vorwärts */
                                ULONG found = 0;
                                
                                searchText = searchDialog(window, &caseSensitive, &fromTop);
                                if(searchText)
                                {
                                    printf("\n=== SUCHEN DEBUG ===\n");
                                    printf("Suchtext: '%s' (Laenge: %ld)\n", searchText, (long)strlen(searchText));                                    
                                    printf("caseSensitive: %d\n", caseSensitive);
                                    printf("fromTop: %d\n", fromTop);
                                    
                                    /* Flags setzen */
                                    if(fromTop)
                                        flags = GF_TEXTEDITOR_Search_FromTop;
                                    
                                    if(caseSensitive)
                                        flags |= GF_TEXTEDITOR_Search_CaseSensitive;
                                    
                                   printf("Flags: 0x%08lx\n", (unsigned long)flags);
                                    printf("  FromTop=%d, Backwards=%d, Next=%d, CaseSens=0x%lx\n",
                                           (int)GF_TEXTEDITOR_Search_FromTop,
                                           (int)GF_TEXTEDITOR_Search_Backwards,
                                           (int)GF_TEXTEDITOR_Search_Next,
                                           (unsigned long)GF_TEXTEDITOR_Search_CaseSensitive);
                                    
                                    /* Texteditor-Info */
                                    {
                                        UBYTE *content = NULL;
                                        ULONG cx = 0, cy = 0;
                                        GetAttr(GA_TEXTEDITOR_CursorX, main_gadgets[GID_TEXTEDITOR], &cx);
                                        GetAttr(GA_TEXTEDITOR_CursorY, main_gadgets[GID_TEXTEDITOR], &cy);
                                        printf("Cursor vor Suche: X=%ld Y=%ld\n", (long)cx, (long)cy);
                                        
                                        content = (UBYTE *)DoGadgetMethod(
                                            main_gadgets[GID_TEXTEDITOR],
                                            window, NULL,
                                            GM_TEXTEDITOR_ExportText, NULL);
                                        if(content)
                                        {
                                            printf("Texteditor-Inhalt (erste 100 Zeichen):\n'%.100s'\n", content);
                                        }
                                    }
                                    
                                    /* Suchen! */
                                    printf("\nRufe DoGadgetMethod auf...\n");
                                    found = DoGadgetMethod(
                                        main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GM_TEXTEDITOR_Search,
                                        NULL,
                                        searchText,
                                        flags);
                                    
                                    printf("Rueckgabe: found=%ld\n", (long)found);
                                    
                                    if(found)
                                    {
                                        ULONG cx2 = 0, cy2 = 0;
                                        GetAttr(GA_TEXTEDITOR_CursorX, main_gadgets[GID_TEXTEDITOR], &cx2);
                                        GetAttr(GA_TEXTEDITOR_CursorY, main_gadgets[GID_TEXTEDITOR], &cy2);
                                        printf("Cursor nach Suche: X=%ld Y=%ld\n", (long)cx2, (long)cy2);
                                        printf(">>> GEFUNDEN! <<<\n");
                                    }
                                    else
                                    {
                                        printf(">>> NICHT GEFUNDEN! <<<\n");
                                        uberReq("Suchtext nicht gefunden!", uber_object, window);
                                    }
                                    printf("===================\n\n");
                                    
                                    FreeVec(searchText);
                                }
                                break;
                            }
                            
                            case MENU_BEARBEITEN_ERSETZEN:
                            {
                                char *searchText = NULL;
                                char *replaceText = NULL;
                                BOOL caseSensitive = FALSE;
                                BOOL replaceAll = FALSE;
                                ULONG searchFlags = GF_TEXTEDITOR_Search_Next;
                                ULONG count = 0;
                                ULONG found;
                                
                                if(replaceDialog(window, &searchText, &replaceText, 
                                                 &caseSensitive, &replaceAll))
                                {
                                    if(searchText)
                                    {
                                        printf("DEBUG: Ersetze '%s' mit '%s' (case=%d, all=%d)\n",
                                               searchText, replaceText ? replaceText : "", 
                                               caseSensitive, replaceAll);
                                        
                                        /* Flags setzen */
                                        searchFlags = GF_TEXTEDITOR_Search_FromTop;  /* Immer von vorne */
                                        if(caseSensitive)
                                            searchFlags |= GF_TEXTEDITOR_Search_CaseSensitive;
                                        
                                        /* Ersetzen-Schleife */
                                        do
                                        {
                                            /* ERST suchen */
                                            found = DoGadgetMethod(
                                                main_gadgets[GID_TEXTEDITOR],
                                                window, NULL,
                                                GM_TEXTEDITOR_Search,
                                                NULL,
                                                searchText,
                                                searchFlags);
                                            
                                            if(found)
                                            {
                                                /* Gefunden! DANN ersetzen */
                                                DoGadgetMethod(
                                                    main_gadgets[GID_TEXTEDITOR],
                                                    window, NULL,
                                                    GM_TEXTEDITOR_Replace,
                                                    NULL,
                                                    replaceText ? replaceText : "",
                                                    0);  /* flags=0 */
                                                
                                                count++;
                                                
                                                /* Nächstes Vorkommen suchen (nicht mehr FromTop!) */
                                                searchFlags = GF_TEXTEDITOR_Search_Next;
                                                if(caseSensitive)
                                                    searchFlags |= GF_TEXTEDITOR_Search_CaseSensitive;
                                            }
                                        }
                                        while(found && replaceAll);
                                        
                                        printf("DEBUG: %ld Ersetzungen\n", (long)count);
                                        
                                        if(count == 0)
                                        {
                                            uberReq("Suchtext nicht gefunden!", uber_object, window);
                                        }
                                        else if(replaceAll && count > 0)
                                        {
                                            char msgBuf[128];
                                            sprintf(msgBuf, "%ld Vorkommen ersetzt!", (long)count);
                                            uberReq(msgBuf, uber_object, window);
                                        }
                                        
                                        FreeVec(searchText);
                                        if(replaceText) FreeVec(replaceText);
                                    }
                                }
                                break;
                            }

                            case MENU_BEARBEITEN_GOTO:
                            {
                                ULONG cy = 0;
                                ULONG lineNum;
                                GetAttr(GA_TEXTEDITOR_CursorY,
                                        main_gadgets[GID_TEXTEDITOR], &cy);
                                lineNum = gotoLineDialog(window, cy + 1);
                                if(lineNum > 0)
                                {
                                    sprintf(gotoLineBuf, "GOTOLINE %ld", (long)lineNum);
                                    DoGadgetMethod(
                                        main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GM_TEXTEDITOR_ARexxCmd, NULL,
                                        gotoLineBuf);
                                }
                                break;
                            }
                            
                            case MENU_PREFS_ALL:
                            {
                                printf(">>> gefunden <<<\n");
                                    PrefsDialog();
                                break;

                            }
                        }
                        break;
                    } /* WMHI_MENUPICK */

                    /* ---- Gadgets ---- */
                    case WMHI_GADGETUP:
                    {
                        switch(result & WMHI_GADGETMASK)
                        {
                            /* ---- ClickTab-Handler (Dokument-Wechsel) ---- */
                            case GID_TAB_0:
                            {
                                ULONG tabIndex = 0;
                                
                                /* Aktuellen Tab-Index auslesen */
                                GetAttr(CLICKTAB_Current,
                                    main_gadgets[GID_TAB_0], &tabIndex);
                                
                                /* Aktuellen Buffer speichern */
                                saveCurrentDocumentBuffer(main_gadgets[GID_TEXTEDITOR], window);
                                
                                /* Zu anderem Dokument wechseln */
                                if(tabIndex < 10 && RCed.documents[tabIndex].isOpen)
                                {
                                    switchDocument(tabIndex, main_gadgets[GID_TEXTEDITOR], window);
                                    SetWindowTitles(window,
                                        RCed.documents[tabIndex].dateiname,
                                        (UBYTE *)~0);
                                }
                                break;
                            }

                            case GID_EmptyB:
                            {
                                struct RCedDocument *doc = getCurrentDocument();
                                if(frageReq("\033cText löschen?",
                                            "_Ja|_Nein",
                                            neu_object, window) == 1)
                                {
                                    DoGadgetMethod(
                                        main_gadgets[GID_TEXTEDITOR],
                                        window, NULL,
                                        GM_TEXTEDITOR_ClearText, NULL);
                                    if(doc)
                                    {
                                        if(doc->buffer)
                                            FreeVec(doc->buffer);
                                        doc->buffer = (char *)AllocVec(1, MEMF_CLEAR);
                                        if(doc->buffer)
                                            doc->buffer[0] = '\0';
                                        strcpy(doc->dateiname, "Unbenannt");
                                        doc->hasChanged = FALSE;
                                        doc->fileLines = 0;
                                        doc->fileSize = 0;
                                        SetWindowTitles(window,
                                                        doc->dateiname,
                                                        (UBYTE *)~0);
                                    }
                                }
                                break;
                            }

                            case GID_VSCROLL:
                            {
                                ULONG top = 0;
                                GetAttr(SCROLLER_Top,
                                        main_gadgets[GID_VSCROLL], &top);
                                SetGadgetAttrs(
                                    main_gadgets[GID_TEXTEDITOR],
                                    window, NULL,
                                    GA_TEXTEDITOR_Prop_First, top,
                                    TAG_DONE);
                                break;
                            }


                        }
                        break;
                    } /* WMHI_GADGETUP */
                }
            }

            if(wait & timerSig)
            {
                ULONG cx = 0, cy = 0;
                ULONG pFirst = 0, pEntries = 0, pVisible = 0;
                ULONG currentChanged = 0;
                struct RCedDocument *doc = getCurrentDocument();
                WaitIO((struct IORequest *)timerReq);
                GetAttr(GA_TEXTEDITOR_CursorX, main_gadgets[GID_TEXTEDITOR], &cx);
                GetAttr(GA_TEXTEDITOR_CursorY, main_gadgets[GID_TEXTEDITOR], &cy);
                
                if(doc)
                {
                    updateStatusBar(cy, cx, doc->fileLines, doc->fileSize,
                                    main_gadgets[GID_STATUSBAR], window);
                }
                
                GetAttr(GA_TEXTEDITOR_Prop_First,   main_gadgets[GID_TEXTEDITOR], &pFirst);
                GetAttr(GA_TEXTEDITOR_Prop_Entries, main_gadgets[GID_TEXTEDITOR], &pEntries);
                GetAttr(GA_TEXTEDITOR_Prop_Visible, main_gadgets[GID_TEXTEDITOR], &pVisible);
                if(pEntries < 1) pEntries = 1;
                if(pVisible < 1) pVisible = 1;

                /* Texteditor -> Scroller: nur diese Richtung im Timer!
                 * Scroller -> Texteditor nur via WMHI_GADGETUP.        */
                SetGadgetAttrs(main_gadgets[GID_VSCROLL], window, NULL,
                    SCROLLER_Total,   pEntries,
                    SCROLLER_Visible, pVisible,
                    SCROLLER_Top,     pFirst,
                    TAG_DONE);

                /* Fenstertitel mit Asterisk aktualisieren, wenn sich HasChanged ändert */
                GetAttr(GA_TEXTEDITOR_HasChanged,
                        main_gadgets[GID_TEXTEDITOR], &currentChanged);
                if(doc && currentChanged != lastChanged)
                {
                    if(currentChanged)
                    {
                        /* Text wurde geändert - Asterisk hinzufügen */
                        sprintf(titleBuffer, "%s *", doc->dateiname);
                    }
                    else
                    {
                        /* Text wurde gespeichert - kein Asterisk */
                        sprintf(titleBuffer, "%s", doc->dateiname);
                    }
                    SetWindowTitles(window, titleBuffer, (UBYTE *)~0);
                    lastChanged = currentChanged;
                }

                timerReq->tr_node.io_Command = TR_ADDREQUEST;
                timerReq->tr_time.tv_secs    = 0;
                timerReq->tr_time.tv_micro   = 1000000; 
                SendIO((struct IORequest *)timerReq);
            }
        }
    } /* while(!done) */

cleanup_obj:
    if(timerOpen)
    {
        AbortIO((struct IORequest *)timerReq);
        WaitIO((struct IORequest *)timerReq);
        CloseDevice((struct IORequest *)timerReq);
    }
    if(timerReq)  DeleteIORequest((struct IORequest *)timerReq);
    if(timerPort) DeleteMsgPort(timerPort);
    DisposeObject(beenden_object);
    DisposeObject(uber_object);
    DisposeObject(neu_object);
    DisposeObject(Win_Object);

cleanup_libs:
    closeLibraries();

cleanup:
    {
        int i;
        
        /* ClickTab-Nodes freigeben */
        {
            struct Node *node;
            while((node = RemHead((struct List *)&RCed.tabList)) != NULL)
            {
                FreeClickTabNode(node);
            }
        }
        
        for(i = 0; i < 10; i++)
        {
            if(RCed.documents[i].buffer)
                FreeVec(RCed.documents[i].buffer);
        }
    }
    if(retASL) FreeVec(retASL);

    return RETURN_OK;
}