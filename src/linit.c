/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUNA_LIB

/*
** If you embed Lua in your program and need to open the standard
** libraries, call lunaL_openlibs in your program. If you need a
** different set of libraries, copy this file to your project and edit
** it to suit your needs.
**
** You can also *preload* libraries, so that a later 'require' can
** open the library, which is already linked to the application.
** For that, do the following code:
**
**  lunaL_getsubtable(L, LUNA_REGISTRYINDEX, LUNA_PRELOAD_TABLE);
**  luna_pushcfunction(L, luaopen_modname);
**  luna_setfield(L, -2, modname);
**  luna_pop(L, 1);  // remove PRELOAD table
*/

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"


/*
** these libs are loaded by lua.c and are readily available to any Lua
** program
*/
static const lunaL_Reg loadedlibs[] = {
  {LUNA_GNAME, luaopen_base},
  {LUNA_LOADLIBNAME, luaopen_package},
  {LUNA_COLIBNAME, luaopen_coroutine},
  {LUNA_TABLIBNAME, luaopen_table},
  {LUNA_IOLIBNAME, luaopen_io},
  {LUNA_OSLIBNAME, luaopen_os},
  {LUNA_STRLIBNAME, luaopen_string},
  {LUNA_MATHLIBNAME, luaopen_math},
  {LUNA_UTF8LIBNAME, luaopen_utf8},
  {LUNA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};


LUALIB_API void lunaL_openlibs (luna_State *L) {
  const lunaL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    lunaL_requiref(L, lib->name, lib->func, 1);
    luna_pop(L, 1);  /* remove lib */
  }
}

