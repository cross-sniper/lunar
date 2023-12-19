/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUNA_LIB

#include "lprefix.h"


#include <stdlib.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static luna_State *getco (luna_State *L) {
  luna_State *co = luna_tothread(L, 1);
  lunaL_argexpected(L, co, 1, "thread");
  return co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static int auxresume (luna_State *L, luna_State *co, int narg) {
  int status, nres;
  if (l_unlikely(!luna_checkstack(co, narg))) {
    luna_pushliteral(L, "too many arguments to resume");
    return -1;  /* error flag */
  }
  luna_xmove(L, co, narg);
  status = luna_resume(co, L, narg, &nres);
  if (l_likely(status == LUNA_OK || status == LUNA_YIELD)) {
    if (l_unlikely(!luna_checkstack(L, nres + 1))) {
      luna_pop(co, nres);  /* remove results anyway */
      luna_pushliteral(L, "too many results to resume");
      return -1;  /* error flag */
    }
    luna_xmove(co, L, nres);  /* move yielded values */
    return nres;
  }
  else {
    luna_xmove(co, L, 1);  /* move error message */
    return -1;  /* error flag */
  }
}


static int lunaB_coresume (luna_State *L) {
  luna_State *co = getco(L);
  int r;
  r = auxresume(L, co, luna_gettop(L) - 1);
  if (l_unlikely(r < 0)) {
    luna_pushboolean(L, 0);
    luna_insert(L, -2);
    return 2;  /* return false + error message */
  }
  else {
    luna_pushboolean(L, 1);
    luna_insert(L, -(r + 1));
    return r + 1;  /* return true + 'resume' returns */
  }
}


static int lunaB_auxwrap (luna_State *L) {
  luna_State *co = luna_tothread(L, luna_upvalueindex(1));
  int r = auxresume(L, co, luna_gettop(L));
  if (l_unlikely(r < 0)) {  /* error? */
    int stat = luna_status(co);
    if (stat != LUNA_OK && stat != LUNA_YIELD) {  /* error in the coroutine? */
      stat = luna_closethread(co, L);  /* close its tbc variables */
      luna_assert(stat != LUNA_OK);
      luna_xmove(co, L, 1);  /* move error message to the caller */
    }
    if (stat != LUNA_ERRMEM &&  /* not a memory error and ... */
        luna_type(L, -1) == LUNA_TSTRING) {  /* ... error object is a string? */
      lunaL_where(L, 1);  /* add extra info, if available */
      luna_insert(L, -2);
      luna_concat(L, 2);
    }
    return luna_error(L);  /* propagate error */
  }
  return r;
}


static int lunaB_cocreate (luna_State *L) {
  luna_State *NL;
  lunaL_checktype(L, 1, LUNA_TFUNCTION);
  NL = luna_newthread(L);
  luna_pushvalue(L, 1);  /* move function to top */
  luna_xmove(L, NL, 1);  /* move function from L to NL */
  return 1;
}


static int lunaB_cowrap (luna_State *L) {
  lunaB_cocreate(L);
  luna_pushcclosure(L, lunaB_auxwrap, 1);
  return 1;
}


static int lunaB_yield (luna_State *L) {
  return luna_yield(L, luna_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static int auxstatus (luna_State *L, luna_State *co) {
  if (L == co) return COS_RUN;
  else {
    switch (luna_status(co)) {
      case LUNA_YIELD:
        return COS_YIELD;
      case LUNA_OK: {
        luna_Debug ar;
        if (luna_getstack(co, 0, &ar))  /* does it have frames? */
          return COS_NORM;  /* it is running */
        else if (luna_gettop(co) == 0)
            return COS_DEAD;
        else
          return COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        return COS_DEAD;
    }
  }
}


static int lunaB_costatus (luna_State *L) {
  luna_State *co = getco(L);
  luna_pushstring(L, statname[auxstatus(L, co)]);
  return 1;
}


static int lunaB_yieldable (luna_State *L) {
  luna_State *co = luna_isnone(L, 1) ? L : getco(L);
  luna_pushboolean(L, luna_isyieldable(co));
  return 1;
}


static int lunaB_corunning (luna_State *L) {
  int ismain = luna_pushthread(L);
  luna_pushboolean(L, ismain);
  return 2;
}


static int lunaB_close (luna_State *L) {
  luna_State *co = getco(L);
  int status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = luna_closethread(co, L);
      if (status == LUNA_OK) {
        luna_pushboolean(L, 1);
        return 1;
      }
      else {
        luna_pushboolean(L, 0);
        luna_xmove(co, L, 1);  /* move error message */
        return 2;
      }
    }
    default:  /* normal or running coroutine */
      return lunaL_error(L, "cannot close a %s coroutine", statname[status]);
  }
}


static const lunaL_Reg co_funcs[] = {
  {"create", lunaB_cocreate},
  {"resume", lunaB_coresume},
  {"running", lunaB_corunning},
  {"status", lunaB_costatus},
  {"wrap", lunaB_cowrap},
  {"yield", lunaB_yield},
  {"isyieldable", lunaB_yieldable},
  {"close", lunaB_close},
  {NULL, NULL}
};



LUAMOD_API int luaopen_coroutine (luna_State *L) {
  lunaL_newlib(L, co_funcs);
  return 1;
}

