/* RC_menu.h - Menüdefinitionen */

struct NewMenu mainMenu[] =
{
    { NM_TITLE, "Projekt",          NULL, 0,                      0, NULL },
    { NM_ITEM,  "Neu",              "N",  0,                      0, NULL }, /* Index 0 */
    { NM_ITEM,  "Laden...",         "L",  0,                      0, NULL }, /* Index 1 */
    { NM_ITEM,  "Laden in neuem Tab...", "N",  0,                      0, NULL }, /* Index 2 */
    { NM_ITEM,  NM_BARLABEL,        NULL, 0,                      0, NULL }, /* Index 3 */
    { NM_ITEM,  "Speichern",        "S",  0,                      0, NULL }, /* Index  4*/
    { NM_ITEM,  "Speichern als...", "A",  0,                      0, NULL }, /* Index 5*/
    { NM_ITEM,  NM_BARLABEL,        NULL, 0,                      0, NULL }, /* Index 6*/
    { NM_ITEM,  "Über",            NULL, 0,                      0, NULL }, /* Index 7 */
    { NM_ITEM,  NM_BARLABEL,        NULL, 0,                      0, NULL }, /* Index 8 */
    { NM_ITEM,  "Beenden",          "Q",  0,                      0, NULL }, /* Index 9 */

    { NM_TITLE, "Bearbeiten",       NULL, 0,                      0, NULL },
    { NM_ITEM,  "Suchen...",        "F",  0,                      0, NULL }, /* Index 0 */
    { NM_ITEM,  "Ersetzen...",      "H",  0,                      0, NULL }, /* Index 1 */
    { NM_ITEM,  NM_BARLABEL,        NULL, 0,                      0, NULL }, /* Index 2 */
    { NM_ITEM,  "Gehe zu Zeile...", "G",  0,                      0, NULL }, /* Index 3 */

    { NM_TITLE, "Einstellungen",    NULL, 0,                      0, NULL },
    { NM_ITEM,  "Zeilennummer",     NULL, CHECKIT | MENUTOGGLE,   0, NULL }, /* Index 0 */
    { NM_ITEM,  "Einstellungen",    NULL,  0,                      0, NULL }, /* Index 1 */

    { NM_END,   NULL,               NULL, 0,                      0, NULL }
};

/* Projekt-Menü */
#define MENU_PROJEKT_NEU         FULLMENUNUM(0, 0, NOSUB)
#define MENU_PROJEKT_LADEN       FULLMENUNUM(0, 1, NOSUB)
#define MENU_PROJEKT_SPEICHERN   FULLMENUNUM(0, 3, NOSUB)
#define MENU_PROJEKT_SPEICHERNALS FULLMENUNUM(0, 4, NOSUB)
#define MENU_PROJEKT_LADEN_NEU_TAB FULLMENUNUM(0, 2, NOSUB)
#define MENU_PROJEKT_UBER        FULLMENUNUM(0, 7, NOSUB)
#define MENU_PROJEKT_BEENDEN     FULLMENUNUM(0, 9, NOSUB)

/* Bearbeiten-Menü */
#define MENU_BEARBEITEN_SUCHEN   FULLMENUNUM(1, 0, NOSUB)
#define MENU_BEARBEITEN_ERSETZEN FULLMENUNUM(1, 1, NOSUB)
#define MENU_BEARBEITEN_GOTO     FULLMENUNUM(1, 3, NOSUB)

/* Einstellungen-Menü */
#define MENU_EINSTELLUNG_ZN      FULLMENUNUM(2, 0, NOSUB)
#define MENU_PREFS_ALL      FULLMENUNUM(2, 1, NOSUB)
