/*
 * EXTFUNC.H
 * This file contains interfaces and macros to
 * entend functionspointer accross the 128k limit,
 * while generalizing function-calls via macros.
 * 
 * By default (EXTFUNC_NOEXT) the overhead-
 * extra code switches off (if not needed on the 
 * current CPU) and implements usual 
 * function pointers.
 * So it is always safe to use this way.
 * 
 * While implementing this, the EIND register
 * isn't used and therefore doesn't change.
 * However it is backed up anyway... 
 * 
 * EXTFUNC should be better linked as "NEAR" code
 * by defining "EXTFUNCNEAR" as a section in the
 * first 128KiB of MCU-Flash.
 * The code-space EXTFUNC occupies is minimal.
 * All EXTFUNC functions created by the macros are
 * automatically "EXTFUNCFAR" and can be placed aboved
 * the 128KiB limit when defining the section. 
 * 
 * 
 * This is version 20170107T1200ZSB
 *
 * Stephan Baerwolf (matrixstorm@gmx.de), Schwansee 2017
 * (please contact me at least before commercial use)
 * 
 * 
 * 
 * The usual proceeding is as follows:
 * 
 * Please initialize the library once via "extfunc_initialize();".
 * Only an initialized library (for example in the beginning of "main(...)",
 * can be assumed to work properly.
 * After beeing used, it also can be deinitialized/finalized with
 * "extfunc_finalize();"
 * 
 * 
 * A function type no longer is defined by an immediate typedef.
 * Instead a macro therefore is used with similar syntax:
 * "EXTFUNC_typedef(returntype, typename, ...)"
 * So "typedef int (*mycallback)(char, void*)" becomes
 * "EXTFUNC_typedef(int, mycallback, char, void*)".
 * 
 * The declared type will be named/called "EXTFUNC_functype(typename)"
 * So "mycallback" will become "EXTFUNC_functype(mycallback)"
 * 
 * When defining a function, the macro "EXTFUNC(returntype, funcname, ...)"
 * should be used. In order to implement a function for "mycallback",
 * "int aPossibleCallback(char rx, void* argv) {...}" implements like this:
 * "EXTFUNC(int, aPossibleCallback, char rx, void* argv) {....}"
 * 
 * A variable (of type "EXTFUNC_functype(...)") is set to an pointer
 * of a function by invoking "define EXTFUNC_getPtr(functionname, typename)"
 * So "fktptrVar = &aPossibleCallback;" becomes
 * "fktptrVar = EXTFUNC_getPtr(aPossibleCallback, mycallback);"
 * Important to note is the changed pointerisation without the "&aPoss,,,"
 * 
 * Finally to call such a funktion pointer variable, use
 *                               "EXTFUNC_callptr(funcptr, typename, ...)"
 * So a common call "result=fktptrVar(aCharVar, NULL);" is done as
 * "result=EXTFUNC_callptr(fktptrVar, mycallback, aCharVar, NULL);"
 * 
 * It is also possible to call the declared functions directly, without
 * assigning them to a pointer variable: "define EXTFUNC_callByName(funcname, ...)"
 * Analog "result=aPossibleCallback(aCharVar, NULL);" becomes
 * "result=EXTFUNC_callByName(aPossibleCallback, aCharVar, NULL);"
 */

#ifndef EXTFUNC_H_5dd52e4c69e34e8bb1670a1691f01e78
#define EXTFUNC_H_5dd52e4c69e34e8bb1670a1691f01e78 	1

#ifdef EXTFUNCINCLUDEDEFINES
#	include "defines.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#ifdef EXTFUNC_C_5dd52e4c69e34e8bb1670a1691f01e78
#	define EXTFUNCPUBLIC
#else
#	define EXTFUNCPUBLIC	extern
#endif

/* for bootloaders using his */
#ifdef APIPAGEADDR
#	include "Config/auxsection.h"
#	define EXTFUNCAUXSECTION	AUXSECTION3
#endif

#ifndef EXTFUNCNEAR
#	define EXTFUNCNEAR
#endif

#ifndef EXTFUNCAUXSECTION
#	define EXTFUNCAUXSECTION
#endif

#ifndef EXTFUNCFAR
#	define EXTFUNCFAR EXTFUNCAUXSECTION
#endif

#ifndef __EXTFUNC_EIND
#	ifdef EIND
#		define __EXTFUNC_EIND	EIND
#	endif
#endif

#ifndef EXTFUNC_NOEXT
#	ifdef __EXTFUNC_EIND
#		define EXTFUNC_NOEXT 0
#	else
#		define EXTFUNC_NOEXT 1
#	endif
#endif


#ifndef __EXTFUNC_SpecialTrampolineCode
/* Of course since "__extfunc_trampolineCallTarget" is a weak symbol, it can be overridden afterall */
#	define __EXTFUNC_SpecialTrampolineCode	1
#endif


