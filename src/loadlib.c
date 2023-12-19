/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#define loadlib_c
#define LUNA_LIB

#include "lprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


/*
** LUNA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUNA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#if !defined(LUNA_CSUBSEP)
#define LUNA_CSUBSEP		LUNA_DIRSEP
#endif

#if !defined(LUNA_LSUBSEP)
#define LUNA_LSUBSEP		LUNA_DIRSEP
#endif


/* prefix for open functions in C libraries */
#define LUNA_POF		"luaopen_"

/* separator for open functions in C libraries */
#define LUNA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
static const char *const CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((void)0)


/*
** Special type equivalent to '(void*)' for functions in gcc
** (to suppress warnings when converting function pointers)
*/
typedef void (*voidf)(void);


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
static void lsys_unloadlib (void *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
static void *lsys_load (luna_State *L, const char *path, int seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
static luna_CFunction lsys_sym (luna_State *L, void *lib, const char *sym);




#if defined(LUNA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface.
** The dlfcn interface is available in Linux, SunOS, Solaris, IRIX, FreeBSD,
** NetBSD, AIX 4.2, HPUX 11, and  probably most other Unix flavors, at least
** as an emulation layer on top of native functions.
** =========================================================================
*/

#include <dlfcn.h>

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#if defined(__GNUC__)
#define cast_func(p) (__extension__ (luna_CFunction)(p))
#else
#define cast_func(p) ((luna_CFunction)(p))
#endif


static void lsys_unloadlib (void *lib) {
  dlclose(lib);
}


static void *lsys_load (luna_State *L, const char *path, int seeglb) {
  void *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  if (l_unlikely(lib == NULL))
    luna_pushstring(L, dlerror());
  return lib;
}


static luna_CFunction lsys_sym (luna_State *L, void *lib, const char *sym) {
  luna_CFunction f = cast_func(dlsym(lib, sym));
  if (l_unlikely(f == NULL))
    luna_pushstring(L, dlerror());
  return f;
}

/* }====================================================== */



#elif defined(LUNA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#include <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#if !defined(LUNA_LLE_FLAGS)
#define LUNA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUNA_EXEC_DIR with the executable's path.
*/
static void setprogdir (luna_State *L) {
  char buff[MAX_PATH + 1];
  char *lb;
  DWORD nsize = sizeof(buff)/sizeof(char);
  DWORD n = GetModuleFileNameA(NULL, buff, nsize);  /* get exec. name */
  if (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == NULL)
    lunaL_error(L, "unable to get ModuleFileName");
  else {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    lunaL_gsub(L, luna_tostring(L, -1), LUNA_EXEC_DIR, buff);
    luna_remove(L, -2);  /* remove original string */
  }
}




static void pusherror (luna_State *L) {
  int error = GetLastError();
  char buffer[128];
  if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer)/sizeof(char), NULL))
    luna_pushstring(L, buffer);
  else
    luna_pushfstring(L, "system error %d\n", error);
}

static void lsys_unloadlib (void *lib) {
  FreeLibrary((HMODULE)lib);
}


static void *lsys_load (luna_State *L, const char *path, int seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, LUNA_LLE_FLAGS);
  (void)(seeglb);  /* not used: symbols are 'global' by default */
  if (lib == NULL) pusherror(L);
  return lib;
}


static luna_CFunction lsys_sym (luna_State *L, void *lib, const char *sym) {
  luna_CFunction f = (luna_CFunction)(voidf)GetProcAddress((HMODULE)lib, sym);
  if (f == NULL) pusherror(L);
  return f;
}

/* }====================================================== */


#else				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


static void lsys_unloadlib (void *lib) {
  (void)(lib);  /* not used */
}


static void *lsys_load (luna_State *L, const char *path, int seeglb) {
  (void)(path); (void)(seeglb);  /* not used */
  luna_pushliteral(L, DLMSG);
  return NULL;
}


static luna_CFunction lsys_sym (luna_State *L, void *lib, const char *sym) {
  (void)(lib); (void)(sym);  /* not used */
  luna_pushliteral(L, DLMSG);
  return NULL;
}

/* }====================================================== */
#endif				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUNA_PATH_VAR and LUNA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#if !defined(LUNA_PATH_VAR)
#define LUNA_PATH_VAR    "LUNA_PATH"
#endif

#if !defined(LUNA_CPATH_VAR)
#define LUNA_CPATH_VAR   "LUNA_CPATH"
#endif



/*
** return registry.LUNA_NOENV as a boolean
*/
static int noenv (luna_State *L) {
  int b;
  luna_getfield(L, LUNA_REGISTRYINDEX, "LUNA_NOENV");
  b = luna_toboolean(L, -1);
  luna_pop(L, 1);  /* remove value */
  return b;
}


