/*
** $Id: lmathlib.c $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUNA_LIB

#include "lprefix.h"


#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


static int math_abs (luna_State *L) {
  if (luna_isinteger(L, 1)) {
    luna_Integer n = luna_tointeger(L, 1);
    if (n < 0) n = (luna_Integer)(0u - (luna_Unsigned)n);
    luna_pushinteger(L, n);
  }
  else
    luna_pushnumber(L, l_mathop(fabs)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_sin (luna_State *L) {
  luna_pushnumber(L, l_mathop(sin)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_cos (luna_State *L) {
  luna_pushnumber(L, l_mathop(cos)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_tan (luna_State *L) {
  luna_pushnumber(L, l_mathop(tan)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_asin (luna_State *L) {
  luna_pushnumber(L, l_mathop(asin)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_acos (luna_State *L) {
  luna_pushnumber(L, l_mathop(acos)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_atan (luna_State *L) {
  luna_Number y = lunaL_checknumber(L, 1);
  luna_Number x = lunaL_optnumber(L, 2, 1);
  luna_pushnumber(L, l_mathop(atan2)(y, x));
  return 1;
}


static int math_toint (luna_State *L) {
  int valid;
  luna_Integer n = luna_tointegerx(L, 1, &valid);
  if (l_likely(valid))
    luna_pushinteger(L, n);
  else {
    lunaL_checkany(L, 1);
    lunaL_pushfail(L);  /* value is not convertible to integer */
  }
  return 1;
}


