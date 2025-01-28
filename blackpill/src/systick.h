#ifndef SYSTICK_H
#define SYSTICK_H

#include <core_cm4.h>

#include "rcc.h"

// time for one tick in microseconds
static void systick_init(uint32_t us) {
    uint32_t ticks = SYSCLK_FREQUENCY / 1000000 * us;
    if (ticks - 1 > 0xffffff) return;
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0b111;
    RCC->APB2ENR |= 1 << 14;
}
static volatile uint32_t s_ticks;
void SysTick_Handler(void) {
	s_ticks+=1;
}

void delay(uint32_t ms) {
	uint32_t until = s_ticks + ms;
	while (s_ticks < until) (void) 0;
}

bool timer_expired(uint32_t* t, uint32_t prd, uint32_t now) {
	if (now + prd < *t) *t = 0;
	if (*t == 0) *t = now + prd;
	if (*t > now) return false;
	*t = (now - *t) > prd ? now + prd : *t + prd;
	return true;
}

#endif // SYSTICK_H