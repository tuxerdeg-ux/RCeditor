/* RC_req.h - Requester-Hilfsfunktionen
 *
 * BUGFIX: DoMethodA() kommt aus amiga.lib, Proto ist in
 *         <clib/alib_protos.h> -- ohne diesen Include meldet
 *          VBCC "implicit declaration".
 */

#include <intuition/classusr.h>
#include <clib/alib_protos.h>   /* DoMethodA(), DoMethod() */

Object *requesterNew(char *titel)
{
    Object *obj;

    obj = RequesterObject,
              REQ_TitleText, titel,
          EndMember;

    return obj;
}

ULONG OpenRequesterTags(Object *obj, struct Window *win, ULONG Tag1, ...)
{
    struct orRequest msg[1];

    msg->MethodID  = RM_OPENREQ;
    msg->or_Window = win;
    msg->or_Screen = NULL;
    msg->or_Attrs  = (struct TagItem *)&Tag1;

    return DoMethodA(obj, (Msg)msg);
}

ULONG beendenReq(char *text, Object *obj, struct Window *win)
{
    return OpenRequesterTags(obj, win,
        REQ_Type,       REQTYPE_INFO,
        REQ_BodyText,   text,
        REQ_GadgetText, "_Ja|_Nein",
        TAG_DONE);
}

ULONG frageReq(char *text, char *gadgets, Object *obj, struct Window *win)
{
    return OpenRequesterTags(obj, win,
        REQ_Type,       REQTYPE_INFO,
        REQ_BodyText,   text,
        REQ_GadgetText, gadgets,
        TAG_DONE);
}

ULONG uberReq(char *text, Object *obj, struct Window *win)
{
    return OpenRequesterTags(obj, win,
        REQ_Type,       REQTYPE_INFO,
        REQ_BodyText,   text,
        REQ_GadgetText, "_OK",
        TAG_DONE);
}

ULONG infoReq(char *text, Object *obj, struct Window *win)
{
    return OpenRequesterTags(obj, win,
        REQ_Type,       REQTYPE_INFO,
        REQ_BodyText,   text,
        REQ_GadgetText, "_OK",
        TAG_DONE);
}