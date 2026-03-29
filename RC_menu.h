/* RC_menu.h - Menüdefinitionen */

struct NewMenu mainMenu[] =
{
    { NM_TITLE, "Projekt",               NULL, 0,                    0, NULL },
    { NM_ITEM,  "Neu",                   "N",  0,                    0, NULL }, /* Index 0 */
    { NM_ITEM,  "Laden...",              "L",  0,                    0, NULL }, /* Index 1 */
    { NM_ITEM,  "Laden in neuem Tab...", NULL, 0,                    0, NULL }, /* Index 2 */
    { NM_ITEM,  NM_BARLABEL,             NULL, 0,                    0, NULL }, /* Index 3 */
    { NM_ITEM,  "Speichern",             "S",  0,                    0, NULL }, /* Index 4 */
    { NM_ITEM,  "Speichern als...",      "A",  0,                    0, NULL }, /* Index 5 */
    { NM_ITEM,  NM_BARLABEL,             NULL, 0,                    0, NULL }, /* Index 6 */
    { NM_ITEM,  "Ueber",                 NULL, 0,                    0, NULL }, /* Index 7 */
    { NM_ITEM,  NM_BARLABEL,             NULL, 0,                    0, NULL }, /* Index 8 */
    { NM_ITEM,  "Beenden",               "Q",  0,                    0, NULL }, /* Index 9 */

    { NM_TITLE, "Bearbeiten",            NULL, 0,                    0, NULL },
    { NM_ITEM,  "Ausschneiden",          "X",  0,                    0, NULL }, /* Index 0 */
    { NM_ITEM,  "Kopieren",              "C",  0,                    0, NULL }, /* Index 1 */
    { NM_ITEM,  "Einfuegen",             "V",  0,                    0, NULL }, /* Index 2 */
    { NM_ITEM,  NM_BARLABEL,             NULL, 0,                    0, NULL }, /* Index 3 */
    { NM_ITEM,  "Rueckgaengig",          "Z",  0,                    0, NULL }, /* Index 4 */
    { NM_ITEM,  "Wiederholen",           "Y",  0,                    0, NULL }, /* Index 5 */
    { NM_ITEM,  NM_BARLABEL,             NULL, 0,                    0, NULL }, /* Index 6 */
    { NM_ITEM,  "Suchen...",             "F",  0,                    0, NULL }, /* Index 7 */
    { NM_ITEM,  "Ersetzen...",           "H",  0,                    0, NULL }, /* Index 8 */
    { NM_ITEM,  NM_BARLABEL,             NULL, 0,                    0, NULL }, /* Index 9 */
    { NM_ITEM,  "Gehe zu Zeile...",      "G",  0,                    0, NULL }, /* Index 10 */

    { NM_TITLE, "Einstellungen",         NULL, 0,                    0, NULL },
    { NM_ITEM,  "Preferences",           NULL, 0,                    0, NULL }, /* Index 0 */

    { NM_END,   NULL,                    NULL, 0,                    0, NULL }
};

/* Projekt-Menü
 *  0: Neu
 *  1: Laden...
 *  2: Laden in neuem Tab...
 *  3: NM_BARLABEL
 *  4: Speichern
 *  5: Speichern als...
 *  6: NM_BARLABEL
 *  7: Ueber
 *  8: NM_BARLABEL
 *  9: Beenden
 */
#define MENU_PROJEKT_NEU           FULLMENUNUM(0, 0, NOSUB)
#define MENU_PROJEKT_LADEN         FULLMENUNUM(0, 1, NOSUB)
#define MENU_PROJEKT_LADEN_NEU_TAB FULLMENUNUM(0, 2, NOSUB)
#define MENU_PROJEKT_SPEICHERN     FULLMENUNUM(0, 4, NOSUB)
#define MENU_PROJEKT_SPEICHERNALS  FULLMENUNUM(0, 5, NOSUB)
#define MENU_PROJEKT_UBER          FULLMENUNUM(0, 7, NOSUB)
#define MENU_PROJEKT_BEENDEN       FULLMENUNUM(0, 9, NOSUB)

/* Bearbeiten-Menü
 *  0: Ausschneiden
 *  1: Kopieren
 *  2: Einfuegen
 *  3: NM_BARLABEL
 *  4: Rueckgaengig
 *  5: Wiederholen
 *  6: NM_BARLABEL
 *  7: Suchen...
 *  8: Ersetzen...
 *  9: NM_BARLABEL
 * 10: Gehe zu Zeile...
 */
#define MENU_BEARBEITEN_CUT        FULLMENUNUM(1, 0, NOSUB)
#define MENU_BEARBEITEN_COPY       FULLMENUNUM(1, 1, NOSUB)
#define MENU_BEARBEITEN_PASTE      FULLMENUNUM(1, 2, NOSUB)
#define MENU_BEARBEITEN_UNDO       FULLMENUNUM(1, 4, NOSUB)
#define MENU_BEARBEITEN_REDO       FULLMENUNUM(1, 5, NOSUB)
#define MENU_BEARBEITEN_SUCHEN     FULLMENUNUM(1, 7, NOSUB)
#define MENU_BEARBEITEN_ERSETZEN   FULLMENUNUM(1, 8, NOSUB)
#define MENU_BEARBEITEN_GOTO       FULLMENUNUM(1, 10, NOSUB)

/* Einstellungen-Menü
 *  0: Preferences
 */
#define MENU_PREFS_ALL             FULLMENUNUM(2, 0, NOSUB)