/*
** Set a path
*/
static void setpath (luna_State *L, const char *fieldname,
                                   const char *envname,
                                   const char *dft) {
  const char *dftmark;
  const char *nver = luna_pushfstring(L, "%s%s", envname, LUNA_VERSUFFIX);
  const char *path = getenv(nver);  /* try versioned name */
  if (path == NULL)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  if (path == NULL || noenv(L))  /* no environment variable? */
    luna_pushstring(L, dft);  /* use default */
  else if ((dftmark = strstr(path, LUNA_PATH_SEP LUNA_PATH_SEP)) == NULL)
    luna_pushstring(L, path);  /* nothing to change */
  else {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    lunaL_Buffer b;
    lunaL_buffinit(L, &b);
    if (path < dftmark) {  /* is there a prefix before ';;'? */
      lunaL_addlstring(&b, path, dftmark - path);  /* add it */
      lunaL_addchar(&b, *LUNA_PATH_SEP);
    }
    lunaL_addstring(&b, dft);  /* add default */
    if (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      lunaL_addchar(&b, *LUNA_PATH_SEP);
      lunaL_addlstring(&b, dftmark + 2, (path + len - 2) - dftmark);
    }
    lunaL_pushresult(&b);
  }
  setprogdir(L);
  luna_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  luna_pop(L, 1);  /* pop versioned variable name ('nver') */
}

/* }================================================================== */


/*
** return registry.CLIBS[path]
*/
static void *checkclib (luna_State *L, const char *path) {
  void *plib;
  luna_getfield(L, LUNA_REGISTRYINDEX, CLIBS);
  luna_getfield(L, -1, path);
  plib = luna_touserdata(L, -1);  /* plib = CLIBS[path] */
  luna_pop(L, 2);  /* pop CLIBS table and 'plib' */
  return plib;
}


/*
** registry.CLIBS[path] = plib        -- for queries
** registry.CLIBS[#CLIBS + 1] = plib  -- also keep a list of all libraries
*/
static void addtoclib (luna_State *L, const char *path, void *plib) {
  luna_getfield(L, LUNA_REGISTRYINDEX, CLIBS);
  luna_pushlightuserdata(L, plib);
  luna_pushvalue(L, -1);
  luna_setfield(L, -3, path);  /* CLIBS[path] = plib */
  luna_rawseti(L, -2, lunaL_len(L, -2) + 1);  /* CLIBS[#CLIBS + 1] = plib */
  luna_pop(L, 1);  /* pop CLIBS table */
}


/*
** __gc tag method for CLIBS table: calls 'lsys_unloadlib' for all lib
** handles in list CLIBS
*/
static int gctm (luna_State *L) {
  luna_Integer n = lunaL_len(L, 1);
  for (; n >= 1; n--) {  /* for each handle, in reverse order */
    luna_rawgeti(L, 1, n);  /* get handle CLIBS[n] */
    lsys_unloadlib(luna_touserdata(L, -1));
    luna_pop(L, 1);  /* pop handle */
  }
  return 0;
}



/* error codes for 'lookforfunc' */
#define ERRLIB		1
#define ERRFUNC		2

/*
** Look for a C function named 'sym' in a dynamically loaded library
** 'path'.
** First, check whether the library is already loaded; if not, try
** to load it.
** Then, if 'sym' is '*', return true (as library has been loaded).
** Otherwise, look for symbol 'sym' in the library and push a
** C function with that symbol.
** Return 0 and 'true' or a function in the stack; in case of
** errors, return an error code and an error message in the stack.
*/
static int lookforfunc (luna_State *L, const char *path, const char *sym) {
  void *reg = checkclib(L, path);  /* check loaded C libraries */
  if (reg == NULL) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    if (reg == NULL) return ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  if (*sym == '*') {  /* loading only library (no function)? */
    luna_pushboolean(L, 1);  /* return 'true' */
    return 0;  /* no errors */
  }
  else {
    luna_CFunction f = lsys_sym(L, reg, sym);
    if (f == NULL)
      return ERRFUNC;  /* unable to find function */
    luna_pushcfunction(L, f);  /* else create new function */
    return 0;  /* no errors */
  }
}


