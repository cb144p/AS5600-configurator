// Startup code

#include "handlers.h"

// Extern c is required because the c++ compiler changes the symbol name.
// This causes the linker script to be unable to find the symbol _reset, because it doesn't exist.
extern "C" __attribute__((naked, noreturn)) void _reset(void) {
  // Initialise memory
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *src = &_sbss; src < &_ebss; src++) *src = 0;
  for (long *src = &_sdata, *dst = &_sidata; src < &_edata;) *src++ = *dst++;
  // Check if I need to do something with the exidx or not.

  // Call main()
  extern int main(void);
  main();
  for (;;) (void) 0;  // Infinite loop
}

extern void SysTick_Handler(); // Defined in systick.h
extern "C" void _estack(void); // Defined in link.ld and as such needs c-style linking

// 16 standard and 91 STM32-specific handlers
// Also doesn't get recognized by the linker without the c-style linking. 
// Apparently the sections get mangled too.
extern "C" __attribute__((section(".vectors"))) void (*const tab[16 + nvicRegCount])(void) = {
    _estack, _reset, 0, Hard_Handler, Memory_Handler, Bus_Handler, Usage_Handler, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler}; // TODO: replace with CMSIS alternate?