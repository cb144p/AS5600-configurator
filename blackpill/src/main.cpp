#include <stdint.h>
#include <stm32f411xe.h>
#include <core_cm4.h>

#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "itimer.h"

void SystemInit() {
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); // enable fpu
    FLASH->ACR |= 3UL | FLASH_ACR_PRFTEN;
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY_Msk));
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE | PLL_P << RCC_PLLCFGR_PLLP_Pos | PLL_Q << RCC_PLLCFGR_PLLQ_Pos | PLL_N << RCC_PLLCFGR_PLLN_Pos | PLL_M << RCC_PLLCFGR_PLLM_Pos;
    RCC->CFGR |= HPRE << RCC_CFGR_HPRE_Pos | PPRE1 << RCC_CFGR_PPRE1_Pos | PPRE2 << RCC_CFGR_PPRE2_Pos | RTCPRE << RCC_CFGR_RTCPRE_Pos;
    RCC->CR |= RCC_CR_PLLON_Msk;
    while (!(RCC->CR & RCC_CR_PLLRDY_Msk));
    RCC->CFGR |= SW;
    systick_init(1000);
}


int main() {
    SystemInit();

    ITimer stim = ITimer::periodicInit(TIM3, 1000);
    stim.enableInterrupt();



    uint16_t ledB = PIN('C', 13);
    gpio_set_mode(ledB, GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL, GPIO_OSPEED_LOW_SPEED, GPIO_PUPD_NONE);
    bool ledState = true;
    gpio_write(ledB, !ledState);
    ledState = !ledState;
    

    uint32_t timer = 0, period = 2000;
    while (true) {
        if (timer_expired(&timer, period, s_ticks)) {
            gpio_write(ledB, !ledState);
            ledState = !ledState;
        }
    }
}