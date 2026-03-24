#define VERSION  0
#define REVISION 1
#define DATE     "22.03.2026"
#define VERS     "RC 0.1"
#define VSTRING  "RC 0.1 (22.03.2026) \r\n"

/* BUGFIX: VERSTAG darf kein \0 im version-String sein!
 * Das \0 beendet den String sofort nach "Version:".
 * Deshalb: VERSTAG nur als eigenständiges Array für den
 * AmigaOS-Versionsscanner, NICHT in version einbetten.
 */
static const char verstag[] = "\0$VER: RC 0.1 (22.03.2026)";
static const char *version  = "RC 0.1 (22.03.2026)";
