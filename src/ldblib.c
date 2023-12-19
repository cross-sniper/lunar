/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUNA_LIB

#include "lprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
static const char *const HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
static void checkstack (luna_State *L, luna_State *L1, int n) {
  if (l_unlikely(L != L1 && !luna_checkstack(L1, n)))
    lunaL_error(L, "stack overflow");
}


static int db_getregistry (luna_State *L) {
  luna_pushvalue(L, LUNA_REGISTRYINDEX);
  return 1;
}


static int db_getmetatable (luna_State *L) {
  lunaL_checkany(L, 1);
  if (!luna_getmetatable(L, 1)) {
    luna_pushnil(L);  /* no metatable */
  }
  return 1;
}


static int db_setmetatable (luna_State *L) {
  int t = luna_type(L, 2);
  lunaL_argexpected(L, t == LUNA_TNIL || t == LUNA_TTABLE, 2, "nil or table");
  luna_settop(L, 2);
  luna_setmetatable(L, 1);
  return 1;  /* return 1st argument */
}


static int db_getuservalue (luna_State *L) {
  int n = (int)lunaL_optinteger(L, 2, 1);
  if (luna_type(L, 1) != LUNA_TUSERDATA)
    lunaL_pushfail(L);
  else if (luna_getiuservalue(L, 1, n) != LUNA_TNONE) {
    luna_pushboolean(L, 1);
    return 2;
  }
  return 1;
}


