/*
 * main.c
 * avrcontext example 1
 * Simple Demo showing usage of cpucontext 
 */

#include "extfunc.h"
#include "cpucontext.h"

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



static 	uint8_t		second_stack[256];
static	cpucontext_t	second_context;

EXTFUNC(int8_t, second_main, void* parameters)  {
  /* do some stuff in the second context */
  while (true) {
    /* do some more repeating stuff - switch back to primary context regularly */
    EXTFUNC_callByName(cpucontext_switch, cpucontext_main_context);
  }
  return 0; /* in case this function will exit, its return value is processed as exitcode */
}




int main(void) {

  /* cpucontext is based on extfunc - so init it first */
  extfunc_initialize();

  /* 
   * initialize the lib before using it - do it as EXTFUNC recommends it
   *
   * This will create the first context "cpucontext_main_context", consisting
   * of the original execution path here in "main()".
   */
  EXTFUNC_callByName(cpucontext_initialize);

  /* create a second context starting in function "second_main" */
  EXTFUNC_callByName(cpucontext_create, &second_context, second_stack, sizeof(second_stack), EXTFUNC_getPtr(second_main, CPUCONTEXT_entry_t), NULL);


  // do stuff in main context...
  do {
    // do some regular stuff here and switch frequently to second context to get stuff done there, too
    EXTFUNC_callByName(cpucontext_switch, &second_context);
  } while (true);

  EXTFUNC_callByName(cpucontext_finalize);
  extfunc_finalize();
  return 0;
}