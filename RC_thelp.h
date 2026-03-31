/* ==================================================================
 * Dokument-Verwaltung f?r Multi-Document-Support
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

/* Gibt aktuelles Dokument zur?ck */
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

/* Speichert den aktuellen Buffer zur?ck ins aktive Dokument */
/* Entfernt alle ESC-Sequenzen aus einem Buffer.
 * Gibt einen neuen AllocVec-Buffer zurueck -- Caller muss FreeVec() aufrufen.
 * Noetig weil GM_TEXTEDITOR_ExportText die internen Style-Markierungen
 * als ESC-Sequenzen exportiert, die wir nicht im raw Buffer haben wollen.
 */
static char *stripEscSequences(const char *src)
{
    char *dst, *d;
    const char *s;
    ULONG len;

    if(!src) return NULL;
    len = strlen(src);
    dst = (char *)AllocVec(len + 1, MEMF_CLEAR);
    if(!dst) return NULL;

    d = dst;
    s = src;
    while(*s)
    {
        if(*s == '\033')  /* ESC-Zeichen */
        {
            s++;
            /* Ueberspringe das Steuerbyte */
            if(*s == 'p' && *(s+1))  /* \033p[x] = Farbe */
                s += 2;
            else if(*s == '[')       /* \033[...m ANSI-Sequenz */
            {
                while(*s && *s != 'm') s++;
                if(*s) s++;
            }
            else if(*s)              /* \033b, \033n, \033i, \033u etc. */
                s++;
            continue;
        }
        *d++ = *s++;
    }
    *d = '\0';
    return dst;
}

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
        /* ESC-Sequenzen entfernen -- wir speichern immer reinen Text */
        char *clean = stripEscSequences((char *)exported);

        if(doc->buffer)
            FreeVec(doc->buffer);

        doc->buffer = clean ? clean : (char *)AllocVec(1, MEMF_CLEAR);

        /* Datei-Statistiken berechnen */
        doc->fileSize = doc->buffer ? strlen(doc->buffer) : 0;
        doc->fileLines = 0;
        {
            int i;
            for(i = 0; i < (int)doc->fileSize; i++)
                if(doc->buffer[i] == '\n')
                    doc->fileLines++;
        }
        if(doc->fileSize > 0)
            doc->fileLines++;
    }
}

/* ?ffnet ein neues leeres Dokument */
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

/* Schlie?t ein Dokument */
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
                /* Tab wurde geschlossen - Dokument schlie?en */
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

/* F?gt einen neuen Tab f?r das Dokument hinzu */
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
    
    /* Gehe durch alle Tabs und aktualisiere ?ber UserData -> Dokumentindex */
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

