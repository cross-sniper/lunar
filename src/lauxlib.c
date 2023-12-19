/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUNA_LIB

#include "lprefix.h"


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#include "lua.h"

#include "lauxlib.h"


#if !defined(MAX_SIZET)
/* maximum value for size_t */
#define MAX_SIZET	((size_t)(~(size_t)0))
#endif


/*
** {======================================================
** Traceback
** =======================================================
*/


#define LEVELS1	10	/* size of the first part of the stack */
#define LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
static int findfield (luna_State *L, int objidx, int level) {
  if (level == 0 || !luna_istable(L, -1))
    return 0;  /* not found */
  luna_pushnil(L);  /* start 'next' loop */
  while (luna_next(L, -2)) {  /* for each pair in table */
    if (luna_type(L, -2) == LUNA_TSTRING) {  /* ignore non-string keys */
      if (luna_rawequal(L, objidx, -1)) {  /* found object? */
        luna_pop(L, 1);  /* remove value (but keep name) */
        return 1;
      }
      else if (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        luna_pushliteral(L, ".");  /* place '.' between the two names */
        luna_replace(L, -3);  /* (in the slot occupied by table) */
        luna_concat(L, 3);  /* lib_name.field_name */
        return 1;
      }
    }
    luna_pop(L, 1);  /* remove value */
  }
  return 0;  /* not found */
}


/*
**reads a file, and returns its contents, or returns 0
*/
static int luna_readfile(luna_State *L) {
    const char *filename = lunaL_checkstring(L, 1);
    FILE *file = fopen(filename, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);
        char *contents = (char *)malloc(size + 1);
        if (contents) {
            fread(contents, 1, size, file);
            contents[size] = '\0';
            fclose(file);
            luna_pushstring(L, contents);
            free(contents);
            return 1;
        }
        fclose(file);
    }
    return 0;  // File not found or error occurred
}



