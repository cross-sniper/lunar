/*
** $Id: luaconf.h $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#ifndef luaconf_h
#define luaconf_h

#include <limits.h>
#include <stddef.h>


/*
** ===================================================================
** General Configuration File for Lua
**
** Some definitions here can be changed externally, through the compiler
** (e.g., with '-D' options): They are commented out or protected
** by '#if !defined' guards. However, several other definitions
** should be changed directly here, either because they affect the
** Lua ABI (by making the changes here, you ensure that all software
** connected to Lua, such as C libraries, will be compiled with the same
** configuration); or because they are seldom changed.
**
** Search for "@@" to find all configurable definitions.
** ===================================================================
*/


/*
** {====================================================================
** System Configuration: macros to adapt (if needed) Lua to some
** particular platform, for instance restricting it to C89.
** =====================================================================
*/

/*
@@ LUNA_USE_C89 controls the use of non-ISO-C89 features.
** Define it if you want Lua to avoid the use of a few C99 features
** or Windows-specific features on Windows.
*/
/* #define LUNA_USE_C89 */


/*
** By default, Lua on Windows use (some) specific Windows features
*/
#if !defined(LUNA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#define LUNA_USE_WINDOWS  /* enable goodies for regular Windows */
#endif


#if defined(LUNA_USE_WINDOWS)
#define LUNA_DL_DLL	/* enable support for DLL */
#define LUNA_USE_C89	/* broadly, Windows is C89 */
#endif


#if defined(LUNA_USE_LINUX)
#define LUNA_USE_POSIX
#define LUNA_USE_DLOPEN		/* needs an extra library: -ldl */
#endif


#if defined(LUNA_USE_MACOSX)
#define LUNA_USE_POSIX
#define LUNA_USE_DLOPEN		/* MacOS does not need -ldl */
#endif


#if defined(LUNA_USE_IOS)
#define LUNA_USE_POSIX
#define LUNA_USE_DLOPEN
#endif


/*
@@ LUAI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#define LUAI_IS32INT	((UINT_MAX >> 30) >= 3)

/* }================================================================== */



/*
** {==================================================================
** Configuration for Number types. These options should not be
** set externally, because any other code connected to Lua must
** use the same configuration.
** ===================================================================
*/

/*
@@ LUNA_INT_TYPE defines the type for Lua integers.
@@ LUNA_FLOAT_TYPE defines the type for Lua floats.
** Lua should work fine with any mix of these options supported
** by your C compiler. The usual configurations are 64-bit integers
** and 'double' (the default), 32-bit integers and 'float' (for
** restricted platforms), and 'long'/'double' (for C compilers not
** compliant with C99, which may not have support for 'long long').
*/

/* predefined options for LUNA_INT_TYPE */
#define LUNA_INT_INT		1
#define LUNA_INT_LONG		2
#define LUNA_INT_LONGLONG	3

/* predefined options for LUNA_FLOAT_TYPE */
#define LUNA_FLOAT_FLOAT		1
#define LUNA_FLOAT_DOUBLE	2
#define LUNA_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit Lua) */
#define LUNA_INT_DEFAULT		LUNA_INT_LONGLONG
#define LUNA_FLOAT_DEFAULT	LUNA_FLOAT_DOUBLE


/*
@@ LUNA_32BITS enables Lua with 32-bit integers and 32-bit floats.
*/
#define LUNA_32BITS	0


/*
@@ LUNA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#if defined(LUNA_USE_C89) && !defined(LUNA_USE_WINDOWS)
#define LUNA_C89_NUMBERS		1
#else
#define LUNA_C89_NUMBERS		0
#endif


#if LUNA_32BITS		/* { */
/*
** 32-bit integers and 'float'
*/
#if LUAI_IS32INT  /* use 'int' if big enough */
#define LUNA_INT_TYPE	LUNA_INT_INT
#else  /* otherwise use 'long' */
#define LUNA_INT_TYPE	LUNA_INT_LONG
#endif
#define LUNA_FLOAT_TYPE	LUNA_FLOAT_FLOAT

