/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUNA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static int lunaB_print (luna_State *L) {
  int n = luna_gettop(L);  /* number of arguments */
  int i;
  for (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    const char *s = lunaL_tolstring(L, i, &l);  /* convert it to string */
    if (i > 1)  /* not the first element? */
      luna_writestring("\t", 1);  /* add a tab before it */
    luna_writestring(s, l);  /* print it */
    luna_pop(L, 1);  /* pop result */
  }
  luna_writeline();
  return 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
static int lunaB_warn (luna_State *L) {
  int n = luna_gettop(L);  /* number of arguments */
  int i;
  lunaL_checkstring(L, 1);  /* at least one argument */
  for (i = 2; i <= n; i++)
    lunaL_checkstring(L, i);  /* make sure all arguments are strings */
  for (i = 1; i < n; i++)  /* compose warning */
    luna_warning(L, luna_tostring(L, i), 1);
  luna_warning(L, luna_tostring(L, n), 0);  /* close warning */
  return 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const char *b_str2int (const char *s, int base, luna_Integer *pn) {
  luna_Unsigned n = 0;
  int neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  if (*s == '-') { s++; neg = 1; }  /* handle sign */
  else if (*s == '+') s++;
  if (!isalnum((unsigned char)*s))  /* no digit? */
    return NULL;
  do {
    int digit = (isdigit((unsigned char)*s)) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (digit >= base) return NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } while (isalnum((unsigned char)*s));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (luna_Integer)((neg) ? (0u - n) : n);
  return s;
}


static int lunaB_tonumber (luna_State *L) {
  if (luna_isnoneornil(L, 2)) {  /* standard conversion? */
    if (luna_type(L, 1) == LUNA_TNUMBER) {  /* already a number? */
      luna_settop(L, 1);  /* yes; return it */
      return 1;
    }
    else {
      size_t l;
      const char *s = luna_tolstring(L, 1, &l);
      if (s != NULL && luna_stringtonumber(L, s) == l + 1)
        return 1;  /* successful conversion to number */
      /* else not a number */
      lunaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  else {
    size_t l;
    const char *s;
    luna_Integer n = 0;  /* to avoid warnings */
    luna_Integer base = lunaL_checkinteger(L, 2);
    lunaL_checktype(L, 1, LUNA_TSTRING);  /* no numbers as strings */
    s = luna_tolstring(L, 1, &l);
    lunaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    if (b_str2int(s, (int)base, &n) == s + l) {
      luna_pushinteger(L, n);
      return 1;
    }  /* else not a number */
  }  /* else not a number */
  lunaL_pushfail(L);  /* not a number */
  return 1;
}


static int lunaB_error (luna_State *L) {
  int level = (int)lunaL_optinteger(L, 2, 1);
  luna_settop(L, 1);
  if (luna_type(L, 1) == LUNA_TSTRING && level > 0) {
    lunaL_where(L, level);   /* add extra information */
    luna_pushvalue(L, 1);
    luna_concat(L, 2);
  }
  return luna_error(L);
}


static int lunaB_getmetatable (luna_State *L) {
  lunaL_checkany(L, 1);
  if (!luna_getmetatable(L, 1)) {
    luna_pushnil(L);
    return 1;  /* no metatable */
  }
  lunaL_getmetafield(L, 1, "__metatable");
  return 1;  /* returns either __metatable field (if present) or metatable */
}


static int lunaB_setmetatable (luna_State *L) {
  int t = luna_type(L, 2);
  lunaL_checktype(L, 1, LUNA_TTABLE);
  lunaL_argexpected(L, t == LUNA_TNIL || t == LUNA_TTABLE, 2, "nil or table");
  if (l_unlikely(lunaL_getmetafield(L, 1, "__metatable") != LUNA_TNIL))
    return lunaL_error(L, "cannot change a protected metatable");
  luna_settop(L, 2);
  luna_setmetatable(L, 1);
  return 1;
}


static int lunaB_rawequal (luna_State *L) {
  lunaL_checkany(L, 1);
  lunaL_checkany(L, 2);
  luna_pushboolean(L, luna_rawequal(L, 1, 2));
  return 1;
}


static int lunaB_rawlen (luna_State *L) {
  int t = luna_type(L, 1);
  lunaL_argexpected(L, t == LUNA_TTABLE || t == LUNA_TSTRING, 1,
                      "table or string");
  luna_pushinteger(L, luna_rawlen(L, 1));
  return 1;
}


static int lunaB_rawget (luna_State *L) {
  lunaL_checktype(L, 1, LUNA_TTABLE);
  lunaL_checkany(L, 2);
  luna_settop(L, 2);
  luna_rawget(L, 1);
  return 1;
}

static int lunaB_rawset (luna_State *L) {
  lunaL_checktype(L, 1, LUNA_TTABLE);
  lunaL_checkany(L, 2);
  lunaL_checkany(L, 3);
  luna_settop(L, 3);
  luna_rawset(L, 1);
  return 1;
}


static int pushmode (luna_State *L, int oldmode) {
  if (oldmode == -1)
    lunaL_pushfail(L);  /* invalid call to 'luna_gc' */
  else
    luna_pushstring(L, (oldmode == LUNA_GCINC) ? "incremental"
                                             : "generational");
  return 1;
}


/*
** check whether call to 'luna_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { if (res == -1) break; }

static int lunaB_collectgarbage (luna_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", "generational", "incremental", NULL};
  static const int optsnum[] = {LUNA_GCSTOP, LUNA_GCRESTART, LUNA_GCCOLLECT,
    LUNA_GCCOUNT, LUNA_GCSTEP, LUNA_GCSETPAUSE, LUNA_GCSETSTEPMUL,
    LUNA_GCISRUNNING, LUNA_GCGEN, LUNA_GCINC};
  int o = optsnum[lunaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    case LUNA_GCCOUNT: {
      int k = luna_gc(L, o);
      int b = luna_gc(L, LUNA_GCCOUNTB);
      checkvalres(k);
      luna_pushnumber(L, (luna_Number)k + ((luna_Number)b/1024));
      return 1;
    }
    case LUNA_GCSTEP: {
      int step = (int)lunaL_optinteger(L, 2, 0);
      int res = luna_gc(L, o, step);
      checkvalres(res);
      luna_pushboolean(L, res);
      return 1;
    }
    case LUNA_GCSETPAUSE:
    case LUNA_GCSETSTEPMUL: {
      int p = (int)lunaL_optinteger(L, 2, 0);
      int previous = luna_gc(L, o, p);
      checkvalres(previous);
      luna_pushinteger(L, previous);
      return 1;
    }
    case LUNA_GCISRUNNING: {
      int res = luna_gc(L, o);
      checkvalres(res);
      luna_pushboolean(L, res);
      return 1;
    }
    case LUNA_GCGEN: {
      int minormul = (int)lunaL_optinteger(L, 2, 0);
      int majormul = (int)lunaL_optinteger(L, 3, 0);
      return pushmode(L, luna_gc(L, o, minormul, majormul));
    }
    case LUNA_GCINC: {
      int pause = (int)lunaL_optinteger(L, 2, 0);
      int stepmul = (int)lunaL_optinteger(L, 3, 0);
      int stepsize = (int)lunaL_optinteger(L, 4, 0);
      return pushmode(L, luna_gc(L, o, pause, stepmul, stepsize));
    }
    default: {
      int res = luna_gc(L, o);
      checkvalres(res);
      luna_pushinteger(L, res);
      return 1;
    }
  }
  lunaL_pushfail(L);  /* invalid call (inside a finalizer) */
  return 1;
}


static int lunaB_type (luna_State *L) {
  int t = luna_type(L, 1);
  lunaL_argcheck(L, t != LUNA_TNONE, 1, "value expected");
  luna_pushstring(L, luna_typename(L, t));
  return 1;
}


static int lunaB_next (luna_State *L) {
  lunaL_checktype(L, 1, LUNA_TTABLE);
  luna_settop(L, 2);  /* create a 2nd argument if there isn't one */
  if (luna_next(L, 1))
    return 2;
  else {
    luna_pushnil(L);
    return 1;
  }
}


static int pairscont (luna_State *L, int status, luna_KContext k) {
  (void)L; (void)status; (void)k;  /* unused */
  return 3;
}

static int lunaB_pairs (luna_State *L) {
  lunaL_checkany(L, 1);
  if (lunaL_getmetafield(L, 1, "__pairs") == LUNA_TNIL) {  /* no metamethod? */
    luna_pushcfunction(L, lunaB_next);  /* will return generator, */
    luna_pushvalue(L, 1);  /* state, */
    luna_pushnil(L);  /* and initial value */
  }
  else {
    luna_pushvalue(L, 1);  /* argument 'self' to metamethod */
    luna_callk(L, 1, 3, 0, pairscont);  /* get 3 values from metamethod */
  }
  return 3;
}


/*
** Traversal function for 'ipairs'
*/
static int ipairsaux (luna_State *L) {
  luna_Integer i = lunaL_checkinteger(L, 2);
  i = lunaL_intop(+, i, 1);
  luna_pushinteger(L, i);
  return (luna_geti(L, 1, i) == LUNA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static int lunaB_ipairs (luna_State *L) {
  lunaL_checkany(L, 1);
  luna_pushcfunction(L, ipairsaux);  /* iteration function */
  luna_pushvalue(L, 1);  /* state */
  luna_pushinteger(L, 0);  /* initial value */
  return 3;
}


static int load_aux (luna_State *L, int status, int envidx) {
  if (l_likely(status == LUNA_OK)) {
    if (envidx != 0) {  /* 'env' parameter? */
      luna_pushvalue(L, envidx);  /* environment for loaded function */
      if (!luna_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        luna_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    lunaL_pushfail(L);
    luna_insert(L, -2);  /* put before error message */
    return 2;  /* return fail plus error message */
  }
}


static int lunaB_loadfile (luna_State *L) {
  const char *fname = lunaL_optstring(L, 1, NULL);
  const char *mode = lunaL_optstring(L, 2, NULL);
  int env = (!luna_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = lunaL_loadfilex(L, fname, mode);
  return load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'luna_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader (luna_State *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  lunaL_checkstack(L, 2, "too many nested functions");
  luna_pushvalue(L, 1);  /* get function */
  luna_call(L, 0, 1);  /* call it */
  if (luna_isnil(L, -1)) {
    luna_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (l_unlikely(!luna_isstring(L, -1)))
    lunaL_error(L, "reader function must return a string");
  luna_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
  return luna_tolstring(L, RESERVEDSLOT, size);
}


static int lunaB_load (luna_State *L) {
  int status;
  size_t l;
  const char *s = luna_tolstring(L, 1, &l);
  const char *mode = lunaL_optstring(L, 3, "bt");
  int env = (!luna_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = lunaL_optstring(L, 2, s);
    status = lunaL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = lunaL_optstring(L, 2, "=(load)");
    lunaL_checktype(L, 1, LUNA_TFUNCTION);
    luna_settop(L, RESERVEDSLOT);  /* create reserved slot */
    status = luna_load(L, generic_reader, NULL, chunkname, mode);
  }
  return load_aux(L, status, env);
}

/* }====================================================== */


static int dofilecont (luna_State *L, int d1, luna_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'luna_Kfunction' prototype */
  return luna_gettop(L) - 1;
}


static int lunaB_dofile (luna_State *L) {
  const char *fname = lunaL_optstring(L, 1, NULL);
  luna_settop(L, 1);
  if (l_unlikely(lunaL_loadfile(L, fname) != LUNA_OK))
    return luna_error(L);
  luna_callk(L, 0, LUNA_MULTRET, 0, dofilecont);
  return dofilecont(L, 0, 0);
}


static int lunaB_assert (luna_State *L) {
  if (l_likely(luna_toboolean(L, 1)))  /* condition is true? */
    return luna_gettop(L);  /* return all arguments */
  else {  /* error */
    lunaL_checkany(L, 1);  /* there must be a condition */
    luna_remove(L, 1);  /* remove it */
    luna_pushliteral(L, "assertion failed!");  /* default message */
    luna_settop(L, 1);  /* leave only message (default if no other one) */
    return lunaB_error(L);  /* call 'error' */
  }
}


static int lunaB_select (luna_State *L) {
  int n = luna_gettop(L);
  if (luna_type(L, 1) == LUNA_TSTRING && *luna_tostring(L, 1) == '#') {
    luna_pushinteger(L, n-1);
    return 1;
  }
  else {
    luna_Integer i = lunaL_checkinteger(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    lunaL_argcheck(L, 1 <= i, 1, "index out of range");
    return n - (int)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int finishpcall (luna_State *L, int status, luna_KContext extra) {
  if (l_unlikely(status != LUNA_OK && status != LUNA_YIELD)) {  /* error? */
    luna_pushboolean(L, 0);  /* first result (false) */
    luna_pushvalue(L, -2);  /* error message */
    return 2;  /* return false, msg */
  }
  else
    return luna_gettop(L) - (int)extra;  /* return all results */
}


static int lunaB_pcall (luna_State *L) {
  int status;
  lunaL_checkany(L, 1);
  luna_pushboolean(L, 1);  /* first result if no errors */
  luna_insert(L, 1);  /* put it in place */
  status = luna_pcallk(L, luna_gettop(L) - 2, LUNA_MULTRET, 0, 0, finishpcall);
  return finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'luna_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static int lunaB_xpcall (luna_State *L) {
  int status;
  int n = luna_gettop(L);
  lunaL_checktype(L, 2, LUNA_TFUNCTION);  /* check error function */
  luna_pushboolean(L, 1);  /* first result */
  luna_pushvalue(L, 1);  /* function */
  luna_rotate(L, 3, 2);  /* move them below function's arguments */
  status = luna_pcallk(L, n - 2, LUNA_MULTRET, 2, 2, finishpcall);
  return finishpcall(L, status, 2);
}


static int lunaB_tostring (luna_State *L) {
  lunaL_checkany(L, 1);
  lunaL_tolstring(L, 1, NULL);
  return 1;
}

#include "custom/includes.c"

LUAMOD_API int luaopen_base (luna_State *L) {
  /* open lib into global table */
  luna_pushglobaltable(L);
  lunaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  luna_pushvalue(L, -1);
  luna_setfield(L, -2, LUNA_GNAME);
  /* set global _VERSION */
  luna_pushliteral(L, LUNA_VERSION);
  luna_setfield(L, -2, "_VERSION");
  return 1;
}