static int db_setuservalue (luna_State *L) {
  int n = (int)lunaL_optinteger(L, 3, 1);
  lunaL_checktype(L, 1, LUNA_TUSERDATA);
  lunaL_checkany(L, 2);
  luna_settop(L, 2);
  if (!luna_setiuservalue(L, 1, n))
    lunaL_pushfail(L);
  return 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static luna_State *getthread (luna_State *L, int *arg) {
  if (luna_isthread(L, 1)) {
    *arg = 1;
    return luna_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'luna_settable', used by 'db_getinfo' to put results
** from 'luna_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
static void settabss (luna_State *L, const char *k, const char *v) {
  luna_pushstring(L, v);
  luna_setfield(L, -2, k);
}

static void settabsi (luna_State *L, const char *k, int v) {
  luna_pushinteger(L, v);
  luna_setfield(L, -2, k);
}

static void settabsb (luna_State *L, const char *k, int v) {
  luna_pushboolean(L, v);
  luna_setfield(L, -2, k);
}


/*
** In function 'db_getinfo', the call to 'luna_getinfo' may push
** results on the stack; later it creates the result table to put
** these objects. Function 'treatstackoption' puts the result from
** 'luna_getinfo' on top of the result table so that it can call
** 'luna_setfield'.
*/
static void treatstackoption (luna_State *L, luna_State *L1, const char *fname) {
  if (L == L1)
    luna_rotate(L, -2, 1);  /* exchange object and table */
  else
    luna_xmove(L1, L, 1);  /* move object to the "main" stack */
  luna_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'luna_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'luna_getinfo'.
*/
static int db_getinfo (luna_State *L) {
  luna_Debug ar;
  int arg;
  luna_State *L1 = getthread(L, &arg);
  const char *options = lunaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  lunaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  if (luna_isfunction(L, arg + 1)) {  /* info about a function? */
    options = luna_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    luna_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    luna_xmove(L, L1, 1);
  }
  else {  /* stack level */
    if (!luna_getstack(L1, (int)lunaL_checkinteger(L, arg + 1), &ar)) {
      lunaL_pushfail(L);  /* level out of range */
      return 1;
    }
  }
  if (!luna_getinfo(L1, options, &ar))
    return lunaL_argerror(L, arg+2, "invalid option");
  luna_newtable(L);  /* table to collect results */
  if (strchr(options, 'S')) {
    luna_pushlstring(L, ar.source, ar.srclen);
    luna_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  if (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  if (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  if (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  if (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  if (strchr(options, 't'))
    settabsb(L, "istailcall", ar.istailcall);
  if (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  if (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  return 1;  /* return table */
}


static int db_getlocal (luna_State *L) {
  int arg;
  luna_State *L1 = getthread(L, &arg);
  int nvar = (int)lunaL_checkinteger(L, arg + 2);  /* local-variable index */
  if (luna_isfunction(L, arg + 1)) {  /* function argument? */
    luna_pushvalue(L, arg + 1);  /* push function */
    luna_pushstring(L, luna_getlocal(L, NULL, nvar));  /* push local name */
    return 1;  /* return only name (there is no value) */
  }
  else {  /* stack-level argument */
    luna_Debug ar;
    const char *name;
    int level = (int)lunaL_checkinteger(L, arg + 1);
    if (l_unlikely(!luna_getstack(L1, level, &ar)))  /* out of range? */
      return lunaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = luna_getlocal(L1, &ar, nvar);
    if (name) {
      luna_xmove(L1, L, 1);  /* move local value */
      luna_pushstring(L, name);  /* push name */
      luna_rotate(L, -2, 1);  /* re-order */
      return 2;
    }
    else {
      lunaL_pushfail(L);  /* no name (nor value) */
      return 1;
    }
  }
}


static int db_setlocal (luna_State *L) {
  int arg;
  const char *name;
  luna_State *L1 = getthread(L, &arg);
  luna_Debug ar;
  int level = (int)lunaL_checkinteger(L, arg + 1);
  int nvar = (int)lunaL_checkinteger(L, arg + 2);
  if (l_unlikely(!luna_getstack(L1, level, &ar)))  /* out of range? */
    return lunaL_argerror(L, arg+1, "level out of range");
  lunaL_checkany(L, arg+3);
  luna_settop(L, arg+3);
  checkstack(L, L1, 1);
  luna_xmove(L, L1, 1);
  name = luna_setlocal(L1, &ar, nvar);
  if (name == NULL)
    luna_pop(L1, 1);  /* pop value (if not popped by 'luna_setlocal') */
  luna_pushstring(L, name);
  return 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
static int auxupvalue (luna_State *L, int get) {
  const char *name;
  int n = (int)lunaL_checkinteger(L, 2);  /* upvalue index */
  lunaL_checktype(L, 1, LUNA_TFUNCTION);  /* closure */
  name = get ? luna_getupvalue(L, 1, n) : luna_setupvalue(L, 1, n);
  if (name == NULL) return 0;
  luna_pushstring(L, name);
  luna_insert(L, -(get+1));  /* no-op if get is false */
  return get + 1;
}


static int db_getupvalue (luna_State *L) {
  return auxupvalue(L, 1);
}


static int db_setupvalue (luna_State *L) {
  lunaL_checkany(L, 3);
  return auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
static void *checkupval (luna_State *L, int argf, int argnup, int *pnup) {
  void *id;
  int nup = (int)lunaL_checkinteger(L, argnup);  /* upvalue index */
  lunaL_checktype(L, argf, LUNA_TFUNCTION);  /* closure */
  id = luna_upvalueid(L, argf, nup);
  if (pnup) {
    lunaL_argcheck(L, id != NULL, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  return id;
}


static int db_upvalueid (luna_State *L) {
  void *id = checkupval(L, 1, 2, NULL);
  if (id != NULL)
    luna_pushlightuserdata(L, id);
  else
    lunaL_pushfail(L);
  return 1;
}


static int db_upvaluejoin (luna_State *L) {
  int n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  lunaL_argcheck(L, !luna_iscfunction(L, 1), 1, "Lua function expected");
  lunaL_argcheck(L, !luna_iscfunction(L, 3), 3, "Lua function expected");
  luna_upvaluejoin(L, 1, n1, 3, n2);
  return 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
static void hookf (luna_State *L, luna_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  luna_getfield(L, LUNA_REGISTRYINDEX, HOOKKEY);
  luna_pushthread(L);
  if (luna_rawget(L, -2) == LUNA_TFUNCTION) {  /* is there a hook function? */
    luna_pushstring(L, hooknames[(int)ar->event]);  /* push event name */
    if (ar->currentline >= 0)
      luna_pushinteger(L, ar->currentline);  /* push current line */
    else luna_pushnil(L);
    luna_assert(luna_getinfo(L, "lS", ar));
    luna_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static int makemask (const char *smask, int count) {
  int mask = 0;
  if (strchr(smask, 'c')) mask |= LUNA_MASKCALL;
  if (strchr(smask, 'r')) mask |= LUNA_MASKRET;
  if (strchr(smask, 'l')) mask |= LUNA_MASKLINE;
  if (count > 0) mask |= LUNA_MASKCOUNT;
  return mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
static char *unmakemask (int mask, char *smask) {
  int i = 0;
  if (mask & LUNA_MASKCALL) smask[i++] = 'c';
  if (mask & LUNA_MASKRET) smask[i++] = 'r';
  if (mask & LUNA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  return smask;
}


static int db_sethook (luna_State *L) {
  int arg, mask, count;
  luna_Hook func;
  luna_State *L1 = getthread(L, &arg);
  if (luna_isnoneornil(L, arg+1)) {  /* no hook? */
    luna_settop(L, arg+1);
    func = NULL; mask = 0; count = 0;  /* turn off hooks */
  }
  else {
    const char *smask = lunaL_checkstring(L, arg+2);
    lunaL_checktype(L, arg+1, LUNA_TFUNCTION);
    count = (int)lunaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, count);
  }
  if (!lunaL_getsubtable(L, LUNA_REGISTRYINDEX, HOOKKEY)) {
    /* table just created; initialize it */
    luna_pushliteral(L, "k");
    luna_setfield(L, -2, "__mode");  /** hooktable.__mode = "k" */
    luna_pushvalue(L, -1);
    luna_setmetatable(L, -2);  /* metatable(hooktable) = hooktable */
  }
  checkstack(L, L1, 1);
  luna_pushthread(L1); luna_xmove(L1, L, 1);  /* key (thread) */
  luna_pushvalue(L, arg + 1);  /* value (hook function) */
  luna_rawset(L, -3);  /* hooktable[L1] = new Lua hook */
  luna_sethook(L1, func, mask, count);
  return 0;
}


static int db_gethook (luna_State *L) {
  int arg;
  luna_State *L1 = getthread(L, &arg);
  char buff[5];
  int mask = luna_gethookmask(L1);
  luna_Hook hook = luna_gethook(L1);
  if (hook == NULL) {  /* no hook? */
    lunaL_pushfail(L);
    return 1;
  }
  else if (hook != hookf)  /* external hook? */
    luna_pushliteral(L, "external hook");
  else {  /* hook table must exist */
    luna_getfield(L, LUNA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    luna_pushthread(L1); luna_xmove(L1, L, 1);
    luna_rawget(L, -2);   /* 1st result = hooktable[L1] */
    luna_remove(L, -2);  /* remove hook table */
  }
  luna_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  luna_pushinteger(L, luna_gethookcount(L1));  /* 3rd result = count */
  return 3;
}


static int db_debug (luna_State *L) {
  for (;;) {
    char buffer[250];
    luna_writestringerror("%s", "luna_debug> ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL ||
        strcmp(buffer, "cont\n") == 0)
      return 0;
    if (lunaL_loadbuffer(L, buffer, strlen(buffer), "=(debug command)") ||
        luna_pcall(L, 0, 0, 0))
      luna_writestringerror("%s\n", lunaL_tolstring(L, -1, NULL));
    luna_settop(L, 0);  /* remove eventual returns */
  }
}


static int db_traceback (luna_State *L) {
  int arg;
  luna_State *L1 = getthread(L, &arg);
  const char *msg = luna_tostring(L, arg + 1);
  if (msg == NULL && !luna_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    luna_pushvalue(L, arg + 1);  /* return it untouched */
  else {
    int level = (int)lunaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    lunaL_traceback(L, L1, msg, level);
  }
  return 1;
}


static int db_setcstacklimit (luna_State *L) {
  int limit = (int)lunaL_checkinteger(L, 1);
  int res = luna_setcstacklimit(L, limit);
  luna_pushinteger(L, res);
  return 1;
}


static const lunaL_Reg dblib[] = {
  {"debug", db_debug},
  {"getuservalue", db_getuservalue},
  {"gethook", db_gethook},
  {"getinfo", db_getinfo},
  {"getlocal", db_getlocal},
  {"getregistry", db_getregistry},
  {"getmetatable", db_getmetatable},
  {"getupvalue", db_getupvalue},
  {"upvaluejoin", db_upvaluejoin},
  {"upvalueid", db_upvalueid},
  {"setuservalue", db_setuservalue},
  {"sethook", db_sethook},
  {"setlocal", db_setlocal},
  {"setmetatable", db_setmetatable},
  {"setupvalue", db_setupvalue},
  {"traceback", db_traceback},
  {"setcstacklimit", db_setcstacklimit},
  {NULL, NULL}
};


LUAMOD_API int luaopen_debug (luna_State *L) {
  lunaL_newlib(L, dblib);
  return 1;
}

