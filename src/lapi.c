/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUNA_CORE

#include "lprefix.h"


#include <limits.h>
#include <stdarg.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"



const char luna_ident[] =
  "$LuaVersion: " LUNA_COPYRIGHT " $"
  "$LuaAuthors: " LUNA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
** '!ttisnil(o)' implies 'o != &G(L)->nilvalue', so it is not needed.
** However, it covers the most common cases in a faster way.
*/
#define isvalid(L, o)	(!ttisnil(o) || o != &G(L)->nilvalue)


/* test for pseudo index */
#define ispseudo(i)		((i) <= LUNA_REGISTRYINDEX)

/* test for upvalue */
#define isupvalue(i)		((i) < LUNA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
static TValue *index2value (luna_State *L, int idx) {
  CallInfo *ci = L->ci;
  if (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->top.p - (ci->func.p + 1), "unacceptable index");
    if (o >= L->top.p) return &G(L)->nilvalue;
    else return s2v(o);
  }
  else if (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->top.p - (ci->func.p + 1),
                 "invalid index");
    return s2v(L->top.p + idx);
  }
  else if (idx == LUNA_REGISTRYINDEX)
    return &G(L)->l_registry;
  else {  /* upvalues */
    idx = LUNA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    if (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      return (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    else {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      return &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
l_sinline StkId index2stack (luna_State *L, int idx) {
  CallInfo *ci = L->ci;
  if (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->top.p, "invalid index");
    return o;
  }
  else {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->top.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    return L->top.p + idx;
  }
}


LUNA_API int luna_checkstack (luna_State *L, int n) {
  int res;
  CallInfo *ci;
  luna_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  if (L->stack_last.p - L->top.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  else  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  if (res && ci->top.p < L->top.p + n)
    ci->top.p = L->top.p + n;  /* adjust frame top */
  luna_unlock(L);
  return res;
}


LUNA_API void luna_xmove (luna_State *from, luna_State *to, int n) {
  int i;
  if (from == to) return;
  luna_lock(to);
  api_checknelems(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->top.p - to->top.p >= n, "stack overflow");
  from->top.p -= n;
  for (i = 0; i < n; i++) {
    setobjs2s(to, to->top.p, from->top.p + i);
    to->top.p++;  /* stack already checked by previous 'api_check' */
  }
  luna_unlock(to);
}


LUNA_API luna_CFunction luna_atpanic (luna_State *L, luna_CFunction panicf) {
  luna_CFunction old;
  luna_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  luna_unlock(L);
  return old;
}


LUNA_API luna_Number luna_version (luna_State *L) {
  UNUSED(L);
  return LUNA_VERSION_NUM;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUNA_API int luna_absindex (luna_State *L, int idx) {
  return (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->top.p - L->ci->func.p) + idx;
}


LUNA_API int luna_gettop (luna_State *L) {
  return cast_int(L->top.p - (L->ci->func.p + 1));
}


LUNA_API void luna_settop (luna_State *L, int idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  luna_lock(L);
  ci = L->ci;
  func = ci->func.p;
  if (idx >= 0) {
    api_check(L, idx <= ci->top.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->top.p;
    for (; diff > 0; diff--)
      setnilvalue(s2v(L->top.p++));  /* clear new slots */
  }
  else {
    api_check(L, -(idx+1) <= (L->top.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  api_check(L, L->tbclist.p < L->top.p, "previous pop of an unclosed slot");
  newtop = L->top.p + diff;
  if (diff < 0 && L->tbclist.p >= newtop) {
    luna_assert(hastocloseCfunc(ci->nresults));
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->top.p = newtop;  /* correct top only after closing any upvalue */
  luna_unlock(L);
}


LUNA_API void luna_closeslot (luna_State *L, int idx) {
  StkId level;
  luna_lock(L);
  level = index2stack(L, idx);
  api_check(L, hastocloseCfunc(L->ci->nresults) && L->tbclist.p == level,
     "no variable to close at given level");
  level = luaF_close(L, level, CLOSEKTOP, 0);
  setnilvalue(s2v(level));
  luna_unlock(L);
}


/*
** Reverse the stack segment from 'from' to 'to'
** (auxiliary to 'luna_rotate')
** Note that we move(copy) only the value inside the stack.
** (We do not move additional fields that may exist.)
*/
l_sinline void reverse (luna_State *L, StkId from, StkId to) {
  for (; from < to; from++, to--) {
    TValue temp;
    setobj(L, &temp, s2v(from));
    setobjs2s(L, from, to);
    setobj2s(L, to, &temp);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUNA_API void luna_rotate (luna_State *L, int idx, int n) {
  StkId p, t, m;
  luna_lock(L);
  t = L->top.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  luna_unlock(L);
}


LUNA_API void luna_copy (luna_State *L, int fromidx, int toidx) {
  TValue *fr, *to;
  luna_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  if (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUNA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  luna_unlock(L);
}


LUNA_API void luna_pushvalue (luna_State *L, int idx) {
  luna_lock(L);
  setobj2s(L, L->top.p, index2value(L, idx));
  api_incr_top(L);
  luna_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUNA_API int luna_type (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (isvalid(L, o) ? ttype(o) : LUNA_TNONE);
}


LUNA_API const char *luna_typename (luna_State *L, int t) {
  UNUSED(L);
  api_check(L, LUNA_TNONE <= t && t < LUNA_NUMTYPES, "invalid type");
  return ttypename(t);
}


LUNA_API int luna_iscfunction (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (ttislcf(o) || (ttisCclosure(o)));
}


LUNA_API int luna_isinteger (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return ttisinteger(o);
}


LUNA_API int luna_isnumber (luna_State *L, int idx) {
  luna_Number n;
  const TValue *o = index2value(L, idx);
  return tonumber(o, &n);
}


LUNA_API int luna_isstring (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (ttisstring(o) || cvt2str(o));
}


LUNA_API int luna_isuserdata (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUNA_API int luna_rawequal (luna_State *L, int index1, int index2) {
  const TValue *o1 = index2value(L, index1);
  const TValue *o2 = index2value(L, index2);
  return (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUNA_API void luna_arith (luna_State *L, int op) {
  luna_lock(L);
  if (op != LUNA_OPUNM && op != LUNA_OPBNOT)
    api_checknelems(L, 2);  /* all other operations expect two operands */
  else {  /* for unary operations, add fake 2nd operand */
    api_checknelems(L, 1);
    setobjs2s(L, L->top.p, L->top.p - 1);
    api_incr_top(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->top.p - 2), s2v(L->top.p - 1), L->top.p - 2);
  L->top.p--;  /* remove second operand */
  luna_unlock(L);
}


LUNA_API int luna_compare (luna_State *L, int index1, int index2, int op) {
  const TValue *o1;
  const TValue *o2;
  int i = 0;
  luna_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  if (isvalid(L, o1) && isvalid(L, o2)) {
    switch (op) {
      case LUNA_OPEQ: i = luaV_equalobj(L, o1, o2); break;
      case LUNA_OPLT: i = luaV_lessthan(L, o1, o2); break;
      case LUNA_OPLE: i = luaV_lessequal(L, o1, o2); break;
      default: api_check(L, 0, "invalid option");
    }
  }
  luna_unlock(L);
  return i;
}


LUNA_API size_t luna_stringtonumber (luna_State *L, const char *s) {
  size_t sz = luaO_str2num(s, s2v(L->top.p));
  if (sz != 0)
    api_incr_top(L);
  return sz;
}


LUNA_API luna_Number luna_tonumberx (luna_State *L, int idx, int *pisnum) {
  luna_Number n = 0;
  const TValue *o = index2value(L, idx);
  int isnum = tonumber(o, &n);
  if (pisnum)
    *pisnum = isnum;
  return n;
}


LUNA_API luna_Integer luna_tointegerx (luna_State *L, int idx, int *pisnum) {
  luna_Integer res = 0;
  const TValue *o = index2value(L, idx);
  int isnum = tointeger(o, &res);
  if (pisnum)
    *pisnum = isnum;
  return res;
}


LUNA_API int luna_toboolean (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return !l_isfalse(o);
}


LUNA_API const char *luna_tolstring (luna_State *L, int idx, size_t *len) {
  TValue *o;
  luna_lock(L);
  o = index2value(L, idx);
  if (!ttisstring(o)) {
    if (!cvt2str(o)) {  /* not convertible? */
      if (len != NULL) *len = 0;
      luna_unlock(L);
      return NULL;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  if (len != NULL)
    *len = tsslen(tsvalue(o));
  luna_unlock(L);
  return getstr(tsvalue(o));
}


LUNA_API luna_Unsigned luna_rawlen (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  switch (ttypetag(o)) {
    case LUNA_VSHRSTR: return tsvalue(o)->shrlen;
    case LUNA_VLNGSTR: return tsvalue(o)->u.lnglen;
    case LUNA_VUSERDATA: return uvalue(o)->len;
    case LUNA_VTABLE: return luaH_getn(hvalue(o));
    default: return 0;
  }
}


LUNA_API luna_CFunction luna_tocfunction (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  if (ttislcf(o)) return fvalue(o);
  else if (ttisCclosure(o))
    return clCvalue(o)->f;
  else return NULL;  /* not a C function */
}


l_sinline void *touserdata (const TValue *o) {
  switch (ttype(o)) {
    case LUNA_TUSERDATA: return getudatamem(uvalue(o));
    case LUNA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}


LUNA_API void *luna_touserdata (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return touserdata(o);
}


LUNA_API luna_State *luna_tothread (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ANSI C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUNA_API const void *luna_topointer (luna_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  switch (ttypetag(o)) {
    case LUNA_VLCF: return cast_voidp(cast_sizet(fvalue(o)));
    case LUNA_VUSERDATA: case LUNA_VLIGHTUSERDATA:
      return touserdata(o);
    default: {
      if (iscollectable(o))
        return gcvalue(o);
      else
        return NULL;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUNA_API void luna_pushnil (luna_State *L) {
  luna_lock(L);
  setnilvalue(s2v(L->top.p));
  api_incr_top(L);
  luna_unlock(L);
}


LUNA_API void luna_pushnumber (luna_State *L, luna_Number n) {
  luna_lock(L);
  setfltvalue(s2v(L->top.p), n);
  api_incr_top(L);
  luna_unlock(L);
}


LUNA_API void luna_pushinteger (luna_State *L, luna_Integer n) {
  luna_lock(L);
  setivalue(s2v(L->top.p), n);
  api_incr_top(L);
  luna_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUNA_API const char *luna_pushlstring (luna_State *L, const char *s, size_t len) {
  TString *ts;
  luna_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->top.p, ts);
  api_incr_top(L);
  luaC_checkGC(L);
  luna_unlock(L);
  return getstr(ts);
}


LUNA_API const char *luna_pushstring (luna_State *L, const char *s) {
  luna_lock(L);
  if (s == NULL)
    setnilvalue(s2v(L->top.p));
  else {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->top.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_top(L);
  luaC_checkGC(L);
  luna_unlock(L);
  return s;
}


LUNA_API const char *luna_pushvfstring (luna_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  luna_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  luna_unlock(L);
  return ret;
}


LUNA_API const char *luna_pushfstring (luna_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  luna_lock(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  luaC_checkGC(L);
  luna_unlock(L);
  return ret;
}


LUNA_API void luna_pushcclosure (luna_State *L, luna_CFunction fn, int n) {
  luna_lock(L);
  if (n == 0) {
    setfvalue(s2v(L->top.p), fn);
    api_incr_top(L);
  }
  else {
    CClosure *cl;
    api_checknelems(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    L->top.p -= n;
    while (n--) {
      setobj2n(L, &cl->upvalue[n], s2v(L->top.p + n));
      /* does not need barrier because closure is white */
      luna_assert(iswhite(cl));
    }
    setclCvalue(L, s2v(L->top.p), cl);
    api_incr_top(L);
    luaC_checkGC(L);
  }
  luna_unlock(L);
}


LUNA_API void luna_pushboolean (luna_State *L, int b) {
  luna_lock(L);
  if (b)
    setbtvalue(s2v(L->top.p));
  else
    setbfvalue(s2v(L->top.p));
  api_incr_top(L);
  luna_unlock(L);
}


LUNA_API void luna_pushlightuserdata (luna_State *L, void *p) {
  luna_lock(L);
  setpvalue(s2v(L->top.p), p);
  api_incr_top(L);
  luna_unlock(L);
}


LUNA_API int luna_pushthread (luna_State *L) {
  luna_lock(L);
  setthvalue(L, s2v(L->top.p), L);
  api_incr_top(L);
  luna_unlock(L);
  return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/


l_sinline int auxgetstr (luna_State *L, const TValue *t, const char *k) {
  const TValue *slot;
  TString *str = luaS_new(L, k);
  if (luaV_fastget(L, t, str, slot, luaH_getstr)) {
    setobj2s(L, L->top.p, slot);
    api_incr_top(L);
  }
  else {
    setsvalue2s(L, L->top.p, str);
    api_incr_top(L);
    luaV_finishget(L, t, s2v(L->top.p - 1), L->top.p - 1, slot);
  }
  luna_unlock(L);
  return ttype(s2v(L->top.p - 1));
}


/*
** Get the global table in the registry. Since all predefined
** indices in the registry were inserted right when the registry
** was created and never removed, they must always be in the array
** part of the registry.
*/
#define getGtable(L)  \
	(&hvalue(&G(L)->l_registry)->array[LUNA_RIDX_GLOBALS - 1])


LUNA_API int luna_getglobal (luna_State *L, const char *name) {
  const TValue *G;
  luna_lock(L);
  G = getGtable(L);
  return auxgetstr(L, G, name);
}


LUNA_API int luna_gettable (luna_State *L, int idx) {
  const TValue *slot;
  TValue *t;
  luna_lock(L);
  t = index2value(L, idx);
  if (luaV_fastget(L, t, s2v(L->top.p - 1), slot, luaH_get)) {
    setobj2s(L, L->top.p - 1, slot);
  }
  else
    luaV_finishget(L, t, s2v(L->top.p - 1), L->top.p - 1, slot);
  luna_unlock(L);
  return ttype(s2v(L->top.p - 1));
}


LUNA_API int luna_getfield (luna_State *L, int idx, const char *k) {
  luna_lock(L);
  return auxgetstr(L, index2value(L, idx), k);
}


LUNA_API int luna_geti (luna_State *L, int idx, luna_Integer n) {
  TValue *t;
  const TValue *slot;
  luna_lock(L);
  t = index2value(L, idx);
  if (luaV_fastgeti(L, t, n, slot)) {
    setobj2s(L, L->top.p, slot);
  }
  else {
    TValue aux;
    setivalue(&aux, n);
    luaV_finishget(L, t, &aux, L->top.p, slot);
  }
  api_incr_top(L);
  luna_unlock(L);
  return ttype(s2v(L->top.p - 1));
}


l_sinline int finishrawget (luna_State *L, const TValue *val) {
  if (isempty(val))  /* avoid copying empty items to the stack */
    setnilvalue(s2v(L->top.p));
  else
    setobj2s(L, L->top.p, val);
  api_incr_top(L);
  luna_unlock(L);
  return ttype(s2v(L->top.p - 1));
}


static Table *gettable (luna_State *L, int idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  return hvalue(t);
}


LUNA_API int luna_rawget (luna_State *L, int idx) {
  Table *t;
  const TValue *val;
  luna_lock(L);
  api_checknelems(L, 1);
  t = gettable(L, idx);
  val = luaH_get(t, s2v(L->top.p - 1));
  L->top.p--;  /* remove key */
  return finishrawget(L, val);
}


LUNA_API int luna_rawgeti (luna_State *L, int idx, luna_Integer n) {
  Table *t;
  luna_lock(L);
  t = gettable(L, idx);
  return finishrawget(L, luaH_getint(t, n));
}


LUNA_API int luna_rawgetp (luna_State *L, int idx, const void *p) {
  Table *t;
  TValue k;
  luna_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  return finishrawget(L, luaH_get(t, &k));
}


LUNA_API void luna_createtable (luna_State *L, int narray, int nrec) {
  Table *t;
  luna_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->top.p, t);
  api_incr_top(L);
  if (narray > 0 || nrec > 0)
    luaH_resize(L, t, narray, nrec);
  luaC_checkGC(L);
  luna_unlock(L);
}


LUNA_API int luna_getmetatable (luna_State *L, int objindex) {
  const TValue *obj;
  Table *mt;
  int res = 0;
  luna_lock(L);
  obj = index2value(L, objindex);
  switch (ttype(obj)) {
    case LUNA_TTABLE:
      mt = hvalue(obj)->metatable;
      break;
    case LUNA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(obj)];
      break;
  }
  if (mt != NULL) {
    sethvalue2s(L, L->top.p, mt);
    api_incr_top(L);
    res = 1;
  }
  luna_unlock(L);
  return res;
}


LUNA_API int luna_getiuservalue (luna_State *L, int idx, int n) {
  TValue *o;
  int t;
  luna_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  if (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue(s2v(L->top.p));
    t = LUNA_TNONE;
  }
  else {
    setobj2s(L, L->top.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->top.p));
  }
  api_incr_top(L);
  luna_unlock(L);
  return t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
static void auxsetstr (luna_State *L, const TValue *t, const char *k) {
  const TValue *slot;
  TString *str = luaS_new(L, k);
  api_checknelems(L, 1);
  if (luaV_fastget(L, t, str, slot, luaH_getstr)) {
    luaV_finishfastset(L, t, slot, s2v(L->top.p - 1));
    L->top.p--;  /* pop value */
  }
  else {
    setsvalue2s(L, L->top.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_top(L);
    luaV_finishset(L, t, s2v(L->top.p - 1), s2v(L->top.p - 2), slot);
    L->top.p -= 2;  /* pop value and key */
  }
  luna_unlock(L);  /* lock done by caller */
}


LUNA_API void luna_setglobal (luna_State *L, const char *name) {
  const TValue *G;
  luna_lock(L);  /* unlock done in 'auxsetstr' */
  G = getGtable(L);
  auxsetstr(L, G, name);
}


LUNA_API void luna_settable (luna_State *L, int idx) {
  TValue *t;
  const TValue *slot;
  luna_lock(L);
  api_checknelems(L, 2);
  t = index2value(L, idx);
  if (luaV_fastget(L, t, s2v(L->top.p - 2), slot, luaH_get)) {
    luaV_finishfastset(L, t, slot, s2v(L->top.p - 1));
  }
  else
    luaV_finishset(L, t, s2v(L->top.p - 2), s2v(L->top.p - 1), slot);
  L->top.p -= 2;  /* pop index and value */
  luna_unlock(L);
}


LUNA_API void luna_setfield (luna_State *L, int idx, const char *k) {
  luna_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUNA_API void luna_seti (luna_State *L, int idx, luna_Integer n) {
  TValue *t;
  const TValue *slot;
  luna_lock(L);
  api_checknelems(L, 1);
  t = index2value(L, idx);
  if (luaV_fastgeti(L, t, n, slot)) {
    luaV_finishfastset(L, t, slot, s2v(L->top.p - 1));
  }
  else {
    TValue aux;
    setivalue(&aux, n);
    luaV_finishset(L, t, &aux, s2v(L->top.p - 1), slot);
  }
  L->top.p--;  /* pop value */
  luna_unlock(L);
}


static void aux_rawset (luna_State *L, int idx, TValue *key, int n) {
  Table *t;
  luna_lock(L);
  api_checknelems(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, key, s2v(L->top.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->top.p - 1));
  L->top.p -= n;
  luna_unlock(L);
}


LUNA_API void luna_rawset (luna_State *L, int idx) {
  aux_rawset(L, idx, s2v(L->top.p - 2), 2);
}


LUNA_API void luna_rawsetp (luna_State *L, int idx, const void *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUNA_API void luna_rawseti (luna_State *L, int idx, luna_Integer n) {
  Table *t;
  luna_lock(L);
  api_checknelems(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->top.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->top.p - 1));
  L->top.p--;
  luna_unlock(L);
}


LUNA_API int luna_setmetatable (luna_State *L, int objindex) {
  TValue *obj;
  Table *mt;
  luna_lock(L);
  api_checknelems(L, 1);
  obj = index2value(L, objindex);
  if (ttisnil(s2v(L->top.p - 1)))
    mt = NULL;
  else {
    api_check(L, ttistable(s2v(L->top.p - 1)), "table expected");
    mt = hvalue(s2v(L->top.p - 1));
  }
  switch (ttype(obj)) {
    case LUNA_TTABLE: {
      hvalue(obj)->metatable = mt;
      if (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      break;
    }
    case LUNA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      if (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      break;
    }
    default: {
      G(L)->mt[ttype(obj)] = mt;
      break;
    }
  }
  L->top.p--;
  luna_unlock(L);
  return 1;
}


LUNA_API int luna_setiuservalue (luna_State *L, int idx, int n) {
  TValue *o;
  int res;
  luna_lock(L);
  api_checknelems(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  if (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  else {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->top.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->top.p - 1));
    res = 1;
  }
  L->top.p--;
  luna_unlock(L);
  return res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUNA_MULTRET \
               || (L->ci->top.p - L->top.p >= (nr) - (na)), \
	"results from function overflow current stack size")


LUNA_API void luna_callk (luna_State *L, int nargs, int nresults,
                        luna_KContext ctx, luna_KFunction k) {
  StkId func;
  luna_lock(L);
  api_check(L, k == NULL || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checknelems(L, nargs+1);
  api_check(L, L->status == LUNA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->top.p - (nargs+1);
  if (k != NULL && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  else  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  luna_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to 'f_call' */
  StkId func;
  int nresults;
};


static void f_call (luna_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUNA_API int luna_pcallk (luna_State *L, int nargs, int nresults, int errfunc,
                        luna_KContext ctx, luna_KFunction k) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  luna_lock(L);
  api_check(L, k == NULL || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checknelems(L, nargs+1);
  api_check(L, L->status == LUNA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->top.p - (nargs+1);  /* function to be called */
  if (k == NULL || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  else {  /* prepare continuation (call is already protected by 'resume') */
    CallInfo *ci = L->ci;
    ci->u.c.k = k;  /* save continuation */
    ci->u.c.ctx = ctx;  /* save context */
    /* save information for error recovery */
    ci->u2.funcidx = cast_int(savestack(L, c.func));
    ci->u.c.old_errfunc = L->errfunc;
    L->errfunc = func;
    setoah(ci->callstatus, L->allowhook);  /* save value of 'allowhook' */
    ci->callstatus |= CIST_YPCALL;  /* function can do error recovery */
    luaD_call(L, c.func, nresults);  /* do the call */
    ci->callstatus &= ~CIST_YPCALL;
    L->errfunc = ci->u.c.old_errfunc;
    status = LUNA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  luna_unlock(L);
  return status;
}


LUNA_API int luna_load (luna_State *L, luna_Reader reader, void *data,
                      const char *chunkname, const char *mode) {
  ZIO z;
  int status;
  luna_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  if (status == LUNA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->top.p - 1));  /* get new function */
    if (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      const TValue *gt = getGtable(L);
      /* set global table as 1st upvalue of 'f' (may be LUNA_ENV) */
      setobj(L, f->upvals[0]->v.p, gt);
      luaC_barrier(L, f->upvals[0], gt);
    }
  }
  luna_unlock(L);
  return status;
}


LUNA_API int luna_dump (luna_State *L, luna_Writer writer, void *data, int strip) {
  int status;
  TValue *o;
  luna_lock(L);
  api_checknelems(L, 1);
  o = s2v(L->top.p - 1);
  if (isLfunction(o))
    status = luaU_dump(L, getproto(o), writer, data, strip);
  else
    status = 1;
  luna_unlock(L);
  return status;
}


LUNA_API int luna_status (luna_State *L) {
  return L->status;
}


/*
** Garbage-collection function
*/
LUNA_API int luna_gc (luna_State *L, int what, ...) {
  va_list argp;
  int res = 0;
  global_State *g = G(L);
  if (g->gcstp & GCSTPGC)  /* internal stop? */
    return -1;  /* all options are invalid when stopped */
  luna_lock(L);
  va_start(argp, what);
  switch (what) {
    case LUNA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      break;
    }
    case LUNA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (GCSTPGC must be already zero here) */
      break;
    }
    case LUNA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      break;
    }
    case LUNA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      break;
    }
    case LUNA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      break;
    }
    case LUNA_GCSTEP: {
      int data = va_arg(argp, int);
      l_mem debt = 1;  /* =1 to signal that it did an actual step */
      lu_byte oldstp = g->gcstp;
      g->gcstp = 0;  /* allow GC to run (GCSTPGC must be zero here) */
      if (data == 0) {
        luaE_setdebt(g, 0);  /* do a basic step */
        luaC_step(L);
      }
      else {  /* add 'data' to total debt */
        debt = cast(l_mem, data) * 1024 + g->GCdebt;
        luaE_setdebt(g, debt);
        luaC_checkGC(L);
      }
      g->gcstp = oldstp;  /* restore previous state */
      if (debt > 0 && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      break;
    }
    case LUNA_GCSETPAUSE: {
      int data = va_arg(argp, int);
      res = getgcparam(g->gcpause);
      setgcparam(g->gcpause, data);
      break;
    }
    case LUNA_GCSETSTEPMUL: {
      int data = va_arg(argp, int);
      res = getgcparam(g->gcstepmul);
      setgcparam(g->gcstepmul, data);
      break;
    }
    case LUNA_GCISRUNNING: {
      res = gcrunning(g);
      break;
    }
    case LUNA_GCGEN: {
      int minormul = va_arg(argp, int);
      int majormul = va_arg(argp, int);
      res = isdecGCmodegen(g) ? LUNA_GCGEN : LUNA_GCINC;
      if (minormul != 0)
        g->genminormul = minormul;
      if (majormul != 0)
        setgcparam(g->genmajormul, majormul);
      luaC_changemode(L, KGC_GEN);
      break;
    }
    case LUNA_GCINC: {
      int pause = va_arg(argp, int);
      int stepmul = va_arg(argp, int);
      int stepsize = va_arg(argp, int);
      res = isdecGCmodegen(g) ? LUNA_GCGEN : LUNA_GCINC;
      if (pause != 0)
        setgcparam(g->gcpause, pause);
      if (stepmul != 0)
        setgcparam(g->gcstepmul, stepmul);
      if (stepsize != 0)
        g->gcstepsize = stepsize;
      luaC_changemode(L, KGC_INC);
      break;
    }
    default: res = -1;  /* invalid option */
  }
  va_end(argp);
  luna_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/


LUNA_API int luna_error (luna_State *L) {
  TValue *errobj;
  luna_lock(L);
  errobj = s2v(L->top.p - 1);
  api_checknelems(L, 1);
  /* error object is the memory error message? */
  if (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  else
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  return 0;  /* to avoid warnings */
}


LUNA_API int luna_next (luna_State *L, int idx) {
  Table *t;
  int more;
  luna_lock(L);
  api_checknelems(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->top.p - 1);
  if (more) {
    api_incr_top(L);
  }
  else  /* no more elements */
    L->top.p -= 1;  /* remove key */
  luna_unlock(L);
  return more;
}


LUNA_API void luna_toclose (luna_State *L, int idx) {
  int nresults;
  StkId o;
  luna_lock(L);
  o = index2stack(L, idx);
  nresults = L->ci->nresults;
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  if (!hastocloseCfunc(nresults))  /* function not marked yet? */
    L->ci->nresults = codeNresults(nresults);  /* mark it */
  luna_assert(hastocloseCfunc(L->ci->nresults));
  luna_unlock(L);
}


LUNA_API void luna_concat (luna_State *L, int n) {
  luna_lock(L);
  api_checknelems(L, n);
  if (n > 0)
    luaV_concat(L, n);
  else {  /* nothing to concatenate */
    setsvalue2s(L, L->top.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_top(L);
  }
  luaC_checkGC(L);
  luna_unlock(L);
}


LUNA_API void luna_len (luna_State *L, int idx) {
  TValue *t;
  luna_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->top.p, t);
  api_incr_top(L);
  luna_unlock(L);
}


LUNA_API luna_Alloc luna_getallocf (luna_State *L, void **ud) {
  luna_Alloc f;
  luna_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  luna_unlock(L);
  return f;
}


LUNA_API void luna_setallocf (luna_State *L, luna_Alloc f, void *ud) {
  luna_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  luna_unlock(L);
}


void luna_setwarnf (luna_State *L, luna_WarnFunction f, void *ud) {
  luna_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  luna_unlock(L);
}


void luna_warning (luna_State *L, const char *msg, int tocont) {
  luna_lock(L);
  luaE_warning(L, msg, tocont);
  luna_unlock(L);
}



LUNA_API void *luna_newuserdatauv (luna_State *L, size_t size, int nuvalue) {
  Udata *u;
  luna_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < USHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, nuvalue);
  setuvalue(L, s2v(L->top.p), u);
  api_incr_top(L);
  luaC_checkGC(L);
  luna_unlock(L);
  return getudatamem(u);
}



static const char *aux_upvalue (TValue *fi, int n, TValue **val,
                                GCObject **owner) {
  switch (ttypetag(fi)) {
    case LUNA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      if (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        return NULL;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      if (owner) *owner = obj2gco(f);
      return "";
    }
    case LUNA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      if (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        return NULL;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      if (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      return (name == NULL) ? "(no name)" : getstr(name);
    }
    default: return NULL;  /* not a closure */
  }
}


LUNA_API const char *luna_getupvalue (luna_State *L, int funcindex, int n) {
  const char *name;
  TValue *val = NULL;  /* to avoid warnings */
  luna_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, NULL);
  if (name) {
    setobj2s(L, L->top.p, val);
    api_incr_top(L);
  }
  luna_unlock(L);
  return name;
}


LUNA_API const char *luna_setupvalue (luna_State *L, int funcindex, int n) {
  const char *name;
  TValue *val = NULL;  /* to avoid warnings */
  GCObject *owner = NULL;  /* to avoid warnings */
  TValue *fi;
  luna_lock(L);
  fi = index2value(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  if (name) {
    L->top.p--;
    setobj(L, val, s2v(L->top.p));
    luaC_barrier(L, owner, val);
  }
  luna_unlock(L);
  return name;
}


static UpVal **getupvalref (luna_State *L, int fidx, int n, LClosure **pf) {
  static const UpVal *const nullup = NULL;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  if (pf) *pf = f;
  if (1 <= n && n <= f->p->sizeupvalues)
    return &f->upvals[n - 1];  /* get its upvalue pointer */
  else
    return (UpVal**)&nullup;
}


LUNA_API void *luna_upvalueid (luna_State *L, int fidx, int n) {
  TValue *fi = index2value(L, fidx);
  switch (ttypetag(fi)) {
    case LUNA_VLCL: {  /* lua closure */
      return *getupvalref(L, fidx, n, NULL);
    }
    case LUNA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      if (1 <= n && n <= f->nupvalues)
        return &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    case LUNA_VLCF:
      return NULL;  /* light C functions have no upvalues */
    default: {
      api_check(L, 0, "function expected");
      return NULL;
    }
  }
}


LUNA_API void luna_upvaluejoin (luna_State *L, int fidx1, int n1,
                                            int fidx2, int n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, NULL);
  api_check(L, *up1 != NULL && *up2 != NULL, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


