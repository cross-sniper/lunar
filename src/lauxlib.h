/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "luaconf.h"
#include "lua.h"


/* global table */
#define LUNA_GNAME	"_G"


typedef struct lunaL_Buffer lunaL_Buffer;


/* extra error code for 'lunaL_loadfilex' */
#define LUNA_ERRFILE     (LUNA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUNA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUNA_PRELOAD_TABLE	"_PRELOAD"


typedef struct lunaL_Reg {
  const char *name;
  luna_CFunction func;
} lunaL_Reg;


#define LUAL_NUMSIZES	(sizeof(luna_Integer)*16 + sizeof(luna_Number))

LUALIB_API void (lunaL_checkversion_) (luna_State *L, luna_Number ver, size_t sz);
#define lunaL_checkversion(L)  \
	  lunaL_checkversion_(L, LUNA_VERSION_NUM, LUAL_NUMSIZES)

LUALIB_API int (lunaL_getmetafield) (luna_State *L, int obj, const char *e);
LUALIB_API int (lunaL_callmeta) (luna_State *L, int obj, const char *e);
LUALIB_API const char *(lunaL_tolstring) (luna_State *L, int idx, size_t *len);
LUALIB_API int (lunaL_argerror) (luna_State *L, int arg, const char *extramsg);
LUALIB_API int (lunaL_typeerror) (luna_State *L, int arg, const char *tname);
LUALIB_API const char *(lunaL_checklstring) (luna_State *L, int arg,
                                                          size_t *l);
LUALIB_API const char *(lunaL_optlstring) (luna_State *L, int arg,
                                          const char *def, size_t *l);
LUALIB_API luna_Number (lunaL_checknumber) (luna_State *L, int arg);
LUALIB_API luna_Number (lunaL_optnumber) (luna_State *L, int arg, luna_Number def);

LUALIB_API luna_Integer (lunaL_checkinteger) (luna_State *L, int arg);
LUALIB_API luna_Integer (lunaL_optinteger) (luna_State *L, int arg,
                                          luna_Integer def);

LUALIB_API void (lunaL_checkstack) (luna_State *L, int sz, const char *msg);
LUALIB_API void (lunaL_checktype) (luna_State *L, int arg, int t);
LUALIB_API void (lunaL_checkany) (luna_State *L, int arg);

LUALIB_API int   (lunaL_newmetatable) (luna_State *L, const char *tname);
LUALIB_API void  (lunaL_setmetatable) (luna_State *L, const char *tname);
LUALIB_API void *(lunaL_testudata) (luna_State *L, int ud, const char *tname);
LUALIB_API void *(lunaL_checkudata) (luna_State *L, int ud, const char *tname);

LUALIB_API void (lunaL_where) (luna_State *L, int lvl);
LUALIB_API int (lunaL_error) (luna_State *L, const char *fmt, ...);

LUALIB_API int (lunaL_checkoption) (luna_State *L, int arg, const char *def,
                                   const char *const lst[]);

LUALIB_API int (lunaL_fileresult) (luna_State *L, int stat, const char *fname);
LUALIB_API int (lunaL_execresult) (luna_State *L, int stat);


/* predefined references */
#define LUNA_NOREF       (-2)
#define LUNA_REFNIL      (-1)

LUALIB_API int (lunaL_ref) (luna_State *L, int t);
LUALIB_API void (lunaL_unref) (luna_State *L, int t, int ref);

LUALIB_API int (lunaL_loadfilex) (luna_State *L, const char *filename,
                                               const char *mode);

#define lunaL_loadfile(L,f)	lunaL_loadfilex(L,f,NULL)

