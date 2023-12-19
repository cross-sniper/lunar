/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ifndef ltests_h
#define ltests_h


#include <stdio.h>
#include <stdlib.h>

/* test Lua with compatibility code */
#define LUNA_COMPAT_MATHLIB
#define LUNA_COMPAT_LT_LE


#define LUNA_DEBUG


/* turn on assertions */
#define LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#define UNUSED(x)       (x=0, (void)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#undef l_sprintf
#if !defined(LUNA_USE_C89)
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#else
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#endif


/* get a chance to test code without jump tables */
#define LUNA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#define LUNA_RAND32


/* memory-allocator control variables */
typedef struct Memcontrol {
  int failnext;
  unsigned long numblocks;
  unsigned long total;
  unsigned long maxmem;
  unsigned long memlimit;
  unsigned long countlimit;
  unsigned long objcount[LUNA_NUMTYPES];
} Memcontrol;

LUNA_API Memcontrol l_memcontrol;


/*
** generic variable for debug tricks
*/
extern void *l_Trick;



/*
** Function to traverse and check all memory used by Lua
*/
LUAI_FUNC int luna_checkmemory (luna_State *L);

/*
** Function to print an object GC-friendly
*/
struct GCObject;
LUAI_FUNC void luna_printobj (luna_State *L, struct GCObject *o);


/* test for lock/unlock */

struct L_EXTRA { int lock; int *plock; };
#undef LUNA_EXTRASPACE
#define LUNA_EXTRASPACE	sizeof(struct L_EXTRA)
#define getlock(l)	cast(struct L_EXTRA*, luna_getextraspace(l))
#define luai_userstateopen(l)  \
	(getlock(l)->lock = 0, getlock(l)->plock = &(getlock(l)->lock))
#define luai_userstateclose(l)  \
  luna_assert(getlock(l)->lock == 1 && getlock(l)->plock == &(getlock(l)->lock))
#define luai_userstatethread(l,l1) \
  luna_assert(getlock(l1)->plock == getlock(l)->plock)
#define luai_userstatefree(l,l1) \
  luna_assert(getlock(l)->plock == getlock(l1)->plock)
#define luna_lock(l)     luna_assert((*getlock(l)->plock)++ == 0)
#define luna_unlock(l)   luna_assert(--(*getlock(l)->plock) == 0)



LUNA_API int lunaB_opentests (luna_State *L);

LUNA_API void *debug_realloc (void *ud, void *block,
                             size_t osize, size_t nsize);

#if defined(luna_c)
#define lunaL_newstate()		luna_newstate(debug_realloc, &l_memcontrol)
#define lunaL_openlibs(L)  \
  { (lunaL_openlibs)(L); \
     lunaL_requiref(L, "T", lunaB_opentests, 1); \
     luna_pop(L, 1); }
#endif



/* change some sizes to give some bugs a chance */

#undef LUAL_BUFFERSIZE
#define LUAL_BUFFERSIZE		23
#define MINSTRTABSIZE		2
#define MAXIWTHABS		3

#define STRCACHE_N	23
#define STRCACHE_M	5

#undef LUAI_USER_ALIGNMENT_T
#define LUAI_USER_ALIGNMENT_T   union { char b[sizeof(void*) * 8]; }


/*
** This one is not compatible with tests for opcode optimizations,
** as it blocks some optimizations
#define MAXINDEXRK	0
*/


/* make stack-overflow tests run faster */
#undef LUAI_MAXSTACK
#define LUAI_MAXSTACK   50000


/* test mode uses more stack space */
#undef LUAI_MAXCCALLS
#define LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#undef luna_strx2number
#undef luna_number2strx


#endif