#if (EXTFUNC_NOEXT)
typedef void* extfuncptr_t;
#else
typedef union __extfuncptr_t {
  struct {
    uint16_t lower128k;
    uint8_t  upperextension;
  };
  struct {
    uint8_t ptr_lsb;
    uint8_t ptr_med;
    uint8_t ptr_msb;
  };
} __attribute__ ((packed)) extfuncptr_t;
#endif

#ifdef __EXTFUNC_EIND
EXTFUNCPUBLIC uint8_t __extfunc_EIND_default;
#endif




/*
 * "extfunc_initialize" initializes the library by executing some preparation
 * code, before all other stuff can be used.
 * "__extfunc_initialize" is for backward compatibility.
 */
#define __extfunc_initialize		extfunc_initialize
EXTFUNCPUBLIC int8_t extfunc_initialize(void);


/*
 * "extfunc_finalize" deinitializes the library.
 * "__extfunc_finalize" is for backward compatibility.
 */
#define __extfunc_finalize		extfunc_finalize
EXTFUNCPUBLIC int8_t extfunc_finalize(void);


/* 
 * "extfunc_PTRisNULL" takes an extfunc functionpointer of universal type ("extfuncptr_t")
 * and checks if it points to NULL . Since extfunc pointers can be 24 bit in length, this
 * generalizes the default gcc "NULL".
 * 
 * For backward compatibility the macro "__extfuncptr__builtin_isNULL" does the same.
 * For specific types of EXTFUNC function pointers, use "EXTFUNC_isNULL(ptr)" instead.
 * 
 * Both return "true" in case "ptr" is the eqivalent of NULL in its value.
 * False is returned otherwise.
 */
#define __extfuncptr__builtin_isNULL	extfunc_PTRisNULL
EXTFUNCPUBLIC bool extfunc_PTRisNULL(extfuncptr_t ptr) EXTFUNCAUXSECTION;


/*
 * "extfunc_setNULL" sets an extfunc functionpointer of universal type ("extfuncptr_t")
 * to the equivalent of NULL.
 * Each individual extfunc-function type implements an universal type representation.
 */
EXTFUNCPUBLIC void extfunc_setNULL(extfuncptr_t *ptr) EXTFUNCAUXSECTION;



#if 0 /* do not activate this - only for ducumenting the macros */

/*
 * "EXTFUNC_typedef" replaces the usual "typedef" for defining a function
 * data type.
 * In case the corresponding function's aren't using any parameter arguments,
 * "EXTFUNC_voidtypedef" should be used.
 * 
 * The defined type will accessable as "EXTFUNC_functype(typename)"
 */
EXTFUNC_typedef(returntype, typename, ...);
EXTFUNC_voidtypedef(returntype, typename);


/*
 * "EXTFUNC_functype" makes an "EXTFUNC_typedef" function-type accessable
 * for declaring variables, or expressing a variable's type.
 */
EXTFUNC_functype(typename)


/*
 * "EXTFUNC_head" declares the head of a function "funcname".
 * For example useable in forward declarations.
 * 
 * In case the function shall not use any argument parameters
 * "EXTFUNC_voidhead" should be used.
 */
EXTFUNC_head(returntype, funcname, ...);
EXTFUNC_voidhead(returntype, funcname);


/*
 * "EXTFUNC_head" is for implementing functioncode like "returntype funcname(...) { ;; }".
 * Of course the body "{ ;; }" of the function is appended as ususal C code.
 * 
 * In case the function shall not use any argument parameters
 * "EXTFUNC_void" should be used.
 */
EXTFUNC(returntype, funcname, ...) { ;; }
EXTFUNC_void(returntype, funcname) { ;; }


/*
 * "EXTFUNC_getPtr" returns the extended function pointer of an implemented
 * function called "functionname". The function should be coded using the
 * "EXTFUNC" (and "EXTFUNC_head") statements and be of the same structure as
 * its corresponding function type "typename", previously defined by using
 * "EXTFUNC_typedef".
 * 
 * "EXTFUNC_getPtr" can be used to set pointer-variables to their value.
 */
EXTFUNC_getPtr(functionname, typename);


/*
 * "EXTFUNC_callptr" should be used to call a function pointer "funcptr" of
 * type "typename" (previously defined by using "EXTFUNC_typedef") with all its
 * arguments.
 * 
 * It returns the function's result of a type, previously defined in the "typename".
 */
EXTFUNC_callptr(funcptr, typename, ...);


/*
 * "EXTFUNC_callByName" calls a function (previously defined using "EXTFUNC") directly,
 * and without using a function pointer from a variable (like "EXTFUNC_callptr" would do).
 * 
 * It returns the function's result and type.
 */
EXTFUNC_callByName(funcname, ...);




#endif


