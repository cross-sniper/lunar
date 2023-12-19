/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef luna_h
#define luna_h

#include <stdarg.h>
#include <stddef.h>

#define LUNA_VERSION  "Luna v 0.7"
#define LUNA_RELEASE  "Luna v 0.7"

#define LUNA_COPYRIGHT	LUNA_RELEASE "\nLuna, a modified version of lua(Lua 5.4.6), by cross hedgehog"
#define LUNA_AUTHORS	"cross hedgehog"


#define LUNA_VERSION_MAJOR_N	5
#define LUNA_VERSION_MINOR_N	4
#define LUNA_VERSION_RELEASE_N	6

#define LUNA_VERSION_NUM  (LUNA_VERSION_MAJOR_N * 100 + LUNA_VERSION_MINOR_N)
#define LUNA_VERSION_RELEASE_NUM  (LUNA_VERSION_NUM * 100 + LUNA_VERSION_RELEASE_N)


#include "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#define LUNA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'luna_pcall' and 'luna_call' */
#define LUNA_MULTRET	(-1)


/*
** Pseudo-indices
** (-LUAI_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
*/
#define LUNA_REGISTRYINDEX	(-LUAI_MAXSTACK - 1000)
#define luna_upvalueindex(i)	(LUNA_REGISTRYINDEX - (i))


/* thread status */
#define LUNA_OK		0
#define LUNA_YIELD	1
#define LUNA_ERRRUN	2
#define LUNA_ERRSYNTAX	3
#define LUNA_ERRMEM	4
#define LUNA_ERRERR	5


typedef struct luna_State luna_State;


/*
** basic types
*/
#define LUNA_TNONE		(-1)

#define LUNA_TNIL		0
#define LUNA_TBOOLEAN		1
#define LUNA_TLIGHTUSERDATA	2
#define LUNA_TNUMBER		3
#define LUNA_TSTRING		4
#define LUNA_TTABLE		5
#define LUNA_TFUNCTION		6
#define LUNA_TUSERDATA		7
#define LUNA_TTHREAD		8

#define LUNA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#define LUNA_MINSTACK	20


/* predefined values in the registry */
#define LUNA_RIDX_MAINTHREAD	1
#define LUNA_RIDX_GLOBALS	2
#define LUNA_RIDX_LAST		LUNA_RIDX_GLOBALS


/* type of numbers in Lua */
typedef LUNA_NUMBER luna_Number;


/* type for integer functions */
typedef LUNA_INTEGER luna_Integer;

/* unsigned integer type */
typedef LUNA_UNSIGNED luna_Unsigned;

/* type for continuation-function contexts */
typedef LUNA_KCONTEXT luna_KContext;


/*
** Type for C functions registered with Lua
*/
typedef int (*luna_CFunction) (luna_State *L);

/*
** Type for continuation functions
*/
typedef int (*luna_KFunction) (luna_State *L, int status, luna_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*luna_Reader) (luna_State *L, void *ud, size_t *sz);