#elif LUNA_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#define LUNA_INT_TYPE	LUNA_INT_LONG
#define LUNA_FLOAT_TYPE	LUNA_FLOAT_DOUBLE

#else		/* }{ */
/* use defaults */

#define LUNA_INT_TYPE	LUNA_INT_DEFAULT
#define LUNA_FLOAT_TYPE	LUNA_FLOAT_DEFAULT

#endif				/* } */


/* }================================================================== */



/*
** {==================================================================
** Configuration for Paths.
** ===================================================================
*/

/*
** LUNA_PATH_SEP is the character that separates templates in a path.
** LUNA_PATH_MARK is the string that marks the substitution points in a
** template.
** LUNA_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
*/
#define LUNA_PATH_SEP            ";"
#define LUNA_PATH_MARK           "?"
#define LUNA_EXEC_DIR            "!"


/*
@@ LUNA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUNA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#define LUNA_VDIR	LUNA_VERSION_MAJOR "." LUNA_VERSION_MINOR
#if defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LUNA_LDIR	"!\\lua\\"
#define LUNA_CDIR	"!\\"
#define LUNA_SHRDIR	"!\\..\\share\\lua\\" LUNA_VDIR "\\"

#if !defined(LUNA_PATH_DEFAULT)
#define LUNA_PATH_DEFAULT  \
		LUNA_LDIR"?.lua;"  LUNA_LDIR"?\\init.lua;" \
		LUNA_CDIR"?.lua;"  LUNA_CDIR"?\\init.lua;" \
		LUNA_SHRDIR"?.lua;" LUNA_SHRDIR"?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#endif

#if !defined(LUNA_CPATH_DEFAULT)
#define LUNA_CPATH_DEFAULT \
		LUNA_CDIR"?.dll;" \
		LUNA_CDIR"..\\lib\\lua\\" LUNA_VDIR "\\?.dll;" \
		LUNA_CDIR"loadall.dll;" ".\\?.dll"
#endif

#else			/* }{ */

#define LUNA_ROOT	"/usr/local/"
#define LUNA_LDIR	LUNA_ROOT "share/lua/" LUNA_VDIR "/"
#define LUNA_CDIR	LUNA_ROOT "lib/lua/" LUNA_VDIR "/"

#if !defined(LUNA_PATH_DEFAULT)
#define LUNA_PATH_DEFAULT  \
		LUNA_LDIR"?.lua;"  LUNA_LDIR"?/init.lua;" \
		LUNA_CDIR"?.lua;"  LUNA_CDIR"?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#endif

#if !defined(LUNA_CPATH_DEFAULT)
#define LUNA_CPATH_DEFAULT \
		LUNA_CDIR"?.so;" LUNA_CDIR"loadall.so;" "./?.so"
#endif

#endif			/* } */


/*
@@ LUNA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#if !defined(LUNA_DIRSEP)

#if defined(_WIN32)
#define LUNA_DIRSEP	"\\"
#else
#define LUNA_DIRSEP	"/"
#endif

#endif


/*
** LUNA_IGMARK is a mark to ignore all after it when building the
** module name (e.g., used to build the luaopen_ function name).
** Typically, the sufix after the mark is the module version,
** as in "mod-v1.2.so".
*/
#define LUNA_IGMARK		"-"

/* }================================================================== */


/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
*/

/*
@@ LUNA_API is a mark for all core API functions.
@@ LUALIB_API is a mark for all auxiliary library functions.
@@ LUAMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** LUNA_BUILD_AS_DLL to get it).
*/
#if defined(LUNA_BUILD_AS_DLL)	/* { */

#if defined(LUNA_CORE) || defined(LUNA_LIB)	/* { */
#define LUNA_API __declspec(dllexport)
#else						/* }{ */
#define LUNA_API __declspec(dllimport)
#endif						/* } */

#else				/* }{ */

#define LUNA_API		extern

#endif				/* } */


/*
** More often than not the libs go together with the core.
*/
#define LUALIB_API	LUNA_API
#define LUAMOD_API	LUNA_API


