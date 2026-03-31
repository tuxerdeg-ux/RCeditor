/* RC_highlight.h - Syntax-Highlighting fuer C und Shell
 *
 * Funktioniert ueber den eingebauten ImportHook des texteditor.gadget:
 * Der Text wird vor dem Laden mit ESC-Sequenzen versehen.
 *
 * ESC-Sequenzen (Plain ImportHook):
 *   \033b        = Bold ein
 *   \033n        = Normal (reset)
 *   \033p[x]     = Farbe x aus Colormap (x = '1'..'9')
 *
 * Colormap-Belegung (Standard):
 *   1 = Shine      -> benutzt fuer Strings   (hell)
 *   2 = Halfshine  -> benutzt fuer Kommentare
 *   6 = Text       -> benutzt fuer Keywords  (bold)
 *
 * Einschraenkung laut Doku:
 *   "Don't use custom hooks yet. Although the above hooks import style
 *    correctly, the style information will be garbaged if the user
 *    edits the text."
 *   -> Highlighting nur beim Laden, nicht beim Tippen.
 *
 * Verwendung:
 *   char *highlighted = highlightText(rawBuffer, HIGHLIGHT_C);
 *   SetGadgetAttrs(editor, win, NULL,
 *       GA_TEXTEDITOR_Contents, highlighted, TAG_DONE);
 *   FreeVec(highlighted);
 */

#ifndef RC_HIGHLIGHT_H
#define RC_HIGHLIGHT_H

/* Sprach-Modi */
#define HIGHLIGHT_NONE  0
#define HIGHLIGHT_C     1
#define HIGHLIGHT_SHELL 2

void updateTabLabels(struct Window *win); // Forward declaration

/* ------------------------------------------------------------------ */
/* C-Keywords                                                          */
/* ------------------------------------------------------------------ */
static const char *cKeywords[] = {
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long",
    "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while",
    /* C-Praeprozessor */
    "#include", "#define", "#ifdef", "#ifndef", "#endif",
    "#else", "#elif", "#undef", "#pragma", "#if",
    NULL
};

/* ------------------------------------------------------------------ */
/* Shell-Keywords                                                      */
/* ------------------------------------------------------------------ */
static const char *shellKeywords[] = {
    "if", "then", "else", "endif", "end",
    "while", "endwhile", "do", "enddo",
    "for", "endfor", "next",
    "echo", "set", "unset", "setenv",
    "cd", "dir", "list", "copy", "delete", "rename",
    "run", "execute", "skip", "lab", "fail",
    "stack", "path", "assign", "makedir",
    "resident", "endshell", "quit",
    NULL
};

/* ------------------------------------------------------------------ */
/* Hilfsfunktion: pruefe ob Zeichen ein Wort-Trenner ist              */
/* ------------------------------------------------------------------ */
static BOOL isSep(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',' ||
            c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '=' || c == '<' || c == '>' || c == '!' ||
            c == '&' || c == '|' || c == '^' || c == '~' ||
            c == '%' || c == '\0');
}

