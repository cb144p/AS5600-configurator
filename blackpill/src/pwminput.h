#ifndef PWMINPUT_H
#define PWMINPUT_H

#include <stdint.h>
#include <stm32f411xe.h>
#include <cmath>

#include "rcc.h"
#include "gpio.h"

// No protection. Separated from the main class because the timer is affected by external factors.
class PWMInput {
public:
    TIM_TypeDef* tim = nullptr;
    uint16_t psc = 0; // Real prescaler value, not just the one in the register.
    uint8_t status = 0;

    PWMInput() {};
    ~PWMInput() {};

    PWMInput(TIM_TypeDef* _tim, uint16_t _psc) {
        tim = _tim;
        psc = _psc;

        tim->PSC = psc - 1;
    }

    // TODO: add the ability to change channel
    PWMInput(TIM_TypeDef* _tim, float frequency, uint8_t channel = 0) {
        tim = _tim;
        
        if ((uint32_t)tim == TIM1_BASE) {
            RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
            RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST_Msk;
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
        } else if ((uint32_t)tim < TIM1_BASE) {
            if ((uint32_t)tim == TIM2_BASE) {
                RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
                RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST_Msk;
                RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
            }
            else {
                uint8_t bs = (uint32_t)(tim - TIM2_BASE)/0x400;
                RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST << bs;
                RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST_Msk << bs;
                RCC->APB1ENR |= RCC_APB1ENR_TIM2EN << bs;
            }
        }
        else {
            uint8_t bs = ((uint32_t)(tim - TIM9_BASE)/0x400 + RCC_APB2ENR_TIM9EN_Pos);
            RCC->APB2RSTR |= RCC_APB2RSTR_TIM9RST << bs;
            RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM9RST_Msk << bs;
            RCC->APB2ENR |= RCC_APB2ENR_TIM9EN << bs;
        }
        
        tim->SMCR &= ~TIM_SMCR_SMS_Msk;
        tim->SMCR |= TIM_SMCR_SMS_2 | 0b101 << TIM_SMCR_TS_Pos;
        tim->PSC = (uint16_t)ceilf((float)AHB_FREQUENCY / 65536.0f / frequency) - 1; // See RM0383 page 94. When running at full speed, the timer will always have the HCLK frequency.
        tim->ARR = 0xFFFF; // Upcounting so it starts immediately. Doesn't really matter though because it gets reset on first edge.
        tim->CCMR1 |= 0b01 << TIM_CCMR1_CC1S_Pos | 0b10 << TIM_CCMR1_CC2S_Pos; // Map CC1 and CC2 to T1
        tim->CCMR1 &= ~(TIM_CCMR1_IC1PSC | TIM_CCMR1_IC2PSC);
        tim->CCER &= ~TIM_CCER_CC1P;
        tim->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E | TIM_CCER_CC2E;
        tim->CR1 |= TIM_CR1_CEN;
        psc = tim->PSC + 1;
    }
    
    // TODO: implement channel selection
    uint16_t readPWMCount(uint8_t channel = 0) {
        return tim->CCR2;
    }

    float readPWMFrequency(uint8_t channel = 0) {
        return (float)AHB_FREQUENCY / psc / tim->CCR1;
    }

    float readDutyCycle(uint8_t channel = 0) {
        return ((float)tim->CCR2) / tim->CCR1;
    }
    
    float readOnTime(uint8_t channel = 0) {
        return 1.0f * psc / AHB_FREQUENCY * tim->CCR2;
    }

};

#endif // PWMINPUT_H