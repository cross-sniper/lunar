/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#include "llimits.h"
#include "lobject.h"
#include "lstate.h"
#include "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/
#define luaD_checkstackaux(L,n,pre,pos)  \
	if (l_unlikely(L->stack_last.p - L->top.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
        else { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(void)0,(void)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->stack.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->stack.p) + (n))


/* macro to check stack size, preserving 'p' */
#define checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/* macro to check stack size and GC, preserving 'p' */
#define checkstackGCp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p);  /* save 'p' */ \
    luaC_checkGC(L),  /* stack grow uses memory */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/* macro to check stack size and GC */
#define checkstackGC(L,fsize)  \
	luaD_checkstackaux(L, (fsize), luaC_checkGC(L), (void)0)


/* type of protected functions, to be ran by 'runprotected' */
typedef void (*Pfunc) (luna_State *L, void *ud);

LUAI_FUNC void luaD_seterrorobj (luna_State *L, int errcode, StkId oldtop);
LUAI_FUNC int luaD_protectedparser (luna_State *L, ZIO *z, const char *name,
                                                  const char *mode);
LUAI_FUNC void luaD_hook (luna_State *L, int event, int line,
                                        int fTransfer, int nTransfer);
LUAI_FUNC void luaD_hookcall (luna_State *L, CallInfo *ci);
LUAI_FUNC int luaD_pretailcall (luna_State *L, CallInfo *ci, StkId func,
                                              int narg1, int delta);
LUAI_FUNC CallInfo *luaD_precall (luna_State *L, StkId func, int nResults);
LUAI_FUNC void luaD_call (luna_State *L, StkId func, int nResults);
LUAI_FUNC void luaD_callnoyield (luna_State *L, StkId func, int nResults);
LUAI_FUNC int luaD_closeprotected (luna_State *L, ptrdiff_t level, int status);
LUAI_FUNC int luaD_pcall (luna_State *L, Pfunc func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC void luaD_poscall (luna_State *L, CallInfo *ci, int nres);
LUAI_FUNC int luaD_reallocstack (luna_State *L, int newsize, int raiseerror);
LUAI_FUNC int luaD_growstack (luna_State *L, int n, int raiseerror);
LUAI_FUNC void luaD_shrinkstack (luna_State *L);
LUAI_FUNC void luaD_inctop (luna_State *L);

LUAI_FUNC l_noret luaD_throw (luna_State *L, int errcode);
LUAI_FUNC int luaD_rawrunprotected (luna_State *L, Pfunc f, void *ud);

#endif

