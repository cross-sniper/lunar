/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "lua.h"


/* version suffix for environment variable names */
#define LUNA_VERSUFFIX          "_" LUNA_VERSION_MAJOR "_" LUNA_VERSION_MINOR


LUAMOD_API int (luaopen_base) (luna_State *L);

#define LUNA_COLIBNAME	"coroutine"
LUAMOD_API int (luaopen_coroutine) (luna_State *L);

#define LUNA_TABLIBNAME	"table"
LUAMOD_API int (luaopen_table) (luna_State *L);

#define LUNA_IOLIBNAME	"io"
LUAMOD_API int (luaopen_io) (luna_State *L);

#define LUNA_OSLIBNAME	"os"
LUAMOD_API int (luaopen_os) (luna_State *L);

#define LUNA_STRLIBNAME	"string"
LUAMOD_API int (luaopen_string) (luna_State *L);

#define LUNA_UTF8LIBNAME	"utf8"
LUAMOD_API int (luaopen_utf8) (luna_State *L);

#define LUNA_MATHLIBNAME	"math"
LUAMOD_API int (luaopen_math) (luna_State *L);

#define LUNA_DBLIBNAME	"debug"
LUAMOD_API int (luaopen_debug) (luna_State *L);

#define LUNA_LOADLIBNAME	"package"
LUAMOD_API int (luaopen_package) (luna_State *L);


/* open all previous libraries */
LUALIB_API void (lunaL_openlibs) (luna_State *L);


#endif