static int ll_loadlib (luna_State *L) {
  const char *path = lunaL_checkstring(L, 1);
  const char *init = lunaL_checkstring(L, 2);
  int stat = lookforfunc(L, path, init);
  if (l_likely(stat == 0))  /* no errors? */
    return 1;  /* return the loaded function */
  else {  /* error; error message is on stack top */
    lunaL_pushfail(L);
    luna_insert(L, -2);
    luna_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    return 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


static int readable (const char *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  if (f == NULL) return 0;  /* open failed */
  fclose(f);
  return 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
static const char *getnextfilename (char **path, char *end) {
  char *sep;
  char *name = *path;
  if (name == end)
    return NULL;  /* no more names */
  else if (*name == '\0') {  /* from previous iteration? */
    *name = *LUNA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUNA_PATH_SEP);  /* find next separator */
  if (sep == NULL)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  return name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
static void pusherrornotfound (luna_State *L, const char *path) {
  lunaL_Buffer b;
  lunaL_buffinit(L, &b);
  lunaL_addstring(&b, "no file '");
  lunaL_addgsub(&b, path, LUNA_PATH_SEP, "'\n\tno file '");
  lunaL_addstring(&b, "'");
  lunaL_pushresult(&b);
}


static const char *searchpath (luna_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  lunaL_Buffer buff;
  char *pathname;  /* path with name inserted */
  char *endpathname;  /* its end */
  const char *filename;
  /* separator is non-empty and appears in 'name'? */
  if (*sep != '\0' && strchr(name, *sep) != NULL)
    name = lunaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  lunaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  lunaL_addgsub(&buff, path, LUNA_PATH_MARK, name);
  lunaL_addchar(&buff, '\0');
  pathname = lunaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + lunaL_bufflen(&buff) - 1;
  while ((filename = getnextfilename(&pathname, endpathname)) != NULL) {
    if (readable(filename))  /* does file exist and is readable? */
      return luna_pushstring(L, filename);  /* save and return name */
  }
  lunaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, luna_tostring(L, -1));  /* create error message */
  return NULL;  /* not found */
}


static int ll_searchpath (luna_State *L) {
  const char *f = searchpath(L, lunaL_checkstring(L, 1),
                                lunaL_checkstring(L, 2),
                                lunaL_optstring(L, 3, "."),
                                lunaL_optstring(L, 4, LUNA_DIRSEP));
  if (f != NULL) return 1;
  else {  /* error message is on top of the stack */
    lunaL_pushfail(L);
    luna_insert(L, -2);
    return 2;  /* return fail + error message */
  }
}


static const char *findfile (luna_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  luna_getfield(L, luna_upvalueindex(1), pname);
  path = luna_tostring(L, -1);
  if (l_unlikely(path == NULL))
    lunaL_error(L, "'package.%s' must be a string", pname);
  return searchpath(L, name, path, ".", dirsep);
}


static int checkload (luna_State *L, int stat, const char *filename) {
  if (l_likely(stat)) {  /* module loaded successfully? */
    luna_pushstring(L, filename);  /* will be 2nd argument to module */
    return 2;  /* return open function and file name */
  }
  else
    return lunaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          luna_tostring(L, 1), filename, luna_tostring(L, -1));
}


static int searcher_Lua (luna_State *L) {
  const char *filename;
  const char *name = lunaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUNA_LSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (lunaL_loadfile(L, filename) == LUNA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
static int loadfunc (luna_State *L, const char *filename, const char *modname) {
  const char *openfunc;
  const char *mark;
  modname = lunaL_gsub(L, modname, ".", LUNA_OFSEP);
  mark = strchr(modname, *LUNA_IGMARK);
  if (mark) {
    int stat;
    openfunc = luna_pushlstring(L, modname, mark - modname);
    openfunc = luna_pushfstring(L, LUNA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    if (stat != ERRFUNC) return stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = luna_pushfstring(L, LUNA_POF"%s", modname);
  return lookforfunc(L, filename, openfunc);
}


static int searcher_C (luna_State *L) {
  const char *name = lunaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "cpath", LUNA_CSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


static int searcher_Croot (luna_State *L) {
  const char *filename;
  const char *name = lunaL_checkstring(L, 1);
  const char *p = strchr(name, '.');
  int stat;
  if (p == NULL) return 0;  /* is root */
  luna_pushlstring(L, name, p - name);
  filename = findfile(L, luna_tostring(L, -1), "cpath", LUNA_CSUBSEP);
  if (filename == NULL) return 1;  /* root not found */
  if ((stat = loadfunc(L, filename, name)) != 0) {
    if (stat != ERRFUNC)
      return checkload(L, 0, filename);  /* real error */
    else {  /* open function not found */
      luna_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      return 1;
    }
  }
  luna_pushstring(L, filename);  /* will be 2nd argument to module */
  return 2;
}


static int searcher_preload (luna_State *L) {
  const char *name = lunaL_checkstring(L, 1);
  luna_getfield(L, LUNA_REGISTRYINDEX, LUNA_PRELOAD_TABLE);
  if (luna_getfield(L, -1, name) == LUNA_TNIL) {  /* not found? */
    luna_pushfstring(L, "no field package.preload['%s']", name);
    return 1;
  }
  else {
    luna_pushliteral(L, ":preload:");
    return 2;
  }
}


static void findloader (luna_State *L, const char *name) {
  int i;
  lunaL_Buffer msg;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  if (l_unlikely(luna_getfield(L, luna_upvalueindex(1), "searchers")
                 != LUNA_TTABLE))
    lunaL_error(L, "'package.searchers' must be a table");
  lunaL_buffinit(L, &msg);
  /*  iterate over available searchers to find a loader */
  for (i = 1; ; i++) {
    lunaL_addstring(&msg, "\n\t");  /* error-message prefix */
    if (l_unlikely(luna_rawgeti(L, 3, i) == LUNA_TNIL)) {  /* no more searchers? */
      luna_pop(L, 1);  /* remove nil */
      lunaL_buffsub(&msg, 2);  /* remove prefix */
      lunaL_pushresult(&msg);  /* create error message */
      lunaL_error(L, "module '%s' not found:%s", name, luna_tostring(L, -1));
    }
    luna_pushstring(L, name);
    luna_call(L, 1, 2);  /* call it */
    if (luna_isfunction(L, -2))  /* did it find a loader? */
      return;  /* module loader found */
    else if (luna_isstring(L, -2)) {  /* searcher returned error message? */
      luna_pop(L, 1);  /* remove extra return */
      lunaL_addvalue(&msg);  /* concatenate error message */
    }
    else {  /* no error message */
      luna_pop(L, 2);  /* remove both returns */
      lunaL_buffsub(&msg, 2);  /* remove prefix */
    }
  }
}


static int ll_require (luna_State *L) {
  const char *name = lunaL_checkstring(L, 1);
  luna_settop(L, 1);  /* LOADED table will be at index 2 */
  luna_getfield(L, LUNA_REGISTRYINDEX, LUNA_LOADED_TABLE);
  luna_getfield(L, 2, name);  /* LOADED[name] */
  if (luna_toboolean(L, -1))  /* is it there? */
    return 1;  /* package is already loaded */
  /* else must load package */
  luna_pop(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  luna_rotate(L, -2, 1);  /* function <-> loader data */
  luna_pushvalue(L, 1);  /* name is 1st argument to module loader */
  luna_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  luna_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  if (!luna_isnil(L, -1))  /* non-nil return? */
    luna_setfield(L, 2, name);  /* LOADED[name] = returned value */
  else
    luna_pop(L, 1);  /* pop nil */
  if (luna_getfield(L, 2, name) == LUNA_TNIL) {   /* module set no value? */
    luna_pushboolean(L, 1);  /* use true as result */
    luna_copy(L, -1, -2);  /* replace loader result */
    luna_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  luna_rotate(L, -2, 1);  /* loader data <-> module result  */
  return 2;  /* return module result and loader data */
}

/* }====================================================== */




static const lunaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", NULL},
  {"cpath", NULL},
  {"path", NULL},
  {"searchers", NULL},
  {"loaded", NULL},
  {NULL, NULL}
};


static const lunaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {NULL, NULL}
};


static void createsearcherstable (luna_State *L) {
  static const luna_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    NULL
  };
  int i;
  /* create 'searchers' table */
  luna_createtable(L, sizeof(searchers)/sizeof(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  for (i=0; searchers[i] != NULL; i++) {
    luna_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    luna_pushcclosure(L, searchers[i], 1);
    luna_rawseti(L, -2, i+1);
  }
  luna_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


/*
** create table CLIBS to keep track of loaded C libraries,
** setting a finalizer to close all libraries when closing state.
*/
static void createclibstable (luna_State *L) {
  lunaL_getsubtable(L, LUNA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  luna_createtable(L, 0, 1);  /* create metatable for CLIBS */
  luna_pushcfunction(L, gctm);
  luna_setfield(L, -2, "__gc");  /* set finalizer for CLIBS table */
  luna_setmetatable(L, -2);
}


LUAMOD_API int luaopen_package (luna_State *L) {
  createclibstable(L);
  lunaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUNA_PATH_VAR, LUNA_PATH_DEFAULT);
  setpath(L, "cpath", LUNA_CPATH_VAR, LUNA_CPATH_DEFAULT);
  /* store config information */
  luna_pushliteral(L, LUNA_DIRSEP "\n" LUNA_PATH_SEP "\n" LUNA_PATH_MARK "\n"
                     LUNA_EXEC_DIR "\n" LUNA_IGMARK "\n");
  luna_setfield(L, -2, "config");
  /* set field 'loaded' */
  lunaL_getsubtable(L, LUNA_REGISTRYINDEX, LUNA_LOADED_TABLE);
  luna_setfield(L, -2, "loaded");
  /* set field 'preload' */
  lunaL_getsubtable(L, LUNA_REGISTRYINDEX, LUNA_PRELOAD_TABLE);
  luna_setfield(L, -2, "preload");
  luna_pushglobaltable(L);
  luna_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  lunaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  luna_pop(L, 1);  /* pop global table */
  return 1;  /* return 'package' table */
}

