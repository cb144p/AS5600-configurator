#ifndef RCC_H
#define RCC_H

#include <stdint.h>

#include <stm32f411xe.h>

#define PLL_N 96
#define PLL_M 25
#define PLL_P 0
#define PLL_Q 4
#define SW 0b10
#define HPRE 0
#define PPRE1 0b100
#define PPRE2 0
#define RTCPRE 25

// TODO: change this to constinit

#define LSE_FREQUENCY 32768UL
#define LSI_FREQUENCY 32768UL // Not to be trusted
#define HSI_FREQUENCY 16000000UL
#define HSE_FREQUENCY 25000000UL
#define PLL_FREQUENCY HSE_FREQUENCY * PLL_N / PLL_M / ((PLL_P + 1) * 2)
#define SYSCLK_FREQUENCY PLL_FREQUENCY
#define AHB_FREQUENCY PLL_FREQUENCY / (HPRE + 1)
#define APB1_FREQUENCY AHB_FREQUENCY / 2
#define APB2_FREQUENCY AHB_FREQUENCY


#endif // RCC_H