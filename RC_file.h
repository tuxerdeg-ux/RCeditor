/* RC_file.h - Datei-Operationen mit AmigaDOS-Funktionen
 *
 * 
 *   - Seek()-Rückgabewert korrekt ausgewertet:
 *     Seek() gibt die VORHERIGE Position zurück, nicht die neue.
 *     Bei Fehler kommt -1 -- das wurde bisher nicht geprüft,
 *     was zu AllocVec(0) oder AllocVec(riesig) führen konnte.
 *   - Chunk-weises Lesen bleibt erhalten (sicher für große Dateien).
 *   - Write() statt FPuts() bleibt erhalten (sicher für Binärdaten).
 */

void saveText(char *name, UBYTE *buffer)
{
    BPTR file;
    long len;

    if(!name || !buffer)
        return;

    file = Open(name, MODE_NEWFILE);
    if(file)
    {
        len = strlen((char *)buffer);
        if(len > 0)
        {
            /* Write statt FPuts -- sicher für große Daten */
            Write(file, buffer, len);
        }
        Close(file);
    }
}

UBYTE *loadText(char *name)
{
    BPTR  file;
    char *buffer    = NULL;
    LONG  filesize  = 0;
    LONG  totalRead = 0;
    LONG  bytesRead;
    LONG  seekResult;

    #define CHUNK_SIZE 8192  /* 8 KB Chunks */

    if(!name)
        return NULL;

    file = Open(name, MODE_OLDFILE);
    if(!file)
        return NULL;

    /* -------------------------------------------------------
     * Dateigröße ermitteln.
     *
     * Seek(file, 0, OFFSET_END)       springt ans Ende und
     *   gibt die VORHERIGE Position (= 0) zurück.
     * Seek(file, 0, OFFSET_BEGINNING) springt zum Anfang und
     *   gibt die VORHERIGE Position (= Dateiende = Größe) zurück.
     *
     * BUGFIX: Bei Fehler liefert Seek() -1.
     *         Das wurde früher nicht geprüft!
     * ------------------------------------------------------- */
    if(Seek(file, 0, OFFSET_END) < 0)
    {
        Close(file);
        return NULL;
    }

    seekResult = Seek(file, 0, OFFSET_BEGINNING);
    if(seekResult < 0)
    {
        Close(file);
        return NULL;
    }

    filesize = seekResult;   /* Vorherige Position = Dateiende = Größe */

    if(filesize > 0)
    {
        /* Speicher allokieren (+1 für Nullbyte) */
        buffer = (char *)AllocVec((ULONG)(filesize + 1), MEMF_CLEAR);

        if(buffer)
        {
            /* Datei in Chunks einlesen -- Read() kann weniger
             * zurückgeben als angefordert, daher Schleife.      */
            while(totalRead < filesize)
            {
                LONG toRead = filesize - totalRead;
                if(toRead > CHUNK_SIZE)
                    toRead = CHUNK_SIZE;

                bytesRead = Read(file, buffer + totalRead, toRead);

                if(bytesRead <= 0)
                {
                    /* Lesefehler: Speicher freigeben */
                    if(bytesRead < 0)
                    {
                        FreeVec(buffer);
                        buffer = NULL;
                    }
                    break;
                }

                totalRead += bytesRead;

                /* Weniger als angefordert -> EOF erreicht */
                if(bytesRead < toRead)
                    break;
            }

            if(buffer)
                buffer[totalRead] = '\0';  /* Null-Terminierung */
        }
    }
    else
    {
        /* Leere Datei -- leeren String zurückgeben */
        buffer = (char *)AllocVec(1, MEMF_CLEAR);
        if(buffer)
            buffer[0] = '\0';
    }

    Close(file);
    return (UBYTE *)buffer;
}
