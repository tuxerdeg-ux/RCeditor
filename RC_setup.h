/* RC_setup.h - Gadget-IDs, Library-Handles, openLibraries/closeLibraries */

#ifndef RC_SETUP_H
#define RC_SETUP_H

#define MAX_DOCUMENTS 10

enum
{
    GID_MAIN = 0,
    GID_ZeilNub,
    GID_TEXTEDITOR,
    GID_EmptyB,
    GID_REMBUTTON,
    GID_REPLACE,
    GID_QUIT,
    GID_test,
    GID_STATUSBAR,   /* Informationsbalken Cursorposition */
    GID_VSCROLL,     /* Vertikaler Scrollbalken           */
    GID_TAB_0,       /* ClickTab-Gadget */
    GID_LAST
};


extern struct Library *WindowBase;
extern struct Library *ButtonBase;
extern struct Library *StringBase;
extern struct Library *LabelBase;
extern struct Library *GadToolsBase;
extern struct Library *LayoutBase;
extern struct Library *RequesterBase;
extern struct Library *AslBase;
extern struct Library *TextFieldBase;
extern struct Library *ScrollerBase;
extern struct Library *CheckBoxBase;
extern struct Library *IconBase;
extern struct Library *ClickTabBase;
extern struct Library *IntegerBase;
/* Struktur f?r ein einzelnes Dokument */
struct RCedDocument
{
    char dateiname[256];     /* Dateiname */
    char *buffer;            /* Textinhalt */
    ULONG hasChanged;        /* Text wurde ge?ndert */
    ULONG fileLines;         /* Anzahl Zeilen */
    ULONG fileSize;          /* Gr??e in Bytes */
    BOOL isOpen;             /* Dokument offen? */
};

/* Globale Struktur f?r Multi-Dokument-Support */
struct RCed
{
    struct RCedDocument documents[10];  /* Bis zu 10 Dokumente */
    ULONG activeDocIndex;               /* Index des aktiven Dokuments (0-9) */
    struct MinList tabList;             /* Liste der ClickTab-Nodes */
    APTR tabCallback;                   /* ClickTab Callback-Objekt */
    APTR tabObject;                     /* Das ClickTab-Gadget selbst */
};

extern struct RCed RCed;

void closeLibraries(void)
{
    if(WindowBase)    CloseLibrary(WindowBase);
    if(StringBase)    CloseLibrary(StringBase);
    if(ScrollerBase)  CloseLibrary(ScrollerBase);
    if(CheckBoxBase)  CloseLibrary(CheckBoxBase);
    if(TextFieldBase) CloseLibrary(TextFieldBase);
    if(LayoutBase)    CloseLibrary(LayoutBase);
    if(RequesterBase) CloseLibrary(RequesterBase);
    if(AslBase)       CloseLibrary(AslBase);
    if(ClickTabBase)  CloseLibrary(ClickTabBase);
    if(LabelBase) CloseLibrary(LabelBase);
    if(IntegerBase)   CloseLibrary(IntegerBase);
}

BOOL openLibraries(void)
{
    BOOL ret = FALSE;

    if((WindowBase    = OpenLibrary("window.class",              0L)) &&
       (StringBase    = OpenLibrary("gadgets/string.gadget",     0L)) &&
       (ScrollerBase  = OpenLibrary("gadgets/scroller.gadget",   0L)) &&
       (CheckBoxBase  = OpenLibrary("gadgets/checkbox.gadget",   0L)) &&
       (TextFieldBase = OpenLibrary("gadgets/texteditor.gadget", 0L)) &&
       (LayoutBase    = OpenLibrary("gadgets/layout.gadget",     0L)) &&
       (RequesterBase = OpenLibrary("requester.class",           0L)) &&
       (AslBase       = OpenLibrary("asl.library",               0L)) &&
       (ClickTabBase  = OpenLibrary("gadgets/clicktab.gadget",   0L)) &&
       (LabelBase = OpenLibrary("images/label.image", 0L)) &&
       (IntegerBase   = OpenLibrary("gadgets/integer.gadget",    0L)))
       
    {
        ret = TRUE;
    }

    if(!ret) closeLibraries();

    return ret;
}
#endif /* RC_SETUP_H */