/*
** Search for a name for a function in all loaded modules
*/
static int pushglobalfuncname (luna_State *L, luna_Debug *ar) {
  int top = luna_gettop(L);
  luna_getinfo(L, "f", ar);  /* push function */
  luna_getfield(L, LUNA_REGISTRYINDEX, LUNA_LOADED_TABLE);
  if (findfield(L, top + 1, 2)) {
    const char *name = luna_tostring(L, -1);
    if (strncmp(name, LUNA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      luna_pushstring(L, name + 3);  /* push name without prefix */
      luna_remove(L, -2);  /* remove original name */
    }
    luna_copy(L, -1, top + 1);  /* copy name to proper place */
    luna_settop(L, top + 1);  /* remove table "loaded" and name copy */
    return 1;
  }
  else {
    luna_settop(L, top);  /* remove function and global table */
    return 0;
  }
}


static void pushfuncname (luna_State *L, luna_Debug *ar) {
  if (pushglobalfuncname(L, ar)) {  /* try first a global name */
    luna_pushfstring(L, "function '%s'", luna_tostring(L, -1));
    luna_remove(L, -2);  /* remove name */
  }
  else if (*ar->namewhat != '\0')  /* is there a name from code? */
    luna_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  else if (*ar->what == 'm')  /* main? */
      luna_pushliteral(L, "main chunk");
  else if (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    luna_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  else  /* nothing left... */
    luna_pushliteral(L, "?");
}


static int lastlevel (luna_State *L) {
  luna_Debug ar;
  int li = 1, le = 1;
  /* find an upper bound */
  while (luna_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  while (li < le) {
    int m = (li + le)/2;
    if (luna_getstack(L, m, &ar)) li = m + 1;
    else le = m;
  }
  return le - 1;
}


LUALIB_API void lunaL_traceback (luna_State *L, luna_State *L1,
                                const char *msg, int level) {
  lunaL_Buffer b;
  luna_Debug ar;
  int last = lastlevel(L1);
  int limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  lunaL_buffinit(L, &b);
  if (msg) {
    lunaL_addstring(&b, msg);
    lunaL_addchar(&b, '\n');
  }
  lunaL_addstring(&b, "stack traceback:");
  while (luna_getstack(L1, level++, &ar)) {
    if (limit2show-- == 0) {  /* too many levels? */
      int n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      luna_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      lunaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    else {
      luna_getinfo(L1, "Slnt", &ar);
      if (ar.currentline <= 0)
        luna_pushfstring(L, "\n\t%s: in ", ar.short_src);
      else
        luna_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      lunaL_addvalue(&b);
      pushfuncname(L, &ar);
      lunaL_addvalue(&b);
      if (ar.istailcall)
        lunaL_addstring(&b, "\n\t(...tail calls...)");
    }
  }
  lunaL_pushresult(&b);
}

/* }====================================================== */


/*
** {======================================================
** Error-report functions
** =======================================================
*/

LUALIB_API int lunaL_argerror (luna_State *L, int arg, const char *extramsg) {
  luna_Debug ar;
  if (!luna_getstack(L, 0, &ar))  /* no stack frame? */
    return lunaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  luna_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    arg--;  /* do not count 'self' */
    if (arg == 0)  /* error is in the self argument itself? */
      return lunaL_error(L, "calling '%s' on bad self (%s)",
                           ar.name, extramsg);
  }
  if (ar.name == NULL)
    ar.name = (pushglobalfuncname(L, &ar)) ? luna_tostring(L, -1) : "?";
  return lunaL_error(L, "bad argument #%d to '%s' (%s)",
                        arg, ar.name, extramsg);
}


LUALIB_API int lunaL_typeerror (luna_State *L, int arg, const char *tname) {
  const char *msg;
  const char *typearg;  /* name for the type of the actual argument */
  if (lunaL_getmetafield(L, arg, "__name") == LUNA_TSTRING)
    typearg = luna_tostring(L, -1);  /* use the given type name */
  else if (luna_type(L, arg) == LUNA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  else
    typearg = lunaL_typename(L, arg);  /* standard name */
  msg = luna_pushfstring(L, "%s expected, got %s", tname, typearg);
  return lunaL_argerror(L, arg, msg);
}


static void tag_error (luna_State *L, int arg, int tag) {
  lunaL_typeerror(L, arg, luna_typename(L, tag));
}


/*
** The use of 'luna_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void lunaL_where (luna_State *L, int level) {
  luna_Debug ar;
  if (luna_getstack(L, level, &ar)) {  /* check function at level */
    luna_getinfo(L, "Sl", &ar);  /* get info about it */
    if (ar.currentline > 0) {  /* is there info? */
      luna_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      return;
    }
  }
  luna_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'luna_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
LUALIB_API int lunaL_error (luna_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  lunaL_where(L, 1);
  luna_pushvfstring(L, fmt, argp);
  va_end(argp);
  luna_concat(L, 2);
  return luna_error(L);
}


LUALIB_API int lunaL_fileresult (luna_State *L, int stat, const char *fname) {
  int en = errno;  /* calls to Lua API may change this value */
  if (stat) {
    luna_pushboolean(L, 1);
    return 1;
  }
  else {
    lunaL_pushfail(L);
    if (fname)
      luna_pushfstring(L, "%s: %s", fname, strerror(en));
    else
      luna_pushstring(L, strerror(en));
    luna_pushinteger(L, en);
    return 3;
  }
}


#if !defined(l_inspectstat)	/* { */

#if defined(LUNA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   if (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   else if (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#else

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API int lunaL_execresult (luna_State *L, int stat) {
  if (stat != 0 && errno != 0)  /* error with an 'errno'? */
    return lunaL_fileresult(L, 0, NULL);
  else {
    const char *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    if (*what == 'e' && stat == 0)  /* successful termination? */
      luna_pushboolean(L, 1);
    else
      lunaL_pushfail(L);
    luna_pushstring(L, what);
    luna_pushinteger(L, stat);
    return 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API int lunaL_newmetatable (luna_State *L, const char *tname) {
  if (lunaL_getmetatable(L, tname) != LUNA_TNIL)  /* name already in use? */
    return 0;  /* leave previous value on top, but return 0 */
  luna_pop(L, 1);
  luna_createtable(L, 0, 2);  /* create metatable */
  luna_pushstring(L, tname);
  luna_setfield(L, -2, "__name");  /* metatable.__name = tname */
  luna_pushvalue(L, -1);
  luna_setfield(L, LUNA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  return 1;
}


LUALIB_API void lunaL_setmetatable (luna_State *L, const char *tname) {
  lunaL_getmetatable(L, tname);
  luna_setmetatable(L, -2);
}


LUALIB_API void *lunaL_testudata (luna_State *L, int ud, const char *tname) {
  void *p = luna_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (luna_getmetatable(L, ud)) {  /* does it have a metatable? */
      lunaL_getmetatable(L, tname);  /* get correct metatable */
      if (!luna_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      luna_pop(L, 2);  /* remove both metatables */
      return p;
    }
  }
  return NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *lunaL_checkudata (luna_State *L, int ud, const char *tname) {
  void *p = lunaL_testudata(L, ud, tname);
  lunaL_argexpected(L, p != NULL, ud, tname);
  return p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API int lunaL_checkoption (luna_State *L, int arg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? lunaL_optstring(L, arg, def) :
                             lunaL_checkstring(L, arg);
  int i;
  for (i=0; lst[i]; i++)
    if (strcmp(lst[i], name) == 0)
      return i;
  return lunaL_argerror(L, arg,
                       luna_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void lunaL_checkstack (luna_State *L, int space, const char *msg) {
  if (l_unlikely(!luna_checkstack(L, space))) {
    if (msg)
      lunaL_error(L, "stack overflow (%s)", msg);
    else
      lunaL_error(L, "stack overflow");
  }
}


LUALIB_API void lunaL_checktype (luna_State *L, int arg, int t) {
  if (l_unlikely(luna_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API void lunaL_checkany (luna_State *L, int arg) {
  if (l_unlikely(luna_type(L, arg) == LUNA_TNONE))
    lunaL_argerror(L, arg, "value expected");
}


LUALIB_API const char *lunaL_checklstring (luna_State *L, int arg, size_t *len) {
  const char *s = luna_tolstring(L, arg, len);
  if (l_unlikely(!s)) tag_error(L, arg, LUNA_TSTRING);
  return s;
}


LUALIB_API const char *lunaL_optlstring (luna_State *L, int arg,
                                        const char *def, size_t *len) {
  if (luna_isnoneornil(L, arg)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return lunaL_checklstring(L, arg, len);
}


LUALIB_API luna_Number lunaL_checknumber (luna_State *L, int arg) {
  int isnum;
  luna_Number d = luna_tonumberx(L, arg, &isnum);
  if (l_unlikely(!isnum))
    tag_error(L, arg, LUNA_TNUMBER);
  return d;
}


LUALIB_API luna_Number lunaL_optnumber (luna_State *L, int arg, luna_Number def) {
  return lunaL_opt(L, lunaL_checknumber, arg, def);
}


static void interror (luna_State *L, int arg) {
  if (luna_isnumber(L, arg))
    lunaL_argerror(L, arg, "number has no integer representation");
  else
    tag_error(L, arg, LUNA_TNUMBER);
}


LUALIB_API luna_Integer lunaL_checkinteger (luna_State *L, int arg) {
  int isnum;
  luna_Integer d = luna_tointegerx(L, arg, &isnum);
  if (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  return d;
}


LUALIB_API luna_Integer lunaL_optinteger (luna_State *L, int arg,
                                                      luna_Integer def) {
  return lunaL_opt(L, lunaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
typedef struct UBox {
  void *box;
  size_t bsize;
} UBox;


static void *resizebox (luna_State *L, int idx, size_t newsize) {
  void *ud;
  luna_Alloc allocf = luna_getallocf(L, &ud);
  UBox *box = (UBox *)luna_touserdata(L, idx);
  void *temp = allocf(ud, box->box, box->bsize, newsize);
  if (l_unlikely(temp == NULL && newsize > 0)) {  /* allocation error? */
    luna_pushliteral(L, "not enough memory");
    luna_error(L);  /* raise a memory error */
  }
  box->box = temp;
  box->bsize = newsize;
  return temp;
}


static int boxgc (luna_State *L) {
  resizebox(L, 1, 0);
  return 0;
}


static const lunaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {NULL, NULL}
};

static void newbox (luna_State *L) {
  UBox *box = (UBox *)luna_newuserdatauv(L, sizeof(UBox), 0);
  box->box = NULL;
  box->bsize = 0;
  if (lunaL_newmetatable(L, "_UBOX*"))  /* creating metatable? */
    lunaL_setfuncs(L, boxmt, 0);  /* set its metamethods */
  luna_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#define buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#define checkbufferlevel(B,idx)  \
  luna_assert(buffonstack(B) ? luna_touserdata(B->L, idx) != NULL  \
                            : luna_touserdata(B->L, idx) == (void*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes. (The test for "not big enough" also gets the case when the
** computation of 'newsize' overflows.)
*/
static size_t newbuffsize (lunaL_Buffer *B, size_t sz) {
  size_t newsize = (B->size / 2) * 3;  /* buffer size * 1.5 */
  if (l_unlikely(MAX_SIZET - sz < B->n))  /* overflow in (B->n + sz)? */
    return lunaL_error(B->L, "buffer too large");
  if (newsize < B->n + sz)  /* not big enough? */
    newsize = B->n + sz;
  return newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
static char *prepbuffsize (lunaL_Buffer *B, size_t sz, int boxidx) {
  checkbufferlevel(B, boxidx);
  if (B->size - B->n >= sz)  /* enough space? */
    return B->b + B->n;
  else {
    luna_State *L = B->L;
    char *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    if (buffonstack(B))  /* buffer already has a box? */
      newbuff = (char *)resizebox(L, boxidx, newsize);  /* resize it */
    else {  /* no box yet */
      luna_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      luna_insert(L, boxidx);  /* move box to its intended position */
      luna_toclose(L, boxidx);
      newbuff = (char *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * sizeof(char));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    return newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API char *lunaL_prepbuffsize (lunaL_Buffer *B, size_t sz) {
  return prepbuffsize(B, sz, -1);
}


LUALIB_API void lunaL_addlstring (lunaL_Buffer *B, const char *s, size_t l) {
  if (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    char *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * sizeof(char));
    lunaL_addsize(B, l);
  }
}


LUALIB_API void lunaL_addstring (lunaL_Buffer *B, const char *s) {
  lunaL_addlstring(B, s, strlen(s));
}


LUALIB_API void lunaL_pushresult (lunaL_Buffer *B) {
  luna_State *L = B->L;
  checkbufferlevel(B, -1);
  luna_pushlstring(L, B->b, B->n);
  if (buffonstack(B))
    luna_closeslot(L, -2);  /* close the box */
  luna_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API void lunaL_pushresultsize (lunaL_Buffer *B, size_t sz) {
  lunaL_addsize(B, sz);
  lunaL_pushresult(B);
}


/*
** 'lunaL_addvalue' is the only function in the Buffer system where the
** box (if existent) is not on the top of the stack. So, instead of
** calling 'lunaL_addlstring', it replicates the code using -2 as the
** last argument to 'prepbuffsize', signaling that the box is (or will
** be) below the string being added to the buffer. (Box creation can
** trigger an emergency GC, so we should not remove the string from the
** stack before we have the space guaranteed.)
*/
LUALIB_API void lunaL_addvalue (lunaL_Buffer *B) {
  luna_State *L = B->L;
  size_t len;
  const char *s = luna_tolstring(L, -1, &len);
  char *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * sizeof(char));
  lunaL_addsize(B, len);
  luna_pop(L, 1);  /* pop string */
}


LUALIB_API void lunaL_buffinit (luna_State *L, lunaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  luna_pushlightuserdata(L, (void*)B);  /* push placeholder */
}


LUALIB_API char *lunaL_buffinitsize (luna_State *L, lunaL_Buffer *B, size_t sz) {
  lunaL_buffinit(L, B);
  return prepbuffsize(B, sz, -1);
}

/* }====================================================== */


/*
** {======================================================
** Reference system
** =======================================================
*/

/* index of free-list header (after the predefined values) */
#define freelist	(LUNA_RIDX_LAST + 1)

/*
** The previously freed references form a linked list:
** t[freelist] is the index of a first free index, or zero if list is
** empty; t[t[freelist]] is the index of the second element; etc.
*/
LUALIB_API int lunaL_ref (luna_State *L, int t) {
  int ref;
  if (luna_isnil(L, -1)) {
    luna_pop(L, 1);  /* remove from stack */
    return LUNA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = luna_absindex(L, t);
  if (luna_rawgeti(L, t, freelist) == LUNA_TNIL) {  /* first access? */
    ref = 0;  /* list is empty */
    luna_pushinteger(L, 0);  /* initialize as an empty list */
    luna_rawseti(L, t, freelist);  /* ref = t[freelist] = 0 */
  }
  else {  /* already initialized */
    luna_assert(luna_isinteger(L, -1));
    ref = (int)luna_tointeger(L, -1);  /* ref = t[freelist] */
  }
  luna_pop(L, 1);  /* remove element from stack */
  if (ref != 0) {  /* any free element? */
    luna_rawgeti(L, t, ref);  /* remove it from list */
    luna_rawseti(L, t, freelist);  /* (t[freelist] = t[ref]) */
  }
  else  /* no free elements */
    ref = (int)luna_rawlen(L, t) + 1;  /* get a new reference */
  luna_rawseti(L, t, ref);
  return ref;
}


LUALIB_API void lunaL_unref (luna_State *L, int t, int ref) {
  if (ref >= 0) {
    t = luna_absindex(L, t);
    luna_rawgeti(L, t, freelist);
    luna_assert(luna_isinteger(L, -1));
    luna_rawseti(L, t, ref);  /* t[ref] = t[freelist] */
    luna_pushinteger(L, ref);
    luna_rawseti(L, t, freelist);  /* t[freelist] = ref */
  }
}

/* }====================================================== */


/*
** {======================================================
** Load functions
** =======================================================
*/

typedef struct LoadF {
  int n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  char buff[BUFSIZ];  /* area for reading file */
} LoadF;


static const char *getF (luna_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  (void)L;  /* not used */
  if (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  else {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    if (feof(lf->f)) return NULL;
    *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);  /* read block */
  }
  return lf->buff;
}


static int errfile (luna_State *L, const char *what, int fnameindex) {
  const char *serr = strerror(errno);
  const char *filename = luna_tostring(L, fnameindex) + 1;
  luna_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
  luna_remove(L, fnameindex);
  return LUNA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
static int skipBOM (FILE *f) {
  int c = getc(f);  /* read first character */
  if (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    return getc(f);  /* ignore BOM and return next char */
  else  /* no (valid) BOM */
    return c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static int skipcomment (FILE *f, int *cp) {
  int c = *cp = skipBOM(f);
  if (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(f);
    } while (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    return 1;  /* there was a comment */
  }
  else return 0;  /* no comment */
}


LUALIB_API int lunaL_loadfilex (luna_State *L, const char *filename,
                                             const char *mode) {
  LoadF lf;
  int status, readstatus;
  int c;
  int fnameindex = luna_gettop(L) + 1;  /* index of filename on the stack */
  if (filename == NULL) {
    luna_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  else {
    luna_pushfstring(L, "@%s", filename);
    lf.f = fopen(filename, "r");
    if (lf.f == NULL) return errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  if (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  if (c == LUNA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    if (filename) {  /* "real" file? */
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  if (c != EOF)
    lf.buff[lf.n++] = c;  /* 'c' is the first character of the stream */
  status = luna_load(L, getF, &lf, luna_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  if (filename) fclose(lf.f);  /* close file (even in case of errors) */
  if (readstatus) {
    luna_settop(L, fnameindex);  /* ignore results from 'luna_load' */
    return errfile(L, "read", fnameindex);
  }
  luna_remove(L, fnameindex);
  return status;
}


typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;


static const char *getS (luna_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  (void)L;  /* not used */
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}


LUALIB_API int lunaL_loadbufferx (luna_State *L, const char *buff, size_t size,
                                 const char *name, const char *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  return luna_load(L, getS, &ls, name, mode);
}


LUALIB_API int lunaL_loadstring (luna_State *L, const char *s) {
  return lunaL_loadbuffer(L, s, strlen(s), s);
}

/* }====================================================== */



LUALIB_API int lunaL_getmetafield (luna_State *L, int obj, const char *event) {
  if (!luna_getmetatable(L, obj))  /* no metatable? */
    return LUNA_TNIL;
  else {
    int tt;
    luna_pushstring(L, event);
    tt = luna_rawget(L, -2);
    if (tt == LUNA_TNIL)  /* is metafield nil? */
      luna_pop(L, 2);  /* remove metatable and metafield */
    else
      luna_remove(L, -2);  /* remove only metatable */
    return tt;  /* return metafield type */
  }
}


LUALIB_API int lunaL_callmeta (luna_State *L, int obj, const char *event) {
  obj = luna_absindex(L, obj);
  if (lunaL_getmetafield(L, obj, event) == LUNA_TNIL)  /* no metafield? */
    return 0;
  luna_pushvalue(L, obj);
  luna_call(L, 1, 1);
  return 1;
}


LUALIB_API luna_Integer lunaL_len (luna_State *L, int idx) {
  luna_Integer l;
  int isnum;
  luna_len(L, idx);
  l = luna_tointegerx(L, -1, &isnum);
  if (l_unlikely(!isnum))
    lunaL_error(L, "object length is not an integer");
  luna_pop(L, 1);  /* remove object */
  return l;
}


LUALIB_API const char *lunaL_tolstring (luna_State *L, int idx, size_t *len) {
  idx = luna_absindex(L,idx);
  if (lunaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    if (!luna_isstring(L, -1))
      lunaL_error(L, "'__tostring' must return a string");
  }
  else {
    switch (luna_type(L, idx)) {
      case LUNA_TNUMBER: {
        if (luna_isinteger(L, idx))
          luna_pushfstring(L, "%I", (LUAI_UACINT)luna_tointeger(L, idx));
        else
          luna_pushfstring(L, "%f", (LUAI_UACNUMBER)luna_tonumber(L, idx));
        break;
      }
      case LUNA_TSTRING:
        luna_pushvalue(L, idx);
        break;
      case LUNA_TBOOLEAN:
        luna_pushstring(L, (luna_toboolean(L, idx) ? "true" : "false"));
        break;
      case LUNA_TNIL:
        luna_pushliteral(L, "nil");
        break;
      default: {
        int tt = lunaL_getmetafield(L, idx, "__name");  /* try name */
        const char *kind = (tt == LUNA_TSTRING) ? luna_tostring(L, -1) :
                                                 lunaL_typename(L, idx);
        luna_pushfstring(L, "%s: %p", kind, luna_topointer(L, idx));
        if (tt != LUNA_TNIL)
          luna_remove(L, -2);  /* remove '__name' */
        break;
      }
    }
  }
  return luna_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void lunaL_setfuncs (luna_State *L, const lunaL_Reg *l, int nup) {
  lunaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    if (l->func == NULL)  /* place holder? */
      luna_pushboolean(L, 0);
    else {
      int i;
      for (i = 0; i < nup; i++)  /* copy upvalues to the top */
        luna_pushvalue(L, -nup);
      luna_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    luna_setfield(L, -(nup + 2), l->name);
  }
  luna_pop(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API int lunaL_getsubtable (luna_State *L, int idx, const char *fname) {
  if (luna_getfield(L, idx, fname) == LUNA_TTABLE)
    return 1;  /* table already there */
  else {
    luna_pop(L, 1);  /* remove previous result */
    idx = luna_absindex(L, idx);
    luna_newtable(L);
    luna_pushvalue(L, -1);  /* copy to be left at top */
    luna_setfield(L, idx, fname);  /* assign new table to field */
    return 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void lunaL_requiref (luna_State *L, const char *modname,
                               luna_CFunction openf, int glb) {
  lunaL_getsubtable(L, LUNA_REGISTRYINDEX, LUNA_LOADED_TABLE);
  luna_getfield(L, -1, modname);  /* LOADED[modname] */
  if (!luna_toboolean(L, -1)) {  /* package not already loaded? */
    luna_pop(L, 1);  /* remove field */
    luna_pushcfunction(L, openf);
    luna_pushstring(L, modname);  /* argument to open function */
    luna_call(L, 1, 1);  /* call 'openf' to open module */
    luna_pushvalue(L, -1);  /* make copy of module (call result) */
    luna_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  luna_remove(L, -2);  /* remove LOADED table */
  if (glb) {
    luna_pushvalue(L, -1);  /* copy of module */
    luna_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API void lunaL_addgsub (lunaL_Buffer *b, const char *s,
                                     const char *p, const char *r) {
  const char *wild;
  size_t l = strlen(p);
  while ((wild = strstr(s, p)) != NULL) {
    lunaL_addlstring(b, s, wild - s);  /* push prefix */
    lunaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  lunaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API const char *lunaL_gsub (luna_State *L, const char *s,
                                  const char *p, const char *r) {
  lunaL_Buffer b;
  lunaL_buffinit(L, &b);
  lunaL_addgsub(&b, s, p, r);
  lunaL_pushresult(&b);
  return luna_tostring(L, -1);
}


static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud; (void)osize;  /* not used */
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static int panic (luna_State *L) {
  const char *msg = luna_tostring(L, -1);
  if (msg == NULL) msg = "error object is not a string";
  luna_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        msg);
  return 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
static void warnfoff (void *ud, const char *message, int tocont);
static void warnfon (void *ud, const char *message, int tocont);
static void warnfcont (void *ud, const char *message, int tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
static int checkcontrol (luna_State *L, const char *message, int tocont) {
  if (tocont || *(message++) != '@')  /* not a control message? */
    return 0;
  else {
    if (strcmp(message, "off") == 0)
      luna_setwarnf(L, warnfoff, L);  /* turn warnings off */
    else if (strcmp(message, "on") == 0)
      luna_setwarnf(L, warnfon, L);   /* turn warnings on */
    return 1;  /* it was a control message */
  }
}


static void warnfoff (void *ud, const char *message, int tocont) {
  checkcontrol((luna_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
static void warnfcont (void *ud, const char *message, int tocont) {
  luna_State *L = (luna_State *)ud;
  luna_writestringerror("%s", message);  /* write message */
  if (tocont)  /* not the last part? */
    luna_setwarnf(L, warnfcont, L);  /* to be continued */
  else {  /* last part */
    luna_writestringerror("%s", "\n");  /* finish message with end-of-line */
    luna_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


static void warnfon (void *ud, const char *message, int tocont) {
  if (checkcontrol((luna_State *)ud, message, tocont))  /* control message? */
    return;  /* nothing else to be done */
  luna_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}


LUALIB_API luna_State *lunaL_newstate (void) {
  luna_State *L = luna_newstate(l_alloc, NULL);
  if (l_likely(L)) {
    luna_atpanic(L, &panic);
    luna_setwarnf(L, warnfoff, L);  /* default is warnings off */
  }
  return L;
}


LUALIB_API void lunaL_checkversion_ (luna_State *L, luna_Number ver, size_t sz) {
  luna_Number v = luna_version(L);
  if (sz != LUAL_NUMSIZES)  /* check numeric types */
    lunaL_error(L, "core and library have incompatible numeric types");
  else if (v != ver)
    lunaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