static void pushnumint (luna_State *L, luna_Number d) {
  luna_Integer n;
  if (luna_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    luna_pushinteger(L, n);  /* result is integer */
  else
    luna_pushnumber(L, d);  /* result is float */
}


static int math_floor (luna_State *L) {
  if (luna_isinteger(L, 1))
    luna_settop(L, 1);  /* integer is its own floor */
  else {
    luna_Number d = l_mathop(floor)(lunaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_ceil (luna_State *L) {
  if (luna_isinteger(L, 1))
    luna_settop(L, 1);  /* integer is its own ceil */
  else {
    luna_Number d = l_mathop(ceil)(lunaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_fmod (luna_State *L) {
  if (luna_isinteger(L, 1) && luna_isinteger(L, 2)) {
    luna_Integer d = luna_tointeger(L, 2);
    if ((luna_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      lunaL_argcheck(L, d != 0, 2, "zero");
      luna_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      luna_pushinteger(L, luna_tointeger(L, 1) % d);
  }
  else
    luna_pushnumber(L, l_mathop(fmod)(lunaL_checknumber(L, 1),
                                     lunaL_checknumber(L, 2)));
  return 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when luna_Number is not
** 'double'.
*/
static int math_modf (luna_State *L) {
  if (luna_isinteger(L ,1)) {
    luna_settop(L, 1);  /* number is its own integer part */
    luna_pushnumber(L, 0);  /* no fractional part */
  }
  else {
    luna_Number n = lunaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    luna_Number ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    luna_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  return 2;
}


static int math_sqrt (luna_State *L) {
  luna_pushnumber(L, l_mathop(sqrt)(lunaL_checknumber(L, 1)));
  return 1;
}


static int math_ult (luna_State *L) {
  luna_Integer a = lunaL_checkinteger(L, 1);
  luna_Integer b = lunaL_checkinteger(L, 2);
  luna_pushboolean(L, (luna_Unsigned)a < (luna_Unsigned)b);
  return 1;
}

static int math_log (luna_State *L) {
  luna_Number x = lunaL_checknumber(L, 1);
  luna_Number res;
  if (luna_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  else {
    luna_Number base = lunaL_checknumber(L, 2);
#if !defined(LUNA_USE_C89)
    if (base == l_mathop(2.0))
      res = l_mathop(log2)(x);
    else
#endif
    if (base == l_mathop(10.0))
      res = l_mathop(log10)(x);
    else
      res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  luna_pushnumber(L, res);
  return 1;
}

static int math_exp (luna_State *L) {
  luna_pushnumber(L, l_mathop(exp)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_deg (luna_State *L) {
  luna_pushnumber(L, lunaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  return 1;
}

static int math_rad (luna_State *L) {
  luna_pushnumber(L, lunaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  return 1;
}


static int math_min (luna_State *L) {
  int n = luna_gettop(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  lunaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (luna_compare(L, i, imin, LUNA_OPLT))
      imin = i;
  }
  luna_pushvalue(L, imin);
  return 1;
}


static int math_max (luna_State *L) {
  int n = luna_gettop(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  lunaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (luna_compare(L, imax, i, LUNA_OPLT))
      imax = i;
  }
  luna_pushvalue(L, imax);
  return 1;
}


static int math_type (luna_State *L) {
  if (luna_type(L, 1) == LUNA_TNUMBER)
    luna_pushstring(L, (luna_isinteger(L, 1)) ? "integer" : "float");
  else {
    lunaL_checkany(L, 1);
    lunaL_pushfail(L);
  }
  return 1;
}



/*
** {==================================================================
** Pseudo-Random Number Generator based on 'xoshiro256**'.
** ===================================================================
*/

/*
** This code uses lots of shifts. ANSI C does not allow shifts greater
** than or equal to the width of the type being shifted, so some shifts
** are written in convoluted ways to match that restriction. For
** preprocessor tests, it assumes a width of 32 bits, so the maximum
** shift there is 31 bits.
*/


/* number of binary digits in the mantissa of a float */
#define FIGS	l_floatatt(MANT_DIG)

#if FIGS > 64
/* there are only 64 random bits; use them all */
#undef FIGS
#define FIGS	64
#endif


/*
** LUNA_RAND32 forces the use of 32-bit integers in the implementation
** of the PRN generator (mainly for testing).
*/
#if !defined(LUNA_RAND32) && !defined(Rand64)

/* try to find an integer type with at least 64 bits */

#if ((ULONG_MAX >> 31) >> 31) >= 3

/* 'long' has at least 64 bits */
#define Rand64		unsigned long
#define SRand64		long

#elif !defined(LUNA_USE_C89) && defined(LLONG_MAX)

/* there is a 'long long' type (which must have at least 64 bits) */
#define Rand64		unsigned long long
#define SRand64		long long

#elif ((LUNA_MAXUNSIGNED >> 31) >> 31) >= 3

/* 'luna_Unsigned' has at least 64 bits */
#define Rand64		luna_Unsigned
#define SRand64		luna_Integer

#endif

#endif


#if defined(Rand64)  /* { */

/*
** Standard implementation, using 64-bit integers.
** If 'Rand64' has more than 64 bits, the extra bits do not interfere
** with the 64 initial bits, except in a right shift. Moreover, the
** final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim64(x)	((x) & 0xffffffffffffffffu)


/* rotate left 'x' by 'n' bits */
static Rand64 rotl (Rand64 x, int n) {
  return (x << n) | (trim64(x) >> (64 - n));
}

static Rand64 nextrand (Rand64 *state) {
  Rand64 state0 = state[0];
  Rand64 state1 = state[1];
  Rand64 state2 = state[2] ^ state0;
  Rand64 state3 = state[3] ^ state1;
  Rand64 res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  return res;
}


/*
** Convert bits from a random integer into a float in the
** interval [0,1), getting the higher FIG bits from the
** random unsigned integer and converting that to a float.
** Some old Microsoft compilers cannot cast an unsigned long
** to a floating-point number, so we use a signed long as an
** intermediary. When luna_Number is float or double, the shift ensures
** that 'sx' is non negative; in that case, a good compiler will remove
** the correction.
*/

/* must throw out the extra (64 - FIGS) bits */
#define shift64_FIG	(64 - FIGS)

/* 2^(-FIGS) == 2^-1 / 2^(FIGS-1) */
#define scaleFIG	(l_mathop(0.5) / ((Rand64)1 << (FIGS - 1)))

static luna_Number I2d (Rand64 x) {
  SRand64 sx = (SRand64)(trim64(x) >> shift64_FIG);
  luna_Number res = (luna_Number)(sx) * scaleFIG;
  if (sx < 0)
    res += 1.0;  /* correct the two's complement if negative */
  luna_assert(0 <= res && res < 1);
  return res;
}

/* convert a 'Rand64' to a 'luna_Unsigned' */
#define I2UInt(x)	((luna_Unsigned)trim64(x))

/* convert a 'luna_Unsigned' to a 'Rand64' */
#define Int2I(x)	((Rand64)(x))


#else	/* no 'Rand64'   }{ */

/* get an integer with at least 32 bits */
#if LUAI_IS32INT
typedef unsigned int lu_int32;
#else
typedef unsigned long lu_int32;
#endif


/*
** Use two 32-bit integers to represent a 64-bit quantity.
*/
typedef struct Rand64 {
  lu_int32 h;  /* higher half */
  lu_int32 l;  /* lower half */
} Rand64;


/*
** If 'lu_int32' has more than 32 bits, the extra bits do not interfere
** with the 32 initial bits, except in a right shift and comparisons.
** Moreover, the final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim32(x)	((x) & 0xffffffffu)


/*
** basic operations on 'Rand64' values
*/

/* build a new Rand64 value */
static Rand64 packI (lu_int32 h, lu_int32 l) {
  Rand64 result;
  result.h = h;
  result.l = l;
  return result;
}

/* return i << n */
static Rand64 Ishl (Rand64 i, int n) {
  luna_assert(n > 0 && n < 32);
  return packI((i.h << n) | (trim32(i.l) >> (32 - n)), i.l << n);
}

/* i1 ^= i2 */
static void Ixor (Rand64 *i1, Rand64 i2) {
  i1->h ^= i2.h;
  i1->l ^= i2.l;
}

/* return i1 + i2 */
static Rand64 Iadd (Rand64 i1, Rand64 i2) {
  Rand64 result = packI(i1.h + i2.h, i1.l + i2.l);
  if (trim32(result.l) < trim32(i1.l))  /* carry? */
    result.h++;
  return result;
}

/* return i * 5 */
static Rand64 times5 (Rand64 i) {
  return Iadd(Ishl(i, 2), i);  /* i * 5 == (i << 2) + i */
}

/* return i * 9 */
static Rand64 times9 (Rand64 i) {
  return Iadd(Ishl(i, 3), i);  /* i * 9 == (i << 3) + i */
}

/* return 'i' rotated left 'n' bits */
static Rand64 rotl (Rand64 i, int n) {
  luna_assert(n > 0 && n < 32);
  return packI((i.h << n) | (trim32(i.l) >> (32 - n)),
               (trim32(i.h) >> (32 - n)) | (i.l << n));
}

/* for offsets larger than 32, rotate right by 64 - offset */
static Rand64 rotl1 (Rand64 i, int n) {
  luna_assert(n > 32 && n < 64);
  n = 64 - n;
  return packI((trim32(i.h) >> n) | (i.l << (32 - n)),
               (i.h << (32 - n)) | (trim32(i.l) >> n));
}

/*
** implementation of 'xoshiro256**' algorithm on 'Rand64' values
*/
static Rand64 nextrand (Rand64 *state) {
  Rand64 res = times9(rotl(times5(state[1]), 7));
  Rand64 t = Ishl(state[1], 17);
  Ixor(&state[2], state[0]);
  Ixor(&state[3], state[1]);
  Ixor(&state[1], state[2]);
  Ixor(&state[0], state[3]);
  Ixor(&state[2], t);
  state[3] = rotl1(state[3], 45);
  return res;
}


/*
** Converts a 'Rand64' into a float.
*/

/* an unsigned 1 with proper type */
#define UONE		((lu_int32)1)


#if FIGS <= 32

/* 2^(-FIGS) */
#define scaleFIG       (l_mathop(0.5) / (UONE << (FIGS - 1)))

/*
** get up to 32 bits from higher half, shifting right to
** throw out the extra bits.
*/
static luna_Number I2d (Rand64 x) {
  luna_Number h = (luna_Number)(trim32(x.h) >> (32 - FIGS));
  return h * scaleFIG;
}

#else	/* 32 < FIGS <= 64 */

/* 2^(-FIGS) = 1.0 / 2^30 / 2^3 / 2^(FIGS-33) */
#define scaleFIG  \
    (l_mathop(1.0) / (UONE << 30) / l_mathop(8.0) / (UONE << (FIGS - 33)))

/*
** use FIGS - 32 bits from lower half, throwing out the other
** (32 - (FIGS - 32)) = (64 - FIGS) bits
*/
#define shiftLOW	(64 - FIGS)

/*
** higher 32 bits go after those (FIGS - 32) bits: shiftHI = 2^(FIGS - 32)
*/
#define shiftHI		((luna_Number)(UONE << (FIGS - 33)) * l_mathop(2.0))


static luna_Number I2d (Rand64 x) {
  luna_Number h = (luna_Number)trim32(x.h) * shiftHI;
  luna_Number l = (luna_Number)(trim32(x.l) >> shiftLOW);
  return (h + l) * scaleFIG;
}

#endif


/* convert a 'Rand64' to a 'luna_Unsigned' */
static luna_Unsigned I2UInt (Rand64 x) {
  return (((luna_Unsigned)trim32(x.h) << 31) << 1) | (luna_Unsigned)trim32(x.l);
}

/* convert a 'luna_Unsigned' to a 'Rand64' */
static Rand64 Int2I (luna_Unsigned n) {
  return packI((lu_int32)((n >> 31) >> 1), (lu_int32)n);
}

#endif  /* } */


/*
** A state uses four 'Rand64' values.
*/
typedef struct {
  Rand64 s[4];
} RanState;


/*
** Project the random integer 'ran' into the interval [0, n].
** Because 'ran' has 2^B possible values, the projection can only be
** uniform when the size of the interval is a power of 2 (exact
** division). Otherwise, to get a uniform projection into [0, n], we
** first compute 'lim', the smallest Mersenne number not smaller than
** 'n'. We then project 'ran' into the interval [0, lim].  If the result
** is inside [0, n], we are done. Otherwise, we try with another 'ran',
** until we have a result inside the interval.
*/
static luna_Unsigned project (luna_Unsigned ran, luna_Unsigned n,
                             RanState *state) {
  if ((n & (n + 1)) == 0)  /* is 'n + 1' a power of 2? */
    return ran & n;  /* no bias */
  else {
    luna_Unsigned lim = n;
    /* compute the smallest (2^b - 1) not smaller than 'n' */
    lim |= (lim >> 1);
    lim |= (lim >> 2);
    lim |= (lim >> 4);
    lim |= (lim >> 8);
    lim |= (lim >> 16);
#if (LUNA_MAXUNSIGNED >> 31) >= 3
    lim |= (lim >> 32);  /* integer type has more than 32 bits */
#endif
    luna_assert((lim & (lim + 1)) == 0  /* 'lim + 1' is a power of 2, */
      && lim >= n  /* not smaller than 'n', */
      && (lim >> 1) < n);  /* and it is the smallest one */
    while ((ran &= lim) > n)  /* project 'ran' into [0..lim] */
      ran = I2UInt(nextrand(state->s));  /* not inside [0..n]? try again */
    return ran;
  }
}


static int math_random (luna_State *L) {
  luna_Integer low, up;
  luna_Unsigned p;
  RanState *state = (RanState *)luna_touserdata(L, luna_upvalueindex(1));
  Rand64 rv = nextrand(state->s);  /* next pseudo-random value */
  switch (luna_gettop(L)) {  /* check number of arguments */
    case 0: {  /* no arguments */
      luna_pushnumber(L, I2d(rv));  /* float between 0 and 1 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = lunaL_checkinteger(L, 1);
      if (up == 0) {  /* single 0 as argument? */
        luna_pushinteger(L, I2UInt(rv));  /* full random integer */
        return 1;
      }
      break;
    }
    case 2: {  /* lower and upper limits */
      low = lunaL_checkinteger(L, 1);
      up = lunaL_checkinteger(L, 2);
      break;
    }
    default: return lunaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  lunaL_argcheck(L, low <= up, 1, "interval is empty");
  /* project random integer into the interval [0, up - low] */
  p = project(I2UInt(rv), (luna_Unsigned)up - (luna_Unsigned)low, state);
  luna_pushinteger(L, p + (luna_Unsigned)low);
  return 1;
}


static void setseed (luna_State *L, Rand64 *state,
                     luna_Unsigned n1, luna_Unsigned n2) {
  int i;
  state[0] = Int2I(n1);
  state[1] = Int2I(0xff);  /* avoid a zero state */
  state[2] = Int2I(n2);
  state[3] = Int2I(0);
  for (i = 0; i < 16; i++)
    nextrand(state);  /* discard initial values to "spread" seed */
  luna_pushinteger(L, n1);
  luna_pushinteger(L, n2);
}


/*
** Set a "random" seed. To get some randomness, use the current time
** and the address of 'L' (in case the machine does address space layout
** randomization).
*/
static void randseed (luna_State *L, RanState *state) {
  luna_Unsigned seed1 = (luna_Unsigned)time(NULL);
  luna_Unsigned seed2 = (luna_Unsigned)(size_t)L;
  setseed(L, state->s, seed1, seed2);
}


static int math_randomseed (luna_State *L) {
  RanState *state = (RanState *)luna_touserdata(L, luna_upvalueindex(1));
  if (luna_isnone(L, 1)) {
    randseed(L, state);
  }
  else {
    luna_Integer n1 = lunaL_checkinteger(L, 1);
    luna_Integer n2 = lunaL_optinteger(L, 2, 0);
    setseed(L, state->s, n1, n2);
  }
  return 2;  /* return seeds */
}


static const lunaL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {NULL, NULL}
};


/*
** Register the random functions and initialize their state.
*/
static void setrandfunc (luna_State *L) {
  RanState *state = (RanState *)luna_newuserdatauv(L, sizeof(RanState), 0);
  randseed(L, state);  /* initialize with a "random" seed */
  luna_pop(L, 2);  /* remove pushed seeds */
  lunaL_setfuncs(L, randfuncs, 1);
}

/* }================================================================== */


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#if defined(LUNA_COMPAT_MATHLIB)

static int math_cosh (luna_State *L) {
  luna_pushnumber(L, l_mathop(cosh)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_sinh (luna_State *L) {
  luna_pushnumber(L, l_mathop(sinh)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_tanh (luna_State *L) {
  luna_pushnumber(L, l_mathop(tanh)(lunaL_checknumber(L, 1)));
  return 1;
}

static int math_pow (luna_State *L) {
  luna_Number x = lunaL_checknumber(L, 1);
  luna_Number y = lunaL_checknumber(L, 2);
  luna_pushnumber(L, l_mathop(pow)(x, y));
  return 1;
}

static int math_frexp (luna_State *L) {
  int e;
  luna_pushnumber(L, l_mathop(frexp)(lunaL_checknumber(L, 1), &e));
  luna_pushinteger(L, e);
  return 2;
}

static int math_ldexp (luna_State *L) {
  luna_Number x = lunaL_checknumber(L, 1);
  int ep = (int)lunaL_checkinteger(L, 2);
  luna_pushnumber(L, l_mathop(ldexp)(x, ep));
  return 1;
}

static int math_log10 (luna_State *L) {
  luna_pushnumber(L, l_mathop(log10)(lunaL_checknumber(L, 1)));
  return 1;
}

#endif
/* }================================================================== */



static const lunaL_Reg mathlib[] = {
  {"abs",   math_abs},
  {"acos",  math_acos},
  {"asin",  math_asin},
  {"atan",  math_atan},
  {"ceil",  math_ceil},
  {"cos",   math_cos},
  {"deg",   math_deg},
  {"exp",   math_exp},
  {"tointeger", math_toint},
  {"floor", math_floor},
  {"fmod",   math_fmod},
  {"ult",   math_ult},
  {"log",   math_log},
  {"max",   math_max},
  {"min",   math_min},
  {"modf",   math_modf},
  {"rad",   math_rad},
  {"sin",   math_sin},
  {"sqrt",  math_sqrt},
  {"tan",   math_tan},
  {"type", math_type},
#if defined(LUNA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh",   math_cosh},
  {"sinh",   math_sinh},
  {"tanh",   math_tanh},
  {"pow",   math_pow},
  {"frexp", math_frexp},
  {"ldexp", math_ldexp},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"random", NULL},
  {"randomseed", NULL},
  {"pi", NULL},
  {"huge", NULL},
  {"maxinteger", NULL},
  {"mininteger", NULL},
  {NULL, NULL}
};


/*
** Open math library
*/
LUAMOD_API int luaopen_math (luna_State *L) {
  lunaL_newlib(L, mathlib);
  luna_pushnumber(L, PI);
  luna_setfield(L, -2, "pi");
  luna_pushnumber(L, (luna_Number)HUGE_VAL);
  luna_setfield(L, -2, "huge");
  luna_pushinteger(L, LUNA_MAXINTEGER);
  luna_setfield(L, -2, "maxinteger");
  luna_pushinteger(L, LUNA_MININTEGER);
  luna_setfield(L, -2, "mininteger");
  setrandfunc(L);
  return 1;
}