/*
@@ LUAI_FUNC is a mark for all extern functions that are not to be
** exported to outside modules.
@@ LUAI_DDEF and LUAI_DDEC are marks for all extern (const) variables,
** none of which to be exported to outside modules (LUAI_DDEF for
** definitions and LUAI_DDEC for declarations).
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when Lua is compiled as a shared library. Not all elf targets support
** this attribute. Unfortunately, gcc does not offer a way to check
** whether the target offers that support, and those without support
** give a warning about it. To avoid these warnings, change to the
** default definition.
*/
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    defined(__ELF__)		/* { */
#define LUAI_FUNC	__attribute__((visibility("internal"))) extern
#else				/* }{ */
#define LUAI_FUNC	extern
#endif				/* } */

#define LUAI_DDEC(dec)	LUAI_FUNC dec
#define LUAI_DDEF	/* empty */

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUNA_COMPAT_5_3 controls other macros for compatibility with Lua 5.3.
** You can define it to get all options, or change specific options
** to fit your specific needs.
*/
#if defined(LUNA_COMPAT_5_3)	/* { */

/*
@@ LUNA_COMPAT_MATHLIB controls the presence of several deprecated
** functions in the mathematical library.
** (These functions were already officially removed in 5.3;
** nevertheless they are still available here.)
*/
#define LUNA_COMPAT_MATHLIB

/*
@@ LUNA_COMPAT_APIINTCASTS controls the presence of macros for
** manipulating other integer types (luna_pushunsigned, luna_tounsigned,
** lunaL_checkint, lunaL_checklong, etc.)
** (These macros were also officially removed in 5.3, but they are still
** available here.)
*/
#define LUNA_COMPAT_APIINTCASTS


/*
@@ LUNA_COMPAT_LT_LE controls the emulation of the '__le' metamethod
** using '__lt'.
*/
#define LUNA_COMPAT_LT_LE


/*
@@ The following macros supply trivial compatibility for some
** changes in the API. The macros themselves document how to
** change your code to avoid using them.
** (Once more, these macros were officially removed in 5.3, but they are
** still available here.)
*/
#define luna_strlen(L,i)		luna_rawlen(L, (i))

#define luna_objlen(L,i)		luna_rawlen(L, (i))

#define luna_equal(L,idx1,idx2)		luna_compare(L,(idx1),(idx2),LUNA_OPEQ)
#define luna_lessthan(L,idx1,idx2)	luna_compare(L,(idx1),(idx2),LUNA_OPLT)

#endif				/* } */

/* }================================================================== */



/*
** {==================================================================
** Configuration for Numbers (low-level part).
** Change these definitions if no predefined LUNA_FLOAT_* / LUNA_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
@@ LUAI_UACNUMBER is the result of a 'default argument promotion'
@@ over a floating number.
@@ l_floatatt(x) corrects float attribute 'x' to the proper float type
** by prefixing it with one of FLT/DBL/LDBL.
@@ LUNA_NUMBER_FRMLEN is the length modifier for writing floats.
@@ LUNA_NUMBER_FMT is the format for writing floats.
@@ luna_number2str converts a float to a string.
@@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ l_floor takes the floor of a float.
@@ luna_str2number converts a decimal numeral to a number.
*/


/* The following definitions are good for most cases here */

#define l_floor(x)		(l_mathop(floor)(x))

#define luna_number2str(s,sz,n)  \
	l_sprintf((s), sz, LUNA_NUMBER_FMT, (LUAI_UACNUMBER)(n))

/*
@@ luna_numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if float is not within the range of
** a luna_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#define luna_numbertointeger(n,p) \
  ((n) >= (LUNA_NUMBER)(LUNA_MININTEGER) && \
   (n) < -(LUNA_NUMBER)(LUNA_MININTEGER) && \
      (*(p) = (LUNA_INTEGER)(n), 1))


/* now the variable definitions */

#if LUNA_FLOAT_TYPE == LUNA_FLOAT_FLOAT		/* { single float */

