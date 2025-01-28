#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <core_cm4.h>
#include <stm32f411xe.h>
#include <m-profile/cmsis_gcc_m.h>

#include "rcc.h"

#define BOOTLOADER_ADDRESS (uint32_t *)0x1FFF76DE

void jumpToBootLoader() {
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;
	RCC->APB1ENR = 0;
	RCC->APB2ENR = 0;
	RCC->CR |= RCC_CR_HSION;
	while (!(RCC->CR & RCC_CR_HSIRDY_Msk));
	RCC->CFGR &= ~(RCC_CFGR_SW_Msk | RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk | RCC_CFGR_MCO1_Msk | RCC_CFGR_MCO2_Msk);
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
	RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_CSSON | RCC_CR_HSEON);
	RCC->CR &= ~(RCC_CR_HSEBYP);
	while ((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY);
    RCC->CFGR = 0;
	RCC->CIR |= (RCC_CIR_LSIRDYC | RCC_CIR_LSERDYC | RCC_CIR_HSIRDYC | RCC_CIR_HSERDYC | RCC_CIR_PLLRDYC | RCC_CIR_CSSC);
	RCC->CIR = 0;
	RCC->CSR = 0;
	RCC->APB1RSTR = 0xFFFFFFFF;
	RCC->APB1RSTR = 0;
	RCC->APB2RSTR = 0xFFFFFFFF;
	RCC->APB2RSTR = 0;
	RCC->AHB1RSTR = 0xFFFFFFFF;
	RCC->AHB1RSTR = 0x0;
	RCC->AHB2RSTR = 0xFFFFFFFF;
	RCC->AHB2RSTR = 0x0;
	RCC->AHB3RSTR = 0xFFFFFFFF;
	RCC->AHB3RSTR = 0x0;
    
	for (int i = 0; i < 8; i++) {
		NVIC->ICER[i] = 0x0;
		NVIC->ICPR[i] = 0x0;
	}

    __set_MSP(*BOOTLOADER_ADDRESS);
    ((void (*)(void))(*(BOOTLOADER_ADDRESS + 1)))();
}

#endif // BOOTLOADER_H