/* ------------------------------------------------------------------ */
/* highlightText()                                                     */
/*                                                                     */
/* Gibt einen neuen AllocVec-Buffer zurueck der den Text mit          */
/* ESC-Sequenzen versehen enthaelt.                                   */
/* Caller muss FreeVec() aufrufen!                                    */
/*                                                                     */
/* mode: HIGHLIGHT_C oder HIGHLIGHT_SHELL                             */
/* ------------------------------------------------------------------ */
char *highlightText(const char *src, int mode)
{
    const char **keywords;
    ULONG srcLen, dstMax;
    char *dst, *d;
    const char *s;
    BOOL inComment  = FALSE;  /* C: /* ... */
    BOOL inLineComment = FALSE; /* C: // ... oder Shell: # ... */
    BOOL inString   = FALSE;
    char stringChar = 0;

    if(!src) return NULL;

    if(mode == HIGHLIGHT_C)
        keywords = cKeywords;
    else if(mode == HIGHLIGHT_SHELL)
        keywords = shellKeywords;
    else
    {
        /* Kein Highlighting -- Kopie zurueckgeben */
        ULONG l = strlen(src) + 1;
        dst = (char *)AllocVec(l, MEMF_CLEAR);
        if(dst) strcpy(dst, src);
        return dst;
    }

    srcLen = strlen(src);
    /* Worst case: jedes Zeichen bekommt ESC-Sequenzen (~8 Bytes extra) */
    dstMax = srcLen * 10 + 1;
    dst = (char *)AllocVec(dstMax, MEMF_CLEAR);
    if(!dst) return NULL;

    d = dst;
    s = src;

    while(*s)
    {
        /* ---- Zeilenende: Zust?nde zur?cksetzen ---- */
        if(*s == '\n')
        {
            if(inComment || inLineComment || inString)
            {
                /* Normal zur?cksetzen */
                *d++ = '\033'; *d++ = 'n';
            }
            inLineComment = FALSE;
            if(!inComment) inString = FALSE;
            *d++ = *s++;
            continue;
        }

        /* ---- Bereits in Kommentar  ---- */
        if(inComment)
        {
            *d++ = *s;
            if(*s == '*' && *(s+1) == '/')
            {
                *d++ = '/';
                s += 2;
                /* Kommentar-Ende: Normal */
                *d++ = '\033'; *d++ = 'n';
                inComment = FALSE;
            }
            else s++;
            continue;
        }

        /* ---- Bereits in Zeilenkommentar ---- */
        if(inLineComment)
        {
            *d++ = *s++;
            continue;
        }

        /* ---- Bereits in String ---- */
        if(inString)
        {
            *d++ = *s;
            if(*s == '\\' && *(s+1))
            {
                /* Escape-Zeichen im String */
                s++;
                *d++ = *s++;
            }
            else if(*s == stringChar)
            {
                s++;
                *d++ = '\033'; *d++ = 'n';
                inString = FALSE;
            }
            else s++;
            continue;
        }

        /* ---- C Block-Kommentar Start: /* ---- */
        if(mode == HIGHLIGHT_C && *s == '/' && *(s+1) == '*')
        {
            *d++ = '\033'; *d++ = 'i'; /* italic fuer Kommentare */
            *d++ = *s++; *d++ = *s++;
            inComment = TRUE;
            continue;
        }

        /* ---- C Zeilen-Kommentar: // ---- */
        if(mode == HIGHLIGHT_C && *s == '/' && *(s+1) == '/')
        {
            *d++ = '\033'; *d++ = 'i'; /* italic fuer Kommentare */
            *d++ = *s++; *d++ = *s++;
            inLineComment = TRUE;
            continue;
        }

        /* ---- Shell Kommentar: # (nur am Zeilenanfang oder nach Space) ---- */
        if(mode == HIGHLIGHT_SHELL && *s == '#')
        {
            *d++ = '\033'; *d++ = 'i'; /* italic fuer Kommentare */
            *d++ = *s++;
            inLineComment = TRUE;
            continue;
        }

        /* ---- String-Erkennung ---- */
        if(*s == '"' || *s == '\'')
        {
            stringChar = *s;
            *d++ = '\033'; *d++ = 'u'; /* underline fuer Strings */
            *d++ = *s++;
            inString = TRUE;
            continue;
        }

        /* ---- Keyword-Erkennung ---- */
        {
            int ki;
            BOOL found = FALSE;

            for(ki = 0; keywords[ki]; ki++)
            {
                int klen = strlen(keywords[ki]);
                /* Muss am Wort-Anfang stehen */
                if(strncmp(s, keywords[ki], klen) == 0 && isSep(s[klen]))
                {
                    /* Keyword: Bold */
                    *d++ = '\033'; *d++ = 'b';
                    strncpy(d, keywords[ki], klen);
                    d += klen;
                    *d++ = '\033'; *d++ = 'n';
                    s += klen;
                    found = TRUE;
                    break;
                }
            }
            if(found) continue;
        }

        /* ---- Normales Zeichen ---- */
        *d++ = *s++;
    }

    /* Abschliessend Normal-Reset falls noch in Zustand */
    if(inComment || inString)
    {
        *d++ = '\033'; *d++ = 'n';
    }

    *d = '\0';
    return dst;
}

/* ------------------------------------------------------------------ */
/* detectFileMode()                                                    */
/*                                                                     */
/* Erkennt den Dateityp anhand der Endung.                            */
/* ------------------------------------------------------------------ */
int detectHighlightMode(const char *filename)
{
    const char *ext;
    int len;

    if(!filename) return HIGHLIGHT_NONE;

    len = strlen(filename);

    /* Dateiendung suchen */
    ext = filename + len - 1;
    while(ext > filename && *ext != '.' && *ext != '/' && *ext != ':')
        ext--;

    if(*ext != '.') return HIGHLIGHT_NONE;

    /* C / C++ */
    if(strcmp(ext, ".c")   == 0) return HIGHLIGHT_C;
    if(strcmp(ext, ".h")   == 0) return HIGHLIGHT_C;
    if(strcmp(ext, ".cpp") == 0) return HIGHLIGHT_C;
    if(strcmp(ext, ".cc")  == 0) return HIGHLIGHT_C;

    /* Amiga Shell Script */
    if(strcmp(ext, ".sh")  == 0) return HIGHLIGHT_SHELL;
    if(strcmp(ext, ".bat") == 0) return HIGHLIGHT_SHELL;
    if(strcmp(ext, ".cmd") == 0) return HIGHLIGHT_SHELL;

    return HIGHLIGHT_NONE;
}



#endif /* RC_HIGHLIGHT_H */