#define LUNA_NUMBER	float

#define l_floatatt(n)		(FLT_##n)

#define LUAI_UACNUMBER	double

#define LUNA_NUMBER_FRMLEN	""
#define LUNA_NUMBER_FMT		"%.7g"

#define l_mathop(op)		op##f

#define luna_str2number(s,p)	strtof((s), (p))


#elif LUNA_FLOAT_TYPE == LUNA_FLOAT_LONGDOUBLE	/* }{ long double */

#define LUNA_NUMBER	long double

#define l_floatatt(n)		(LDBL_##n)

#define LUAI_UACNUMBER	long double

#define LUNA_NUMBER_FRMLEN	"L"
#define LUNA_NUMBER_FMT		"%.19Lg"

#define l_mathop(op)		op##l

#define luna_str2number(s,p)	strtold((s), (p))

#elif LUNA_FLOAT_TYPE == LUNA_FLOAT_DOUBLE	/* }{ double */

#define LUNA_NUMBER	double

#define l_floatatt(n)		(DBL_##n)

#define LUAI_UACNUMBER	double

#define LUNA_NUMBER_FRMLEN	""
#define LUNA_NUMBER_FMT		"%.14g"

#define l_mathop(op)		op

#define luna_str2number(s,p)	strtod((s), (p))

#else						/* }{ */

#error "numeric float type not defined"

#endif					/* } */



/*
@@ LUNA_UNSIGNED is the unsigned version of LUNA_INTEGER.
@@ LUAI_UACINT is the result of a 'default argument promotion'
@@ over a LUNA_INTEGER.
@@ LUNA_INTEGER_FRMLEN is the length modifier for reading/writing integers.
@@ LUNA_INTEGER_FMT is the format for writing integers.
@@ LUNA_MAXINTEGER is the maximum value for a LUNA_INTEGER.
@@ LUNA_MININTEGER is the minimum value for a LUNA_INTEGER.
@@ LUNA_MAXUNSIGNED is the maximum value for a LUNA_UNSIGNED.
@@ luna_integer2str converts an integer to a string.
*/


/* The following definitions are good for most cases here */

#define LUNA_INTEGER_FMT		"%" LUNA_INTEGER_FRMLEN "d"

#define LUAI_UACINT		LUNA_INTEGER

#define luna_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUNA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#define LUNA_UNSIGNED		unsigned LUAI_UACINT


/* now the variable definitions */

#if LUNA_INT_TYPE == LUNA_INT_INT		/* { int */

#define LUNA_INTEGER		int
#define LUNA_INTEGER_FRMLEN	""

#define LUNA_MAXINTEGER		INT_MAX
#define LUNA_MININTEGER		INT_MIN

#define LUNA_MAXUNSIGNED		UINT_MAX

#elif LUNA_INT_TYPE == LUNA_INT_LONG	/* }{ long */

#define LUNA_INTEGER		long
#define LUNA_INTEGER_FRMLEN	"l"

#define LUNA_MAXINTEGER		LONG_MAX
#define LUNA_MININTEGER		LONG_MIN

#define LUNA_MAXUNSIGNED		ULONG_MAX

#elif LUNA_INT_TYPE == LUNA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#if defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#define LUNA_INTEGER		long long
#define LUNA_INTEGER_FRMLEN	"ll"

#define LUNA_MAXINTEGER		LLONG_MAX
#define LUNA_MININTEGER		LLONG_MIN

#define LUNA_MAXUNSIGNED		ULLONG_MAX

#elif defined(LUNA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#define LUNA_INTEGER		__int64
#define LUNA_INTEGER_FRMLEN	"I64"

#define LUNA_MAXINTEGER		_I64_MAX
#define LUNA_MININTEGER		_I64_MIN

#define LUNA_MAXUNSIGNED		_UI64_MAX

#else				/* }{ */

#error "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' for details)"

#endif				/* } */

#else				/* }{ */

