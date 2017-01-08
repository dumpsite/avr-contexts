/*
 * main.c
 * avrcontext example 1
 * Simple Demo showing usage of extfunc 
 */

#include "extfunc.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* http://nongnu.org/avr-libc/user-manual/modules.html */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>


/* 
 * ( I )
 * 
 * defining a function type.
 * This is the equivalent replacement for 
 * "typedef uint16_t (*aFunctionType)(void* parameter1, uint8_t parameter2);"
 */ 
EXTFUNC_typedef(uint16_t, aFunctionType, void* parameter1, uint8_t parameter2);

/*
 * ( II )
 * 
 * declaring a vaiable (function pointer) of that type.
 * This is the equivalent replace for
 * "static aFunctionType aFunctionPointer = NULL;"
 */
static EXTFUNC_functype(aFunctionType) aFunctionPointer = EXTFUNC_NULL;

/*
 * ( III )
 * 
 * forward declaring a function matching the defined type
 * This is the equivalent replace for
 * "uint16_t aFunction(void* parameter1, uint8_t parameter2);"
 */
EXTFUNC_head(uint16_t, aFunction, void* parameter1, uint8_t parameter2);

/*
 * ( IV )
 * 
 * implementing a function matching the defined type
 * This is the equivalent replace for
 * "uint16_t aFunction(void* parameter1, uint8_t parameter2) { return 0; }"
 */
EXTFUNC(uint16_t, aFunction, void* parameter1, uint8_t parameter2) {
  // do sth here...
  
  // return a value...
  return 1;
}

int main(void) {
  uint16_t result = 0;

  /* initialize the lib before using it */
  extfunc_initialize();

  /*
   * ( V )
   * 
   * Do a simple call of the function, without using function pointers.
   * This is the equivalent replacement for
   * "result=aFunction(&result, 123);"
   */
  result=EXTFUNC_callByName(aFunction, &result, 123);

  /*
   * ( VI )
   * 
   * assign the function "aFunction" to the variable "aFuntionPointer"
   * This is the equivalent replacement for
   * "aFunctionPointer=&aFunction;"
   */
  aFunctionPointer=EXTFUNC_getPtr(aFunction, aFunctionType);

  // YOUR CODE HERE:
  do {
    /*
     * ( VII )
     * 
     * call the function pointer by first checking if it is assigned.
     * This is the equivalent replacement for
     * "if (!(aFunctionPointer==NULL)) result=aFunctionPointer(NULL, 210);"
     */
    if (!(EXTFUNC_isNULL(aFunctionPointer))) result=EXTFUNC_callptr(aFunctionPointer, aFunctionType, NULL, 210);
  } while (result);


  extfunc_finalize();
  return 0;
}