#define __EXTFUNC__builtin_avr_flash_segment(var)   \
({ uint8_t tmp;                                     \
   asm volatile (                                   \
       "ldi    %[output],   hh8(%[ptrof])"  "\n\t"  \
       : [output]   "=d" (tmp)                      \
       : [ptrof]    "i"  (&(var))                   \
    );                                              \
   tmp;                                             \
})


#define __EXTFUNC_ptrNULL						{.lower128k=0, .upperextension=0}

#if (EXTFUNC_NOEXT)
#	define __EXTFUNC_head(returntype, funcname, args...)		returntype funcname(args)

#	define	__EXTFUNC_functype(returntype, typename, args...)	__EXTFUNC_head(returntype, (*typename), args)
#	define __EXTFUNC_funcHELPERtypename(typename)			typename
#	define __EXTFUNC_functypename(typename)				typename

#	define EXTFUNC_typedef(returntype, typename, args...)		typedef __EXTFUNC_functype(returntype, __EXTFUNC_funcHELPERtypename(typename), args)

#	define EXTFUNC_callByName(funcname, ...)			funcname(__VA_ARGS__)
#	define EXTFUNC_callptr(funcptr, typename, ...)			funcptr(__VA_ARGS__)


#	define EXTFUNC_NULL						NULL
#	define EXTFUNC_getPtr(functionname, typename)			&functionname

#	define EXTFUNC_isNULL(funcptr)					((funcptr)==NULL)

#else /* *************************************************************************************************************************************************** */
#	define __EXTFUNC_head(returntype, funcname, ...)		returntype funcname(extfuncptr_t *__extfunc_self, ##__VA_ARGS__)

#	define	__EXTFUNC_functype(returntype, typename, ...)		__EXTFUNC_head(returntype, (*typename), ##__VA_ARGS__)
#	define __EXTFUNC_funcHELPERtypename(typename)			__extfunchelperptr_ ## typename
#	define __EXTFUNC_functypename(typename)				extfuncptr_ ## typename

#	define EXTFUNC_callByName(funcname, ...)			funcname(NULL, ##__VA_ARGS__)
EXTFUNCPUBLIC void __extfunc_trampolineCallTarget(extfuncptr_t *__extfunc_self) EXTFUNCNEAR;
#	define EXTFUNC_callptr(funcptr, typename, ...)	({							\
	  __EXTFUNC_funcHELPERtypename(typename) hlpfunc = (void*)&__extfunc_trampolineCallTarget;		\
	  hlpfunc((&(funcptr.xptr)), __VA_ARGS__);								\
})

#	define EXTFUNC_typedef(returntype, typename, ...)							 \
typedef __EXTFUNC_functype(returntype, __EXTFUNC_funcHELPERtypename(typename), ##__VA_ARGS__);			 \
typedef union {struct { __EXTFUNC_funcHELPERtypename(typename) fptr; uint8_t eind; } __attribute__ ((packed)); extfuncptr_t xptr; } __EXTFUNC_functypename(typename)


#	define EXTFUNC_NULL						{.xptr = __EXTFUNC_ptrNULL}
#	define EXTFUNC_getPtr(functionname, typename) ({							\
  __EXTFUNC_functypename(typename) result;									\
  result.fptr = &functionname;											\
  result.eind =	__EXTFUNC__builtin_avr_flash_segment(functionname);						\
  result;													\
})

#	define EXTFUNC_isNULL(funcptr)					extfunc_PTRisNULL((funcptr).xptr)

#endif


/* in order to implement functions, define the function header */
#define EXTFUNC(returntype, funcname, ...)			EXTFUNCFAR __EXTFUNC_head(returntype, funcname, __VA_ARGS__)
#define EXTFUNC_head(returntype, funcname, ...)			EXTFUNCFAR __EXTFUNC_head(returntype, funcname, __VA_ARGS__)

#if (EXTFUNC_NOEXT)
#	define EXTFUNC_voidtypedef(returntype, typename)	EXTFUNC_typedef(returntype, typename, void)
#	define EXTFUNC_void(returntype, funcname)		EXTFUNCFAR __EXTFUNC_head(returntype, funcname, void)
#	define EXTFUNC_voidhead(returntype, funcname)		EXTFUNCFAR __EXTFUNC_head(returntype, funcname, void)
#else
#	define EXTFUNC_voidtypedef(returntype, typename)	EXTFUNC_typedef(returntype, typename)
#	define EXTFUNC_void(returntype, funcname)		EXTFUNCFAR __EXTFUNC_head(returntype, funcname)
#	define EXTFUNC_voidhead(returntype, funcname)		EXTFUNCFAR __EXTFUNC_head(returntype, funcname)
#endif


/* in order to define variables, define the name of the individual functiontype */
#define EXTFUNC_functype(typename)				__EXTFUNC_functypename(typename)

#endif