#error "numeric integer type not defined"

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
@@ l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
** (All uses in Lua have only one format item.)
*/
#if !defined(LUNA_USE_C89)
#define l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#else
#define l_sprintf(s,sz,f,i)	((void)(sz), sprintf(s,f,i))
#endif


/*
@@ luna_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'luna_strx2number' undefined and Lua will provide its own
** implementation.
*/
#if !defined(LUNA_USE_C89)
#define luna_strx2number(s,p)		luna_str2number(s,p)
#endif


/*
@@ luna_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#define luna_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ luna_number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'luna_number2strx' undefined and Lua will
** provide its own implementation.
*/
#if !defined(LUNA_USE_C89)
#define luna_number2strx(L,b,sz,f,n)  \
	((void)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#endif


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#if defined(LUNA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#undef l_mathop  /* variants not available */
#undef luna_str2number
#define l_mathop(op)		(luna_Number)op  /* no variant */
#define luna_str2number(s,p)	((luna_Number)strtod((s), (p)))
#endif


/*
@@ LUNA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#define LUNA_KCONTEXT	ptrdiff_t

#if !defined(LUNA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(INTPTR_MAX)  /* even in C99 this type is optional */
#undef LUNA_KCONTEXT
#define LUNA_KCONTEXT	intptr_t
#endif
#endif


/*
@@ luna_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#if !defined(luna_getlocaledecpoint)
#define luna_getlocaledecpoint()		(localeconv()->decimal_point[0])
#endif


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the Lua API use these macros.
** Define LUNA_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#if !defined(luai_likely)

#if defined(__GNUC__) && !defined(LUNA_NOBUILTIN)
#define luai_likely(x)		(__builtin_expect(((x) != 0), 1))
#define luai_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#else
#define luai_likely(x)		(x)
#define luai_unlikely(x)	(x)
#endif

#endif


#if defined(LUNA_CORE) || defined(LUNA_LIB)
/* shorter names for Lua's own use */
#define l_likely(x)	luai_likely(x)
#define l_unlikely(x)	luai_unlikely(x)
#endif



/* }================================================================== */


/*
** {==================================================================
** Language Variations
** =====================================================================
*/

/*
@@ LUNA_NOCVTN2S/LUNA_NOCVTS2N control how Lua performs some
** coercions. Define LUNA_NOCVTN2S to turn off automatic coercion from
** numbers to strings. Define LUNA_NOCVTS2N to turn off automatic
** coercion from strings to numbers.
*/
/* #define LUNA_NOCVTN2S */
/* #define LUNA_NOCVTS2N */


/*
@@ LUNA_USE_APICHECK turns on several consistency checks on the C API.
** Define it as a help when debugging C code.
*/
#if defined(LUNA_USE_APICHECK)
#include <assert.h>
#define luai_apicheck(l,e)	assert(e)
#endif

/* }================================================================== */


/*
** {==================================================================
** Macros that affect the API and must be stable (that is, must be the
** same when you compile Lua and when you compile code that links to
** Lua).
** =====================================================================
*/

/*
@@ LUAI_MAXSTACK limits the size of the Lua stack.
** CHANGE it if you need a different limit. This limit is arbitrary;
** its only purpose is to stop Lua from consuming unlimited stack
** space (and to reserve some numbers for pseudo-indices).
** (It must fit into max(size_t)/32 and max(int)/2.)
*/
#if LUAI_IS32INT
#define LUAI_MAXSTACK		1000000
#else
#define LUAI_MAXSTACK		15000
#endif


/*
@@ LUNA_EXTRASPACE defines the size of a raw memory area associated with
** a Lua state with very fast access.
** CHANGE it if you need a different size.
*/
#define LUNA_EXTRASPACE		(sizeof(void *))


/*
@@ LUNA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#define LUNA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#define LUAL_BUFFERSIZE   ((int)(16 * sizeof(void*) * sizeof(luna_Number)))


/*
@@ LUAI_MAXALIGN defines fields that, when used in a union, ensure
** maximum alignment for the other items in that union.
*/
#define LUAI_MAXALIGN  luna_Number n; double u; void *s; luna_Integer i; long l

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/





#endif