LUALIB_API int (lunaL_loadbufferx) (luna_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
LUALIB_API int (lunaL_loadstring) (luna_State *L, const char *s);

LUALIB_API luna_State *(lunaL_newstate) (void);

LUALIB_API luna_Integer (lunaL_len) (luna_State *L, int idx);

LUALIB_API void (lunaL_addgsub) (lunaL_Buffer *b, const char *s,
                                     const char *p, const char *r);
LUALIB_API const char *(lunaL_gsub) (luna_State *L, const char *s,
                                    const char *p, const char *r);

LUALIB_API void (lunaL_setfuncs) (luna_State *L, const lunaL_Reg *l, int nup);

LUALIB_API int (lunaL_getsubtable) (luna_State *L, int idx, const char *fname);

LUALIB_API void (lunaL_traceback) (luna_State *L, luna_State *L1,
                                  const char *msg, int level);

LUALIB_API void (lunaL_requiref) (luna_State *L, const char *modname,
                                 luna_CFunction openf, int glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define lunaL_newlibtable(L,l)	\
  luna_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define lunaL_newlib(L,l)  \
  (lunaL_checkversion(L), lunaL_newlibtable(L,l), lunaL_setfuncs(L,l,0))

#define lunaL_argcheck(L, cond,arg,extramsg)	\
	((void)(luai_likely(cond) || lunaL_argerror(L, (arg), (extramsg))))

#define lunaL_argexpected(L,cond,arg,tname)	\
	((void)(luai_likely(cond) || lunaL_typeerror(L, (arg), (tname))))

#define lunaL_checkstring(L,n)	(lunaL_checklstring(L, (n), NULL))
#define lunaL_optstring(L,n,d)	(lunaL_optlstring(L, (n), (d), NULL))

#define lunaL_typename(L,i)	luna_typename(L, luna_type(L,(i)))

#define lunaL_dofile(L, fn) \
	(lunaL_loadfile(L, fn) || luna_pcall(L, 0, LUNA_MULTRET, 0))

#define lunaL_dostring(L, s) \
	(lunaL_loadstring(L, s) || luna_pcall(L, 0, LUNA_MULTRET, 0))

#define lunaL_getmetatable(L,n)	(luna_getfield(L, LUNA_REGISTRYINDEX, (n)))

#define lunaL_opt(L,f,n,d)	(luna_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define lunaL_loadbuffer(L,s,sz,n)	lunaL_loadbufferx(L,s,sz,n,NULL)


/*
** Perform arithmetic operations on luna_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define lunaL_intop(op,v1,v2)  \
	((luna_Integer)((luna_Unsigned)(v1) op (luna_Unsigned)(v2)))


/* push the value used to represent failure/error */
#define lunaL_pushfail(L)	luna_pushnil(L)


/*
** Internal assertions for in-house debugging
*/
#if !defined(luna_assert)

#if defined LUAI_ASSERT
  #include <assert.h>
  #define luna_assert(c)		assert(c)
#else
  #define luna_assert(c)		((void)0)
#endif

#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

struct lunaL_Buffer {
  char *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  luna_State *L;
  union {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    char b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define lunaL_bufflen(bf)	((bf)->n)
#define lunaL_buffaddr(bf)	((bf)->b)


#define lunaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || lunaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define lunaL_addsize(B,s)	((B)->n += (s))

#define lunaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API void (lunaL_buffinit) (luna_State *L, lunaL_Buffer *B);
LUALIB_API char *(lunaL_prepbuffsize) (lunaL_Buffer *B, size_t sz);
LUALIB_API void (lunaL_addlstring) (lunaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (lunaL_addstring) (lunaL_Buffer *B, const char *s);
LUALIB_API void (lunaL_addvalue) (lunaL_Buffer *B);
LUALIB_API void (lunaL_pushresult) (lunaL_Buffer *B);
LUALIB_API void (lunaL_pushresultsize) (lunaL_Buffer *B, size_t sz);
LUALIB_API char *(lunaL_buffinitsize) (luna_State *L, lunaL_Buffer *B, size_t sz);

#define lunaL_prepbuffer(B)	lunaL_prepbuffsize(B, LUAL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUNA_FILEHANDLE' and
** initial structure 'lunaL_Stream' (it may contain other fields
** after that initial structure).
*/

#define LUNA_FILEHANDLE          "FILE*"


typedef struct lunaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  luna_CFunction closef;  /* to close stream (NULL for closed streams) */
} lunaL_Stream;

/* }====================================================== */

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#if !defined(luna_writestring)
#define luna_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output */
#if !defined(luna_writeline)
#define luna_writeline()        (luna_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#if !defined(luna_writestringerror)
#define luna_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#if defined(LUNA_COMPAT_APIINTCASTS)

#define lunaL_checkunsigned(L,a)	((luna_Unsigned)lunaL_checkinteger(L,a))
#define lunaL_optunsigned(L,a,d)	\
	((luna_Unsigned)lunaL_optinteger(L,a,(luna_Integer)(d)))

#define lunaL_checkint(L,n)	((int)lunaL_checkinteger(L, (n)))
#define lunaL_optint(L,n,d)	((int)lunaL_optinteger(L, (n), (d)))

#define lunaL_checklong(L,n)	((long)lunaL_checkinteger(L, (n)))
#define lunaL_optlong(L,n,d)	((long)lunaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