typedef int (*luna_Writer) (luna_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
typedef void * (*luna_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
typedef void (*luna_WarnFunction) (void *ud, const char *msg, int tocont);


/*
** Type used by the debug API to collect debug information
*/
typedef struct luna_Debug luna_Debug;


/*
** Functions to be called by the debugger in specific events
*/
typedef void (*luna_Hook) (luna_State *L, luna_Debug *ar);


/*
** generic extra include file
*/
#if defined(LUNA_USER_H)
#include LUNA_USER_H
#endif


/*
** RCS ident string
*/
extern const char luna_ident[];


/*
** state manipulation
*/
LUNA_API luna_State *(luna_newstate) (luna_Alloc f, void *ud);
LUNA_API void       (luna_close) (luna_State *L);
LUNA_API luna_State *(luna_newthread) (luna_State *L);
LUNA_API int        (luna_closethread) (luna_State *L, luna_State *from);
LUNA_API int        (luna_resetthread) (luna_State *L);  /* Deprecated! */

LUNA_API luna_CFunction (luna_atpanic) (luna_State *L, luna_CFunction panicf);


LUNA_API luna_Number (luna_version) (luna_State *L);


/*
** basic stack manipulation
*/
LUNA_API int   (luna_absindex) (luna_State *L, int idx);
LUNA_API int   (luna_gettop) (luna_State *L);
LUNA_API void  (luna_settop) (luna_State *L, int idx);
LUNA_API void  (luna_pushvalue) (luna_State *L, int idx);
LUNA_API void  (luna_rotate) (luna_State *L, int idx, int n);
LUNA_API void  (luna_copy) (luna_State *L, int fromidx, int toidx);
LUNA_API int   (luna_checkstack) (luna_State *L, int n);

LUNA_API void  (luna_xmove) (luna_State *from, luna_State *to, int n);


/*
** access functions (stack -> C)
*/

LUNA_API int             (luna_isnumber) (luna_State *L, int idx);
LUNA_API int             (luna_isstring) (luna_State *L, int idx);
LUNA_API int             (luna_iscfunction) (luna_State *L, int idx);
LUNA_API int             (luna_isinteger) (luna_State *L, int idx);
LUNA_API int             (luna_isuserdata) (luna_State *L, int idx);
LUNA_API int             (luna_type) (luna_State *L, int idx);
LUNA_API const char     *(luna_typename) (luna_State *L, int tp);

LUNA_API luna_Number      (luna_tonumberx) (luna_State *L, int idx, int *isnum);
LUNA_API luna_Integer     (luna_tointegerx) (luna_State *L, int idx, int *isnum);
LUNA_API int             (luna_toboolean) (luna_State *L, int idx);
LUNA_API const char     *(luna_tolstring) (luna_State *L, int idx, size_t *len);
LUNA_API luna_Unsigned    (luna_rawlen) (luna_State *L, int idx);
LUNA_API luna_CFunction   (luna_tocfunction) (luna_State *L, int idx);
LUNA_API void	       *(luna_touserdata) (luna_State *L, int idx);
LUNA_API luna_State      *(luna_tothread) (luna_State *L, int idx);
LUNA_API const void     *(luna_topointer) (luna_State *L, int idx);


/*
** Comparison and arithmetic functions
*/

#define LUNA_OPADD	0	/* ORDER TM, ORDER OP */
#define LUNA_OPSUB	1
#define LUNA_OPMUL	2
#define LUNA_OPMOD	3
#define LUNA_OPPOW	4
#define LUNA_OPDIV	5
#define LUNA_OPIDIV	6
#define LUNA_OPBAND	7
#define LUNA_OPBOR	8
#define LUNA_OPBXOR	9
#define LUNA_OPSHL	10
#define LUNA_OPSHR	11
#define LUNA_OPUNM	12
#define LUNA_OPBNOT	13

LUNA_API void  (luna_arith) (luna_State *L, int op);

#define LUNA_OPEQ	0
#define LUNA_OPLT	1
#define LUNA_OPLE	2

LUNA_API int   (luna_rawequal) (luna_State *L, int idx1, int idx2);
LUNA_API int   (luna_compare) (luna_State *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack)
*/
LUNA_API void        (luna_pushnil) (luna_State *L);
LUNA_API void        (luna_pushnumber) (luna_State *L, luna_Number n);
LUNA_API void        (luna_pushinteger) (luna_State *L, luna_Integer n);
LUNA_API const char *(luna_pushlstring) (luna_State *L, const char *s, size_t len);
LUNA_API const char *(luna_pushstring) (luna_State *L, const char *s);
LUNA_API const char *(luna_pushvfstring) (luna_State *L, const char *fmt,
                                                      va_list argp);
LUNA_API const char *(luna_pushfstring) (luna_State *L, const char *fmt, ...);
LUNA_API void  (luna_pushcclosure) (luna_State *L, luna_CFunction fn, int n);
LUNA_API void  (luna_pushboolean) (luna_State *L, int b);
LUNA_API void  (luna_pushlightuserdata) (luna_State *L, void *p);
LUNA_API int   (luna_pushthread) (luna_State *L);


/*
** get functions (Lua -> stack)
*/
LUNA_API int (luna_getglobal) (luna_State *L, const char *name);
LUNA_API int (luna_gettable) (luna_State *L, int idx);
LUNA_API int (luna_getfield) (luna_State *L, int idx, const char *k);
LUNA_API int (luna_geti) (luna_State *L, int idx, luna_Integer n);
LUNA_API int (luna_rawget) (luna_State *L, int idx);
LUNA_API int (luna_rawgeti) (luna_State *L, int idx, luna_Integer n);
LUNA_API int (luna_rawgetp) (luna_State *L, int idx, const void *p);

LUNA_API void  (luna_createtable) (luna_State *L, int narr, int nrec);
LUNA_API void *(luna_newuserdatauv) (luna_State *L, size_t sz, int nuvalue);
LUNA_API int   (luna_getmetatable) (luna_State *L, int objindex);
LUNA_API int  (luna_getiuservalue) (luna_State *L, int idx, int n);


/*
** set functions (stack -> Lua)
*/
LUNA_API void  (luna_setglobal) (luna_State *L, const char *name);
LUNA_API void  (luna_settable) (luna_State *L, int idx);
LUNA_API void  (luna_setfield) (luna_State *L, int idx, const char *k);
LUNA_API void  (luna_seti) (luna_State *L, int idx, luna_Integer n);
LUNA_API void  (luna_rawset) (luna_State *L, int idx);
LUNA_API void  (luna_rawseti) (luna_State *L, int idx, luna_Integer n);
LUNA_API void  (luna_rawsetp) (luna_State *L, int idx, const void *p);
LUNA_API int   (luna_setmetatable) (luna_State *L, int objindex);
LUNA_API int   (luna_setiuservalue) (luna_State *L, int idx, int n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUNA_API void  (luna_callk) (luna_State *L, int nargs, int nresults,
                           luna_KContext ctx, luna_KFunction k);
#define luna_call(L,n,r)		luna_callk(L, (n), (r), 0, NULL)

LUNA_API int   (luna_pcallk) (luna_State *L, int nargs, int nresults, int errfunc,
                            luna_KContext ctx, luna_KFunction k);
#define luna_pcall(L,n,r,f)	luna_pcallk(L, (n), (r), (f), 0, NULL)

LUNA_API int   (luna_load) (luna_State *L, luna_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

LUNA_API int (luna_dump) (luna_State *L, luna_Writer writer, void *data, int strip);


/*
** coroutine functions
*/
LUNA_API int  (luna_yieldk)     (luna_State *L, int nresults, luna_KContext ctx,
                               luna_KFunction k);
LUNA_API int  (luna_resume)     (luna_State *L, luna_State *from, int narg,
                               int *nres);
LUNA_API int  (luna_status)     (luna_State *L);
LUNA_API int (luna_isyieldable) (luna_State *L);

#define luna_yield(L,n)		luna_yieldk(L, (n), 0, NULL)


/*
** Warning-related functions
*/
LUNA_API void (luna_setwarnf) (luna_State *L, luna_WarnFunction f, void *ud);
LUNA_API void (luna_warning)  (luna_State *L, const char *msg, int tocont);


/*
** garbage-collection function and options
*/

#define LUNA_GCSTOP		0
#define LUNA_GCRESTART		1
#define LUNA_GCCOLLECT		2
#define LUNA_GCCOUNT		3
#define LUNA_GCCOUNTB		4
#define LUNA_GCSTEP		5
#define LUNA_GCSETPAUSE		6
#define LUNA_GCSETSTEPMUL	7
#define LUNA_GCISRUNNING		9
#define LUNA_GCGEN		10
#define LUNA_GCINC		11

LUNA_API int (luna_gc) (luna_State *L, int what, ...);


/*
** miscellaneous functions
*/

LUNA_API int   (luna_error) (luna_State *L);

LUNA_API int   (luna_next) (luna_State *L, int idx);

LUNA_API void  (luna_concat) (luna_State *L, int n);
LUNA_API void  (luna_len)    (luna_State *L, int idx);

LUNA_API size_t   (luna_stringtonumber) (luna_State *L, const char *s);

LUNA_API luna_Alloc (luna_getallocf) (luna_State *L, void **ud);
LUNA_API void      (luna_setallocf) (luna_State *L, luna_Alloc f, void *ud);

LUNA_API void (luna_toclose) (luna_State *L, int idx);
LUNA_API void (luna_closeslot) (luna_State *L, int idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define luna_getextraspace(L)	((void *)((char *)(L) - LUNA_EXTRASPACE))

#define luna_tonumber(L,i)	luna_tonumberx(L,(i),NULL)
#define luna_tointeger(L,i)	luna_tointegerx(L,(i),NULL)

#define luna_pop(L,n)		luna_settop(L, -(n)-1)

#define luna_newtable(L)		luna_createtable(L, 0, 0)

#define luna_register(L,n,f) (luna_pushcfunction(L, (f)), luna_setglobal(L, (n)))

#define luna_pushcfunction(L,f)	luna_pushcclosure(L, (f), 0)

#define luna_isfunction(L,n)	(luna_type(L, (n)) == LUNA_TFUNCTION)
#define luna_istable(L,n)	(luna_type(L, (n)) == LUNA_TTABLE)
#define luna_islightuserdata(L,n)	(luna_type(L, (n)) == LUNA_TLIGHTUSERDATA)
#define luna_isnil(L,n)		(luna_type(L, (n)) == LUNA_TNIL)
#define luna_isboolean(L,n)	(luna_type(L, (n)) == LUNA_TBOOLEAN)
#define luna_isthread(L,n)	(luna_type(L, (n)) == LUNA_TTHREAD)
#define luna_isnone(L,n)		(luna_type(L, (n)) == LUNA_TNONE)
#define luna_isnoneornil(L, n)	(luna_type(L, (n)) <= 0)

#define luna_pushliteral(L, s)	luna_pushstring(L, "" s)

#define luna_pushglobaltable(L)  \
	((void)luna_rawgeti(L, LUNA_REGISTRYINDEX, LUNA_RIDX_GLOBALS))

#define luna_tostring(L,i)	luna_tolstring(L, (i), NULL)


#define luna_insert(L,idx)	luna_rotate(L, (idx), 1)

#define luna_remove(L,idx)	(luna_rotate(L, (idx), -1), luna_pop(L, 1))

#define luna_replace(L,idx)	(luna_copy(L, -1, (idx)), luna_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/
#if defined(LUNA_COMPAT_APIINTCASTS)

#define luna_pushunsigned(L,n)	luna_pushinteger(L, (luna_Integer)(n))
#define luna_tounsignedx(L,i,is)	((luna_Unsigned)luna_tointegerx(L,i,is))
#define luna_tounsigned(L,i)	luna_tounsignedx(L,(i),NULL)

#endif

#define luna_newuserdata(L,s)	luna_newuserdatauv(L,s,1)
#define luna_getuservalue(L,idx)	luna_getiuservalue(L,idx,1)
#define luna_setuservalue(L,idx)	luna_setiuservalue(L,idx,1)

#define LUNA_NUMTAGS		LUNA_NUMTYPES

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUNA_HOOKCALL	0
#define LUNA_HOOKRET	1
#define LUNA_HOOKLINE	2
#define LUNA_HOOKCOUNT	3
#define LUNA_HOOKTAILCALL 4


/*
** Event masks
*/
#define LUNA_MASKCALL	(1 << LUNA_HOOKCALL)
#define LUNA_MASKRET	(1 << LUNA_HOOKRET)
#define LUNA_MASKLINE	(1 << LUNA_HOOKLINE)
#define LUNA_MASKCOUNT	(1 << LUNA_HOOKCOUNT)


LUNA_API int (luna_getstack) (luna_State *L, int level, luna_Debug *ar);
LUNA_API int (luna_getinfo) (luna_State *L, const char *what, luna_Debug *ar);
LUNA_API const char *(luna_getlocal) (luna_State *L, const luna_Debug *ar, int n);
LUNA_API const char *(luna_setlocal) (luna_State *L, const luna_Debug *ar, int n);
LUNA_API const char *(luna_getupvalue) (luna_State *L, int funcindex, int n);
LUNA_API const char *(luna_setupvalue) (luna_State *L, int funcindex, int n);

LUNA_API void *(luna_upvalueid) (luna_State *L, int fidx, int n);
LUNA_API void  (luna_upvaluejoin) (luna_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

LUNA_API void (luna_sethook) (luna_State *L, luna_Hook func, int mask, int count);
LUNA_API luna_Hook (luna_gethook) (luna_State *L);
LUNA_API int (luna_gethookmask) (luna_State *L);
LUNA_API int (luna_gethookcount) (luna_State *L);

LUNA_API int (luna_setcstacklimit) (luna_State *L, unsigned int limit);

struct luna_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  const char *source;	/* (S) */
  size_t srclen;	/* (S) */
  int currentline;	/* (l) */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  unsigned char nups;	/* (u) number of upvalues */
  unsigned char nparams;/* (u) number of parameters */
  char isvararg;        /* (u) */
  char istailcall;	/* (t) */
  unsigned short ftransfer;   /* (r) index of first value transferred */
  unsigned short ntransfer;   /* (r) number of transferred values */
  char short_src[LUNA_IDSIZE]; /* (S) */
  /* private part */
  struct CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#define LUAI_TOSTRAUX(x)	#x
#define LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#define LUNA_VERSION_MAJOR	LUAI_TOSTR(LUNA_VERSION_MAJOR_N)
#define LUNA_VERSION_MINOR	LUAI_TOSTR(LUNA_VERSION_MINOR_N)
#define LUNA_VERSION_RELEASE	LUAI_TOSTR(LUNA_VERSION_RELEASE_N)



/******************************************************************************
* Copyright (C) 1994-2023 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
